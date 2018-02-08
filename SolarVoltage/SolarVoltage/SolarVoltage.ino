#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define SOLAR_BATT_VOLTAGE_NODE
#define NODE_WITH_ON_OFF_FEATURE

#define MY_RADIO_NRF24
#define MY_NODE_ID SOLAR_VOLTAGE_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_02_NODE_ID
#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Solar Voltage"

#define DEFAULT_R1_VALUE 47.30F
#define DEFAULT_R2_VALUE 9.87F
#define DEFAULT_VOLTS 0.00F
#define DEFAULT_SCALE_FACTOR 0.0027F

AlarmId requestTimer;
AlarmId heartbeatTimer;
AlarmId nodeUpTimer;

boolean sendR1Request;
boolean sendR2Request;
boolean sendScaleFactorRequest;
boolean resistorR1Received;
boolean resistorR2Received;
boolean scaleFactorReceived;
boolean sendRawValues;

byte resistorR1RequestCount;
byte resistorR2RequestCount;
byte scaleFactorRequestCount;

float voltsPerBit;
float resistorR1Value;
float resistorR2Value;
float scaleFactor;

MyMessage solarVoltageMessage(SOLAR_VOLTAGE_ID, V_VOLTAGE);
MyMessage rawValuesMessage(SEND_RAW_VALUE_ID, V_STATUS);
MyMessage rawAnalogValueMessage;

void before()
{
	pinMode(VOLTAGE_SENSE_PIN, INPUT);
	pinMode(THRESHOLD_VOLTAGE_PIN, INPUT);
}

void setup()
{
	resistorR1Value = DEFAULT_R1_VALUE;
	resistorR2Value = DEFAULT_R2_VALUE;
	scaleFactor = DEFAULT_SCALE_FACTOR;
	sendR1Request = true;
	sendR2Request = false;
	sendScaleFactorRequest = false;
	resistorR1Received = false;
	resistorR2Received = false;
	scaleFactorReceived = false;
	resistorR1RequestCount = 0;
	resistorR2RequestCount = 0;
	scaleFactorRequestCount = 0;

	sendRawValues = false;
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
	solarVoltageMessage.setDestination(BATT_VOLTAGE_NODE_ID);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(R1_VALUE_ID, S_CUSTOM, "R1 Value");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(R2_VALUE_ID, S_CUSTOM, "R2 Value");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SCALE_FACTOR_ID, S_CUSTOM, "Scale Factor");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(REF_RAW_VALUE_ID, S_CUSTOM, "Reference Raw");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(INP_RAW_VALUE_ID, S_CUSTOM, "Input Raw");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SEND_RAW_VALUE_ID, S_BINARY, "Send Raw Values");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	if (sendRawValues)
		send(rawValuesMessage.set(TURN_ON));
	else
		send(rawValuesMessage.set(TURN_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{
	if (sendR1Request)
	{
		sendR1Request = false;
		request(R1_VALUE_ID, V_VAR1);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkR1RequestStatus);
		resistorR1RequestCount++;
		if (resistorR1RequestCount == 10)
		{
			MyMessage resistorR1ValueMessage(R1_VALUE_ID, V_VAR1);
			send(resistorR1ValueMessage.set(DEFAULT_R1_VALUE, 2));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
	}
	if (sendR2Request)
	{
		sendR2Request = false;
		request(R2_VALUE_ID, V_VAR2);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkR2RequestStatus);
		resistorR2RequestCount++;
		if (resistorR2RequestCount == 10)
		{
			MyMessage resistorR2ValueMessage(R2_VALUE_ID, V_VAR2);
			send(resistorR2ValueMessage.set(DEFAULT_R2_VALUE, 2));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
	}
	if (sendScaleFactorRequest)
	{
		sendScaleFactorRequest = false;
		request(SCALE_FACTOR_ID, V_VAR3);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkScaleFactorRequestStatus);
		scaleFactorRequestCount++;
		if (scaleFactorRequestCount == 10)
		{
			MyMessage scaleFactorMessage(SCALE_FACTOR_ID, V_VAR3);
			send(scaleFactorMessage.set(DEFAULT_SCALE_FACTOR, 5));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
	}
	Alarm.delay(1);
}

void receive(const MyMessage &message)
{
	switch (message.type)
	{
	case V_VAR1:
		resistorR1Value = message.getFloat();
		if (!resistorR1Received)
		{
			resistorR1Received = true;
			Alarm.free(requestTimer);
			sendR1Request = false;
			request(R1_VALUE_ID, V_VAR1);
			sendR2Request = true;
		}
		break;
	case V_VAR2:
		resistorR2Value = message.getFloat();
		if (!resistorR2Received)
		{
			resistorR2Received = true;
			Alarm.free(requestTimer);
			sendR2Request = false;
			request(R2_VALUE_ID, V_VAR2);
			sendScaleFactorRequest = true;
		}
		break;
	case V_VAR3:
		scaleFactor = message.getFloat();
		if (!scaleFactorReceived)
		{
			scaleFactorReceived = true;
			Alarm.free(requestTimer);
			sendScaleFactorRequest = false;
			request(SCALE_FACTOR_ID, V_VAR3);
			sendNodeUpMessage();
			nodeUpTimer = Alarm.timerRepeat(FIVE_MINUTES, sendNodeUpMessage);
		}
		break;
	case V_VOLTAGE:
		if (resistorR1Received && resistorR2Received && scaleFactorReceived)
			readSolarVoltage();
		break;
	case V_STATUS:
		if (message.getInt())
		{
			sendRawValues = true;
			send(rawValuesMessage.set(TURN_ON));
		}
		else
		{
			sendRawValues = false;
			send(rawValuesMessage.set(TURN_OFF));
		}
		wait(WAIT_AFTER_SEND_MESSAGE);
		break;
	}
}

void readSolarVoltage()
{
	float sensedInputVoltage = 0;
	float thresholdVoltage = 0;
	for (byte readCount = 1; readCount <= 10; readCount++)
	{
		thresholdVoltage = thresholdVoltage + analogRead(THRESHOLD_VOLTAGE_PIN);
		wait(WAIT_50MS);
		sensedInputVoltage = sensedInputVoltage + analogRead(VOLTAGE_SENSE_PIN);
		wait(WAIT_50MS);
	}
	thresholdVoltage = thresholdVoltage / 10;
	
	if (sendRawValues)
	{
		rawAnalogValueMessage.setSensor(REF_RAW_VALUE_ID);
		rawAnalogValueMessage.setType(V_VAR4);
		rawAnalogValueMessage.set(thresholdVoltage, 2);
		send(rawAnalogValueMessage);
		wait(WAIT_AFTER_SEND_MESSAGE);
	}
	thresholdVoltage = thresholdVoltage * 5.0 / 1024;

	voltsPerBit = ((thresholdVoltage * (resistorR1Value + resistorR2Value)) / (resistorR2Value * 1024));

	sensedInputVoltage = sensedInputVoltage / 10;

	if (sendRawValues)
	{
		rawAnalogValueMessage.setSensor(INP_RAW_VALUE_ID);
		rawAnalogValueMessage.setType(V_VAR5);
		rawAnalogValueMessage.set(sensedInputVoltage, 2);
		send(rawAnalogValueMessage);
		wait(WAIT_AFTER_SEND_MESSAGE);
	}

	float solarVoltage = 0.00;
	if (scaleFactor < 0.25)
		solarVoltage = (sensedInputVoltage * voltsPerBit) + scaleFactor;
	else
		solarVoltage = (sensedInputVoltage * voltsPerBit);

	send(solarVoltageMessage.set(solarVoltage, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void sendNodeUpMessage()
{
	MyMessage nodeUpMessage(SOLAR_VOLTAGE_ID, V_VAR4);
	nodeUpMessage.setDestination(BATT_VOLTAGE_NODE_ID);
	send(nodeUpMessage.set(UP));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void checkR1RequestStatus()
{
	if (!resistorR1Received)
		sendR1Request = true;
}

void checkR2RequestStatus()
{
	if (!resistorR2Received)
		sendR2Request = true;
}

void checkScaleFactorRequestStatus()
{
	if (!scaleFactorReceived)
		sendScaleFactorRequest = true;
}