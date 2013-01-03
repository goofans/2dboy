
#include "MMMem.h"
#include "Environment.h"

#include <string.h>
#include <assert.h>

#include <revolution.h>

MMMem::MMMem()
{
}

MMMem::~MMMem()
{
}

void MMMem::Init( void *pPoolMemory, uint32 poolSizeBytes )
{
	memPoolSize = poolSizeBytes;
	pMemPool = (unsigned char *)pPoolMemory;
	assert( pMemPool );

	pSlowHead = pMemPool;
	pFastHead = pMemPool + memPoolSize;
	mpSlowFreeHead = NULL;
	mpFastFreeHead = NULL;
	mpLastSlowAlloc = NULL;
	mHighWatermarkBytes = 0;

#if defined(MEM_SAFTEY_CHECKS)
	allocHeaderSize = sizeof(allocHeader);
	allocFooterSize = sizeof(allocFooter);
#else
	allocHeaderSize = 0;
	allocFooterSize = 0;
#endif
}

void *MMMem::Alloc( uint32 allocSizeBytes, eMMMemType allocType, uint32 allocAlign )
{
	void *pResult = NULL;
	uint32 alignPad, requiredSizeofBlock;

	// align must be non zero and a power of 2 and 4 or greater
	assert( allocAlign >= 4 );
	assert( !(allocAlign & (allocAlign-1)) );
	//assert( allocSizeBytes < memPoolSize );

	if( allocSizeBytes < 4 )
		allocSizeBytes = 4;

	// round alloc size up to a multiple of 4
	allocSizeBytes += (4 - (allocSizeBytes & 3)) & 3;

	uint32 maxAddr = (allocType == eMemSlow) ? (uint32)(pSlowHead) : (uint32)(pMemPool) + memPoolSize;

	//blockHeader *pNewBlock = (allocType == eMemSlow) ? (blockHeader*)pMemPool : (blockHeader*)pFastHead;
	blockHeader *pNewBlock = (allocType == eMemSlow) ? mpSlowFreeHead : mpFastFreeHead;

	while( pNewBlock && ((uint32)(pNewBlock) < maxAddr) )
	{
		// see if we can fit this allocation into an unused block
		if( !pNewBlock->size )
		{
			// the last block can't be unused, if it is, the slow head pointer should have been moved in
			// for fast allocs, the fast head can't be unused

			assert( ((allocType == eMemSlow) && pNewBlock->pNextBlock) || ((allocType == eMemFast) && (pNewBlock != (blockHeader*)pFastHead)) );

			// figure out what would be needed to allocate here
			CalcAllocParams( pNewBlock, allocAlign, allocSizeBytes, &alignPad, &requiredSizeofBlock );

			// figure out how much space we actually have here
			uint32 actualSizeofBlock = 0;
			if( pNewBlock->pNextBlock )
			{
				actualSizeofBlock = (uint32)(pNewBlock->pNextBlock) - (uint32)(pNewBlock);
			}
			else
			{
				actualSizeofBlock = maxAddr - (uint32)(pNewBlock);
			}

			if( requiredSizeofBlock <= actualSizeofBlock )
			{
				blockHeader *pOldPrevFree = pNewBlock->pPrevFree;
				blockHeader *pOldNextFree = pNewBlock->pNextFree;
				blockHeader *pOldSlowFreeHead = mpSlowFreeHead;
				blockHeader *pOldFastFreeHead = mpFastFreeHead;

				// yes, this alloc can be made inside this block
				pResult = MakeAlloc( pNewBlock, pNewBlock->pPrevBlock, pNewBlock->pNextBlock, allocSizeBytes, alignPad );

				// decide if there's enough space left over to bother creating a new, unused block for it
				uint32 minUsableSize = sizeof(blockHeader) + sizeof(blockHeader*) + allocHeaderSize + allocFooterSize + 128;
				if( (actualSizeofBlock-requiredSizeofBlock) > minUsableSize )
				{
					blockHeader *pNewEmpty = (blockHeader *)(void*)((uint32)(pNewBlock) + requiredSizeofBlock);
					MakeAlloc( pNewEmpty, pNewBlock, pNewBlock->pNextBlock, 0, 0 );
					pNewBlock->pNextBlock = pNewEmpty;
					if( pNewEmpty->pNextBlock )
					{
						pNewEmpty->pNextBlock->pPrevBlock = pNewEmpty;
					}

					// since we just made a new free block, link it into the free list
					pNewEmpty->pPrevFree = pOldPrevFree;
					if( pNewEmpty->pPrevFree ) pNewEmpty->pPrevFree->pNextFree = pNewEmpty;
					pNewEmpty->pNextFree = pOldNextFree;
					if( pNewEmpty->pNextFree ) pNewEmpty->pNextFree->pPrevFree = pNewEmpty;
					if( pOldSlowFreeHead == pNewBlock ) mpSlowFreeHead = pNewEmpty;
					if( pOldFastFreeHead == pNewBlock ) mpFastFreeHead = pNewEmpty;
				}

				break;
			}
		}

		//pNewBlock = pNewBlock->pNextBlock;
		pNewBlock = pNewBlock->pNextFree;
	}

	// if you couldn't allocate using an existing free block, try on the end of the slow side
	if( !pResult )
	{
		if( allocType == eMemSlow )
		{
			CalcAllocParams( (blockHeader*)pSlowHead, allocAlign, allocSizeBytes, &alignPad, &requiredSizeofBlock );
			if( (uint32)(pSlowHead) + requiredSizeofBlock <= (uint32)(pFastHead) )
			{
				blockHeader *pEdgeBlock = (blockHeader*)pSlowHead;
				pEdgeBlock->pPrevFree = NULL;
				pEdgeBlock->pNextFree = NULL;
				pResult = MakeAlloc( pEdgeBlock, mpLastSlowAlloc, NULL, allocSizeBytes, alignPad );
				if( mpLastSlowAlloc )
				{
					mpLastSlowAlloc->pNextBlock = (blockHeader*)pSlowHead;
				}
				mpLastSlowAlloc = pEdgeBlock;
				pSlowHead += requiredSizeofBlock;
			}
		}
		else
		{
			CalcAllocParamsRev( (blockHeader*)pFastHead, allocAlign, allocSizeBytes, &alignPad, &requiredSizeofBlock );
			if( (uint32)(pFastHead) - requiredSizeofBlock >= (uint32)(pSlowHead) )
			{
				blockHeader *pNewFastHead = (blockHeader*)(void*)((uint32)(pFastHead) - requiredSizeofBlock);
				int firstFastAlloc = (pFastHead < (pMemPool + memPoolSize)) ? 0 : 1;
				pNewFastHead->pPrevFree = NULL;
				pNewFastHead->pNextFree = NULL;
				pResult = MakeAlloc( pNewFastHead, NULL, firstFastAlloc ? NULL : (blockHeader*)pFastHead, allocSizeBytes, alignPad );
				if( !firstFastAlloc )
				{
					((blockHeader*)pFastHead)->pPrevBlock = pNewFastHead;
				}
				pFastHead -= requiredSizeofBlock;
			}
		}
	}

	uint32 curUsedBytes = ((uint32)pSlowHead - (uint32)pMemPool) + (((uint32)pMemPool + memPoolSize) - (uint32)pFastHead);
	if( (curUsedBytes/(1024*1024)) > (mHighWatermarkBytes/(1024*1024)) )
	{
		envDebugLog( "HIGH MEM USAGE: %s%02d / %02d MB\n", (memPoolSize>(32*1024*1024))?"        ":"", (curUsedBytes/(1024*1024)), (memPoolSize/(1024*1024)) );
	}
	if( curUsedBytes > mHighWatermarkBytes ) mHighWatermarkBytes = curUsedBytes;

	return pResult;
}

void MMMem::CalcAllocParams( blockHeader *pTargetAddr, uint32 alignment, uint32 size, uint32 *alignPad, uint32 *requiredSizeofBlock )
{
	uint32 alignMask = alignment - 1;
	*alignPad = (alignment - (((uint32)(pTargetAddr) + sizeof(blockHeader) + sizeof(blockHeader*) + allocHeaderSize) & alignMask)) & alignMask;
	*requiredSizeofBlock = sizeof(blockHeader) + (*alignPad) + sizeof(blockHeader*) + allocHeaderSize + size + allocFooterSize;
}

void MMMem::CalcAllocParamsRev( blockHeader *pTargetAddr, uint32 alignment, uint32 size, uint32 *alignPad, uint32 *requiredSizeofBlock )
{
	// calculate for making an allocation BEFORE the target addr in memory
	uint32 alignMask = alignment - 1;
	*alignPad = ((uint32)(pTargetAddr) - allocFooterSize - size) & alignMask;
	*requiredSizeofBlock = sizeof(blockHeader) + sizeof(blockHeader*) + allocHeaderSize + size + allocFooterSize;
	*requiredSizeofBlock += *alignPad;
	*alignPad = 0;
}

void *MMMem::MakeAlloc( blockHeader *pTargetAddr, blockHeader *pPrevBlock, blockHeader *pNextBlock, uint32 size, uint32 alignPad )
{
	// set block header fields
	pTargetAddr->pPrevBlock = pPrevBlock;
	pTargetAddr->pNextBlock = pNextBlock;
	pTargetAddr->pAlloc = ((unsigned char *)pTargetAddr) + sizeof(blockHeader) + alignPad + sizeof(blockHeader*) + allocHeaderSize;
	pTargetAddr->size = size;

#if defined(MEM_SAFTEY_CHECKS)
	// set allocation prefix values
	allocHeader *pAllocHeader = (allocHeader*)(((unsigned char *)pTargetAddr) + sizeof(blockHeader) + alignPad + sizeof(blockHeader*));
	pAllocHeader->magic1 = kMagic1;
	pAllocHeader->magic2 = kMagic2;
	pAllocHeader->magic3 = kMagic3;
	pAllocHeader->magic4 = kMagic4;

	// set allocation suffix values
	allocFooter *pAllocFooter = (allocFooter*)(pTargetAddr->pAlloc + size);
	pAllocFooter->magic1 = kMagic1;
	pAllocFooter->magic2 = kMagic2;
	pAllocFooter->magic3 = kMagic3;
	pAllocFooter->magic4 = kMagic4;
#endif

	// stash a block pointer right before the actual alloc, this way we can just grab it on delete instead of having to search
	blockHeader **ppBlockHdr = (blockHeader**)(((uint32)pTargetAddr->pAlloc) - allocHeaderSize - sizeof(blockHeader*));
	*ppBlockHdr = pTargetAddr;

	// remove from free list if this is a new, used allocation
	if( size > 0 )
	{
		if( pTargetAddr == mpSlowFreeHead )
		{
			if( pTargetAddr->pNextFree ) pTargetAddr->pNextFree->pPrevFree = NULL;
			mpSlowFreeHead = pTargetAddr->pNextFree;
		}
		else if( pTargetAddr == mpFastFreeHead )
		{
			if( pTargetAddr->pNextFree ) pTargetAddr->pNextFree->pPrevFree = NULL;
			mpFastFreeHead = pTargetAddr->pNextFree;
		}
		else
		{
			if( pTargetAddr->pPrevFree ) pTargetAddr->pPrevFree->pNextFree = pTargetAddr->pNextFree;
			if( pTargetAddr->pNextFree ) pTargetAddr->pNextFree->pPrevFree = pTargetAddr->pPrevFree;
		}
		
		pTargetAddr->pPrevFree = NULL;
		pTargetAddr->pNextFree = NULL;
	}

	return pTargetAddr->pAlloc;
}

void MMMem::Free( const void *pAllocation )
{
	if( pAllocation )
	{
		blockHeader *pBlockHdr = *((blockHeader**)(((uint32)pAllocation) - allocHeaderSize - sizeof(blockHeader*)));
		assert( pBlockHdr->pAlloc == pAllocation );
		FreeBlock( pBlockHdr );
	}
}

void MMMem::FreeBlock( blockHeader *pBlock )
{
#if defined(MEM_SAFTEY_CHECKS)
	// check the magic padding to make sure it's not damaged
	allocHeader *pAllocHeader = (allocHeader*)(pBlock->pAlloc - allocHeaderSize);
	if( !((pAllocHeader->magic1 == kMagic1) && (pAllocHeader->magic2 == kMagic2) && (pAllocHeader->magic3 == kMagic3) && (pAllocHeader->magic4 == kMagic4)) )
	{
		// corruption!
		*((int*)0) = 0;
	}

	allocFooter *pAllocFooter = (allocFooter*)(pBlock->pAlloc + pBlock->size);
	if( !((pAllocFooter->magic1 == kMagic1) && (pAllocFooter->magic2 == kMagic2) && (pAllocFooter->magic3 == kMagic3) && (pAllocFooter->magic4 == kMagic4)) )
	{
		// corruption!
		*((int*)0) = 0;
	}
#endif

	pBlock->size = 0;
	pBlock->pAlloc = NULL;
	int32 isSlow = ((uint32)(pBlock) < (uint32)(pSlowHead)) ? 1 : 0;

	// consolidate runs of free blocks
	blockHeader *pFirstFreeBlock = pBlock;
	int32 addToFreeList = 1;
	while( pFirstFreeBlock->pPrevBlock && !pFirstFreeBlock->pPrevBlock->size )
	{
		// can't be first if on fast side (because you would have expected the fast head to engulf a free block on the front)
		// can't have more than 1 in a row because they should have been consolidated at the time they were freed
		assert( isSlow || pFirstFreeBlock->pPrevBlock->pPrevBlock );
		assert( !pFirstFreeBlock->pPrevBlock->pPrevBlock || pFirstFreeBlock->pPrevBlock->pPrevBlock->size != 0 );
		
		// don't add to free list if there is a block right before you that is also free
		addToFreeList = 0;
		pFirstFreeBlock = pFirstFreeBlock->pPrevBlock;
	}

	blockHeader *pNextUsedBlock = pBlock->pNextBlock;
	while( pNextUsedBlock && !pNextUsedBlock->size )
	{
		// i think this can't be the last one and there can't be more than 1 free in a row after the block we're freeing right?
		// can't be last is only for slow side (can't be last because you would have expected the slow head to engulf a free block on the end)
		// can't have more than 1 in a row because they should have been consolidated at the time they were freed
		assert( !isSlow || pNextUsedBlock->pNextBlock );
		assert( !pNextUsedBlock->pNextBlock || pNextUsedBlock->pNextBlock->size != 0 );
		
		// have to remove this block from the free list and move the pointers back to the newly freed block
		if( addToFreeList )
		{
			// removing a block with 1 free on the right only
			pBlock->pPrevFree = pNextUsedBlock->pPrevFree;
			if( pBlock->pPrevFree ) pBlock->pPrevFree->pNextFree = pBlock;
			pBlock->pNextFree = pNextUsedBlock->pNextFree;
			if( pBlock->pNextFree ) pBlock->pNextFree->pPrevFree = pBlock;
			if( mpSlowFreeHead == pNextUsedBlock ) mpSlowFreeHead = pBlock;
			if( mpFastFreeHead == pNextUsedBlock ) mpFastFreeHead = pBlock;
		}
		else
		{
			// removing a block that's in between 2 free blocks
			assert( (pBlock->pPrevBlock->size == 0) && (pBlock->pPrevBlock->pNextFree == pNextUsedBlock) && (pNextUsedBlock->pPrevFree == pBlock->pPrevBlock) );
			pBlock->pPrevBlock->pNextFree = pNextUsedBlock->pNextFree;
			if( pNextUsedBlock->pNextFree ) pNextUsedBlock->pNextFree->pPrevFree = pBlock->pPrevBlock;
		}
		addToFreeList = 0;
		
		pNextUsedBlock = pNextUsedBlock->pNextBlock;
	}

	pFirstFreeBlock->pNextBlock = pNextUsedBlock;
	if( pNextUsedBlock )
	{
		pNextUsedBlock->pPrevBlock = pFirstFreeBlock;
	}
	else if( isSlow )
	{
		// don't add to free list if this block was the last one on the slow side
		addToFreeList = 0;
		if( pFirstFreeBlock->pPrevBlock ) pFirstFreeBlock->pPrevBlock->pNextBlock = NULL;
		if( pFirstFreeBlock->pPrevFree ) pFirstFreeBlock->pPrevFree->pNextFree = NULL;
		if( pFirstFreeBlock == mpSlowFreeHead ) mpSlowFreeHead = NULL;
		pSlowHead = (uint8*)pFirstFreeBlock;
		mpLastSlowAlloc = pFirstFreeBlock->pPrevBlock;
	}

	if( (uint32)(pFirstFreeBlock) == (uint32)(pFastHead) )
	{
		// don't add to free list if this block was the first one on the fast side
		addToFreeList = 0;

		// this means that this block was the first one right?
		// if it wasn't, then shouldn't the previous free block have already been consumed by the fast head pointer?
		assert( pFirstFreeBlock == pBlock );
		
		if( pNextUsedBlock )
		{
			pNextUsedBlock->pPrevBlock = NULL;
			pFastHead = (uint8*)pNextUsedBlock;

			// handle { used, ffh, used } and then you free the first used and fast head eats it
			if( mpFastFreeHead && ((uint32)pFastHead > (uint32)mpFastFreeHead) )
			{
				mpFastFreeHead = mpFastFreeHead->pNextFree;
				if( mpFastFreeHead ) mpFastFreeHead->pPrevFree = NULL;
			}
		}
		else
		{
			pFastHead = pMemPool + memPoolSize;
			mpFastFreeHead = NULL;
		}
	}

	// now add into free list if need be
	if( addToFreeList )
	{
		blockHeader **ppFreeHead = (isSlow) ? &mpSlowFreeHead : &mpFastFreeHead;
		if( !(*ppFreeHead) || ((uint32)(*ppFreeHead) > (uint32)pBlock) )
		{
			// pBlock is the new head
			if( (*ppFreeHead) ) (*ppFreeHead)->pPrevFree = pBlock;
			pBlock->pPrevFree = NULL;
			pBlock->pNextFree = (*ppFreeHead);
			(*ppFreeHead) = pBlock;
		}
		else
		{
			// pBlock is linked into the free list after the beginning somewhere
			blockHeader *pTmp = (*ppFreeHead);
			while( pTmp->pNextFree && ((uint32)pTmp->pNextFree < (uint32)pBlock) )
			{
				assert( (uint32)pTmp->pNextFree > (uint32)pTmp );
				pTmp = pTmp->pNextFree;
			}
	
			pBlock->pPrevFree = pTmp;
			pBlock->pNextFree = pTmp->pNextFree;
			pTmp->pNextFree = pBlock;
			if( pBlock->pNextFree ) pBlock->pNextFree->pPrevFree = pBlock;
		}
	}
}

void MMMem::Shutdown()
{
}

int MMMem::GetAllocSize( void *pAlloc )
{
	assert( pAlloc );
	blockHeader *pBlockHdr = *((blockHeader**)(((uint32)pAlloc) - allocHeaderSize - sizeof(blockHeader*)));
	assert( pBlockHdr->pAlloc == pAlloc );
	return pBlockHdr->size;
}

void MMMem::ValidateFreeList()
{
	blockHeader *pTmp = (blockHeader*)pMemPool;
	blockHeader *pFreeListPtr = NULL;

	// validate slow free list
	assert( !mpSlowFreeHead || (mpSlowFreeHead->size == 0) );
	while( pTmp && ((uint32)pTmp < (uint32)pSlowHead) )
	{
		if( pTmp->size == 0 )
		{
			if( !pFreeListPtr )
			{
				assert( mpSlowFreeHead == pTmp );
			}
			else
			{
				assert( pFreeListPtr->pNextFree == pTmp );
				assert( pTmp->pPrevFree == pFreeListPtr );
			}
			pFreeListPtr = pTmp;
		}

		assert( (!pTmp->pNextBlock && (mpLastSlowAlloc == pTmp)) || ((uint32)pTmp->pNextBlock < (uint32)pSlowHead) );
		pTmp = pTmp->pNextBlock;
	}
	assert( !pFreeListPtr || ((pFreeListPtr->size == 0) && (pFreeListPtr->pNextFree == NULL)) );

	// validate fast free list
	assert( !mpFastFreeHead || (mpFastFreeHead->size == 0) );
	pTmp = (blockHeader*)pFastHead;
	pFreeListPtr = NULL;
	while( pTmp && ((uint32)pTmp < ((uint32)(pMemPool) + memPoolSize)) )
	{
		if( pTmp->size == 0 )
		{
			if( !pFreeListPtr )
			{
				assert( mpFastFreeHead == pTmp );
			}
			else
			{
				assert( pFreeListPtr->pNextFree == pTmp );
				assert( pTmp->pPrevFree == pFreeListPtr );
			}
			pFreeListPtr = pTmp;
		}

		assert( !pTmp->pNextBlock || ((uint32)pTmp->pNextBlock < ((uint32)(pMemPool) + memPoolSize)) );
		pTmp = pTmp->pNextBlock;
	}
	assert( !pFreeListPtr || ((pFreeListPtr->size == 0) && (pFreeListPtr->pNextFree == NULL)) );
}

int MMMem::GetPoolStartAddr()
{
	return (u32)pMemPool;
}

