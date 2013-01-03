#pragma once

#include "CrtDbgInc.h"

#include <stdlib.h>
#include <map>

namespace BoyLib
{
	struct AllocInfo
	{
		size_t	size;
		char	file[_MAX_PATH+1];
		int		line;
		int		seq;
	};

	class AllocMap : public std::map<void*,AllocInfo>
	{
	public:
		AllocMap();
		~AllocMap();
	};

	int AllocHook(int allocType, void *userData, size_t size, int blockType,
		long requestNumber, const unsigned char *filename, int lineNumber);
	void DumpClientFunction(void *userPortion, size_t blockSize);

	void MemAddTrack(void* addr,  size_t asize,  const char *fname, int lnum);
	void MemRemoveTrack(void *addr);
	void DumpUnfreed();

	static int gSequenceCounter = 0;
}
