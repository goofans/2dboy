
#ifndef _INC_MMMEM_H_
#define _INC_MMMEM_H_

typedef unsigned int uint32;
typedef signed int int32;
typedef unsigned char uint8;

//#define MEM_SAFTEY_CHECKS

enum eMMMemType
{
	eMemSlow,
	eMemFast
};

struct blockHeader
{
	blockHeader *pPrevBlock;
	blockHeader *pNextBlock;
	blockHeader *pPrevFree;
	blockHeader *pNextFree;
	unsigned char *pAlloc;
	uint32 size;
};

#if defined(MEM_SAFTEY_CHECKS)

	struct allocHeader
	{
		uint32 magic1;
		uint32 magic2;
		uint32 magic3;
		uint32 magic4;
	};

	struct allocFooter
	{
		uint32 magic1;
		uint32 magic2;
		uint32 magic3;
		uint32 magic4;
	};

#endif

class MMMem
{
	public:

		MMMem();
		~MMMem();

		void Init( void *pPoolMemory, uint32 poolSizeBytes );
		void Shutdown();

		void *Alloc( uint32 allocSizeBytes, eMMMemType allocType, uint32 allocAlign );
		void Free( const void *pAllocation );

		int GetAllocSize( void *pAlloc );
		int GetPoolStartAddr();

	private:

		enum
		{
			kMagic1 = 0x12345678,
			kMagic2 = 0x33333333,
			kMagic3 = 0xABCDEFAB,
			kMagic4 = 0xCCCCCCCC,
		};

		void *MakeAlloc( blockHeader *pTargetAddr, blockHeader *pPrevBlock, blockHeader *pNextBlock, uint32 size, uint32 alignPad );
		void CalcAllocParams( blockHeader *pTargetAddr, uint32 alignment, uint32 size, uint32 *alignPad, uint32 *requiredSizeofBlock );
		void CalcAllocParamsRev( blockHeader *pTargetAddr, uint32 alignment, uint32 size, uint32 *alignPad, uint32 *requiredSizeofBlock );
		void FreeBlock( blockHeader *pBlock );

		void ValidateFreeList();

		unsigned char *pMemPool;
		unsigned char *pSlowHead, *pFastHead;
		uint32 memPoolSize;
		blockHeader *mpSlowFreeHead, *mpFastFreeHead;
		blockHeader *mpLastSlowAlloc;
		uint32 mHighWatermarkBytes;
		int32 allocHeaderSize;
		int32 allocFooterSize;
};

#endif
