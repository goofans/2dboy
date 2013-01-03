#pragma once

#include "GamePad.h"
#include <windows.h>
#include <XInput.h>

#pragma comment(lib, "XInput.lib")

namespace Boy
{
	class WinGamePad : public GamePad
	{
	public:

		WinGamePad(int id);
		virtual ~WinGamePad();

		void vibrate(int leftVal=0, int rightVal=0);
	};
}
