// Touch Screen
// http://bildr.org/2011/06/ds-touch-screen-arduino/ (24 jan 2014)
// http://www.microbuilder.eu/Tutorials/LPC2148/ADC/NDSTouchScreen.aspx (24 jan 2014)
// https://www.sparkfun.com/datasheets/LCD/HOW%20DOES%20IT%20WORK.pdf (24 jan 2014)

#include "Arduino.h"
#include "TouchDS.h"

TouchDS::TouchDS (int x1, int x2, int y1, int y2) {
	_x1 = x1;
	_x2 = x2;
	_y1 = y1;
	_y2 = y2;

	// resolution will be mapped between this values
	// if screen is unconstrained this are the limit

	// DS
	_screenYmax = 900;
	_screenYmin = 140;
	_screenXmax = 920;
	_screenXmin = 90;

	// 7"
	//_screenYmax = 880;
	//_screenYmin = 180;
	//_screenXmax = 920;
	//_screenXmin = 90;


	// this is the reslution of the device, all data will be constrained to this
	resolutionX = 1024;
	resolutionY = 1024;

	active = false;
	touchX = 0;
	touchY = 0;
}

bool TouchDS::update() {
	touchX = getX();
	touchY = getY();

	// it would be quite an effort to get a zero with a finger and
	// even if it happens we would rather have the last non zero value
	if( (touchX == 0) | (touchY == 0) ) {
		active = false;
		return active;
	}

	//Serial.print("x: ");
	//Serial.print(touchX);
	//Serial.print(" - y: ");
	//Serial.println(touchY);

	active = true;
	return active;
}



int TouchDS::getX(){
	pinMode(_y1, INPUT); // read the value
	pinMode(_x2, OUTPUT);
	pinMode(_y2, INPUT); // disconnected
	pinMode(_x1, OUTPUT);

	digitalWrite(_x2, LOW); // GND
	digitalWrite(_x1, HIGH); // power

	delay(5); //pause to allow lines to power up

	int x = analogRead(_y1);

	x = map(x, _screenXmin, _screenXmax, resolutionX, 0);
	x = constrain(x, 0, resolutionX);

	return x;
}

int TouchDS::getY(){

	pinMode(_y1, OUTPUT);
	pinMode(_x2, INPUT); // disconnected
	pinMode(_y2, OUTPUT);
	pinMode(_x1, INPUT); // read the value

	digitalWrite(_y1, LOW); // GND
	digitalWrite(_y2, HIGH); // power

	delay(5); //pause to allow lines to power up

	int y = analogRead(_x1);

	y = map(y, _screenYmin, _screenYmax, resolutionY, 0);
	y = constrain(y, 0, resolutionY);

	return y;
}


