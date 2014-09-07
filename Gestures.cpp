// Gestures
// http://www.emanueleferonato.com/2013/03/22/detecting-mouse-gestures-in-flash-with-as3-fixed/ (25 jan 2014)

#include "Arduino.h"
#include "Gestures.h"
#include "math.h"

Gestures::Gestures (int nullX = 0, int nullY = 0) {
	this->_nullX = nullX;
	this->_nullY = nullY;

	this->_x = nullX;
	this->_y = nullY;
	this->_pX = nullX;
	this->_pY = nullY;

	this->startX = nullX;
	this->startY = nullY;
	this->endX = nullX;
	this->endY = nullY;

	this->startTime = 0;
	this->endTime = 0;

	this->updateInterval = 50;
	this->lastUpdateTime = 0;

	// drag tolerance is basically distance from previous point
	// that happended in updateInterval
	//
	// var distance = (dX * dX) + (dY * dY);
	//
	// the bigger the resolution and smaller the screen, the bigger
	// dragTolerance should be
	this->dragTolerance = 3000;

	// millis beyond this its a drag
	this->tapDelay = 300;
	this->gestureTime = 0;

	this->active = false;

	this->gestures = "";
	this->lastDirection = "";
}


void Gestures::update(int x = 0, int y = 0, bool active = false) {

	if( ! this->active & ! active ) {
		return;
	}

	unsigned currTime = millis();
	//Serial.println(currTime);
	//Serial.println(this->lastUpdateTime);
	//Serial.println(this->updateInterval);
	//Serial.println(currTime - this->lastUpdateTime);
	if( ( this->active & ((currTime - this->lastUpdateTime) < this->updateInterval) ) ) {
		// if not enough time has past exit
		return;
	}

	// if we put this after update interval there is less changes of
	// touch-in / touch-out coordinates error
	if( active & (x != 0) & (y != 0) ) {
		this->_x = x;
		this->_y = y;
	}

	/*
	Serial.print("x: ");
	Serial.print(x);
	Serial.print(" - y: ");
	Serial.println(y);
	*/

	if( ! this->active & active) {
		// gesture is not active but gesture device (touch screen) is
		this->start();
		return;
	}

	if( this->active & ! active ) {
		// gestures are active but gesture device is not
		this->end();
		return;
	}

	/*
	Serial.print("update X: ");
	Serial.println(this->_x);
	Serial.print("update Y: ");
	Serial.println(this->_y);
	*/

	this->gestures = this->gestures + this->updateGestures();

	this->lastUpdateTime = currTime;
}

void Gestures::start() {
	this->startTime = millis();
	this->active = true;

	this->startX = this->_pX = this->_x;
	this->startY = this->_pY = this->_y;

	this->lastDirectionSteps = 0;
	this->lastDirection = "";


	// reset gestures if endTime is bigger than tapDelay
	if( (this->startTime - this->endTime) > this->tapDelay )
		this->gestures = "";

	Serial.println("START GESTURES");
}

void Gestures::end() {
	/* touch-in (startX startY) are more accurate, end x y can jump to other values
	this->endX = this->_x;
	this->endY = this->_y;
	*/

	this->endX = this->startX;
	this->endY = this->startY;

	this->active = false;

	this->endTime = millis();
	this->gestureTime = this->endTime - this->startTime;

	// one last time for the sake of the TAP and HOLD
	this->gestures = this->gestures + this->updateGestures();

	// now that we have last gesture we can safely reset
	this->_x = this->_pX = this->_nullX;
	this->_y = this->_pY = this->_nullY;

	// so that the next time it will run right trough it
	this->lastUpdateTime = this->updateInterval;

	Serial.println("END GESTURES");
}

String Gestures::updateGestures() {
	String currentDirection = "";
	unsigned currentTime = millis();

	// this ones are checked before distance so that we save CPU time
	if( ! this->active
			& (this->gestureTime < this->tapDelay)
			& (this->gestures == "" | this->gestures == "T" | this->gestures == "TT")
			) {
		currentDirection = "T";
		Serial.print("Gesture : ");
		Serial.println(currentDirection);
		return currentDirection;
	}

	// make H availabel as soon as possible
	// this also marks gesture as after tapDelay, so no need to check again
	if ( (this->gestures == "")
			& ((currentTime - this->startTime) > this->tapDelay) ) {
		currentDirection = "H";
		Serial.print("Gesture : ");
		Serial.println(currentDirection);
		return currentDirection;
	}

	int dX = this->_pX - this->_x;
	int dY = this->_pY - this->_y;
	// because the drag can get diagonal
	unsigned distance = (dX * dX) + (dY * dY);

	int hdX = this->_hpX - this->_x;
	int hdY = this->_hpY - this->_y;
	unsigned hdistance = (hdX * hdX) + (hdY * hdY);

	// ignore small changes or first run/touch
	if( (distance > this->dragTolerance) & (this->_pX != this->_nullX) & (this->_pY != this->_nullY) ) {

		this->_pX = this->_x;
		this->_pY = this->_y;

		int angle = round(atan2(dX,dY) * 57.3);
		int hangle = round(atan2(hdX,hdY) * 57.3);

		if(angle < 0) angle = 360 + angle;
		if(hangle < 0) hangle = 360 + hangle;

		if( abs(angle-hangle) > 10 )
			return "";

		//Serial.print("Gesture angle diff: ");
		//Serial.println(abs(angle-hangle));

		if( (angle > 25) & (angle < 55) ) {
			currentDirection = "ul";
		}

		if( (angle > 55) & (angle < 115) ) {
			currentDirection = "L";
		}

		if( (angle > 115) & (angle < 155) ) {
			currentDirection = "dl";
		}

		if( (angle > 155) & (angle < 205) ) {
			currentDirection = "D";
		}

		if( (angle > 205) & (angle < 245) ) {
			currentDirection = "dr";
		}

		if( (angle > 245) & (angle < 295) ) {
			currentDirection = "R";
		}

		if( (angle > 295) & (angle < 335) ) {
			currentDirection = "ur";
		}

		if( (angle < 25) | (angle > 335) ) {
			currentDirection = "U";
		}

		if(DEBUG) Serial.print("Gesture currentDirection: ");
		if(DEBUG) Serial.println(currentDirection);

		if( currentDirection != this->lastDirection ) {
			this->lastDirection = currentDirection;
			this->lastDirectionSteps = 1;

			Serial.print("Gesture : ");
			Serial.println(gestures + currentDirection);

			return currentDirection;
		}
		else if ( currentDirection == this->lastDirection ) {
			this->lastDirectionSteps += 1;
		}
	}

	// check halfstep
	if( hdistance > (this->dragTolerance/2) ) {
		//Serial.println("update 2/4 distance");
		this->_hpX = this->_x;
		this->_hpY = this->_y;
	}

	return "";
}

void Gestures::reset() {
	Serial.print("Gesture : ");
	Serial.println(gestures);

	if(DEBUG) Serial.print("Gesture Time : ");
	if(DEBUG) Serial.println(gestureTime);

	if(DEBUG) Serial.print("Gesture lastDirectionSteps : ");
	if(DEBUG) Serial.println(lastDirectionSteps);

	if(DEBUG) Serial.print("Gesture X : ");
	if(DEBUG) Serial.print(endX);

	if(DEBUG) Serial.print(" - Y : ");
	if(DEBUG) Serial.println(endY);

	gestures = "";
	startTime = endTime;
	active = false;
}

