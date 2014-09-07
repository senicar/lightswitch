#ifndef Gestures_h
#define Gestures_h

#define DEBUG false

#include "Arduino.h"
#include "math.h"

class Gestures {
	public:
		Gestures(int nullX, int nullY);
		void update(int x, int y, bool active);
		void start();
		void end();
		void reset();
		String updateGestures();
		unsigned startTime;
		unsigned endTime;
		unsigned lastUpdateTime;
		unsigned updateInterval;

		// this is used to all the gliches with touch
		// screen can send two touches on one touch first is a milisecond glich
		// ignore it
		unsigned minGestureTime;

		int lastDirectionSteps;
		int startX;
		int startY;
		int endX;
		int endY;

		unsigned dragTolerance;
		unsigned tapDelay;
		unsigned gestureTime;

		bool active;

		String gestures;
		String lastDirection;
	private:
		int _x;
		int _y;
		int _pX;
		int _pY;
		int _hpX;
		int _hpY;
		int _nullX;
		int _nullY;
};

#endif

