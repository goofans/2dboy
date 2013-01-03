#include "WinStorage.h"

#include "BoyLib/UString.h"
#include <stdio.h>

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

WinStorage::WinStorage() :
	mFileKey( 0 )
{
}

WinStorage::~WinStorage()
{
}

Storage::StorageResult WinStorage::FileOpen( const char *pFilePathUtf8, int modeFlags, BoyFileHandle *pFileHandleOut )
{
	StorageResult result = STORAGE_FAIL;

	// must specify a read/write and a create/open flag
	// must specify a handle to set
	if( (modeFlags & STORAGE_MODE_MASK) && (modeFlags & STORAGE_DISPO_MASK) && pFileHandleOut )
	{
		const wchar_t *pModeStr = NULL;
		switch( modeFlags )
		{
			case STORAGE_MODE_READ | STORAGE_MUST_EXIST:
				pModeStr = L"rb";
				break;

			case STORAGE_MODE_WRITE | STORAGE_MUST_EXIST:
				pModeStr = L"r+b";
				break;

			case STORAGE_MODE_WRITE | STORAGE_OPEN_ALWAYS:
				pModeStr = L"w+b";
				break;
		}

		// check for bad flag combo
		if( pModeStr )
		{
			FILE *f = NULL;
			Boy::UString pFilePathUnicode(pFilePathUtf8);
			int openResult = _wfopen_s( &f, pFilePathUnicode.wc_str(), pModeStr );
			if( openResult == 0 )
			{
				++mFileKey;
				mOpenFiles[ mFileKey ] = f;
				*pFileHandleOut = (BoyFileHandle)mFileKey;
				result = STORAGE_OK;
			}
		}
	}
	
	return result;
}

Storage::StorageResult WinStorage::FileRead( BoyFileHandle fileHandle, void *pBuffer, int readSizeBytes )
{
	StorageResult result = STORAGE_FAIL;

	// validate file handle and dest buffer
	FILE *f = GetFilePtr( fileHandle );
	if( f && pBuffer )
	{
		int bytesRead = (int)fread( pBuffer, 1, readSizeBytes, f );
		if( bytesRead == readSizeBytes )
		{
			// any time you don't read exactly as much as you want, the call fails
			// always use FileGetSize() to figure out exactly how much you want to read ahead of time if need be
			result = STORAGE_OK;
		}
	}

	return result;
}

Storage::StorageResult WinStorage::FileWrite( BoyFileHandle fileHandle, const void *pBuffer, int writeSizeBytes )
{
	StorageResult result = STORAGE_FAIL;

	// validate file handle and src buffer
	FILE *f = GetFilePtr( fileHandle );
	if( f && pBuffer )
	{
		int bytesWritten = (int)fwrite( pBuffer, 1, writeSizeBytes, f );
		if( bytesWritten == writeSizeBytes )
		{
			result = STORAGE_OK;
		}
	}

	return result;
}

Storage::StorageResult WinStorage::FileClose( BoyFileHandle fileHandle )
{
	StorageResult result = STORAGE_FAIL;

	// validate file handle
	FILE *f = GetFilePtr( fileHandle );
	if( f )
	{
		int closeResult = fclose( f );
		if( closeResult != EOF )
		{
			int key = (int)fileHandle;
			mOpenFiles.erase( key );
			result = STORAGE_OK;
		}
	}

	return result;
}

int WinStorage::FileGetSize( BoyFileHandle openFileHandle )
{
	int sizeBytes = -1;

	// validate file handle
	FILE *f = GetFilePtr( openFileHandle );
	if( f )
	{
		long int origPos = ftell( f );
		fseek( f, 0, SEEK_END );
		sizeBytes = ftell( f );
		fseek( f, origPos, SEEK_SET );
	}

	return sizeBytes;
}

FILE *WinStorage::GetFilePtr( BoyFileHandle hFile )
{
	FILE *pRet = NULL;

	int key = (int)hFile;
	std::map<int,FILE*>::iterator i = mOpenFiles.find( key );
	if( i != mOpenFiles.end() )
	{
		pRet = i->second;
	}

	return pRet;
}
