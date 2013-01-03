#pragma once

#include "Boy/Controller.h"
#include <vector>

namespace Boy
{
	class KeyboardListener;

	class Keyboard : public Controller
	{
	public:

		enum Key
		{
			KEY_BACKSPACE	= 0xffffff00,
			KEY_TAB			= 0xffffff01,
			KEY_RETURN		= 0xffffff02,
			KEY_SHIFT		= 0xffffff03,
			KEY_CONTROL		= 0xffffff04,
			KEY_PAUSE		= 0xffffff05,
			KEY_ESCAPE		= 0xffffff06,
			KEY_END			= 0xffffff07,
			KEY_HOME		= 0xffffff08,
			KEY_LEFT		= 0xffffff09,
			KEY_UP			= 0xffffff0a,
			KEY_RIGHT		= 0xffffff0b,
			KEY_DOWN		= 0xffffff0c,
			KEY_INSERT		= 0xffffff0d,
			KEY_DELETE		= 0xffffff0e,
			KEY_F1			= 0xffffff0f,
			KEY_F2			= 0xffffff10,
			KEY_F3			= 0xffffff11,
			KEY_F4			= 0xffffff12,
			KEY_F5			= 0xffffff13,
			KEY_F6			= 0xffffff14,
			KEY_F7			= 0xffffff15,
			KEY_F8			= 0xffffff16,
			KEY_F9			= 0xffffff17,
			KEY_F10			= 0xffffff18,
			KEY_F11			= 0xffffff19,
			KEY_F12			= 0xffffff1a,

			KEY_UNKNOWN		= 0xffffffFF
		};

		enum Modifiers
		{
			KEYMOD_NONE		= 0x00,
			KEYMOD_ALT		= 0x01,
			KEYMOD_SHIFT	= 0x02,
			KEYMOD_CTRL		= 0x04,
			KEYMOD_META		= 0x08,
			KEYMOD_NUMLOCK	= 0x10,
			KEYMOD_CAPSLOCK	= 0x20,

			/*  Two masks to easily distinguish between active modifiers (which
				are held down by the user) and passive modifiers. */
			KEYMOD_ACTIVE	= 0x0f,  /* alt, shift, ctrl, meta */
			KEYMOD_PASSIVE	= 0x30   /* num lock, caps lock */
		};

		Keyboard();
		virtual ~Keyboard();

		void addListener(KeyboardListener *listener);
		void removeListener(KeyboardListener *listener);

		void fireKeyDownEvent(wchar_t unicode, Key key, Modifiers mods);
		void fireKeyUpEvent(wchar_t unicode, Key key, Modifiers mods);

	protected:

		std::vector<KeyboardListener*> mListeners;

	};
}
