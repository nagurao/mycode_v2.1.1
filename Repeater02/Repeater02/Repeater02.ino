#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define STATUS_LEDS

#define MY_RADIO_NRF24
#define MY_REPEATER_FEATURE
#define MY_NODE_ID REPEATER_02_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_01_NODE_ID
#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Repeater 02"

AlarmId heartbeatTimer;

void before()
{

}

void setup()
{
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, __DATE__);
}

void loop()
{
	Alarm.delay(1);
}