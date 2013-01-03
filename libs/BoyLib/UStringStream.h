#pragma once

#include "BoyUtil.h"
#include "UString.h"

namespace Boy
{
	class UStringStream
	{
	public:

		UStringStream(const char *utf8);
		~UStringStream();

		bool eof();
		UString readUntil(const UString &delimiters, wchar_t *delimFound=NULL, bool respectEscapeChar=false);
		UString readUntil(wchar_t delimiter, bool respectEscapeChar=false);
		wchar_t skipUntil(const UString &delimiters);
		void skipUntil(wchar_t delimiter);
		void skip(wchar_t skipChar);
		wchar_t get();

	private:

		UString mString;
		int mPos;

	};
}