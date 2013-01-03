#include "Font.h"

#include <assert.h>
#include "Boy/Util.h"
#include "BoyLib/BoyUtil.h"
#include "BoyLib/UStringStream.h"
#include <iostream>
#include <sstream>
#include "Graphics.h"
#include "Image.h"
#include "ResourceLoader.h"
#include "Storage.h"

using namespace Boy;

#include "BoyLib/CrtDbgNew.h"

Font::Font(ResourceLoader *loader, const std::string &path) : Resource(loader,path)
{
	assert(loader!=NULL);
	mImage = mLoader->createImage(path);
	mHeight = -1;
	mSpacing = 0;
	mLineSpacing = -1;
	mScale = 1;
}

Font::~Font()
{
	std::map<wchar_t,FontChar*>::iterator iter;
	for (iter=mChars.begin() ; iter!=mChars.end() ; iter++)
	{
		delete iter->second;
	}
	mChars.clear();

	delete mImage;
}

bool Font::hasChar(wchar_t ch)
{
	return mChars.find(ch)!=mChars.end();
}

void Font::addChar(wchar_t ch)
{
	// make sure this character has not already been added:
	assert(mChars.find(ch)==mChars.end());

	// create a new font character:
	FontChar *fchar = new FontChar;
	fchar->width = -1;

	// remember it:
	mChars[ch] = fchar;
}

void Font::setWidth(wchar_t ch, int width)
{
	// make sure this character has been added:
	assert(mChars.find(ch)!=mChars.end());

	// set its width:
	mChars[ch]->width = width;
}

void Font::setSubrect(wchar_t ch, const BoyLib::Rect &subrect)
{
	// make sure this character has been added:
	assert(mChars.find(ch)!=mChars.end());

	// set its width:
	mChars[ch]->subrect = subrect;

	// update the height:
	mHeight = std::max(mHeight, subrect.getHeight());
}

void Font::setOffset(wchar_t ch, const BoyLib::Vector2 &offset)
{
	// make sure this character has been added:
	assert(mChars.find(ch)!=mChars.end());

	// set its width:
	mChars[ch]->offset = offset;
}

int Font::getHeight()
{
	assert(mHeight>0);
	return mHeight*mScale;
}

int Font::getStringWidth(const Boy::UString &str)
{
	int width = 0;
	FontChar *fcCurr = NULL;
	FontChar *fcPrev = NULL;

	int size = str.length();
	for (int i=0 ; i<size ; i++)
	{
		wchar_t currChar = str[i];

		// get the font character:
		fcCurr = mChars[currChar];

		// skip this char if we have no data for it:
		if (fcCurr==NULL)
		{
			continue;
		}

		// if this is not the first char:
		if (fcPrev!=NULL)
		{
			std::map<wchar_t,int>::iterator kernIter = fcPrev->kearningValues.find(currChar);
			if (kernIter != fcPrev->kearningValues.end())
			{
				// add the kearning:
				width += kernIter->second;
			}
		}

		// add the width of the character:
		width += fcCurr->width + mSpacing;

		// the current char is the new previous char:
		fcPrev = fcCurr;
	}

	return width * mScale;
}

int Font::getLineSpacing()
{
	return (mLineSpacing>=0 ? mLineSpacing : mHeight) * mScale;
}

float Font::drawString(Graphics *g, const Boy::UString &str, float scale)
{
	scale *= mScale;
	g->pushTransform();

	// iterate over all characters of the string:
	FontChar *fcCurr = NULL;
	FontChar *fcPrev = NULL;
	float prevCharWidth = 0;
	int numChars = str.length();
	float totalWidth = 0;
	for (int i=0 ; i<numChars ; i++)
	{
		wchar_t currChar = str[i];

		// get the font character:
		fcCurr = mChars[currChar];

		// skip this char if we have no data for it:
		if (fcCurr==NULL)
		{
			continue;
		}

		// if this is not the first char:
		float kerning = 0;
		if (fcPrev!=NULL)
		{
			std::map<wchar_t,int>::iterator kernIter = fcPrev->kearningValues.find(currChar);
			if (kernIter != fcPrev->kearningValues.end())
			{
				// remember the kearning:
				kerning = kernIter->second;
			}
		}

		// kearning value + half character width + prev char width:
		float dx = kerning + fcCurr->width/2.0f*scale + prevCharWidth;
		g->preTranslate(dx, 0);

		// keep track of total width:
		totalWidth += dx;

		// scale the char:
		g->pushTransform();
			// scale:
			g->preScale(scale,scale);
			// draw the character:
			g->drawImage(mImage, fcCurr->subrect);
		g->popTransform();

		// remember the width we should increment by:
		prevCharWidth = (fcCurr->width - fcCurr->width/2.0f + mSpacing) * scale;

		// the current char is the new previous char:
		fcPrev = fcCurr;
	}
	g->popTransform();

	totalWidth += prevCharWidth;

	return totalWidth;
}

void Font::loadCharList(Boy::UStringStream &fontStream, std::vector<wchar_t> &charList)
{
	Boy::UString buf;

	while (true)
	{
		// find the next single or double quote:
		fontStream.skipUntil("'\"");

		// read the char:
		wchar_t ch = fontStream.get();

		// add it to the font:
		charList.push_back(ch);

		// find the next comma or close paren:
		wchar_t delim = fontStream.skipUntil(",)");

		// if we've reached the end of the list:
		if (delim==')')
		{
			// read the rest of the line:
			fontStream.skipUntil("\r\n");

			// let's get out of here:
			break;
		}
	}
}
void Font::loadWidthList(Boy::UStringStream &fontStream, std::vector<int> &widthList)
{
	Boy::UString buf;

	// find the next paren:
	fontStream.skipUntil('(');

	while (true)
	{
		// read up to the next comma
		wchar_t delim;
		buf = fontStream.readUntil(",)",&delim);

		// get the width:
		assert(buf.isAsciiOnly());
		int width = atoi(buf.toUtf8());

		// set the width:
		widthList.push_back(width);

		// if we've reached the end of the list:
		if (delim==')')
		{
			// read the rest of the line:
			fontStream.skipUntil("\r\n");

			// let's get out of here:
			break;
		}
	}
}

void Font::loadRectList(Boy::UStringStream &fontStream, std::vector<BoyLib::Rect> &rectList)
{
	Boy::UString buf;
	int x,y,width,height;

	// find the next paren:
	fontStream.skipUntil('(');

	while (true)
	{
		// find beginning of next rect:
		fontStream.skipUntil('(');

		// read the rect:
		buf = fontStream.readUntil(',');
		assert(buf.isAsciiOnly());
		x = atoi(buf.toUtf8());
		buf = fontStream.readUntil(',');
		assert(buf.isAsciiOnly());
		y = atoi(buf.toUtf8());
		buf = fontStream.readUntil(',');
		assert(buf.isAsciiOnly());
		width = atoi(buf.toUtf8());
		buf = fontStream.readUntil(')');
		assert(buf.isAsciiOnly());
		height = atoi(buf.toUtf8());

		// remember the rect:
		rectList.push_back(BoyLib::Rect(x,y,width,height));

		// find the next comma or close paren:
		wchar_t delim = fontStream.skipUntil(",)");

		// if we've reached the end of the list:
		if (delim==')')
		{
			// read the rest of the line:
			fontStream.skipUntil("\r\n");

			// let's get out of here:
			break;
		}
	}
}

void Font::loadOffsetList(Boy::UStringStream &fontStream, std::vector<BoyLib::Vector2> &offsetList)
{
	Boy::UString buf;
	int x,y;

	// find the next paren:
	fontStream.skipUntil('(');

	while (true)
	{
		// find beginning of next offset:
		fontStream.skipUntil('(');

		// read the offset:
		buf = fontStream.readUntil(',');
		assert(buf.isAsciiOnly());
		x = atoi(buf.toUtf8());
		buf = fontStream.readUntil(')');
		assert(buf.isAsciiOnly());
		y = atoi(buf.toUtf8());

		// remember the offset:
		offsetList.push_back(BoyLib::Vector2(x,y));

		// find the next comma or close paren:
		wchar_t delim = fontStream.skipUntil(",)");

		// if we've reached the end of the list:
		if (delim==')')
		{
			// read the rest of the line:
			fontStream.skipUntil("\r\n");

			// let's get out of here:
			break;
		}
	}
}

void Font::loadKerningPairs(Boy::UStringStream &fontStream, std::vector<Boy::UString> &kerningPairs)
{
	Boy::UString buf;

	// skip to the start of the first pair:
	fontStream.skipUntil('"');

	while (true)
	{
		// read the kerning pair:
		buf = fontStream.readUntil('"', true);
		buf.collapseEscapes();

		// remember the pair:
		kerningPairs.push_back(buf);

		// find the beginning of the next pair or a close paren:
		wchar_t delim = fontStream.skipUntil("\")");

		// if we've reached the end of the list:
		if (delim==')')
		{
			// read the rest of the line:
			fontStream.skipUntil("\r\n");

			// let's get out of here:
			break;
		}
	}
}

void Font::loadKerningValues(Boy::UStringStream &fontStream, std::vector<int> &kerningValues)
{
	Boy::UString buf;

	// find the next paren:
	fontStream.skipUntil('(');

	while (true)
	{
		// read up to the next comma
		wchar_t delim;
		buf = fontStream.readUntil(",)",&delim);

		// get the kerning value:
		assert(buf.isAsciiOnly());
		int value = atoi(buf.toUtf8());

		// set the width:
		kerningValues.push_back(value);

		// if we've reached the end of the list:
		if (delim==')')
		{
			// read the rest of the line:
			fontStream.skipUntil("\r\n");

			// let's get out of here:
			break;
		}
	}
}

bool Font::init(bool includeSounds)
{
	// create the image:
	assert(mImage != NULL);
	if (mImage->load()==false)
	{
		// we couldn't load the image, fail:
		return false;
	}

	// if we've already loaded the chars:
	if (mChars.size()>0)
	{
		// we're done:
		return true;
	}

	// open and read the font file:
	BoyFileHandle hFile;
	Storage *pStorage = Environment::instance()->getStorage();
	Storage::StorageResult result = pStorage->FileOpen( (mPath+".txt").c_str(), Storage::STORAGE_MODE_READ | Storage::STORAGE_MUST_EXIST, &hFile );
	assert( result == Storage::STORAGE_OK );
	int fsize = pStorage->FileGetSize( hFile );
	char *pFileData = new char[ fsize+1 ];
	memset( pFileData, 0, fsize+1 );
	result = pStorage->FileRead( hFile, pFileData, fsize );
	assert( result == Storage::STORAGE_OK );
	pStorage->FileClose( hFile );

	// convert to unicode string stream:
	Boy::UStringStream fontStream( pFileData );
	delete [] pFileData;

	// data holders:
	std::vector<wchar_t> charList;
	std::vector<int> widthList;
	std::vector<BoyLib::Rect> rectList;
	std::vector<BoyLib::Vector2> offsetList;
	std::vector<Boy::UString> kerningPairs;
	std::vector<int> kerningValues;

	Boy::UString buf;
	while (!fontStream.eof())
	{
		// read the next word:
		buf = fontStream.readUntil(' ').trim();

		// if we have encountered a definition:
		if (buf == "Define")
		{
			// read the next word:
			buf = fontStream.readUntil(" \r\n\t");

			// if we have encountered a character list:
			if (buf=="CharList")
			{
				loadCharList(fontStream, charList);
			}
			// if we have encountered a width list:
			else if (buf=="WidthList")
			{
				loadWidthList(fontStream, widthList);
				assert(widthList.size()==charList.size());
			}
			// if we have encountered a rect list:
			else if (buf=="RectList")
			{
				loadRectList(fontStream, rectList);
				assert(rectList.size()==charList.size());
			}
			// if we have encountered an offset list:
			else if (buf=="OffsetList")
			{
				loadOffsetList(fontStream, offsetList);
				assert(offsetList.size()==charList.size());
			}
			// if we have encountered kerning pairs:
			else if (buf=="KerningPairs")
			{
				loadKerningPairs(fontStream, kerningPairs);
			}
			// if we have encountered kerning values:
			else if (buf=="KerningValues")
			{
				loadKerningValues(fontStream, kerningValues);
				assert(kerningValues.size()==kerningPairs.size());
			}
			else
			{
				assert(false);
			}
		}
		else if (buf=="LayerSetScale")
		{
			// skip the layer name:
			fontStream.skip(' ');
			fontStream.skipUntil(' ');

			// read the scale:
			buf = fontStream.readUntil(';');

			// set the scale:
			assert(buf.isAsciiOnly());
			mScale = atof(buf.toUtf8());
		}
		else if (buf=="LayerSetSpacing")
		{
			// skip the layer name:
			fontStream.skip(' ');
			fontStream.skipUntil(' ');

			// read the spacing:
			buf = fontStream.readUntil(';');

			// set the spacing:
			assert(buf.isAsciiOnly());
			mSpacing = atoi(buf.toUtf8());
		}
		else if (buf=="LayerSetCharWidths")
		{
			// skip the layer name:
			fontStream.skip(' ');
			fontStream.skipUntil(' ');

			// if this is an inline declaration:
			if (fontStream.get()=='(')
			{
				// skip the "'":
				fontStream.get();

				// get the char:
				wchar_t ch = fontStream.get();
				assert(ch==' ');

				// skip the "') (":
				fontStream.get();
				fontStream.get();
				fontStream.get();
				fontStream.get();

				// get the width:
				buf = fontStream.readUntil(')');
				assert(buf.isAsciiOnly());
				int width = atoi(buf.toUtf8());
				assert(width>0);

				// add the space char:
				charList.push_back(' ');
				widthList.push_back(width);
				rectList.push_back(BoyLib::Rect(0,0,0,0));
				offsetList.push_back(0);
			}

			// skip the rest of the line:
			fontStream.skipUntil(';');
		}
		else
		{
			// dump the rest of the line:
			fontStream.skipUntil("\r\n");
		}
	}

	// construct the font:
	int numChars = (int)charList.size();
	for (int i=0 ; i<numChars ; i++)
	{
		wchar_t ch = charList[i];
		addChar(ch);
		setWidth(ch, widthList[i]);
		setSubrect(ch, rectList[i]);
		setOffset(ch, offsetList[i]);
	}

	// add kerning info:
	int numKerningPairs = (int)kerningPairs.size();
	for (int i=0 ; i<numKerningPairs ; i++)
	{
		Boy::UString pair = kerningPairs[i];
		setKerning(pair[0],pair[1],kerningValues[i]);
	}

	// success
	return true;
}

void Font::destroy(bool includeSounds)
{
	mImage->release();
}

void Font::setKerning(wchar_t ch1, wchar_t ch2, int kvalue)
{
	FontChar *fc = mChars[ch1];
	fc->kearningValues[ch2] = kvalue;
}
