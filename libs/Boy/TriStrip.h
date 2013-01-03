#pragma once

#include "BoyLib/CrtDbgInc.h"
#include "Graphics.h"

namespace Boy
{
	class TriStrip
	{
	public:

		TriStrip() {}
		virtual ~TriStrip() {}

		virtual void setColor(Color color) = 0;

		virtual void setVertPos(int i, float x, float y, float z=0) = 0;
		virtual void setVertTex(int i, float u, float v) = 0;
		virtual void setVertColor(int i, Color color) = 0;
	};
}
