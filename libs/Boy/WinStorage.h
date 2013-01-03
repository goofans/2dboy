
#pragma once

#include "Storage.h"
#include <map>

namespace Boy
{

	class WinStorage : public Storage
	{
	
		public:

			WinStorage();
			virtual ~WinStorage();

			// storage implementation
			virtual StorageResult FileOpen( const char *pFilePathUtf8, int modeFlags, BoyFileHandle *pFileHandleOut );
			virtual StorageResult FileRead( BoyFileHandle fileHandle, void *pBuffer, int readSizeBytes );
			virtual StorageResult FileWrite( BoyFileHandle fileHandle, const void *pBuffer, int writeSizeBytes );
			virtual StorageResult FileClose( BoyFileHandle fileHandle );
			virtual int FileGetSize( BoyFileHandle openFileHandle );

		private:

			FILE *GetFilePtr( BoyFileHandle hFile );

			int mFileKey;
			std::map<int,FILE*> mOpenFiles;

	};

}
