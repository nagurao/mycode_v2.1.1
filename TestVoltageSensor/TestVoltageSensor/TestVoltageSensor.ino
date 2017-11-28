#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define MY_RADIO_NRF24
#define MY_NODE_ID 251

#define R1_VALUE_ID 1
#define R2_VALUE_ID 2
#define VOLTAGE_ID 3
#define VOLTAGE_SENSE_PIN A0
#define THRESHOLD_VOLTAGE_PIN A1
#define MOSFET_DRIVER_PIN 3

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Voltage Test"

#define DEFAULT_R1_VALUE 10.00F
#define DEFAULT_R2_VALUE 10.00F
#define DEFAULT_VOLTS 0.00F


AlarmId heartbeatTimer;
AlarmId getVoltageTimer;
AlarmId requestTimer;

boolean sendR1Request;
boolean sendR2Request;

boolean resistorR1Received;
boolean resistorR2Received;

byte resistorR1RequestCount;
byte resistorR2RequestCount;

float voltsPerBit;
float voltage;
float resistorR1Value;
float resistorR2Value;

MyMessage voltageMessage(VOLTAGE_ID, V_VOLTAGE);


void before()
{
	pinMode(VOLTAGE_SENSE_PIN, INPUT);
	pinMode(THRESHOLD_VOLTAGE_PIN, INPUT);
	pinMode(MOSFET_DRIVER_PIN, OUTPUT);
}

void setup()
{
	resistorR1Value = DEFAULT_R1_VALUE;
	resistorR2Value = DEFAULT_R2_VALUE;
	sendR1Request = true;
	sendR2Request = false;
	resistorR1Received = false;
	resistorR2Received = false;
	resistorR1RequestCount = 0;
	resistorR2RequestCount = 0;
	digitalWrite(MOSFET_DRIVER_PIN, LOW);
	voltage = 0;
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);

}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, __DATE__);
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(R1_VALUE_ID, S_CUSTOM, "R1 Value");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(R2_VALUE_ID, S_CUSTOM, "R2 Value");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(VOLTAGE_ID, S_MULTIMETER, "Voltage");
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
			getVoltageTimer = Alarm.timerRepeat(FIVE_MINUTES, getVoltage);
		}
		break;
	}
}

void getVoltage()
{
	float sensedInputVoltage = 0;
	float thresholdVoltage = 0;
	digitalWrite(MOSFET_DRIVER_PIN, HIGH);
	for (byte readCount = 1; readCount <= 10; readCount++)
	{
		thresholdVoltage = thresholdVoltage + analogRead(THRESHOLD_VOLTAGE_PIN);
		wait(WAIT_50MS);
		sensedInputVoltage = sensedInputVoltage + analogRead(VOLTAGE_SENSE_PIN);
		wait(WAIT_50MS);
	}
	thresholdVoltage = thresholdVoltage / 10;
	thresholdVoltage = thresholdVoltage * 5.0 / 1024;

	voltsPerBit = ((thresholdVoltage * (resistorR1Value + resistorR2Value)) / (resistorR2Value * 1024));

	sensedInputVoltage = sensedInputVoltage / 10;
	voltage = (sensedInputVoltage * voltsPerBit);

	send(voltageMessage.set(voltage, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);
	digitalWrite(MOSFET_DRIVER_PIN, LOW);
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
