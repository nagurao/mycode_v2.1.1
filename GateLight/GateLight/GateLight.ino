#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define LIGHT_NODE
#define NODE_HAS_RELAY

#define MY_RADIO_NRF24
#define MY_NODE_ID GATELIGHT_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_02_NODE_ID
#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Gate Light"

byte lightOnDurationRequestCount;
boolean lightOnDurationReceived;
boolean sendlightOnDurationRequest;
int lightOnDuration;

AlarmId requestTimer;
AlarmId heartbeatTimer;

MyMessage lightRelayMessage(LIGHT_RELAY_ID, V_STATUS);
MyMessage staircaseLightRelayMessage(LIGHT_RELAY_ID, V_STATUS);
MyMessage thingspeakMessage(WIFI_NODEMCU_ID, V_CUSTOM);

void before()
{
	pinMode(LIGHT_RELAY_PIN, OUTPUT);
}

void setup()
{
	digitalWrite(LIGHT_RELAY_PIN, LOW);
	lightOnDurationReceived = false;
	sendlightOnDurationRequest = true;
	staircaseLightRelayMessage.setDestination(STAIRCASE_LIGHT_NODE_ID);
	staircaseLightRelayMessage.setType(V_STATUS);
	staircaseLightRelayMessage.setSensor(LIGHT_RELAY_ID);
	thingspeakMessage.setDestination(THINGSPEAK_NODE_ID);
	thingspeakMessage.setType(V_CUSTOM);
	thingspeakMessage.setSensor(WIFI_NODEMCU_ID);
	lightOnDurationRequestCount = 0;
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(LIGHT_RELAY_ID, S_BINARY, "Gate Light Relay");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(LIGHT_DURATION_ID, S_CUSTOM, "Light On Duration");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(lightRelayMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{
	if (sendlightOnDurationRequest)
	{
		sendlightOnDurationRequest = false;
		request(LIGHT_DURATION_ID, V_VAR1);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkLightOnDurationRequest);
		lightOnDurationRequestCount++;
		if (lightOnDurationRequestCount == 10)
		{
			MyMessage lightOnDurationMessage(LIGHT_DURATION_ID, V_VAR1);
			send(lightOnDurationMessage.set(DEFAULT_LIGHT_ON_DURATION));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
	}

	Alarm.delay(1);
}

void receive(const MyMessage &message)
{
	int newLightOnDuration;
	switch (message.type)
	{
	case V_VAR1:
		newLightOnDuration = message.getInt();

		if (lightOnDurationReceived && newLightOnDuration > 0 && newLightOnDuration <= 600)
		{
			lightOnDuration = newLightOnDuration;
			MyMessage lightOnDurationMessage(LIGHT_DURATION_ID, V_VAR1);
			send(lightOnDurationMessage.set(lightOnDuration));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}

		if (!lightOnDurationReceived)
		{
			if (newLightOnDuration > 0 && newLightOnDuration <= 600)
				lightOnDuration = newLightOnDuration;
			else
			{
				lightOnDuration = DEFAULT_LIGHT_ON_DURATION;
				MyMessage lightOnDurationMessage(LIGHT_DURATION_ID, V_VAR1);
				send(lightOnDurationMessage.set(lightOnDuration));
				wait(WAIT_AFTER_SEND_MESSAGE);
			}
			lightOnDurationReceived = true;
			Alarm.free(requestTimer);
			request(LIGHT_DURATION_ID, V_VAR1);
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
		break;
	case V_STATUS:
		switch (message.sender)
		{
		case THINGSPEAK_NODE_ID:
			if (message.getBool())
				turnOnLights();
			else
				turnOffLights();
			break;
		default:
			if (message.getInt())
				turnOnLights();
			else
				turnOffLights();
			break;
		}
		break;
	}
}

void turnOnLights()
{
	digitalWrite(LIGHT_RELAY_PIN, RELAY_ON);
	send(lightRelayMessage.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(staircaseLightRelayMessage.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void turnOffLights()
{
	digitalWrite(LIGHT_RELAY_PIN, RELAY_OFF);
	send(lightRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(staircaseLightRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void checkLightOnDurationRequest()
{
	if (!lightOnDurationReceived)
		sendlightOnDurationRequest = true;
}
