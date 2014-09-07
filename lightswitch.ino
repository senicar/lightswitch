// references for the code in this file

#define DEBUG false

#include "Tlc5940.h"
//#include "math.h"
//#include "Timer.h"


/*#include "Functions.h"*/

/*#include "API.h"*/


#include "TouchDS.h"
#include "Gestures.h"
#include "Switcher.h"
#include "IRremote.h"

/* TLC needs two timers and IR needs one, IR cannot be used with Arduino Uno since
 * it has only one timer. Use teensy 2.0++, arduino mega or other arduino compatible
 * board that has three timers. If IR is not needed all reference to it should be removed
 *
 * IMPORTANT: set the right timer in IRRemote library since by default it uses
 * TIMER2, same as TLC
 */

IRrecv irrecv(16);
decode_results ir_results;


/* -------------------------------------------------------*
** PINS
** -------------------------------------------------------*/

// Analog inputs for touch screen
int y1 = A0;
int x2 = A1;
int y2 = A2;
int x1 = A3;


/* The lightswitch circuit has two dip switches and two buttons avialable
 * for testing and changing modes
 *
 * NOTICE : dip1 disables and disables the led 1 on the prototype in favor
 * of external light connected to Velleman K8064 dimmer it. Since led takes
 * some of the power that is better used for dimmer voltage regulation.
 *
 * dip1 switch ON => led enabled, dimmer disabled
 * dip1 switch OFF => led disabled, dimmer enabled
 *
 * dip1 is not connected to the microcontroller
 */

int dip2 = 5;
int button1 = 7;
int button2 = 8;


/* This pin outputs power to all RGB leds that are used as indicators,
 * they are connected to TCL on the last three channels (13,14,15)
 *
 * It is a PWM pin
 */

int indicatorPin = 6;


// Light Sensor Pin
int lightSensorPin = A4;


/* -------------------------------------------------------*
** GLOBAL VARIABLES
** -------------------------------------------------------*/

// how long to hold screen to enter setup mode (millis)
int setupHoldTreshold = 3000;

// default light treshold, we can calibrate it based on this value
int lightSensorTreshold = 200;

// this is the amount of light value change that is needed to trigger action
// the bigger the treshold the bigger the brighter the environment has to be
int lightSensorPreviousTreshold = 30;

// this is the value that was registered the last time light sensor was updated
// light sensor loosly calibrates it self based on it
int lightSensorPreviousValue = 0;

// the last time light sensor triggered the action
unsigned long lightSensorLastTriggered = 0;

// gestures must be initiated with nullX and nullY values, this are the values
// the touch screen returns when it is not active
int nullX = 0;
int nullY = 0;


/* -------------------------------------------------------*
** NETWORK API : TODO : move to its own file
** -------------------------------------------------------*/

class Network {
	private:
	public:
		Switcher & Switch;
		Network(Switcher & switcher) : Switch(switcher) {
			message = "";
			light = 255;
			intensity = 0;
			complete = false;
		};
		String command;
		String message;
		int light;
		int intensity;
		bool complete;

		void update();
		void reset();
		bool process();
};


void Network::update() {
	String c = "";
	String l = "255";
	String i = "0";

	// see if there is any serial data being sent our way
	// for teensy 2.0++, used for communication with API
	if(Serial.available())
		serialEvent();

	if(complete) {
		c = message.substring(message.indexOf(";"));

		if(message.indexOf(":") > 0) {
			l = message.substring(message.indexOf(":")+1);
			c = message.substring(0,message.indexOf(":"));
		}

		if(l.indexOf(":") > 0) {
			i = l.substring(l.indexOf(":")+1);
			l = l.substring(0,l.indexOf(":"));
		}

		if(i.indexOf(":") > 0) {
			i = i.substring(0,i.indexOf(":"));
		}

		command = c;
		light = l.toInt();
		intensity = i.toInt();

		Serial.print("command: ");
		Serial.println(command);

		Serial.print("light: ");
		Serial.println(light);

		Serial.print("intesity: ");
		Serial.println(intensity);

		Serial.print("Full message: ");
		Serial.println(message);
	}
}


/* This processes all the available actions
 * All actions sent are in format:
 *
 *     command:light:intensity;
 *     dim:4:100;
 *
 *
 * this structure is used even if light or intensity is not needed
 * for example:
 *
 *     allOn::;
 */

bool Network::process() {
	if(!complete) return false;

	if(command == "dim") {
		Switch.dim(light,intensity,0);
		return true;
	}

	if(command == "toggle") {
		Switch.toggle(light);
		return true;
	}

	if(command == "turnOn") {
		Switch.turnOn(light);
		return true;
	}

	if(command == "turnOff") {
		Switch.turnOff(light);
		return true;
	}

	if(command == "favOn") {
		Switch.favOn();
		return true;
	}

	if(command == "favOff") {
		Switch.favOff();
		return true;
	}

	if(command == "allOn") {
		Switch.allOn();
		return true;
	}

	if(command == "allOff") {
		Serial.println("command all off");
		Switch.allOff();
		return true;
	}

	if(command == "indicator") {
		Switch.indicator("network indicator", light);
		return true;
	}

	if(command == "regionsInfo") {
		Switch.regionsInfo();
		return true;
	}

	if(command == "lightsInfo") {
		Switch.lightsInfo();
		return true;
	}

	if(command == "info") {
		Serial.print("freeRam: ");
		Serial.println(freeRam());
		Switch.regionsInfo();
		Switch.lightsInfo();
		return true;
	}

	return false;
}


void Network::reset() {
	message = "";
	complete = false;
}


/* -------------------------------------------------------*
** INIT INTERFACES
** -------------------------------------------------------*/

// Initiate interfaces
TouchDS Screen (x1,x2,y1,y2);
Gestures Gest (nullX,nullY);
Switcher Switch(Screen,Gest,Tlc);
Network Net(Switch);


/* -------------------------------------------------------*
** SETUP
** -------------------------------------------------------*/

void setup()
{
	// setup the serial communication used by Serial Monitor and API
	Serial.begin(115200);

	// the delay used to indentify a tap gesture in millis
	//
	// every tap that occurs within tapDelay after the last tap is added
	// to the total number of taps. When tapDelay passes and no other tap has
	// been detected, tap counting stops
	//
	// it is also used as delay after confirmations and such, this is the time
	// of complete inactiviy
	Gest.tapDelay = 300;

	// dragTolerance is distance from previous point that happened
	// in Gest.updateInterval time the bigger the screen resolution
	// the bigger dragtolerance should be
	Gest.dragTolerance = 8000;

	// Set the indicator pin for Switch interface
	Switch.indicatorPin = indicatorPin;

	// Pin modes
	pinMode(indicatorPin, OUTPUT);
	pinMode(dip2, INPUT);
	pinMode(button1, INPUT);
	pinMode(button2, INPUT);

	// Set default light setup
	if(NUM_LIGHTS == 16)
		defaultLightsGrid16();
	if(NUM_LIGHTS == 12)
		defaultLightsGrid12();

	// Initialize TLC library
	Tlc.init();

	// Initialize IR library
	irrecv.enableIRIn();
}


/* -------------------------------------------------------*
** MAIN LOOP
** -------------------------------------------------------*/

void loop()
{
	bool actionTriggered = false;

	/* -------------------------------------------------------*
	** DEBUG
	** -------------------------------------------------------*/

	// Calibrate light sensor
	if(digitalRead(button1) == HIGH) {
		lightSensorCalibrate();
	}

	// This is a debug settings to test if all lights are working
	if(digitalRead(button2) == HIGH) {

		int direction = 1;

		analogWrite(indicatorPin,255);

		for (int channel = 0; channel < 16; channel += direction) {

			Tlc.clear();
			if (channel == 0) {
				/*direction = 1;*/
			} else {
				Tlc.set(channel - 1, 1000);
			}
			Tlc.set(channel, 4095);
			if (channel != 16 - 1) {
				Tlc.set(channel + 1, 1000);
			} else {
				/*direction = -1;*/
			}

			Tlc.update();
			delay(Gest.tapDelay);
		}

		analogWrite(indicatorPin,0);
	}


	/* -------------------------------------------------------*
	** API & SENSORS
	** -------------------------------------------------------*/

	// Decode any IR signal received and act on it if recognized
	if(irrecv.decode(&ir_results) & ! actionTriggered) {
		Switch.indicator("IR RECEIVED", 7);
		actionTriggered = remoteControl(&ir_results);
		delay(Gest.tapDelay);
		Switch.indicator("", 0);
	}

	// API
	Net.update();
	if( ! actionTriggered ) actionTriggered = Net.process();
	Net.reset();

	// check light sensor
	if( ! actionTriggered ) actionTriggered = lightSensorUpdate();


	/* -------------------------------------------------------*
	** UPDATE SWITCH STATE
	** -------------------------------------------------------*/

	// in case any timmer function was activated (sleep...)
	Switch.update();


	/* -------------------------------------------------------*
	** Start recognizing gestures and screen activity
	** -------------------------------------------------------*/

	// update the the touch screen state and coordinates
	Screen.update();

	// If screen is active it updates the Gestures intarface
	Gest.update(Screen.touchX, Screen.touchY, Screen.active);

	// used with gestures to determine what action should be preformed
	// this values must be set after Gest.update, Screen.update else
	// timeDiff will not be correct
	//
	// Notice: we should not forget about tapDelay
	unsigned currentTime = millis();
	unsigned timeDiff = currentTime - Gest.endTime;


	/* -------------------------------------------------------*
	** GESTURE HOLD / SPECIAL ACTIONS
	** -------------------------------------------------------*/

	if( (Gest.gestures.indexOf("H") == 0) ) {

		// set region before moving
		int r = Switch.getRegion(Gest.startX, Gest.startY);

		if (Gest.gestures == "H" & ((currentTime - Gest.startTime) > setupHoldTreshold )) {
			Switch.indicator("SETUP START", 2);
			if( ! Gest.active )
				switchSetup();
		}
	}


	/* -------------------------------------------------------*
	** MAIN GESTURE ACTIONS
	** -------------------------------------------------------*/

	if( ! Gest.active & (Gest.gestures !="") & (timeDiff > Gest.tapDelay) && ! actionTriggered ) {

		bool gestureRecognized = false;

		// Turn light on/off

		if( Gest.gestures == "T" ) {
			int r = Switch.getRegion(Gest.endX, Gest.endY);
			Serial.print("Selected region: ");
			Serial.println(r);
			Switch.toggleRegion(r);
			gestureRecognized = true;
		}

		// Select region and perform action on it

		if( Gest.gestures == "TT" ) {
			int r = Switch.getRegion(Gest.endX, Gest.endY);
			for(int l=0; l < Switch.numLights; l++) {
				if(Switch.lights[l].region == r & Switch.lights[l].intensity) {
					int r = Switch.getRegion(Gest.endX, Gest.endY);
					regionActions( r );
				}
			}

			// even if regionActions was not activated due to region not
			// being actuve we have recognized the gesture
			gestureRecognized = true;
		}

		// EXTRA Actions

		/*if(digitalRead(button2) == HIGH) {*/
			// Sleep on region
			/*if( Gest.gestures == "TTT" ) {*/
				/*int r = Switch.getRegion(Gest.endX, Gest.endY);*/
				/*Switch.animSetRegion(r);*/
				/*gestureRecognized = true;*/
			/*}*/

			/*if( Gest.gestures == "HUD" ) {*/
				/*systemInfo();*/
			/*}*/

			/*if( Gest.gestures == "HDL" | Gest.gestures == "DL" ) {*/
				/*Serial.println("HDL");*/
				/*Switch.clear();*/
				/*gestureRecognized = true;*/
			/*}*/

			/*if( Gest.gestures == "HUR" | Gest.gestures == "UR" ) {*/
				/*Switch.allOn();*/
				/*gestureRecognized = true;*/
			/*}*/

			/*if( Gest.gestures == "HLDR" | Gest.gestures == "LDR" ) {*/
				/*lightSensorCalibrate();*/
				/*gestureRecognized = true;*/
			/*}*/

			/*if( Gest.gestures == "HDRUL" | Gest.gestures == "DRUL" ) {*/
				/*Serial.println("RGB indi START");*/
				/*rgbIndi();*/
				/*gestureRecognized = true;*/
			/*}*/
		/*}*/

		// Toggle default gesture

		if(
			! gestureRecognized ||
			Gest.gestures == "H"
			) {

			Serial.println("Trigger : favToggle");
			Switch.favToggle();
		}

		// gesture have been processed and can be reseted
		Gest.reset();
	}

	Tlc.update();
}


/* -------------------------------------------------------*
** INDIVIDUAL REGION / LIGHT ACTIONS
** -------------------------------------------------------*/

bool regionActions( int region ) {
	// start clean
	Gest.reset();

	Switch.indicator("REGION ACTIONS", 9);

	// in case we set a timmer function
	int clockInterval = 6600;
	Switcher::anim adata;
	adata.dur = 0;
	adata.region = region;
	adata.fade = false;

	while( true ) {

		Switch.indicator("", 9);

		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);
		unsigned currentTime = millis();
		unsigned timeDiff = currentTime - Gest.endTime;

		if( ! Gest.active & (Gest.gestures !="") & (timeDiff > Gest.tapDelay) ) {

			// confirm region modifications
			if( Gest.gestures == "TT" ) {
				if( adata.dur > 0 ) {
					Switch.animate( adata );
				}

				Gest.reset();
				Switch.indicator("END REGION ACTIONS", 3);
				delay(Gest.tapDelay);
				Switch.indicator("",0);
				return true;
			}

			// add sleep time, writing recognition is not fully implemented so
			// it is not used, but there is a sample in Switcher::animSetRegion()
			if( Gest.gestures == "T" ) {
				Switch.indicator("ADD SLEEP TIME", 6);

				adata.dur = adata.dur + clockInterval;

				Serial.print("NEW TIME : ");
				Serial.println(adata.dur/1000);
				delay(Gest.tapDelay);
				Gest.reset();
			}

			if(Gest.gestures == "TTT") {
				adata.fade = true;
				Switch.indicator("FADE", 8);
				delay(Gest.tapDelay);
				Gest.reset();
			}

		}

		// if screen is active and none of gestures have been
		// recognized we assume user is trying to dim
		if( Gest.active & (Gest.gestures !="") & (timeDiff > Gest.tapDelay) ) {
			Serial.println("Trigger : dimmer");
			Switch.dimmer(region);
		}

		// even in region action we should update switch
		Switch.update();

	} // while

	Gest.reset();
	return false;
}


/* -------------------------------------------------------*
** LIGHT SENSOR - update & calibrate
** -------------------------------------------------------*/

/* Update light sensor, toggle default action if considerable change is detected
 */

bool lightSensorUpdate() {
	int data = analogRead(lightSensorPin);

	if( abs(data - lightSensorPreviousValue) >= lightSensorPreviousTreshold
		& (millis() - Switch.lastTimeSwitched) > (Gest.tapDelay * 3)
		) {

		lightSensorLastTriggered = millis();

		if( Switch.pinData > 0 ) {
			Switch.clear();
		}
		else {
			Switch.favOn();
		}

		Serial.print("light sensor previous : ");
		Serial.println(lightSensorPreviousValue);
		Serial.print("light sensor update   : ");
		Serial.println(data);

		lightSensorPreviousValue = data;

		return true;
	}

	// self-calibration
	if((millis() - lightSensorLastTriggered) > Gest.tapDelay) {
		lightSensorPreviousValue = data;
	}

	return false;
}

/* Even though light sensor is can self calibrate to certain extend it is good
 * to calibrate the sensor sensitivity by hand on first run
 */

void lightSensorCalibrate() {
	Serial.println("LIGHT SENSOR calibrate START");
	bool calibrate = true;

	while(calibrate) {

		int data = analogRead(lightSensorPin);
		Serial.print("light sensor: ");
		Serial.println(data);

		if(data > lightSensorTreshold) {
			Tlc.setAll(Switch.FULL_ON);
		}
		else {
			Tlc.clear();
		}

		Tlc.update();

		delay(Gest.tapDelay);

		if(digitalRead(button1) == HIGH) {
			calibrate = false;
		}
	}

	delay(Gest.tapDelay);

	Tlc.clear();
	Tlc.update();
	Serial.println("LIGHT SENSOR calibrate STOP");
}


/* -------------------------------------------------------*
** SERIAL EVENT listener
** -------------------------------------------------------*/

/* SerialEvent occurs whenever a new data comes in the
 * hardware serial RX.  This routine is run between each
 * time loop() runs, so using delay inside loop can delay
 * response.  Multiple bytes of data may be available.
 */

void serialEvent() {
	while (Serial.available()) {
		// get the new byte:
		char inChar = (char)Serial.read();

		Net.message += inChar;

		if (inChar == ';')
			Net.complete = true;
	}
}


/* -------------------------------------------------------*
** RGB CALIBRATION
** -------------------------------------------------------*/

void rgbIndi() {
	Serial.println("RGB indi START");
	bool calibrate = true;
	Switch.clear();

	int rgb = 13;

	Switch.turnOn(13);
	Switch.turnOn(14);
	Switch.turnOn(15);

	while(calibrate) {
		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		int level = map(Screen.touchX, 150, 854, 0, 15);
		level = constrain(level, 0, 15);
		unsigned timeDiff = millis();

		if(Serial.available() > 0) {
			int intensity = Serial.parseInt();

			if(rgb == 13)
				Serial.print("R: ");
			else if(rgb == 14)
				Serial.print("G: ");
			else if(rgb == 15)
				Serial.print("B: ");

			Serial.println(intensity);

			Tlc.set(rgb, intensity);
		}

		Tlc.update();
	}

	Tlc.clear();
	Tlc.update();

	Gest.reset();

	Serial.println("RGB indi STOP");
	/*
	*/
}


/* -------------------------------------------------------*
** DEFAULT LIGHT GRIDS
** -------------------------------------------------------*/

void defaultLightsGrid(int rows=4, int columns=4) {
	int rx = Screen.resolutionX / columns;
	int ry = Screen.resolutionY / rows;

	int light = 0;

	int py = ry/2;
	for(int r = rows; r > 0; r--) {
		int px = rx/2;
		for(int c = columns; c > 0; c--) {
			Serial.print("set light number: ");
			Serial.println(light);

			Switch.regions[light].x = px;
			Switch.regions[light].y = py;
			Switch.lights[light].region = light;

			px += rx;

			if(light < NUM_LIGHTS)
				light++;
		}

		py += ry;
	}

	Switch.lights[5].fav = true;
	Switch.lights[6].fav = true;

	if( rows == 4 ) {
		Switch.lights[9].fav = true;
		Switch.lights[10].fav = true;
	}

	Switch.numLights = NUM_LIGHTS;
	Switch.numRegions = NUM_LIGHTS;
}

void defaultLightsGrid12() {
	defaultLightsGrid(3,4);
}

void defaultLightsGrid16() {
	defaultLightsGrid(4,4);
}


/* -------------------------------------------------------*
** IR REMOTE CONTROL ACTIONS
** -------------------------------------------------------*/

bool remoteControl(decode_results *ir_results) {
	unsigned long code = ir_results->value;
	Serial.print("IR CODE : ");
	Serial.println(code, HEX);

	bool codeRecognized = false;

	Serial.print("FAV CODE: ");
	Serial.println(Switch.irFavCode, HEX);

	Serial.print("TOGGLE CODE: ");
	Serial.println(Switch.irToggleCode, HEX);

	if( code == Switch.irFavCode) {
		Serial.println("FAV TOGGLE");
		Switch.favToggle();
		codeRecognized = true;
	}
	else if( code == Switch.irToggleCode ) {
		Serial.println("ALL TOGGLE");
		if(Switch.pinData > 0)
			Switch.allOff();
		else
			Switch.allOn();

		codeRecognized = true;
	}
	else if( ! codeRecognized ) {
		Serial.println("LOOP REGIONS");

		// Loop all regions for possible match
		for( int r=0; r < Switch.numRegions; r++ ) {
			Serial.print("REGION CODE: ");
			Serial.println(Switch.regions[r].irCode, HEX);

			if(Switch.regions[r].irCode == code) {
				Serial.println("TOGGLE REGIONS");
				Switch.toggleRegion(r);
				codeRecognized = true;
			}
		}
	}

	if( codeRecognized | Switch.irFavCode | Switch.irToggleCode ) {
		irrecv.resume();
		return codeRecognized;
	}

	// if nothing else go to defaults
	Serial.println("NOTHING ELSE MATTERS");

	switch (code) {
		case 0xFFFFFFFF:
			// this is null
			break;
		case 0xFFE01F:
			// dim low
			for(int l=0; l < Switch.numLights; l++) {
				if(Switch.lights[l].pwm > 0) {
					Switch.dim(l,Switch.lights[l].pwm - 1, 0);
					Serial.println("dim low");
				}
			}
			codeRecognized = true;
			break;
		case 0xFFA857:
			// dim high
			for(int l=0; l < Switch.numLights; l++) {
				if(Switch.lights[l].pwm < 15) {
					Switch.dim(l,Switch.lights[l].pwm + 1, 0);
					Serial.println("dim high");
				}
			}
			codeRecognized = true;
			break;
		case 0xFFE21D:
			// 1
			Switch.allOn();
			codeRecognized = true;
			break;
		case 0xFFA25D:
			// 1
			Serial.println("ir all off");
			Switch.allOff();
			codeRecognized = true;
			break;
		case 0xFF30CF:
			// 1
			Switch.toggleRegion(0);
			codeRecognized = true;
			break;
		case 0xFF18E7:
			// 2
			Switch.toggleRegion(1);
			codeRecognized = true;
			break;
		case 0xFF7A85:
			// 3
			Switch.toggleRegion(2);
			codeRecognized = true;
			break;
		case 0xFF10EF:
			// 4
			Switch.toggleRegion(3);
			codeRecognized = true;
			break;
		case 0xFF38C7:
			// 5
			Switch.toggleRegion(4);
			codeRecognized = true;
			break;
		case 0xFF5AA5:
			// 6
			Switch.toggleRegion(5);
			codeRecognized = true;
			break;
		case 0xFF42BD:
			// 7
			Switch.toggleRegion(6);
			codeRecognized = true;
			break;
		case 0xFF4AB5:
			// 8
			Switch.toggleRegion(7);
			codeRecognized = true;
			break;
		case 0xFF52AD:
			// 9
			Switch.toggleRegion(8);
			codeRecognized = true;
			break;
		case 0xFF6897:
			// 10
			Switch.toggleRegion(9);
			codeRecognized = true;
			break;
		case 0xFF9867:
			// 11
			Switch.toggleRegion(10);
			codeRecognized = true;
			break;
		case 0xFFB04F:
			// 12
			Switch.toggleRegion(11);
			codeRecognized = true;
			break;
		default:
			Serial.println("IR code not recognized");
			break;
	} // switch

	irrecv.resume();
	return codeRecognized;
}


/* -------------------------------------------------------*
** HELPFUL FUNCTIONS
** -------------------------------------------------------*/

int freeRam () {
	extern int __heap_start, *__brkval;
	int v;
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}


uint32_t debug_time = 0;
uint32_t debug_start_time = 0;

void debug_start() {
	Serial.println("Debug start");
	debug_start_time = millis();
}

void debug_end() {
	Serial.print("Debug time: ");
	Serial.println(millis() - debug_start_time);
}

/* -------------------------------------------------------*
** SWITCH SETUP
** -------------------------------------------------------*/

// this has it's own loop so take care of updates and resets
void switchSetup() {
	bool setupLights = false;

	switchSetupReset();

	// ignore any previous gesture, setup has it's own loop
	Gest.reset();

	Switch.clear();
	Switch.toggle(Switch.numLights);

	/* -------------------------------------------------------*
	**  Setup individual light, set favs, regions, ir code ...
	** -------------------------------------------------------*/

	while(!setupLights) {
		Switch.indicator("", 2);

		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		unsigned currentTime = millis();
		unsigned timeDiff = currentTime - Gest.endTime;

		// IR code is set by simple clicking a button on an IR remote control

		if(irrecv.decode(&ir_results) & ir_results.value < 4294967295) {
			Serial.println(ir_results.value, HEX);

			Switch.regions[ Switch.numRegions ].irCode = ir_results.value;

			Switch.indicator("Setup : ir code set", 7);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);

			irrecv.resume();
		}
		else if( (Switch.numLights >= NUM_LIGHTS) | ( (Gest.gestures == "H") & ((currentTime - Gest.startTime) > setupHoldTreshold) ) ) {

			Switch.indicator("Setup : lights done", 3);
			if( ! Gest.active ) {
				setupLights = true;
				Gest.reset();
				irrecv.resume();
			}
			delay(Gest.tapDelay);
			Switch.indicator("", 0);
		}
		else if( ! Gest.active & (Gest.gestures == "TT") & (timeDiff > Gest.tapDelay) ) {
			// don't save the light region, got to the next
			// this will make the light unaccessable
			Switch.numLights++;
			Switch.clear();
			Switch.toggle(Switch.numLights);

			Gest.reset();

			Switch.indicator("Setup : light set", 3);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);
		}
		else if( ! Gest.active & (Gest.gestures == "T") & (timeDiff > Gest.tapDelay) ) {
			int r = Switch.getRegion(Gest.endX, Gest.endY, false);

			// number of regions is always smaller or the same as number of lights
			// if getRegion returnes r smaller than number of lights we can assume
			// the region has already been assigned to previous light, so we can
			// simply add them together
			//
			// else create a new region and assign it to the light
			if( r < Switch.numLights ) {
				Switch.lights[Switch.numLights].region = r;
			}
			else {
				Switch.regions[Switch.numRegions].x = Gest.endX;
				Switch.regions[Switch.numRegions].y = Gest.endY;
				Switch.lights[Switch.numLights].region = Switch.numRegions;

				Switch.numRegions++;
			}

			Switch.indicator("Setup : region set", 1);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);

			Gest.reset();
		}
		else if( ! Gest.active & (Gest.gestures == "HL" | Gest.gestures == "L") & (timeDiff > Gest.tapDelay) ) {
			// save light as one of fav
			Switch.lights[Switch.numLights].fav = true;
			Gest.reset();

			Switch.indicator("Setup : marked as FAV", 5);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);

			// turn the bit on
			//Switch.favData |= 1 << Switch.numLights;
		}

		Tlc.update();
	}


	/* -------------------------------------------------------*
	** Set IR code for fav toggle
	** -------------------------------------------------------*/

	bool setupFavIR = false;
	Switch.clear();
	Switch.favOn();
	Tlc.update();
	Switch.indicator("SETUP FAV IR CODE ", 2);
	while(!setupFavIR) {
		Switch.indicator("", 2);

		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		unsigned currentTime = millis();
		unsigned timeDiff = currentTime - Gest.endTime;

		if(irrecv.decode(&ir_results) & ir_results.value < 4294967295) {
			Serial.println(ir_results.value, HEX);

			Switch.irFavCode = ir_results.value;

			Switch.indicator("FAV IR CODE SAVED", 7);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);

			irrecv.resume();
		}

		if( ! Gest.active & (Gest.gestures == "TT") & (timeDiff > Gest.tapDelay) ) {
			setupFavIR = true;
			Gest.reset();

			Switch.indicator("Setup : fav IR set", 3);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);
		}
	}


	/* -------------------------------------------------------*
	** Set IR code for all toggle
	** -------------------------------------------------------*/

	bool setupToggleIR = false;
	Switch.clear();
	Switch.allOn();
	Tlc.update();
	Switch.indicator("SETUP TOGGLE IR CODE ", 2);
	while(!setupToggleIR) {
		Switch.indicator("", 2);

		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		unsigned currentTime = millis();
		unsigned timeDiff = currentTime - Gest.endTime;

		if(irrecv.decode(&ir_results) & ir_results.value < 4294967295) {
			Serial.println(ir_results.value, HEX);

			Switch.irToggleCode = ir_results.value;

			Switch.indicator("TOGGLE IR CODE SAVED", 7);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);

			irrecv.resume();
		}

		if( ! Gest.active & (Gest.gestures == "TT") & (timeDiff > Gest.tapDelay) ) {
			setupToggleIR = true;
			Gest.reset();

			Switch.indicator("Setup : toggle IR set", 3);
			delay(Gest.tapDelay);
			Switch.indicator("", 0);
		}
	}

	Serial.println(Switch.irFavCode, HEX);

	// WE ARE DONE

	Serial.print("Setup : Number of lights : ");
	Serial.println(Switch.numLights);
	Switch.regionsInfo();

	Serial.print("Setup : Number of regions : ");
	Serial.println(Switch.numRegions);
	Switch.lightsInfo();

	Switch.clear();

	Switch.indicator("SETUP DONE", 3);
	delay(Gest.tapDelay);
	Switch.indicator("", 0);

	Gest.reset();
}

void switchSetupReset() {
	Switch.numLights = 0;
	Switch.numRegions = 0;

	Switch.pinData = 0;
	Switch.irToggleCode = 0;
	Switch.irFavCode = 0;

	for(int l = 0; l < NUM_LIGHTS; l++) {
		Switch.lights[l].region = NUM_LIGHTS;
		Switch.lights[l].intensity = Switch.OFF;
		Switch.lights[l].fav = false;
		Switch.lights[l].pwm = 0;

		Switch.regions[l].x = 0;
		Switch.regions[l].y = 0;
		Switch.regions[l].fav = false;
		Switch.regions[l].intensity = 0;
		Switch.regions[l].pwm = 0;
		Switch.regions[l].irCode = 0;
	}
}
