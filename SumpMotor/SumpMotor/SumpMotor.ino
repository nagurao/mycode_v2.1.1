#include <Keypad.h>
#include <SPI.h>
#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>

#define SUMP_RELATED_NODE
#define NODE_HAS_RELAY
#define NODE_WITH_HIGH_LOW_FEATURE
#define WATER_TANK_NODE
#define KEYPAD_1R_2C

#define MY_RADIO_NRF24
#define MY_NODE_ID SUMP_MOTOR_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_02_NODE_ID
#define MY_PARENT_NODE_IS_STATIC


#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Sump Motor"

AlarmId heartbeatTimer;
AlarmId updateTimer;

volatile boolean tank02LowLevel;
volatile boolean tank02HighLevel;
volatile boolean tank03LowLevel;
volatile boolean tank03HighLevel;
boolean sumpMotorOn;

MyMessage thingspeakMessage(WIFI_NODEMCU_ID, V_CUSTOM);
MyMessage sumpMotorRelayMessage(SUMP_MOTOR_RELAY_ID, V_STATUS);
MyMessage tank02Message(SUMP_MOTOR_RELAY_ID, V_STATUS);
MyMessage tank02LowLevelMessage(TANK02_LOW_LEVEL_ID, V_TRIPPED);
MyMessage tank02HighLevelMessage(TANK02_HIGH_LEVEL_ID, V_TRIPPED);
MyMessage tank03LowLevelMessage(TANK03_LOW_LEVEL_ID, V_TRIPPED);
MyMessage tank03HighLevelMessage(TANK03_HIGH_LEVEL_ID, V_TRIPPED);
Keypad keypad = Keypad(makeKeymap(keys), rowsPins, colsPins, ROWS, COLS);


void before()
{
	pinMode(RELAY_PIN, OUTPUT);
	pinMode(MOTOR_STATUS_PIN, OUTPUT);
}

void setup()
{
	keypad.addEventListener(keypadEvent);
	keypad.setDebounceTime(WAIT_50MS);
	sumpMotorOn = false;
	tank02LowLevel = false;
	tank02HighLevel = false;
	tank03LowLevel = false;
	tank03HighLevel = false;

	digitalWrite(RELAY_PIN, LOW);
	digitalWrite(MOTOR_STATUS_PIN, LOW);

	thingspeakMessage.setDestination(THINGSPEAK_NODE_ID);
	thingspeakMessage.setType(V_CUSTOM);
	thingspeakMessage.setSensor(WIFI_NODEMCU_ID);

	tank02LowLevelMessage.setDestination(TANK_02_NODE_ID);
	tank02HighLevelMessage.setDestination(TANK_02_NODE_ID);

	tank03LowLevelMessage.setDestination(TANK_03_NODE_ID);
	tank03HighLevelMessage.setDestination(TANK_03_NODE_ID);

	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
	updateTimer = Alarm.timerRepeat(QUATER_HOUR, sendUpdate);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SUMP_MOTOR_RELAY_ID, S_BINARY, "Sump Motor Relay");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(sumpMotorRelayMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{
	char key = keypad.getKey();
	Alarm.delay(1);
}

void receive(const MyMessage &message)
{
	byte incomingValue;
	switch (message.type)
	{
	case V_STATUS:
		switch (message.sender)
		{
		case THINGSPEAK_NODE_ID:
			incomingValue = message.getBool();
			break;
		default:
			incomingValue = message.getInt();
			break;
		}
		switch (incomingValue)
		{
		case RELAY_ON:
			if (!tank02HighLevel && !tank03LowLevel && !sumpMotorOn)
				turnOnSumpMotor();
			break;
		case RELAY_OFF:
			if (sumpMotorOn)
				turnOffSumpMotor();
			break;
		}
		break;
	case V_TRIPPED:
		switch (message.sensor)
		{
		case TANK02_LOW_LEVEL_ID:
			if (message.getInt())
				tank02LowLevel = LOW_LEVEL;
			else
				tank02LowLevel = NOT_LOW_LEVEL;
			tank02LowLevelMessage.set(tank02LowLevel);
			send(tank02LowLevelMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		case TANK02_HIGH_LEVEL_ID:
			if (message.getInt())
				tank02HighLevel = HIGH_LEVEL;
			else
				tank02HighLevel = NOT_HIGH_LEVEL;
			tank02HighLevelMessage.set(tank02HighLevel);
			send(tank02HighLevelMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		case TANK03_LOW_LEVEL_ID:
			if (message.getInt())
				tank03LowLevel = LOW_LEVEL;
			else
				tank03LowLevel = NOT_LOW_LEVEL;
			tank03LowLevelMessage.set(tank03LowLevel);
			send(tank03LowLevelMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		case TANK03_HIGH_LEVEL_ID:
			if (message.getInt())
				tank03HighLevel = HIGH_LEVEL;
			else
				tank03HighLevel = NOT_HIGH_LEVEL;
			tank03HighLevelMessage.set(tank03HighLevel);
			send(tank03HighLevelMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		}
		if (tank02LowLevel && !tank03LowLevel && !sumpMotorOn)
			turnOnSumpMotor();

		if (sumpMotorOn)
		{
			send(tank02Message.set(RELAY_ON));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}

		if ((tank02HighLevel || tank03LowLevel) && sumpMotorOn)
			turnOffSumpMotor();

		if (!sumpMotorOn)
		{
			send(tank02Message.set(RELAY_OFF));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
		break;
	}
}

void turnOnSumpMotor()
{
	digitalWrite(RELAY_PIN, RELAY_ON);
	send(sumpMotorRelayMessage.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
	digitalWrite(MOTOR_STATUS_PIN, RELAY_ON);
	sumpMotorOn = true;
}

void turnOffSumpMotor()
{
	digitalWrite(RELAY_PIN, RELAY_OFF);
	send(sumpMotorRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	digitalWrite(MOTOR_STATUS_PIN, RELAY_OFF);
	sumpMotorOn = false;
}

void sendUpdate()
{
	if(digitalRead(RELAY_PIN))
		send(sumpMotorRelayMessage.set(RELAY_ON));
	else
		send(sumpMotorRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void keypadEvent(KeypadEvent key)
{
	switch (keypad.getState())
	{
	case PRESSED:
		switch (key)
		{
		case '1':
			if (!tank02HighLevel && !tank03LowLevel && !sumpMotorOn)
				turnOnSumpMotor();
			break;
		case '2':
			if (sumpMotorOn)
				turnOffSumpMotor();
			break;
		}
		break;
	case HOLD:
		switch (key)
		{
		case '1':
			if (!tank02HighLevel && !tank03LowLevel && !sumpMotorOn)
				turnOnSumpMotor();
			break;
		case '2':
			if (sumpMotorOn)
				turnOffSumpMotor();
			break;
		}
		break;
	}
}