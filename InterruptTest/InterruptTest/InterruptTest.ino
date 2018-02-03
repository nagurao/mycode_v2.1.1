
/*
 Name:		InterruptTest.ino
 Created:	06-Jan-18 20:37:55
 Author:	MYS
*/

// the setup function runs once when you press reset or power the board
#include <PinChangeInt.h>
#define LOW_LEVEL_PIN 2
#define HIGH_LEVEL_PIN 3

volatile boolean lowLevel;
volatile boolean updateLowLevelChange;
volatile boolean highLevel;
volatile boolean updateHighLevelChange;

void setup()
{
	Serial.begin(115200);
	pinMode(LOW_LEVEL_PIN, INPUT_PULLUP);
	pinMode(HIGH_LEVEL_PIN, INPUT_PULLUP);
	if (digitalRead(LOW_LEVEL_PIN))
		lowLevel = false;
	else
		lowLevel = true;

	if (digitalRead(HIGH_LEVEL_PIN))
		highLevel = false;
	else
		highLevel = true;

	attachPinChangeInterrupt(LOW_LEVEL_PIN, lowLevelSensor, CHANGE);
	attachPinChangeInterrupt(HIGH_LEVEL_PIN, highLevelSensor, CHANGE);
	updateLowLevelChange = false;
	updateHighLevelChange = false;
	Serial.print("The initial state of Low Level sensor is : ");
	Serial.println(lowLevel, DEC);
	Serial.print("The initial state of High Level sensor is : ");
	Serial.println(highLevel, DEC);
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	delay(1000);
	if (updateLowLevelChange)
	{
		Serial.println("Low Level Sensor toggled");
		Serial.print("Low Level Sensor value is : ");
		Serial.println(lowLevel, DEC);
		updateLowLevelChange = false;
		if (digitalRead(LOW_LEVEL_PIN))
			Serial.println("Low Sensor is reading High Value");
	}
	if (updateHighLevelChange)
	{
		Serial.println("High Level Sensor toggled");
		Serial.print("High Level Sensor value is : ");
		Serial.println(highLevel, DEC);
		updateHighLevelChange = false;
		if (digitalRead(HIGH_LEVEL_PIN))
			Serial.println("High Sensor is reading High Value");
	}
}

void lowLevelSensor()
{
	lowLevel = !lowLevel;
	updateLowLevelChange = true;
}
void highLevelSensor()
{
	highLevel = !highLevel;
	updateHighLevelChange = true;
}