// references for the code in this file

#include "Tlc5940.h"
//#include "math.h"
//#include "Timer.h"

#include "TouchDS.h"
#include "Gestures.h"
#include "Switcher.h"
#include "IRremote.h"

IRrecv irrecv(16);
decode_results results;

int y1 = A0;
int x2 = A1;
int y2 = A2;
int x1 = A3;

// this is for settings
int button1 = 4;
int button2 = 5;

int indicatorPin = 6;

int lightSensorPin = A4;
unsigned long lightActionTime = 0;

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
		void process();
};

void Network::update() {
	String c = "";
	String l = "255";
	String i = "0";

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
		Serial.print("message: ");
		Serial.println(message);
	}

	process();
}

void Network::process() {
	if(!complete) return;

	if(command == "dim")
		Switch.dim(light,intensity,0);

	if(command == "toggle")
		Switch.toggle(light);

	if(command == "turnOn")
		Switch.turnOn(light);

	if(command == "turnOff")
		Switch.turnOff(light);

	if(command == "favOn")
		Switch.favOn();
if(command == "favOff")
		Switch.favOff();

	if(command == "allOn")
		Switch.allOn();

	if(command == "allOff") {
		Serial.println("command all off");
		Switch.allOff();
	}

	if(command == "indicator") {
		Serial.println("indicator");
		Switch.indicator("network", light);
	}

	if(command == "info") {
		Serial.print("freeRam: ");
		Serial.println(freeRam());
		Switch.info();
	}
}

void Network::reset() {
	message = "";
	complete = false;
}


// gestures must be initiated with nullX and nullY values, this are the values
// the touch screen returns when it is not active
int nullX = 0;
int nullY = 0;

TouchDS Screen (x1,x2,y1,y2);
Gestures Gest (nullX,nullY);
Switcher Switch(Screen,Gest,Tlc);
Network Net(Switch);

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

void remoteControl(decode_results *results) {
	unsigned long code = results->value;
	Serial.println(code, HEX);

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
			break;
		case 0xFFA857:
			// dim high
			for(int l=0; l < Switch.numLights; l++) {
				if(Switch.lights[l].pwm < 15) {
					Switch.dim(l,Switch.lights[l].pwm + 1, 0);
					Serial.println("dim high");
				}
			}
			break;
		case 0xFFE21D:
			// 1
			Switch.allOn();
			break;
		case 0xFFA25D:
			// 1
			Serial.println("ir all off");
			Switch.allOff();
			break;
		case 0xFF30CF:
			// 1
			Switch.toggle(0);
			break;
		case 0xFF18E7:
			// 2
			Switch.toggle(1);
			break;
		case 0xFF7A85:
			// 3
			Switch.toggle(2);
			break;
		case 0xFF10EF:
			// 4
			Switch.toggle(3);
			break;
		case 0xFF38C7:
			// 5
			Switch.toggle(4);
			break;
		case 0xFF5AA5:
			// 6
			Switch.toggle(5);
			break;
		case 0xFF42BD:
			// 7
			Switch.toggle(6);
			break;
		case 0xFF4AB5:
			// 8
			Switch.toggle(7);
			break;
		case 0xFF52AD:
			// 9
			Switch.toggle(8);
			break;
		case 0xFF6897:
			// 10
			Switch.toggle(9);
			break;
		case 0xFF9867:
			// 11
			Switch.toggle(10);
			break;
		case 0xFFB04F:
			// 12
			Switch.toggle(11);
			break;
		default:
			Serial.println("IR code not recognized");
			break;
	}
	irrecv.resume();
}
/*
*/

void setup()
{
	Serial.begin(115200);
	Gest.tapDelay = 200;
	Gest.dragTolerance = 8000;

	Switch.indicatorPin = indicatorPin;
	pinMode(indicatorPin, OUTPUT);

	pinMode(button1, INPUT);
	pinMode(button2, INPUT);

	if(NUM_LIGHTS == 16)
		defaultLightsGrid16();
	if(NUM_LIGHTS == 12)
		defaultLightsGrid12();

	Tlc.init();

	irrecv.enableIRIn();
}

void loop()
{
	if(irrecv.decode(&results)) {
		remoteControl(&results);
		Serial.println("IR");
	}
	/*
	*/

	// update the the touch screen state and coordinates
	Screen.update();

	Net.update();
	Net.reset();

	Switch.update();

	// for teensy 2.0++
	if(Serial.available())
		serialEvent();

	// it update must receive the coordinates of the gesture and if the
	// surface from which the gestures are read is active, for example if
	// if finger is touching the touch screen
	if(digitalRead(button2) == HIGH)
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

	// check what gesture was entered but don't forget about tapDelay
	unsigned currentTime = millis();
	unsigned timeDiff = currentTime - Gest.endTime;

	// setup triggers
	// switch between them as soon as time passess
	// this is usefull so that lights can be activated when enough time passes
	if( (Gest.gestures.indexOf("H") == 0) ) {

		// set region before moving
		int r = Switch.getRegion(Gest.startX, Gest.startY);

		if (Gest.gestures == "H" & ((currentTime - Gest.startTime) > 3000)) {
			Switch.indicator("H", 2);
			if(!Gest.active)
				Switch.setup();
		}

		//if( Gest.gestures == "HL" | Gest.gestures == "HR") {
		//	Switch.dimmer(r);
		//}
	}

	if( ! Gest.active & (Gest.gestures !="") & (timeDiff > Gest.tapDelay) ) {

		 if( Gest.gestures == "TT") {
			int r = Switch.getRegion(Gest.startX, Gest.startY);
			Switch.dimmer(r);
		}

		if( Gest.gestures == "T" ) {
			int r = Switch.getRegion(Gest.endX, Gest.endY);
			Serial.print("Selected region: ");
			Serial.println(r);
			Switch.toggleRegion(r);
		}

		// Sleep on region
		if( Gest.gestures == "TTT" ) {
			int r = Switch.getRegion(Gest.endX, Gest.endY);
			Switch.animSetRegion(r);
		}

		/*
		if( Gest.gestures == "HUD" ) {
			systemInfo();
		}
		*/

		if( Gest.gestures == "HD" | Gest.gestures == "D" ) {
			Switch.favOff();
		}

		if( Gest.gestures == "HU" | Gest.gestures == "U" ) {
			Switch.favOn();
		}

		if( Gest.gestures == "HDL" | Gest.gestures == "DL" ) {
			Serial.println("HDL");
			Switch.clear();
		}

		if( Gest.gestures == "HUR" | Gest.gestures == "UR" ) {
			Switch.allOn();
		}

		if( Gest.gestures == "HLDR" | Gest.gestures == "LDR" ) {
			lightSensorCalibrate();
		}

		if( Gest.gestures == "HDRUL" | Gest.gestures == "DRUL" ) {
			Serial.println("RGB indi START");
			rgbIndi();
		}

		// gesture have been processed and can be reseted
		Gest.reset();
	}


	lightSensorUpdate();

	Tlc.update();
}

void lightSensorUpdate() {
	int data = analogRead(lightSensorPin);

	if( (data < 100) & (data > 0) & ((millis() - lightActionTime) > 1000) ) {
		lightActionTime = millis();

		Serial.println(data);
		Serial.println("light sensor update");
		if(Switch.pinData > 0) {
			Switch.clear();
		}
		else {
			Switch.favOn();
		}
	}
}

/*
   SerialEvent occurs whenever a new data comes in the
   hardware serial RX.  This routine is run between each
   time loop() runs, so using delay inside loop can delay
   response.  Multiple bytes of data may be available.
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
** CALIBRATION
** -------------------------------------------------------*/

void rgbIndi() {
	Serial.println("RGB indi START");
	bool calibrate = true;
	Gest.reset();

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


		if(Gest.gestures == "TTT" & (timeDiff > Gest.tapDelay))
			calibrate = false;

		if(Gest.gestures == "T" & (timeDiff > Gest.tapDelay)) {
			if(rgb == 13) rgb = 14;
			else if(rgb == 14) rgb = 15;
			else if(rgb == 15) rgb = 13;

			Serial.print("rgb: ");
			Serial.println(rgb);
			Gest.reset();
		}

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

void lightSensorCalibrate() {
	Serial.println("LIGHT SENSOR calibrate START");
	bool calibrate = true;
	Gest.reset();

	Switch.clear();

	while(calibrate) {
		Screen.update();
		Gest.update(Screen.touchX, Screen.touchY, Screen.active);

		int data = analogRead(lightSensorPin);
		Serial.print("light sensor: ");
		Serial.println(data);

		if(data > 100) {
			Tlc.setAll(4000);
		}
		else {
			Tlc.clear();
		}

		if(Gest.gestures == "TT")
			calibrate = false;

		Tlc.update();
	}

	Tlc.clear();
	Tlc.update();

	Gest.reset();

	Serial.println("IR calibrate STOP");
}
