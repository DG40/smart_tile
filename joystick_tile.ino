// Libraries:
#include "HX711.h"	// - for getting tenzometric data                                        
#include "FastLED.h"	// - for LED-panel control

// LEDs:
#define LED_TYPE NEOPIXEL
#define NUM_STRIPS 15
#define NUM_LEDS_PER_STRIP 15
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];
#define PIN1 22
#define PIN2 24
#define PIN3 26
#define PIN4 28
#define PIN5 30
#define PIN6 32
#define PIN7 34
#define PIN8 36
#define PIN9 38
#define PIN10 40
#define PIN11 42
#define PIN12 44
#define PIN13 46
#define PIN14 48
#define PIN15 50

// DOUT and SCK for tensometric sensors:
uint8_t DOUT_PIN1 = 3;
uint8_t DOUT_PIN2 = 4;
uint8_t DOUT_PIN3 = 5;
uint8_t DOUT_PIN4 = 6;
uint8_t SCK_PIN  = 2;

// Create objects 'scale':
HX711 scale1;
HX711 scale2;
HX711 scale3;
HX711 scale4;

// Raw data from tenzo (for anti-bounce sensors):
float t1 = 0;
float t2 = 0;
float t3 = 0;
float t4 = 0;
int presence_weight = 1000; // threshold means "somebody is standing on the scales"
byte count = 0; // current quantity of "true" data
byte anti_bounce_count = 5; // threshold quantity of "true" data

// Pure data from tenzo (after using anti-bounce):
float tenzoData1 = 0;
float tenzoData2 = 0;
float tenzoData3 = 0;
float tenzoData4 = 0;
float calibration_factor = -190.77;

// Variables for determination of coordinates:
int xC1 = 0;
int yC1 = 0;
int R1 = 0;
int xC2 = 0;
int yC2 = 0;
int R2 = 0;

// Pure coordinates:
int xC = 0;
int yC = 0;

// Variables for rainbow cycle coloring spot (LED [xC, yC]):
byte WheelPos;
byte c[] = {0, 0, 0};

// Variables for UART communication:
String data = "";	// - reception
String ans = "";	// - transmission


void setup()
{
	Serial.begin(9600);
	Serial3.begin(115200);

	scale1.begin(DOUT_PIN1, SCK_PIN);
	scale1.set_scale();
	scale1.tare();
	scale1.set_scale(calibration_factor);

	scale2.begin(DOUT_PIN2, SCK_PIN);
	scale2.set_scale();
	scale2.tare();
	scale2.set_scale(calibration_factor);

	scale3.begin(DOUT_PIN3, SCK_PIN);
	scale3.set_scale();
	scale3.tare();
	scale3.set_scale(calibration_factor);

	scale4.begin(DOUT_PIN4, SCK_PIN);
	scale4.set_scale();
	scale4.tare();
	scale4.set_scale(calibration_factor);

	FastLED.addLeds<LED_TYPE, PIN1>(leds[0], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN2>(leds[1], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN3>(leds[2], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN4>(leds[3], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN5>(leds[4], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN6>(leds[5], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN7>(leds[6], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN8>(leds[7], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN9>(leds[8], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN10>(leds[9], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN11>(leds[10], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN12>(leds[11], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN13>(leds[12], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN14>(leds[13], NUM_LEDS_PER_STRIP);
	FastLED.addLeds<LED_TYPE, PIN15>(leds[14], NUM_LEDS_PER_STRIP);

	for(byte i = 0; i < NUM_STRIPS; i++)
	{    
		for(byte j = 0; j < NUM_LEDS_PER_STRIP; j++) 
		{
			leds[i][j].red   = 0;
			leds[i][j].green = 0;
			leds[i][j].blue  = 0;
		}            
	}
	FastLED.show();
}


void loop()
{
	t1 = scale1.get_units(1);
	t2 = scale2.get_units(1);
	t3 = scale3.get_units(1);
	t4 = scale4.get_units(1);

	if (t1 > presence_weight or t2 > presence_weight or t3 > presence_weight or t4 > presence_weight)
	{
		count++;
		if (count > anti_bounce_count)
		  count = anti_bounce_count;
	}
	else
		count--;
		
	if (count == 255) // < 0 (0 - 1 = 255, becauce byte)
	  count = 0;

	if (count == 0)
	{
		tenzoData1 = 0;
		tenzoData2 = 0;
		tenzoData3 = 0;
		tenzoData4 = 0;
	}
	else if (count == anti_bounce_count)
	{
		tenzoData1 = t1;
		tenzoData2 = t2;
		tenzoData3 = t3;
		tenzoData4 = t4;
	}

	if (tenzoData4 > tenzoData1)
		R1 = floor(tenzoData4/1500) - 1;
	else
		R1 = - floor(tenzoData1/1500) + 1;

	if (tenzoData3 > tenzoData2)
		R2 = floor(tenzoData3/1500) - 1;
	else
		R2 = - floor(tenzoData2/1500) + 1;

	xC1 = R1 + 7;
	if (xC1 > 14)
		xC1 = 14;
	if (xC1 < 0)
		xC1 = 0;

	yC1 = xC1;

	xC2 = R2 + 7;
	if (xC2 > 14)
		xC2 = 14;
	if (xC2 < 0)
		xC2 = 0;

	yC2 = xC2;

	xC2 = 14 - xC2;

	xC = floor((xC1 + xC2)/2);
	yC = floor((yC1 + yC2)/2);

	if(WheelPos < 85) 
	{
		c[0]=WheelPos * 3;
		c[1]=255 - WheelPos * 3;
		c[2]=0;
	} 
	else if(WheelPos < 170) 
	{
		WheelPos -= 85;
		c[0]=255 - WheelPos * 3;
		c[1]=0;
		c[2]=WheelPos * 3;
	} 
	else 
	{
		WheelPos -= 170;
		c[0]=0;
		c[1]=WheelPos * 3;
		c[2]=255 - WheelPos * 3;
	}

	WheelPos++;

	leds[xC][yC] = CRGB(c[0], c[1], c[2]);  

	leds[xC-1][yC] = CRGB(c[0]/2, c[1]/2, c[2]/2); 
	leds[xC][yC-1] = CRGB(c[0]/2, c[1]/2, c[2]/2); 
	leds[xC+1][yC] = CRGB(c[0]/2, c[1]/2, c[2]/2); 
	leds[xC][yC+1] = CRGB(c[0]/2, c[1]/2, c[2]/2); 

	leds[xC-1][yC-1] = CRGB(c[0]/6, c[1]/6, c[2]/6); 
	leds[xC+1][yC-1] = CRGB(c[0]/6, c[1]/6, c[2]/6); 
	leds[xC+1][yC+1] = CRGB(c[0]/6, c[1]/6, c[2]/6); 
	leds[xC-1][yC+1] = CRGB(c[0]/6, c[1]/6, c[2]/6); 

	FastLED.show();

	leds[xC][yC] = CRGB(0,0,0);  

	leds[xC-1][yC] = CRGB(0,0,0);
	leds[xC][yC-1] = CRGB(0,0,0);
	leds[xC+1][yC] = CRGB(0,0,0);
	leds[xC][yC+1] = CRGB(0,0,0);

	leds[xC-1][yC-1] = CRGB(0,0,0);
	leds[xC+1][yC-1] = CRGB(0,0,0);
	leds[xC+1][yC+1] = CRGB(0,0,0);
	leds[xC-1][yC+1] = CRGB(0,0,0);

	if (Serial3.available() > 0) 
	{
		data = Serial3.readStringUntil('\n');
		if (data == "Z") 
		{
			if (tenzoData1 + tenzoData2 + tenzoData3 + tenzoData4 > presence_weight)
				ans = 'X' + String(xC) + 'Y' + String(yC);
			else
				ans = 'W';
		}
	}
}


// For rainbow cycle of LED color:
byte * Wheel(byte WheelPos) 
{
	static byte c[3];

	if(WheelPos < 85) 
	{
		c[0]=WheelPos * 3;
		c[1]=255 - WheelPos * 3;
		c[2]=0;
	} 
	else if(WheelPos < 170) 
	{
		WheelPos -= 85;
		c[0]=255 - WheelPos * 3;
		c[1]=0;
		c[2]=WheelPos * 3;
	} 
	else 
	{
		WheelPos -= 170;
		c[0]=0;
		c[1]=WheelPos * 3;
		c[2]=255 - WheelPos * 3;
	}

	return c;
}
