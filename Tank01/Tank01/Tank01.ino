#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define TANK_01_NODE
#define WATER_TANK_NODE
#define NODE_INTERACTS_WITH_LCD

#define MY_RADIO_NRF24
#define MY_NODE_ID TANK_01_NODE_ID

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Tank 01"

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
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(CURR_WATER_LEVEL_ID, S_WATER, "Tank 01 Water Level");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{

	Alarm.delay(1);
}
void receive(const MyMessage &message)
{

}
