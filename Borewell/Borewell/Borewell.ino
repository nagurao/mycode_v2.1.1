#include <Keypad.h>
#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define BOREWELL_NODE
#define NODE_HAS_RELAY
#define NODE_WITH_HIGH_LOW_FEATURE
#define WATER_TANK_NODE
#define KEYPAD_1R_2C

#define MY_RADIO_NRF24
#define MY_NODE_ID BOREWELL_NODE_ID
//#define MY_PARENT_NODE_ID REPEATER_02_NODE_ID
//#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Borewell Motor"

AlarmId heartbeatTimer;
//AlarmId dryRunTimer;
AlarmId updateTimer;

boolean borewellOn;
boolean tank01LowLevel;
boolean tank01HighLevel;

MyMessage thingspeakMessage(WIFI_NODEMCU_ID, V_CUSTOM);
MyMessage borewellMotorMessage(BOREWELL_MOTOR_STATUS_ID, V_STATUS);
MyMessage borewellMotorOnRelayMessage(BORE_ON_RELAY_ID, V_STATUS);
MyMessage borewellMotorOffRelayMessage(BORE_OFF_RELAY_ID, V_STATUS);
MyMessage tank01Message(BOREWELL_MOTOR_STATUS_ID, V_STATUS);
MyMessage lowLevelMessage(TANK_LOW_LEVEL_ID, V_TRIPPED);
MyMessage highLevelMessage(TANK_HIGH_LEVEL_ID, V_TRIPPED);

Keypad keypad = Keypad(makeKeymap(keys), rowsPins, colsPins, ROWS, COLS);

void before()
{
	pinMode(BORE_ON_RELAY_PIN, OUTPUT);
	pinMode(BORE_OFF_RELAY_PIN, OUTPUT);
	pinMode(MOTOR_STATUS_PIN, OUTPUT);
}

void setup()
{
	keypad.addEventListener(keypadEvent);
	keypad.setDebounceTime(WAIT_50MS);
	borewellOn = false;
	tank01LowLevel = false;
	tank01HighLevel = false;

	digitalWrite(BORE_ON_RELAY_PIN, LOW);
	digitalWrite(BORE_OFF_RELAY_PIN, LOW);
	digitalWrite(MOTOR_STATUS_PIN, LOW);

	thingspeakMessage.setDestination(THINGSPEAK_NODE_ID);
	thingspeakMessage.setType(V_CUSTOM);
	thingspeakMessage.setSensor(WIFI_NODEMCU_ID);

	lowLevelMessage.setDestination(TANK_01_NODE_ID);
	highLevelMessage.setDestination(TANK_01_NODE_ID);

	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
	//dryRunTimer = Alarm.timerRepeat(DRY_RUN_POLL_DURATION, turnOffBorewell);
	//Alarm.disable(dryRunTimer);

	updateTimer = Alarm.timerRepeat(QUATER_HOUR, sendUpdate);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(BOREWELL_MOTOR_STATUS_ID, S_BINARY, "Borewell Motor");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(BORE_ON_RELAY_ID, S_BINARY, "Bore On Relay");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(BORE_OFF_RELAY_ID, S_BINARY, "Bore Off Relay");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);

	send(borewellMotorMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(borewellMotorOnRelayMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(borewellMotorOffRelayMessage.set(RELAY_OFF));
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
	switch (message.type)
	{
	case V_STATUS:
		switch (message.sensor)
		{
		case BORE_ON_RELAY_ID:
			if (!tank01HighLevel && !borewellOn)
				turnOnBorewell();
			break;
		case BORE_OFF_RELAY_ID:
			if (borewellOn)
				turnOffBorewell();
			break;
		}
		break;
	case V_TRIPPED:
		switch (message.sensor)
		{
		case TANK_LOW_LEVEL_ID:
			if (message.getInt())
				tank01LowLevel = LOW_LEVEL;
			else
				tank01LowLevel = NOT_LOW_LEVEL;
			lowLevelMessage.set(tank01LowLevel);
			send(lowLevelMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		case TANK_HIGH_LEVEL_ID:
			if (message.getInt())
				tank01HighLevel = HIGH_LEVEL;
			else
				tank01HighLevel = NOT_HIGH_LEVEL;
			highLevelMessage.set(tank01HighLevel);
			send(highLevelMessage);
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		}

		if (tank01LowLevel && !borewellOn)
			turnOnBorewell();
		
		if (borewellOn)
		{
			send(tank01Message.set(RELAY_ON));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}

		//if (tank01HighLevel && borewellOn)
		//	turnOffBorewell();
		
		if (tank01HighLevel)
			turnOffBorewell();

		if (!borewellOn)
		{
			send(tank01Message.set(RELAY_OFF));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
		break;
	}
}

void turnOnBorewell()
{
	digitalWrite(BORE_ON_RELAY_PIN, RELAY_ON);
	send(borewellMotorOnRelayMessage.set(RELAY_ON));
	Alarm.timerOnce(RELAY_TRIGGER_INTERVAL, toggleOnRelay);
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void toggleOnRelay()
{
	digitalWrite(BORE_ON_RELAY_PIN, RELAY_OFF);
	send(borewellMotorOnRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(borewellMotorMessage.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
	digitalWrite(MOTOR_STATUS_PIN, RELAY_ON);
	borewellOn = true;
	send(tank01Message.set(RELAY_ON));
	wait(WAIT_AFTER_SEND_MESSAGE);
	//Alarm.enable(dryRunTimer);
}

void turnOffBorewell()
{
	digitalWrite(BORE_OFF_RELAY_PIN, RELAY_ON);
	send(borewellMotorOffRelayMessage.set(RELAY_ON));
	Alarm.timerOnce(RELAY_TRIGGER_INTERVAL, toggleOffRelay);
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void toggleOffRelay()
{
	digitalWrite(BORE_OFF_RELAY_PIN, RELAY_OFF);
	send(borewellMotorOffRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(borewellMotorMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	send(thingspeakMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	digitalWrite(MOTOR_STATUS_PIN, RELAY_OFF);
	borewellOn = false;
	send(tank01Message.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);
	//Alarm.disable(dryRunTimer);
}

void sendUpdate()
{
	if (digitalRead(BORE_ON_RELAY_PIN))
		send(borewellMotorOnRelayMessage.set(RELAY_ON));
	else
		send(borewellMotorOnRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);

	if(digitalRead(BORE_OFF_RELAY_PIN))
		send(borewellMotorOffRelayMessage.set(RELAY_ON));
	else
		send(borewellMotorOffRelayMessage.set(RELAY_OFF));
	wait(WAIT_AFTER_SEND_MESSAGE);

	if(digitalRead(MOTOR_STATUS_PIN))
		send(borewellMotorMessage.set(RELAY_ON));
	else
		send(borewellMotorMessage.set(RELAY_OFF));
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
			if (!tank01HighLevel && !borewellOn)
			{
				turnOnBorewell();
			}
			break;
		case '2':
			//if (borewellOn)
			//{
				turnOffBorewell();
			//}
			break;
		}
		break;
	case HOLD:
		switch (key)
		{
		case '1':
			if (!tank01HighLevel && !borewellOn)
			{
				turnOnBorewell();
			}
			break;
		case '2':
			//if (borewellOn)
			//{
				turnOffBorewell();
			//}
			break;
		}
		break;
	}
}