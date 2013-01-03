
#include "Storage.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

Storage::Storage()
{
}

Storage::~Storage()
{
}

Storage::StorageResult Storage::FileGetSize( const char *pFilePath, int *pSizeBytesOut )
{
	StorageResult result = STORAGE_FAIL;

	BoyFileHandle hFile;
	StorageResult openResult = FileOpen( pFilePath, STORAGE_MODE_READ | STORAGE_MUST_EXIST, &hFile );
	if( (openResult == STORAGE_OK) && pSizeBytesOut )
	{
		*pSizeBytesOut = FileGetSize( hFile );
		result = STORAGE_OK;
	}
	if (openResult == STORAGE_OK) FileClose(hFile);

	return result;
}
