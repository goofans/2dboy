#pragma once

#include "BoyLib/CrtDbgInc.h"

namespace Boy
{
	void aesEncrypt(const unsigned char *key, 
		const char *inData, int inDataSize, 
		char **outData, int *outDataSize);

	void aesDecrypt(const unsigned char *key, 
		const char *inData, int inDataSize, 
		char **outData, int *outDataSize);

	bool loadDecrypt(const unsigned char *key, 
		const char *filename, 
		char **outData, int *outDataSize);

}
