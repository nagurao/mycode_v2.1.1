#include <TimeAlarms.h>
#include <Time.h>
#include <SPI.h>

#define NODE_HAS_RELAY
#define LIGHT_NODE

#define MY_RADIO_NRF24
#define MY_NODE_ID STAIRCASE_LIGHT_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_01_NODE_ID
#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Staircase Light"

boolean lightStatusReceived;
boolean sendLightStatusRequest;

byte lightStatusRequstCount;
byte prevLightStatus;

AlarmId lightStatusTimer;
AlarmId heartbeatTimer;
AlarmId lightStatusRequestTimer;

MyMessage lightRelayMessage(LIGHT_RELAY_ID, V_STATUS);
MyMessage thingspeakMessage(WIFI_NODEMCU_ID, V_CUSTOM);

void before()
{
	pinMode(LIGHT_RELAY_PIN, OUTPUT);
}

void setup()
{
	digitalWrite(LIGHT_RELAY_PIN, LOW);
	lightStatusReceived = false;
	sendLightStatusRequest = true;
	lightStatusRequstCount = 0;
	prevLightStatus = 0;
	thingspeakMessage.setDestination(THINGSPEAK_NODE_ID);
	thingspeakMessage.setType(V_CUSTOM);
	thingspeakMessage.setSensor(WIFI_NODEMCU_ID);
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
	lightStatusRequestTimer = Alarm.timerRepeat(HALF_HOUR, requestLightStatus);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(LIGHT_RELAY_ID, S_BINARY, "Staircase Light");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(lightRelayMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{
	if (sendLightStatusRequest)
	{
		sendLightStatusRequest = false;
		request(LIGHT_RELAY_ID, V_STATUS, BALCONYLIGHT_NODE_ID);
		lightStatusTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkLightStatusRequest);
		lightStatusRequstCount++;
		if (lightStatusRequstCount == 30)
		{
			lightStatusReceived = true;
			Alarm.free(lightStatusTimer);
			sendLightStatusRequest = false;
		}
	}
	Alarm.delay(1);
}

void receive(const MyMessage &message)
{
	switch (message.type)
	{
	case V_STATUS:
		if (message.getInt())
		{
			digitalWrite(LIGHT_RELAY_PIN, RELAY_ON);
			send(lightRelayMessage.set(RELAY_ON));
			Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
			if (prevLightStatus != message.getInt())
			{
				send(thingspeakMessage.set(RELAY_ON));
				Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
				prevLightStatus = message.getInt();
			}
		}
		else
		{
			digitalWrite(LIGHT_RELAY_PIN, RELAY_OFF);
			send(lightRelayMessage.set(RELAY_OFF));
			Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
			if (prevLightStatus != message.getInt())
			{
				send(thingspeakMessage.set(RELAY_OFF));
				Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
				prevLightStatus = message.getInt();
			}
		}
		if (!lightStatusReceived)
		{
			lightStatusReceived = true;
			Alarm.free(lightStatusTimer);
			sendLightStatusRequest = false;
		}
		break;
	}
}

void checkLightStatusRequest()
{
	if (!lightStatusReceived)
		sendLightStatusRequest = true;
}

void requestLightStatus()
{
	request(LIGHT_RELAY_ID, V_STATUS, BALCONYLIGHT_NODE_ID);
}