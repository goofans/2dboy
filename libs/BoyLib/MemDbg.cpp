#include "MemDbg.h"

#include <assert.h>
#include <time.h>

using namespace BoyLib;

static bool gShowLeaks = false;
static bool gAllocMapValid = false;
static int gTraceFileLen = 0;
static int gTraceFileNum = 1;
static FILE *gTraceFile = NULL;
static AllocMap gAllocMap;

AllocMap::AllocMap() 
{ 
	gAllocMapValid = true; 
}

AllocMap::~AllocMap() 
{ 
	if (gShowLeaks) 
	{
		DumpUnfreed();		
	}

	gAllocMapValid = false; 
}

int BoyLib::AllocHook(int allocType, 
					  void *userData, 
					  size_t size, 
					  int blockType, 
					  long requestNumber, 
					  const unsigned char *filename, 
					  int lineNumber)
{
	switch (allocType)
	{
	case _HOOK_ALLOC:
		MemAddTrack(userData, size, (const char *)filename, lineNumber);
	case _HOOK_FREE:
		MemRemoveTrack(userData);
	case _HOOK_REALLOC:
	default:
		assert(false);
	}

	return true;
}

void BoyLib::DumpClientFunction(void *userPortion, size_t blockSize)
{
	printf("%s : %d\n",userPortion,blockSize);
}

void BoyLib::MemAddTrack(void *addr,  size_t asize,  const char* fname, int lnum)
{
	if (!gAllocMapValid)
	{
		return;
	}

	gShowLeaks = true;

	AllocInfo &info = gAllocMap[addr];
	strncpy(info.file, fname, sizeof(info.file)-1);
	info.line = lnum;
	info.size = asize;
	info.seq = gSequenceCounter++;
};

void BoyLib::MemRemoveTrack(void* addr)
{
	if (!gAllocMapValid)
	{
		return;
	}

	AllocMap::iterator iter = gAllocMap.find(addr);
	if (iter != gAllocMap.end())
	{
		gAllocMap.erase(iter);
	}
};

void BoyLib::DumpUnfreed()
{
	if (!gAllocMapValid)
	{
		return;
	}

	AllocMap::iterator i;
	size_t totalSize = 0;
	char buf[8192];

#ifdef DUMP_LEAKED_MEM
	char hex_dump[1024];
	char ascii_dump[1024];
	int count = 0;
	int index = 0;
#endif

	FILE* f = fopen("mem_leaks.txt", "wt");
	if (!f)
		return;

	time_t aTime = time(NULL);
	sprintf(buf, "Memory Leak Report for %s\n",	asctime(localtime(&aTime)));
	fprintf(f, buf);
	printf("\n");
	printf(buf);
	for(i = gAllocMap.begin(); i != gAllocMap.end(); i++) 
	{
		sprintf(buf, "%5d : %5d bytes %s(%d)\n", i->second.seq, i->second.size, i->second.file, i->second.line);
		printf(buf);
		fprintf(f, buf);

#ifdef DUMP_LEAKED_MEM
		unsigned char* data = (unsigned char*)i->first;

		for (index = 0; index < i->second.size; index++)
		{			
			unsigned char _c = *data;
			
			if (count == 0)
				sprintf(hex_dump, "\t%02X ", _c);
			else
				sprintf(hex_dump, "%s%02X ", hex_dump, _c);
		
			if ((_c < 32) || (_c > 126))
				_c = '.';

			if (count == 7)
				sprintf(ascii_dump, "%s%c ", ascii_dump, _c);
			else
				sprintf(ascii_dump, "%s%c", count == 0 ? "\t" : ascii_dump, _c);
			

			if (++count == 16)
			{
				count = 0;
				sprintf(buf, "%s\t%s\n", hex_dump, ascii_dump);
				fprintf(f, buf);

				memset((void*)hex_dump, 0, 1024);
				memset((void*)ascii_dump, 0, 1024);
			}

			data++;
		}

		if (count != 0)
		{
			fprintf(f, hex_dump);
			for (index = 0; index < 16 - count; index++)
				fprintf(f, "\t");

			fprintf(f, ascii_dump);

			for (index = 0; index < 16 - count; index++)
				fprintf(f, ".");
		}

		count = 0;
		fprintf(f, "\n\n");
		memset((void*)hex_dump, 0, 1024);	
		memset((void*)ascii_dump, 0, 1024);
#endif 

		totalSize += i->second.size;
	}


	sprintf(buf, "-----------------------------------------------------------\n");
	fprintf(f, buf);
	printf(buf);
	sprintf(buf, "Total Unfreed: %d bytes (%dKB)\n\n", totalSize, totalSize / 1024);
	printf(buf);
	fprintf(f, buf);
}
