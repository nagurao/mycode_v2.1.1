#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>
#include <PinChangeInt.h>

#define TANK_02_NODE
#define WATER_TANK_NODE
#define NODE_INTERACTS_WITH_RELAY
#define NODE_WITH_HIGH_LOW_FEATURE
#define SUMP_RELATED_NODE

#define MY_RADIO_NRF24
#define MY_NODE_ID TANK_02_NODE_ID

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Tank 02"

AlarmId heartbeatTimer;
AlarmId checkLowTimer;
AlarmId checkHighTimer;
AlarmId firstTimeTimer;

volatile boolean sendLowLevelSensorUpdate;
volatile boolean sendHighLevelSensorUpdate;

volatile boolean tankAtLowLevel;
volatile boolean tankAtHighLevel;

boolean tankLowLevelAck;
boolean tankHighLevelAck;

MyMessage sumpMotorNodeMessage(SUMP_MOTOR_RELAY_ID, V_STATUS);
MyMessage sumpMotorAdhocMessage(SUMP_MOTOR_ADHOC_ID, V_STATUS);
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
	present(TANK_LOW_LEVEL_ID, S_DOOR, "Tank 02 Low Level");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(TANK_HIGH_LEVEL_ID, S_DOOR, "Tank 02 High Level");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SUMP_MOTOR_ADHOC_ID, S_BINARY, "Sump Adhoc Switch");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(sumpMotorAdhocMessage.set(RELAY_OFF));
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

		sumpMotorNodeMessage.setDestination(SUMP_MOTOR_NODE_ID);
		sumpMotorNodeMessage.setSensor(TANK_LOW_LEVEL_ID);
		sumpMotorNodeMessage.setType(V_TRIPPED);
		sumpMotorNodeMessage.set(tankAtLowLevel ? LOW_LEVEL : NOT_LOW_LEVEL);
		send(sumpMotorNodeMessage);
		wait(WAIT_AFTER_SEND_MESSAGE);
		checkLowTimer = Alarm.timerOnce(ONE_MINUTE, checkLowLevelAck);
	}

	if (sendHighLevelSensorUpdate)
	{
		sendHighLevelSensorUpdate = false;
		tankHighLevelAck = false;
		send(highLevelMessage.set(tankAtHighLevel ? HIGH_LEVEL : NOT_HIGH_LEVEL));
		wait(WAIT_AFTER_SEND_MESSAGE);

		sumpMotorNodeMessage.setDestination(SUMP_MOTOR_NODE_ID);
		sumpMotorNodeMessage.setSensor(TANK_HIGH_LEVEL_ID);
		sumpMotorNodeMessage.setType(V_TRIPPED);
		sumpMotorNodeMessage.set(tankAtHighLevel ? HIGH_LEVEL : NOT_HIGH_LEVEL);
		send(sumpMotorNodeMessage);
		wait(WAIT_AFTER_SEND_MESSAGE);
		checkHighTimer = Alarm.timerOnce(TWENTY_SECS, checkHighLevelAck);
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
		case SUMP_MOTOR_NODE_ID:
			if (message.getInt())
				sumpMotorAdhocMessage.set(RELAY_ON);
			else
				sumpMotorAdhocMessage.set(RELAY_OFF);
			send(sumpMotorAdhocMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		default:
			if (message.getInt())
				turnOnSumpMotor();
			else
				turnOffSumpMotor();
			break;
		}
		break;
	case V_TRIPPED:
		if (message.sender == SUMP_MOTOR_NODE_ID)
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

void turnOnSumpMotor()
{
	sumpMotorNodeMessage.setDestination(SUMP_MOTOR_NODE_ID);
	sumpMotorNodeMessage.set(RELAY_ON);
	send(sumpMotorNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void turnOffSumpMotor()
{
	sumpMotorNodeMessage.setDestination(SUMP_MOTOR_NODE_ID);
	sumpMotorNodeMessage.set(RELAY_OFF);
	send(sumpMotorNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void checkLowLevelAck()
{
	if (!tankLowLevelAck)
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