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

/*
 * 0 - Off
 * 1 - White
 * 2 - Red
 * 3 - Green
 * 4 - Blue
 * 5 - Yellow
 * 6 - Orange
 * 7 - Cian
 * 8 - Violet
 * 9 - Pink
 *
 * With rgb values
 * */
const int Switcher::states[10][3] = {
	{0    , 0    , 0   },
	{4095 , 4095 , 4095},
	{4095 , 0    , 0   },
	{0    , 4095 , 0   },
	{0    , 0    , 4095},
	{4095 , 4095 , 0   },
	{4095 , 1500 , 0   },
	{0    , 4095 , 4095},
	{2500 , 0    , 4095},
	{4095 , 0    , 4095},
};

void Switcher::toggle(int l) {
	if(l >= NUM_LIGHTS)
		return;

	Serial.println("TOGGLE");
	if((this->pinData & (1 << l)) == 0) {
		this->turnOn(l);
	}
	else {
		this->turnOff(l);
	}
}

void Switcher::turnOn(int l) {
	Serial.println("TURN ON");
	dim(l,FULL_ON);
}

void Switcher::turnOff(int l) {
	dim(l,OFF);
}

void Switcher::favOff() {
	for(int l=0; l < this->numLights; l++) {
		if(this->lights[l].fav) {
			this->dim(l,OFF);
		}
	}
}

void Switcher::favOn() {
	for(int l=0; l < this->numLights; l++) {
		if(this->lights[l].fav) {
			this->dim(l,FULL_ON);
		}
	}
}

void Switcher::allOn() {
	this->setAll(FULL_ON);
}

void Switcher::allOff() {
	this->clear();
}

void Switcher::setAll(int intensity) {
	for(int l=0; l < this->numLights; l++) {
		this->dim(l,intensity);
	}
}

void Switcher::clear() {
	//Tlc.clear();
	for(int l=0; l < NUM_LIGHTS; l++) {
		dim(l,OFF);
	}
	// turn off all 16bits
	this->pinData = 0x00;
}

void Switcher::toggleRegion(int r) {
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			toggle(l);
	}
}

void Switcher::turnOffRegion(int r) {
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			turnOff(l);
	}
}

void Switcher::turnOnRegion(int r) {
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			turnOn(l);
	}
}

void Switcher::dimRegion(int r, int intensity) {
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			dim(l,intensity);
	}
}

void Switcher::dimRegion(int r, int intensity, unsigned dur) {
	for(int l=0; l < numLights; l++) {
		if(lights[l].region == r)
			dim(l,intensity,dur);
	}
}

void Switcher::dim(int l, int intensity) {
	dim(l,intensity,0);
}

void Switcher::dim(int l, int intensity, unsigned dur) {
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
		lights[l].on = false;
	}
	else {
		this->pinData |= (1u << l);
		lights[l].on = true;
	}
}

void Switcher::dimmer(int r) {
	// this is a continues action so we need another loop

	Serial.println("DIMMER START");

	bool dimmerActive = true;
	Gest.reset();

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

		if(Gest.gestures == "TT") {
			indicator("dimmer while TT", 3);
			delay(Gest.tapDelay);
			dimmerActive = false;
		}
/*
		*/

		else if(Screen.active & Gest.gestures !="" & (timeDiff > Gest.tapDelay)) {
			indicator("dimmer while 1", 1, pwm[level]);
			this->dimRegion(r, pwm[level]);
		}
/*
else if (!Screen.active) {
			dimmerActive = false;
		}
*/

		Tlc.update();
	}

        Gest.reset();
	Serial.println("DIMMER STOP");
	indicator("dimmer stop", 0);
}

void Switcher::animate(anim adata) {
	for(int a = 0; a < 3; a++) {
		if(!animBuffer[a].active) {
			animBuffer[a] = adata;
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
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "Rdr" | Gest.gestures == "HRdr") {
				adata.dur = clockInterval * 2;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrD" | Gest.gestures == "HRdrD") {
				adata.dur = clockInterval * 3;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrD" | Gest.gestures == "HRdrD") {
				adata.dur = clockInterval * 4;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdl" | Gest.gestures == "HRdrDdl") {
				adata.dur = clockInterval * 5;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlL" | Gest.gestures == "HRdrDdlL") {
				adata.dur = clockInterval * 6;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLul" | Gest.gestures == "HRdrDdlLul") {
				adata.dur = clockInterval * 7;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLulU" | Gest.gestures == "HRdrDdlLulU") {
				adata.dur = clockInterval * 8;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLulUur" | Gest.gestures == "HRdrDdlLulUur") {
				adata.dur = clockInterval * 9;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "RdrDdlLulUurR" | Gest.gestures == "HRdrDdlLulUurR") {
				adata.dur = clockInterval * 10;
				Serial.println(adata.dur/1000);
				Gest.reset();
			}

			if(Gest.gestures == "TT") {
				indicator("tt", 3);
				durActive = false;
				delay(100);
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
				Serial.println("FADE IN");
				Gest.reset();
			}

			if(Gest.gestures == "HL" | Gest.gestures == "L") {
				adata.fade = true;
				Serial.println("FADE OUT");
				Gest.reset();
			}

			if(Gest.gestures == "TT") {
				indicator("tt fade", 3);
				fadeActive = false;
				delay(100);
				Gest.reset();
			}
		}
	}

	// convert to seconds
	//Serial.println(dimDuration);
	//dimDuration = (dimDuration * 1000);

	animate(adata);
	Serial.println("SLEEP STOP");
	indicator("sleep stop", 0);
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

// this has it's own loop so take care of updates and resets
void Switcher::setup() {
	bool setupDone = false;

	Serial.print("SETUP START");

	indicator("setup start", 2);

	this->setupReset();

	// ignore any previous gesture, setup has it's own loop
	Gest.reset();

	this->clear();
	this->toggle(this->numLights);

	while(!setupDone) {
		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);
		unsigned currentTime = millis();

		if( (this->numLights >= NUM_LIGHTS) | (Gest.gestures == "H") & ((currentTime - Gest.startTime) > 3000) ) {
			indicator("setup H", 3);

			if(!Gest.active)
				setupDone = true;
		}
		else if( ! Gest.active & (Gest.gestures == "T") ) {

			int r = this->getRegion(Gest.endX, Gest.endY, false);

			Serial.print("got region: ");
			Serial.println(r);

			if( r < NUM_LIGHTS ) {
				// region has already been defined so add the new light
				// to existing region
				this->lights[this->numLights].region = r;
			}
			else {
				// if region has not been found create new
				regions[this->numRegions].x = Gest.endX;
				regions[this->numRegions].y = Gest.endY;
				// if none of the previous
				this->lights[this->numLights].region = this->numRegions;

				this->numRegions++;
			}

			// go to next light
			this->numLights++;
			this->clear();
			this->toggle(this->numLights);

			Gest.reset();
		}
		else if( ! Gest.active & (Gest.gestures == "HL") ) {
			// save light as one of fav
			Serial.println("Marked as fav");
			this->lights[this->numLights].fav = true;

			// turn the bit on
			//this->favData |= 1 << this->numLights;

			Gest.reset();
		}
		else if( ! Gest.active & (Gest.gestures == "HR") ) {
			// don't save the light region, got to the next
			// this will make the light unaccessable
			this->numLights++;
			this->clear();
			this->toggle(this->numLights);

			Gest.reset();
		}

		Tlc.update();
	}

	Serial.print("Number of lights: ");
	Serial.println(this->numLights);

	Serial.print("Number of regions: ");
	Serial.println(this->numRegions);

	this->clear();

	Serial.println("SETUP DONE");
	indicator("setup done", 0);
	Gest.reset();
}

void Switcher::setupReset() {
	this->numLights = 0;
	this->numRegions = 0;

	this->pinData = 0;

	for(int l = 0; l < NUM_LIGHTS; l++) {
		this->lights[l].region = NUM_LIGHTS;
		this->lights[l].intensity = OFF;
		this->lights[l].fav = false;
		this->regions[l].x = 0;
		this->regions[l].y = 0;
	}
}


// output switch info
void Switcher::info() {
	for(int i = 0; i < numRegions; i++) {
		// notify the network of the new region
		char message[30];
		sprintf(message,"region:%d,x:%d,y:%d;", i, regions[i].x, regions[i].y);
		Serial.println(message);
	}

	for(int i = 0; i < numLights; i++) {
		// notify the network of the new light
		char message[30];
		sprintf(message,"light:%d,r:%d,i:%d,f:%d,o:%d;", i, lights[i].region, lights[i].intensity, lights[i].fav,lights[i].on);
		Serial.println(message);
	}
}

/*
 * 0 - Red    - Setup
 * 1 - Green  - Set
 * 2 - Blue   -
 * 3 - White  - Dimmer
 * 4 - Yellow - Light selected
 * 5 - Orange -
 * 6 - Cian   - IR
 * 7 - Pink   -
 * 8 - Violet -
 * */
void Switcher::indicator(char message[], int color) {
	indicator(message, color, FULL_ON);
}

void Switcher::indicator(char message[], int color, int intensity) {
	// full on
	analogWrite(indicatorPin, 255);
	Serial.println(message);

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



