#include <assert.h>
#include "BoyLib/UString.h"
#include "BoyLib/Vector2.h"
#include "Graphics.h"
#include <string.h>
#include <vector>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

/*	9 jan 2009: I've reimplemented the BIND_PTR macro as a template function
	because the original violates strict aliasing rules resulting in crashes
	on Linux when compiled with optimizations.  - Maks */
template<class A, class B>
inline void BIND_PTR(A *base, B* &ptr)
{
	if (ptr != NULL)
	{
		unsigned long offset = (unsigned long)ptr;
		assert(offset < (unsigned long)base);
		ptr = (B*)((char*)base + offset);
	}
}

inline void tokenize(const Boy::UString& str, const Boy::UString &delimitersNotReturned, const Boy::UString &delimitersReturned, std::vector<Boy::UString> &tokens)
{
	// make sure the tokens vector is empty:
	tokens.clear();

	// if this is an empty string, just return:
	if (str.length()==0)
	{
		return;
	}

	Boy::UString delim(delimitersNotReturned);
	delim.append(delimitersReturned);
	int strLen = str.length();

	// find first "non-delimiter".
	int prevPos = 0;
	int pos = str.findChar(delim,0,true);

	// if there are no delimiters in this string:
	if (pos < 0)
	{
		tokens.push_back(str);
		return;
	}

	while (pos >= 0)
   	{
		if (pos!=prevPos)
		{
    		// found a token, add it to the vector.
    		tokens.push_back(str.substr(prevPos, pos - prevPos));
		}

		// if this is a delimiter we want returned:
		if (delimitersReturned.findChar(str[pos]) >= 0)
		{
//			printf("delim: %s\n",str.substr(pos,1).toUtf8());
			// add it to the vector:
			tokens.push_back(str.substr(pos,1));
		}
	
    	// advance the position:
    	prevPos = pos+1;

		// if we are past the end of the string:
		if (prevPos >= strLen)
		{
			break;
		}
	
    	// find next "non-delimiter"
    	pos = str.findChar(delim, prevPos, true);

		// if there are no more delimiters:
		if (pos == std::string::npos)
		{
			// add the last token to the vector:
			tokens.push_back(str.substr(prevPos));
		}
	}
}

inline void tokenize(const Boy::UString& str, const Boy::UString& delimiters, std::vector<Boy::UString> &tokens)
{
	return tokenize(str,delimiters,Boy::UString(),tokens);
}

inline void tokenize(const std::string& str, const std::string &delimitersNotReturned, const std::string &delimitersReturned, std::vector<std::string> &tokens)
{
	// make sure the tokens vector is empty:
	tokens.clear();

	// if this is an empty string, just return:
	if (str.length()==0)
	{
		return;
	}

	std::string delim = delimitersNotReturned + delimitersReturned;
	std::string::size_type strLen = (int)str.length();

	// find first "non-delimiter".
	std::string::size_type prevPos = 0;
	std::string::size_type pos = str.find_first_of(delim, 0);

	// if there are no delimiters in this string:
	if (pos == std::string::npos)
	{
		tokens.push_back(str);
		return;
	}

	while (pos != std::string::npos)
   	{
		if (pos!=prevPos)
		{
    		// found a token, add it to the vector.
    		tokens.push_back(str.substr(prevPos, pos - prevPos));
		}

		// if this is a delimiter we want returned:
		if (delimitersReturned.find_first_of(str[pos]) != std::string::npos)
		{
			// add it to the vector:
			tokens.push_back(str.substr(pos,1));
		}
	
    	// advance the position:
    	prevPos = pos+1;

		// if we are past the end of the string:
		if (prevPos >= strLen)
		{
			break;
		}
	
    	// find next "non-delimiter"
    	pos = str.find_first_of(delim, prevPos);

		// if there are no more delimiters:
		if (pos == std::string::npos)
		{
			// add the last token to the vector:
			tokens.push_back(str.substr(prevPos));
		}
	}
}

//same as explode() in php!
inline void tokenize(const std::string& str, const std::string& delimiters, std::vector<std::string> &tokens)
{
	return tokenize(str,delimiters,"",tokens);
}

inline void tokenizeInt(const std::string& str, const std::string& delimiters, std::vector<int> &tokens)
{
	tokens.clear();
	std::vector<std::string> stringTokens;
	tokenize(str,delimiters,stringTokens);
	unsigned long numTokens = (unsigned long)stringTokens.size();
	for (unsigned long i=0 ; i<numTokens ; i++)
	{
		tokens.push_back(atoi(stringTokens[i].c_str()));
	}
}

inline void tokenizeFloat(const std::string& str, const std::string& delimiters, std::vector<float> &tokens)
{
	tokens.clear();
	std::vector<std::string> stringTokens;
	tokenize(str,delimiters,stringTokens);
	unsigned long numTokens = (unsigned long)stringTokens.size();
	for (unsigned long i=0 ; i<numTokens ; i++)
	{
		tokens.push_back((float)atof(stringTokens[i].c_str()));
	}
}

inline bool parseBool(const std::string& str, bool defaultValue)
{
	if (str=="true")
	{
		return true;
	}
	else if (str=="false")
	{
		return false;
	}
	else
	{
		return defaultValue;
	}
}

inline bool parseBool(const char *str, bool defaultValue)
{
	if (str==NULL)
	{
		return defaultValue;
	}

	if (strcmp(str,"true")==0)
	{
		return true;
	}

	assert(strcmp(str,"false")==0);
	return false;
}

inline float parseFloat(const std::string& str)
{
	return (float)atof(str.c_str());
}

inline float parseFloat(const char *str, float defaultValue)
{
	if (str==NULL)
	{
		return defaultValue;
	}

	return (float)atof(str);
}

inline int parseInt(const char *str, int defaultValue)
{
	if (str==NULL)
	{
		return defaultValue;
	}

	return atoi(str);
}

inline unsigned long parseRGB(const std::string& str)
{
	std::vector<int> rgb;
	tokenizeInt(str,", ",rgb);
	return 0xff000000 | rgb[0]<<16 | rgb[1]<<8 | rgb[2];
}

inline unsigned long parseARGB(const char *str, Boy::Color color)
{
	if (str==NULL)
	{
		return color;
	}

	std::vector<int> argb;
	tokenizeInt(str,", ",argb);
	return (unsigned long)(argb[0]<<24 | argb[1]<<16 | argb[2]<<8 | argb[3]);
}

inline unsigned long parseRGBA(const std::string& str)
{
	std::vector<int> rgba;
	tokenizeInt(str,", ",rgba);
	return (unsigned long)(rgba[3]<<24 | rgba[0]<<16 | rgba[1]<<8 | rgba[2]);
}

inline void parseVector2(const std::string &str, BoyLib::Vector2 &vec)
{
	std::vector<float> values;
	tokenizeFloat(str, ", ", values);
	vec.x = values[0];
	vec.y = values[1];
}
/*
inline bool isspacew(wchar_t ch)
{
	if (ch==' ' || ch=='\n' || ch=='\r' || ch=='\t')
	{
		return true;
	}
	return false;
}

inline Boy::UString trim(const Boy::UString& theString)
{
	int length = theString.length();
	int aStartPos = 0;
	while ( aStartPos < length && isspacew(theString[aStartPos]) )
		aStartPos++;

	int anEndPos = length - 1;
	while ( (anEndPos >= 0) && (anEndPos < length) && isspacew(theString[anEndPos]) )
		anEndPos--;

	return theString.substr(aStartPos, anEndPos - aStartPos + 1);
}
*/
