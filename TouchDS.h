#ifndef TouchDS_h
#define TouchDS_h

#include "Arduino.h"

class TouchDS {
	public:
		TouchDS (int x1, int x2, int y1, int y2);
		bool update();
		int getX();
		int getY();
		int touchX;
		int touchY;
		int resolutionX;
		int resolutionY;
		bool active;
		bool calibrated;
	private:
		int _y1;
		int _x2;
		int _y2;
		int _x1;
		int _screenYmin;
		int _screenYmax;
		int _screenXmin;
		int _screenXmax;
};

#endif

