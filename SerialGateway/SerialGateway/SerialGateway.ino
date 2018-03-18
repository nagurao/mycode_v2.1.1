#define MY_DEBUG 
#define MY_RADIO_NRF24
#define MY_RF24_PA_LEVEL RF24_PA_LOW
#define MY_GATEWAY_SERIAL
//#define MY_REPEATER_FEATURE

// Define a lower baud rate for Arduino's running on 8 MHz (Arduino Pro Mini 3.3V & SenseBender)
#if F_CPU == 8000000L
#define MY_BAUD_RATE 38400
#endif

#define MY_DEFAULT_LED_BLINK_PERIOD 300
#define MY_WITH_LEDS_BLINKING_INVERSE

#define MY_DEFAULT_ERR_LED_PIN 4  
#define MY_DEFAULT_RX_LED_PIN  5
#define MY_DEFAULT_TX_LED_PIN  6

#define APPLICATION_NAME "Serial Gateway"

#include <SPI.h>
#include <MySensors.h>  

void setup()
{

}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
}

void loop()
{

}

char* getCodeVersion()
{
	char codeVersion[20];
	strcpy(codeVersion, "");
	strcat(codeVersion, __DATE__);
	strcat(codeVersion, " ");
	strcat(codeVersion, __TIME__);
	return codeVersion;
}