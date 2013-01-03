#include "Clipboard.h"

#if defined(GOO_PLATFORM_WIN32)

	#include "CrtDbgInc.h"

	// hack for getting things to compile with old DXSDK:
	#include <windows.h>
	std::string BoyLib::getClipboardText()
	{
		HGLOBAL hGlobal;		// Global memory handle
		LPSTR lpszData;			// Pointer to clipboard data
		SIZE_T nSize;	// Size of clipboard data

		// First, open the clipboard. OpenClipboard() takes one
		// parameter, the handle of the window that will temporarily
		// be it's owner. If NULL is passed, the current process
		// is assumed.
		OpenClipboard(NULL);

		// Request a pointer to the text on the clipboard.
		hGlobal = GetClipboardData(CF_TEXT);

		// If there was no text on the clipboard, we have
		// been returned a NULL handle.	
		if (hGlobal == NULL) 
		{
			return "";
		}

		// Now we have a global memory handle to the text
		// stored on the clipboard. We have to lock this global
		// handle so that we have access to it.
		lpszData = (LPSTR)GlobalLock(hGlobal);
		// Now get the size of the text on the clipboard.
		nSize = GlobalSize(hGlobal);

		// Now, copy the text into the return buffer. At the
		// end, we need to add a NULL string terminator.
		std::string dataString(lpszData,nSize);

		// Now, simply unlock the global memory pointer
		// and close the clipboard.
		GlobalUnlock(hGlobal);
		CloseClipboard();

		return dataString;
	}

#else

	std::string BoyLib::getClipboardText()
	{
		return "";
	}

#endif
