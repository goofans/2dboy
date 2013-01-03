#pragma once

#include "Keyboard.h"

namespace Boy
{
	class KeyboardListener
	{
	public:

		KeyboardListener() {}
		virtual ~KeyboardListener() {}

		virtual void keyUp(wchar_t unicode, Keyboard::Key key, Keyboard::Modifiers mods) = 0;
		virtual void keyDown(wchar_t unicode, Keyboard::Key key, Keyboard::Modifiers mods) = 0;

	};
}
