#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>
#include <PinChangeInt.h>

#define TANK_01_NODE
#define WATER_TANK_NODE
#define NODE_INTERACTS_WITH_RELAY
#define NODE_WITH_HIGH_LOW_FEATURE

#define MY_RADIO_NRF24
#define MY_NODE_ID TANK_01_NODE_ID

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Tank 01"

AlarmId heartbeatTimer;
AlarmId checkLowTimer;
AlarmId checkHighTimer;
AlarmId firstTimeTimer;

boolean borewellOn;
boolean borewellStatusNotReceived;

volatile boolean sendLowLevelSensorUpdate;
volatile boolean sendHighLevelSensorUpdate;

volatile boolean tankAtLowLevel;
volatile boolean tankAtHighLevel;

boolean tankLowLevelAck;
boolean tankHighLevelAck;

MyMessage borewellNodeMessage;
MyMessage borewellAdhocMessage(BOREWELL_ADHOC_ID, V_STATUS);
MyMessage lowLevelMessage(TANK_LOW_LEVEL_ID, V_TRIPPED);
MyMessage highLevelMessage(TANK_HIGH_LEVEL_ID, V_TRIPPED);

void before()
{
	pinMode(LOW_LEVEL_PIN, INPUT_PULLUP);
	pinMode(HIGH_LEVEL_PIN, INPUT_PULLUP);
	if (digitalRead(LOW_LEVEL_PIN))
		tankAtLowLevel = false;
	else
		tankAtLowLevel = true;

	if (digitalRead(HIGH_LEVEL_PIN))
		tankAtHighLevel = false;
	else
		tankAtHighLevel = true;

	attachPinChangeInterrupt(LOW_LEVEL_PIN, lowLevelSensor, CHANGE);
	attachPinChangeInterrupt(HIGH_LEVEL_PIN, highLevelSensor, CHANGE);
}

void setup()
{
	tankLowLevelAck = false;
	tankHighLevelAck = false;
	sendLowLevelSensorUpdate = false;
	sendHighLevelSensorUpdate = false;
	borewellStatusNotReceived = true;
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
	if (tankAtLowLevel)
		sendLowLevelSensorUpdate = true;
	if (tankAtHighLevel)
		sendHighLevelSensorUpdate = true;
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(TANK_LOW_LEVEL_ID, S_DOOR, "Tank 01 Low Level");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(TANK_HIGH_LEVEL_ID, S_DOOR, "Tank 01 High Level");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(BOREWELL_ADHOC_ID, S_BINARY, "Borewell Adhoc Switch");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(borewellAdhocMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(lowLevelMessage.set(tankAtLowLevel ? LOW_LEVEL : NOT_LOW_LEVEL));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(highLevelMessage.set(tankAtHighLevel ? HIGH_LEVEL : NOT_HIGH_LEVEL));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	firstTimeTimer = Alarm.timerOnce(ONE_MINUTE, firstTime);
}

void loop()
{
	if (sendLowLevelSensorUpdate)
	{
		sendLowLevelSensorUpdate = false;
		tankLowLevelAck = false;
		send(lowLevelMessage.set(tankAtLowLevel ? LOW_LEVEL : NOT_LOW_LEVEL));
		wait(WAIT_AFTER_SEND_MESSAGE);
		
		borewellNodeMessage.setDestination(BOREWELL_NODE_ID);
		borewellNodeMessage.setSensor(TANK_LOW_LEVEL_ID);
		borewellNodeMessage.setType(V_TRIPPED);
		borewellNodeMessage.set(tankAtLowLevel ? LOW_LEVEL : NOT_LOW_LEVEL);
		send(borewellNodeMessage);
		wait(WAIT_AFTER_SEND_MESSAGE);
		checkLowTimer = Alarm.timerOnce(ONE_MINUTE, checkLowLevelAck);
	}

	if (sendHighLevelSensorUpdate)
	{
		sendHighLevelSensorUpdate = false;
		tankHighLevelAck = false;
		send(highLevelMessage.set(tankAtHighLevel ? HIGH_LEVEL : NOT_HIGH_LEVEL));
		wait(WAIT_AFTER_SEND_MESSAGE);

		borewellNodeMessage.setDestination(BOREWELL_NODE_ID);
		borewellNodeMessage.setSensor(TANK_HIGH_LEVEL_ID);
		borewellNodeMessage.setType(V_TRIPPED);
		borewellNodeMessage.set(tankAtHighLevel ? HIGH_LEVEL : NOT_HIGH_LEVEL);
		send(borewellNodeMessage);
		wait(WAIT_AFTER_SEND_MESSAGE);
		checkHighTimer = Alarm.timerOnce(ONE_MINUTE, checkHighLevelAck);
	}
	Alarm.delay(1);
}
void receive(const MyMessage &message)
{
	switch (message.type)
	{
	case V_STATUS:
		switch (message.sender)
		{
		case BOREWELL_NODE_ID:
			if (message.getInt())
			{
				borewellAdhocMessage.set(RELAY_ON);
				borewellOn = true;
			}
			else
			{
				borewellAdhocMessage.set(RELAY_OFF);
				borewellOn = false;
			}
			send(borewellAdhocMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		default:
			if (message.getInt())
				turnOnBorewell();
			else
				turnOffBorewell();
			break;
		}
		break;
	case V_TRIPPED:
		if (message.sender == BOREWELL_NODE_ID)
		{
			switch (message.sensor)
			{
			case TANK_LOW_LEVEL_ID:
				tankLowLevelAck = true;
				break;
			case TANK_HIGH_LEVEL_ID:
				tankHighLevelAck = true;
				break;
			}
		}
	}
}

void lowLevelSensor()
{
	tankAtLowLevel = !tankAtLowLevel;
	sendLowLevelSensorUpdate = true;
}

void highLevelSensor()
{
	tankAtHighLevel = !tankAtHighLevel;
	sendHighLevelSensorUpdate = true;
}

void turnOnBorewell()
{
	borewellNodeMessage.setDestination(BOREWELL_NODE_ID);
	borewellNodeMessage.setSensor(BORE_ON_RELAY_ID);
	borewellNodeMessage.setType(V_STATUS);
	borewellNodeMessage.set(RELAY_ON);
	send(borewellNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void turnOffBorewell()
{
	borewellNodeMessage.setDestination(BOREWELL_NODE_ID);
	borewellNodeMessage.setSensor(BORE_OFF_RELAY_ID);
	borewellNodeMessage.setType(V_STATUS);
	borewellNodeMessage.set(RELAY_ON);
	send(borewellNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void checkLowLevelAck()
{
	if(!tankLowLevelAck)
		sendLowLevelSensorUpdate = true;
}

void checkHighLevelAck()
{
	if (!tankHighLevelAck)
		sendHighLevelSensorUpdate = true;
}

void firstTime()
{
	send(lowLevelMessage.set(tankAtLowLevel ? LOW_LEVEL : NOT_LOW_LEVEL));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(highLevelMessage.set(tankAtHighLevel ? HIGH_LEVEL : NOT_HIGH_LEVEL));
	wait(WAIT_AFTER_SEND_MESSAGE);
}