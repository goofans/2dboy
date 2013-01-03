#pragma once

#ifdef GOO_PLATFORM_WIN32

/*
 * replace all calls to new/delete with calls to our debug version
 * of those calls in order to properly include line numbers and
 * file names in the debug heap info
 */

#include "MemDbg.h"

#if defined(_CRTDBG_MAP_ALLOC) && defined(GOO_PLATFORM_WIN32)

	inline void __cdecl operator delete(void * _P, const char* sFile, int nLine)
	{
		BoyLib::MemRemoveTrack(_P);
		::operator delete(_P);
	}

	inline void __cdecl operator delete[](void * _P, const char* sFile, int nLine)
	{
//		BoyLib::MemRemoveTrack(_P);
		::operator delete[](_P);
	}

	inline void* __cdecl operator new(unsigned int s, const char* sFile, int nLine)
	{
		void *ptr = ::operator new(s, _NORMAL_BLOCK, sFile, nLine);
//		BoyLib::MemAddTrack(ptr, s, sFile, nLine);
		return ptr;
	}

	inline void* __cdecl operator new[](unsigned int s, const char* sFile, int nLine)
	{
		void *ptr = ::operator new[](s, _NORMAL_BLOCK, sFile, nLine);
//		BoyLib::MemAddTrack(ptr, s, sFile, nLine);
		return ptr;
	}

#	define BOY_DEBUG_NEW new(__FILE__, __LINE__)
#	define new BOY_DEBUG_NEW
//#	define BOY_DEBUG_DELETE delete(__FILE__, __LINE__)
//#	define delete BOY_DEBUG_DELETE

#endif

#endif
