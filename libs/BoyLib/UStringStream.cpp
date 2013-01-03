#include "UStringStream.h"

using namespace Boy;

#include "CrtDbgNew.h"

UStringStream::UStringStream(const char *utf8)
{
	mString = utf8;
	mPos = 0;
}

UStringStream::~UStringStream()
{
}

bool UStringStream::eof()
{
	return mPos >= mString.length();
}

UString UStringStream::readUntil(const UString &delimiters, wchar_t *delimFound, bool respectEscapeChar)
{
	// remember current position:
	int currPos = mPos;

	// find the first occurence of one of the delimiters after the current position:
	int firstDelim = mString.findChar(delimiters,mPos,respectEscapeChar);

	// if we couldn't find a delimiter in the remainder of the string:
	if (firstDelim<0)
	{
		// move to the end of the string:
		mPos = mString.length();

		// return the remainder of the string:
		return mString.substr(currPos);
	}
	else
	{
		// skip to char after this delimiter:
		mPos = firstDelim + 1; 

		// return the delimiter we found:
		if (delimFound!=NULL)
		{
			*delimFound = mString[firstDelim];
		}

		// return the string from the current position up to and including the delimiter:
		return mString.substr(currPos,firstDelim-currPos);
	}
}

UString UStringStream::readUntil(wchar_t delimiter, bool respectEscapeChar)
{
	// remember current position:
	int currPos = mPos;

	// find the first occurence of the delimiter after the current position:
	int firstDelim = mString.findChar(delimiter,mPos,respectEscapeChar);

	// if we couldn't find a delimiter in the remainder of the string:
	if (firstDelim<0)
	{
		// move to the end of the string:
		mPos = mString.length();

		// return the remainder of the string:
		return mString.substr(currPos);
	}
	else
	{
		// skip to char after this delimiter:
		mPos = firstDelim + 1; 

		// return the string from the current position up to and including the delimiter:
		return mString.substr(currPos,firstDelim-currPos);
	}
}

wchar_t UStringStream::skipUntil(const UString &delimiters)
{
	wchar_t ch;
	while (true)
	{
		ch = get();
		if (delimiters.findChar(ch)>=0 || eof())
		{
			break;
		}
	}

	return ch;
}

void UStringStream::skipUntil(wchar_t delimiter)
{
	while (true)
	{
		wchar_t ch = get();
		if (ch==delimiter || eof())
		{
			break;
		}
	}
}

void UStringStream::skip(wchar_t skipChar)
{
	while (!eof() && mString[mPos]==skipChar)
	{
		mPos++;
	}
}

wchar_t UStringStream::get()
{
	if (eof())
	{
		return 0;
	}

	wchar_t ch = mString[mPos];
	mPos++;
	return ch;
}


