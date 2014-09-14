#include "Arduino.h"
#include "Tlc5940.h"
#include "TouchDS.h"
#include "Gestures.h"
#include "Switcher.h"

/*
const int Switcher::pwm[16] = {
	20, 30, 45, 65, 90, 130, 200, 330, 450,
	600, 800, 1100, 1700, 2500, 3400, 4095,
};
*/

const int Switcher::pwm[16] = {
	45, 65, 90, 130,
	200, 330, 450, 600,
	800, 1100, 1450, 1700,
	2200, 2800, 3400, FULL_ON,
};

/* -------------------------------------------------------*
** RGB Values
** -------------------------------------------------------*/

const int Switcher::states[10][3] = {
	{0    , 0    , 0   }, // 0 - off
	{4095 , 4095 , 4095}, // 1 - white - dimmer setup
	{4095 , 0    , 0   }, // 2 - red - setup
	{0    , 4095 , 0   }, // 3 - green - confirmation
	{0    , 0    , 4095}, // 4 - blue
	{4095 , 4095 , 0   }, // 5 - yellow - faved
	{4095 , 1500 , 0   }, // 6 - orange - add time
	{0    , 4095 , 4095}, // 7 - cian - IR, network API
	{2500 , 0    , 4095}, // 8 - violet - fade
	{4095 , 0    , 4095}, // 9 - pink - region/light actions
};


// TODO : FIXME : to much granularity in intensity control of individual light and region
// this should be more bind to each other, either control regions or lights

void Switcher::toggle(int l) {
	if(l >= NUM_LIGHTS)
		return;

	if(DEBUG) Serial.println("TOGGLE");
	if((this->pinData & (1 << l)) == 0) {
		this->turnOn(l);
	}
	else {
		this->turnOff(l);
	}
}

void Switcher::turnOn(int l) {
	if(DEBUG) Serial.println("TURN ON");
	dim(l,FULL_ON);
}

void Switcher::turnOff(int l) {
	if(DEBUG) Serial.println("TURN OFF");
	dim(l,OFF);
}

void Switcher::favToggle() {
	if(DEBUG) Serial.println("FAV TOGGLE");
	if(  pinData > 0 )
		allOff();
	else
		favOn();
}

void Switcher::favOff() {
	for(int l=0; l < this->numLights; l++) {
		if(this->lights[l].fav) {
			if(DEBUG) Serial.println("FAV OFF");
			this->dim(l,OFF);
		}
	}

	favState = false;
}

void Switcher::favOn() {
	bool favFound = false;
	for(int l=0; l < this->numLights; l++) {
		if(this->lights[l].fav) {
			if(DEBUG) Serial.println("FAV ON");
			this->dim(l,FULL_ON);
			favFound = true;
		}
	}

	if(!favFound) allOn();

	favState = true;
}

void Switcher::allOn() {
	if(DEBUG) Serial.println("ALL ON");
	this->setAll(FULL_ON);
	favState = true;
}

void Switcher::allOff() {
	if(DEBUG) Serial.println("ALL OFF");
	this->clear();
	favState = false;
}

void Switcher::setAll(int intensity) {
	for(int l=0; l < this->numLights; l++) {
		this->dim(l,intensity);
	}
}

void Switcher::clear() {
	if(DEBUG) Serial.println("CLEAR");
	//Tlc.clear();
	for(int l=0; l < NUM_LIGHTS; l++) {
		dim(l,OFF);
	}
	// turn off all 16bits
	favState = false;
	this->pinData = 0x00;
}

void Switcher::toggleRegion(int r) {
	if(DEBUG) Serial.print("TOGGLE REGION : ");
	if(DEBUG) Serial.println(r);
	if(regions[r].intensity > 0)
		turnOffRegion(r);
	else
		turnOnRegion(r);
}

void Switcher::turnOffRegion(int r) {
	if(DEBUG) Serial.print("TURN OFF REGION : ");
	if(DEBUG) Serial.println(r);

	regions[r].intensity = OFF;
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			turnOff(l);
	}
}

void Switcher::turnOnRegion(int r) {
	if(DEBUG) Serial.print("TURN ON REGION : ");
	if(DEBUG) Serial.println(r);

	regions[r].intensity = FULL_ON;
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			turnOn(l);
	}
}

void Switcher::dimRegion(int r, int intensity) {
	if(DEBUG) Serial.print("DIM REGION : ");
	if(DEBUG) Serial.println(r);

	regions[r].intensity = intensity;
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			dim(l,intensity);
	}
}

void Switcher::dimRegion(int r, int intensity, unsigned dur) {
	if(DEBUG) Serial.print("DIM REGION DURATION : ");
	if(DEBUG) Serial.println(r);

	regions[r].intensity = intensity;
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			dim(l,intensity,dur);
	}
}

void Switcher::dim(int l, int intensity) {
	if(DEBUG) Serial.print("DIM : ");
	if(DEBUG) Serial.println(l);

	dim(l,intensity,0);
}

void Switcher::dim(int l, int intensity, unsigned dur) {
	if(DEBUG) Serial.print("DIM DURATION : ");
	if(DEBUG) Serial.println(l);
	//if(l >= NUM_LIGHTS) return;

	if(intensity > 0 & intensity < 16) {
		lights[l].pwm = intensity;
		intensity = pwm[intensity];
		Serial.println(lights[l].pwm);
	}

	lights[l].intensity = intensity;
	Tlc.set(l,intensity);

	if(intensity == 0) {
		this->pinData &= ~(1u << l);
		//lights[l].on = false;
	}
	else {
		this->pinData |= (1u << l);
		//lights[l].on = true;
	}

	lastTimeSwitched = millis();

	lightInfo(l);
	updateRegion(lights[l].region);
}

void Switcher::dimmer(int r) {
	// this is a continues action so we need another loop

	Serial.println("DIMMER START");

	bool dimmerActive = true;

	//Gest.reset();

	int i = 0;
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r & lights[l].intensity == OFF) {
			Serial.println(lights[l].intensity);
			i++;
		}

		if(lights[l].region == r & lights[l].intensity == FULL_ON)
			indicator("dimmer", 1, lights[l].intensity);
	}

	if(i > 0)
		turnOnRegion(r);

	Tlc.update();
	while(dimmerActive) {
		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		// check what gesture was entered but don't forget about tapDelay
		unsigned timeDiff = millis() - Gest.endTime;

		int level = map(Screen.touchX, 150, 854, 0, pwmNum-1);
		level = constrain(level, 0, pwmNum-1);

		//if(Gest.gestures == "TT") {
			//indicator("dimmer while TT", 3);
			//delay(Gest.tapDelay);
			//dimmerActive = false;
		//}

		if(Screen.active & Gest.gestures !="" & (timeDiff > Gest.tapDelay)) {
			indicator("", 1, pwm[level]);
			this->dimRegion(r, pwm[level]);
		}

		else if (!Screen.active) {
			dimmerActive = false;
		}

		Tlc.update();
	}

	// we reset it by hand since we use dimmer mostly in secondary region action
	// Gest.reset();

	indicator("DIMMER STOP", 0);
}


void Switcher::animate(anim adata) {
	// only two buffers
	for(int a = 0; a < 3; a++) {
		if(!animBuffer[a].active) {
			if(DEBUG) Serial.print("ADDED ANIMATION : ");
			if(DEBUG) Serial.println(a);

			animBuffer[a] = adata;
			Serial.println(regions[ adata.region ].intensity);
			animBuffer[a].startIntensity = regions[ adata.region ].intensity;
			animBuffer[a].endTime = millis() + adata.dur;
			animBuffer[a].active = true;
			// data vas saved no need to continue
			break;
		}
	}
}

void Switcher::animSetRegion(int region) {
	Serial.println("SLEEP START");

	bool durActive = true;
	bool fadeActive = true;
	Screen.update();
	Gest.reset();

	anim adata;

	adata.dur = 0;
	adata.region = region;

	int clockInterval = 6600;

	while(durActive) {
		indicator("dimmer animate", 8);
		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		if(!Gest.active) {
			/* recognize individual numbers
			unsigned number = 10;
			if(Gest.gestures == "urD" | Gest.gestures == "HurD") {
				Serial.println("1");
				number = 1;
				Gest.reset();
			}

			if(Gest.gestures == "LDRDL" | Gest.gestures == "HLDRDL") {
				Serial.println("5");
				number = 5;
				Gest.reset();
			}

			if(Gest.gestures == "RdlR" | Gest.gestures == "HRdlR") {
				Serial.println("2");
				number = 2;
				Gest.reset();
			}

			if(
				Gest.gestures == "RDLU" |
				Gest.gestures == "HRDLU" |

				Gest.gestures == "DLUR" |
				Gest.gestures == "HDLUR" |

				Gest.gestures == "LURD" |
				Gest.gestures == "HLURD" |

				Gest.gestures == "URDL" |
				Gest.gestures == "HURDL" |

				Gest.gestures == "DRUL" |
				Gest.gestures == "HDRUL" |

				Gest.gestures == "LDRU" |
				Gest.gestures == "HLDRU"
				) {
				Serial.println("0");
				number = 0;
				Gest.reset();
			}

			if(dimDuration == 0 && number > 0 && number < 10)
				dimDuration = number;
			else if(number > 0 && number < 10)
				dimDuration = (dimDuration * 10) + number;
			else if(number == 0)
				dimDuration = (dimDuration * 10);
			*/

			/*
			 * Simple clock mechanizm
			 * */

			if(Gest.gestures == "R" | Gest.gestures == "HR") {
				adata.dur = clockInterval;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "Rdr" | Gest.gestures == "HRdr") {
				adata.dur = clockInterval * 2;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrD" | Gest.gestures == "HRdrD") {
				adata.dur = clockInterval * 3;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrD" | Gest.gestures == "HRdrD") {
				adata.dur = clockInterval * 4;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdl" | Gest.gestures == "HRdrDdl") {
				adata.dur = clockInterval * 5;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlL" | Gest.gestures == "HRdrDdlL") {
				adata.dur = clockInterval * 6;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLul" | Gest.gestures == "HRdrDdlLul") {
				adata.dur = clockInterval * 7;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLulU" | Gest.gestures == "HRdrDdlLulU") {
				adata.dur = clockInterval * 8;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLulUur" | Gest.gestures == "HRdrDdlLulUur") {
				adata.dur = clockInterval * 9;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLulUurR" | Gest.gestures == "HRdrDdlLulUurR") {
				adata.dur = clockInterval * 10;
				if(DEBUG) Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "TT") {
				indicator("tt", 3);
				durActive = false;
				delay(Gest.tapDelay);
				Gest.reset();
			}
		}
	}

	while(fadeActive) {
	indicator("fade active", 9);
	Screen.update();
	Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		if(!Gest.active) {

			if(Gest.gestures == "HR" | Gest.gestures == "R") {
				adata.fade = true;
				if(DEBUG) Serial.println("FADE IN");
				Gest.reset();
			}

			if(Gest.gestures == "HL" | Gest.gestures == "L") {
				adata.fade = true;
				if(DEBUG) Serial.println("FADE OUT");
				Gest.reset();
			}

			if(Gest.gestures == "TT") {
				indicator("tt fade", 3);
				fadeActive = false;
				delay(Gest.tapDelay);
				Gest.reset();
			}
		}
	}

	// convert to seconds
	//Serial.println(dimDuration);
	//dimDuration = (dimDuration * 1000);

	animate(adata);
	indicator("SLEEP STOP", 0);
}

int Switcher::getPwmLevel(int intensity) {
	int diff = intensity;
	int diffLevel = pwmNum;
	if(intensity == OFF)
		return 0;

	if(intensity == FULL_ON)
		return pwmNum - 1;

	for(int i = 0; i < pwmNum; i++) {
		int newDiff = abs(pwm[i] - intensity);
		if( newDiff < diff) {
			diffLevel = i;
			diff = newDiff;
		}
	}
	return diffLevel;
}

void Switcher::update() {
	// check and update sleep, animations ...

	// SLEEP
	unsigned long currTime = millis();
	for(int a = 0; a < animBufferNum; a++) {

		if(animBuffer[a].active) {
			if(animBuffer[a].endTime < currTime) {
				Serial.println("dim region");
				dimRegion(animBuffer[a].region, animBuffer[a].endIntensity);
				animBuffer[a].active = false;
			}
			else if(animBuffer[a].fade) {
				unsigned level = map(
									currTime,
									(animBuffer[a].endTime - animBuffer[a].dur),
									animBuffer[a].endTime,
									getPwmLevel(animBuffer[a].startIntensity),
									getPwmLevel(animBuffer[a].endIntensity));


				if( level < pwmNum )
					dimRegion(animBuffer[a].region, pwm[level]);
			}
		}
	}
}


int Switcher::getRegion(int x, int y) {
	this->getRegion(x, y, true);
}

// sloppy is mostly for setup, when we want to select regions inside tolerance,
// when normaly get region you want to select something even if its outside
// of regions tolerance distance
int Switcher::getRegion(int x, int y, bool sloppy = true) {
	// make it out of scope for an array
	int closestRegion = NUM_LIGHTS;
	unsigned long closestDistance = 2147483647L; // this is the biggest the int can get
	for(int r = 0; r < NUM_LIGHTS; r++) {

		int X = regions[r].x;
		int Y = regions[r].y;

		long dx = (X-x);
		long dy = (Y-y);
		unsigned long distance = (dx * dx) + (dy * dy);

		// TODO :  we probably could use abs() here to get the min distance

		if(
			(X != 0) & (Y != 0)
			& (X > (x - this->regionTolerance))
			& (X < (x + this->regionTolerance))
			& (Y > (y - this->regionTolerance))
			& (Y < (y + this->regionTolerance))
			) {

			// FIXME: why do I need something in here for r to be returned properly
			// without this, r=1023
			delay(0);

			// if exact region is found were're done
			return r;
		}
		else if( sloppy & (X != 0) & (Y != 0) & (distance < closestDistance) ) {
			// get closestRegion
			closestDistance = distance;
			closestRegion = r;
		}
	}

	// it should come to this only if exact region isn't found
	// when sloppy = false, it should return MAX_REGIONS
	return closestRegion;
}


void Switcher::updateRegion( int r ) {
	if( r >= numRegions ) return;

	// check if any light in region is active
	int brightest = 0;

	// we assume its off
	regions[r].intensity = brightest;

	for(int l = 0; l < numLights; l++) {
		// set the brightest light
		if( lights[l].region == r & lights[l].intensity > brightest ) {
			brightest = lights[l].intensity;
			regions[r].intensity = brightest;
		}
	}

	// send the update
	regionInfo(r);
}

void Switcher::updateRegions() {
	for(int r = 0; r < numRegions; r++) {
		updateRegion(r);
	}
}

// output switch info

void Switcher::regionInfo( int r ) {
	char message[100];
	int numRegionLights = 0;
	bool hasFav = false;

	// always check all possible lights
	for(int i=0; i < NUM_LIGHTS; i++) {
		if(lights[i].region == r) {
			numRegionLights = numRegionLights + 1;

			if(lights[i].fav)
				hasFav = true;
		}
	}

	regions[r].hasFav = hasFav;

	sprintf(message,"API|regionInfo|region:%d|x:%d|y:%d|intensity:%d|lights:%d|fav:%d|hasFav:%d|irCode:%10X|", r, regions[r].x, regions[r].y,regions[r].intensity, numRegionLights, regions[r].fav, regions[r].hasFav, regions[r].irCode);
	Serial.println(message);
}

void Switcher::regionsInfo() {
	for(int i = 0; i < numRegions; i++) {
		regionInfo(i);
	}
}

void Switcher::lightInfo( int i ) {
	char message[60];
	sprintf(message,"API|lightInfo|light:%d|region:%d|intensity:%d|fav:%d|", i, lights[i].region, lights[i].intensity, lights[i].fav);
	Serial.println(message);
}

void Switcher::lightsInfo() {
	for(int i = 0; i < numLights; i++) {
		lightInfo(i);
	}
}


void Switcher::indicator(char message[], int color) {
	indicator(message, color, FULL_ON);
}


void Switcher::indicator(char message[], int color, int intensity) {
	// full on
	if( strlen(message) > 0 ) {
		analogWrite(indicatorPin, 255);
		Serial.print("INDICATOR : ");
		Serial.print(message);
		Serial.print(" : ");
		Serial.println(color);
	}

	// reset
	Tlc.set(13,0);
	Tlc.set(14,0);
	Tlc.set(15,0);


	if(states[color][0]) {
		Tlc.set(13, map(intensity, 0, FULL_ON, 0, states[color][0]));
	}

	if(states[color][1]) {
		Tlc.set(14, map(intensity, 0, FULL_ON, 0, states[color][1]));
	}

	if(states[color][2]) {
		Tlc.set(15, map(intensity, 0, FULL_ON, 0, states[color][2]));
	}

	Tlc.update();
}
