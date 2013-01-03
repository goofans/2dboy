#pragma once

#include "BoyLib/Rect.h"
#include "BoyLib/UStringStream.h"
#include "BoyLib/Vector2.h"
#include "Resource.h"
#include <string>
#include <vector>

namespace Boy
{
	class Graphics;
	class Image;

	struct FontChar
	{
		int width;
		BoyLib::Rect subrect;
		BoyLib::Vector2 offset;
		std::map<wchar_t,int> kearningValues;
	};

	class Font : public Resource
	{
	public:

		Font(ResourceLoader *loader, const std::string &path);
		virtual ~Font();

		// font construction:
		void addChar(wchar_t ch);
		bool hasChar(wchar_t ch);
		void setWidth(wchar_t ch, int width);
		void setSubrect(wchar_t ch, const BoyLib::Rect &subrect);
		void setOffset(wchar_t ch, const BoyLib::Vector2 &offset);
		void setKerning(wchar_t ch1, wchar_t ch2, int kvalue);

		// font access:
		int getHeight();
		int getStringWidth(const Boy::UString &str);
		int getLineSpacing();

		float drawString(Graphics *g, const Boy::UString &str, float scale=1);

	protected:

		// implementation of Resource:
		virtual bool init(bool includeSounds);
		virtual void destroy(bool includeSounds);

	private:

		// misc:
		void loadCharList(Boy::UStringStream &fontStream, std::vector<wchar_t> &charList);
		void loadWidthList(Boy::UStringStream &fontStream, std::vector<int> &widthList);
		void loadRectList(Boy::UStringStream &fontStream, std::vector<BoyLib::Rect> &rectList);
		void loadOffsetList(Boy::UStringStream &fontStream, std::vector<BoyLib::Vector2> &offsetList);
		void loadKerningPairs(Boy::UStringStream &fontStream, std::vector<Boy::UString> &kerningPairs);
		void loadKerningValues(Boy::UStringStream &fontStream, std::vector<int> &kerningValues);

	private:

		Image *mImage;

		float mHeight;
		float mSpacing;
		float mLineSpacing;
		float mScale;

		std::map<wchar_t,FontChar*> mChars;
		
	};
}

