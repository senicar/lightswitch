#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

#ifndef NUM_LIGHTS
#define NUM_LIGHTS 12
#endif

#define DEBUG false

#ifndef Switcher_h
#define Switcher_h

#include "Arduino.h"
#include "Tlc5940.h"
#include "TouchDS.h"
#include "Gestures.h"

class Switcher {
	private:
		struct light {
			light() : intensity(0), region(NUM_LIGHTS), fav(false) {};
			int region;
			int intensity;
			int pwm;
			bool fav;
		};

		struct region {
			region() : x(-1024), y(-1024), intensity(0), fav(false) {};
			int x;
			int y;
			int intensity;
			int pwm;
			unsigned long irCode;
			bool fav;
			bool hasFav;
		};

	public:
		TouchDS & Screen;
		Gestures & Gest;
		Tlc5940 & Tlc;

		Switcher (TouchDS & screen, Gestures & gest, Tlc5940 & tlc)
			: Screen(screen), Gest(gest), Tlc(tlc)
		{
			numLights = 0;
			numRegions = 0;
			regionTolerance = 100; // this makes a region size 50% left and 50% right, up and down from the point
			pinData = 0;
			indicatorPin = 6;
 			animBufferNum = NUMITEMS(animBuffer);
 			pwmNum = NUMITEMS(pwm);
		};

		static const int OFF = 0;
		static const int FULL_ON = 4095;
		static const int pwm[16];
		static const int states[10][3];

		struct anim {
			anim() : active(false), fade(false), endIntensity(OFF), startIntensity(FULL_ON), region(NUM_LIGHTS), light(NUM_LIGHTS) {};
			unsigned long endTime; // must be set
			unsigned dur; // must be set
			unsigned endIntensity;
			unsigned startIntensity;
			int region;
			int light;
			bool active;
			bool fade;
		};

		light lights[NUM_LIGHTS];
		region regions[NUM_LIGHTS];

		// ir actions
		unsigned long irFavCode;
		unsigned long irToggleCode;

		// maximum 2 buffers are available
		anim animBuffer[2];

		char animBufferNum;
		int pwmNum;
		int getPwmLevel(int intensity);

		// used when external factors of lightness need to be taken into account
		// for example sudden light change can be indicator of switch or sun rays
		unsigned long lastTimeSwitched;

		void setup();
		void setupReset();

		void toggle(int light);
		void turnOff(int light);
		void turnOn(int light);

		void dim(int light, int intensity);
		void dim(int light, int intensity, unsigned dimDuration);
		void dimmer(int region);

		void favOn();
		void favOff();
		void favToggle();
		bool favState;

		void allOn();
		void allOff();

		void toggleRegion(int region);
		void turnOffRegion(int region);
		void turnOnRegion(int region);
		void dimRegion(int region, int intensity);
		void dimRegion(int region, int intensity, unsigned dimDuration);
		void updateRegion(int region);
		void updateRegions();

		void clear();
		void setAll(int intensity);

		void animate(anim a);
		void animSetRegion(int region);

		void update();

		int getRegion(int x, int y);
		int getRegion(int x, int y, bool sloppy);

		int indicatorPin;
		char indicatorFrom; // rgb state
		char indicatorTo; // rgb state
		unsigned indicatorStartTime;
		unsigned indicatorStopTime;
		void indicator(char message[], int color);
		void indicator(char message[], int color, int intensity);

		int numLights;
		int numRegions;
		int regionTolerance;

		// for timer
		bool dimToOff;
		unsigned offTime;

		// this holds the information which pins are on
		int pinData;

		uint16_t state;

		void regionInfo( int i );
		void regionsInfo();

		void lightInfo( int i );
		void lightsInfo();
};

#endif


