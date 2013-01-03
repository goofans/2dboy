#pragma once

#include "BoyLib/Vector2.h"
#include "Controller.h"
#include <vector>

namespace Boy
{
	class GamePadListener;

	class GamePad : public Controller
	{
	public:

		enum Button
		{
			BUTTON_0 = 0,
			BUTTON_1 = 1,
			BUTTON_2 = 2,
			BUTTON_3 = 3,
			BUTTON_L_SHOULDER = 4,
			BUTTON_R_SHOULDER = 5,
			BUTTON_L_STICK = 6,
			BUTTON_R_STICK = 7,
			BUTTON_START = 8,
			BUTTON_AUX = 9,
			BUTTON_DPAD_UP = 10,
			BUTTON_DPAD_DOWN = 11,
			BUTTON_DPAD_LEFT = 12,
			BUTTON_DPAD_RIGHT = 13,
			BUTTON_COUNT = 14,
			BUTTON_UNKNOWN = 0xff
		};

		GamePad(int id);
		virtual ~GamePad();

		void vibrate(int leftVal, int rightVal);

		const BoyLib::Vector2 &getAnalogL();
		const BoyLib::Vector2 &getAnalogR();
		void setAnalogL(float x, float y);
		void setAnalogR(float x, float y);

		float getTriggerL();
		float getTriggerR();
		void setTriggers(float l, float r);

		void setButtonDown(Button button, bool down);
		bool isButtonDown(Button button);

		inline int getId() { return mId; }

		virtual void setEnabled(bool enabled);

		void addListener(GamePadListener *listener);
		void removeListener(GamePadListener *listener);

		// overrides:
		virtual void setConnected(bool connected);

	protected:

		void fireDownEvent(Button button);
		void fireUpEvent(Button button);

	protected:

		std::vector<GamePadListener*> mListeners;
		int mId;
		BoyLib::Vector2 mAnalogL;
		BoyLib::Vector2 mAnalogR;
		float mTriggerL;
		float mTriggerR;
		bool mIsButtonDown[BUTTON_COUNT];
	};
}

