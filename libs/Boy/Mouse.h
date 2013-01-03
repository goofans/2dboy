#pragma once

#include "BoyLib/Vector2.h"
#include "Controller.h"
#include <vector>

namespace Boy
{
	class MouseListener;
	class Image;
	class Graphics;

	class Mouse : public Controller
	{
	public:

		enum Button
		{
			BUTTON_LEFT = 0,
			BUTTON_MIDDLE = 1,
			BUTTON_RIGHT = 2,
			BUTTON_UNKNOWN = 0xff
		};

		Mouse(int id);
		virtual ~Mouse();

		const BoyLib::Vector2 &getPosition();
		void setPosition(const BoyLib::Vector2 &pos);

		inline int getId() { return mId; }

		bool isVisible();
		void setVisible(bool visible);

		bool isInBounds();

		virtual void setEnabled(bool enabled);
		bool getButtonsEnabled();
		void setButtonsEnabled(bool enabled);
		bool isButtonDown(Button button);

		void addListener(MouseListener *listener);
		void removeListener(MouseListener *listener);
		void addLowLevelListener(MouseListener *listener);
		void removeLowLevelListener(MouseListener *listener);

		void fireMoveEvent(float x, float y);
		void fireDownEvent(Button button, int clickCount);
		void fireUpEvent(Button button);
		void fireWheelEvent(int delta);
		void fireEnterEvent();
		void fireLeaveEvent();

		// overrides:
		virtual void setConnected(bool connected);

	private:

		bool isValidMouseCoord(float x, float y);

	protected:

		std::vector<MouseListener*> mListeners;
		std::vector<MouseListener*> mLowLevelListeners;
		BoyLib::Vector2 mPosition;
		bool mIsVisible;
		int mId;
		bool mButtonsEnabled;
		bool mIsInBounds;

		bool mIsButtonDown[3];
	};
}

