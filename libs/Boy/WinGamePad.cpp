#include "WinGamePad.h"

using namespace Boy;

WinGamePad::WinGamePad(int id) : GamePad(id)
{
}

WinGamePad::~WinGamePad()
{
}

void WinGamePad::vibrate(int leftVal, int rightVal)
{
	// Create a Vibraton State
	XINPUT_VIBRATION vibration;

	// Zeroise the Vibration
	ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));

	// Set the Vibration Values
	vibration.wLeftMotorSpeed = leftVal;
	vibration.wRightMotorSpeed = rightVal;

	// Vibrate the controller
	XInputSetState(mId, &vibration);
}

/*
	// update the controllers:
	for (int i=0 ; i<GAMEPAD_COUNT_MAX ; i++)
	{
		if (mGamePads[i]->isConnected())
		{
			WinGamePad *gp = dynamic_cast<WinGamePad*>(mGamePads[i]); 
			XINPUT_STATE gps = gp->getState();

			envDebugLog("wButtons = 0x%x\n",gps.Gamepad.wButtons);

			// buttons:
			if (gps.Gamepad.wButtons!=0 && !mGamePad[0]->isButtonDown(Mouse::BUTTON_LEFT))
			{
				mMice[0]->fireDownEvent(Mouse::BUTTON_LEFT,1);
			}
			else if (gps.Gamepad.wButtons==0 && mMice[0]->isButtonDown(Mouse::BUTTON_LEFT))
			{
				mMice[0]->fireUpEvent(Mouse::BUTTON_LEFT);
			}

			// mouse position:
			BoyLib::Vector2 pos;
			float w = screenWidth();
			float h = screenHeight();
			pos.x = w/2 * (1 + ((float)gps.Gamepad.sThumbLX) / 32768.0f);
			pos.y = h/2 * (1 - ((float)gps.Gamepad.sThumbLY) / 32768.0f);
			mMice[0]->setPosition(pos);
		}
	}
*/
