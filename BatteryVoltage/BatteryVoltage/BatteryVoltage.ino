#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define SOLAR_BATT_VOLTAGE_NODE
#define NODE_INTERACTS_WITH_LCD
#define NODE_HAS_RELAY
#define NODE_WITH_ON_OFF_FEATURE

#define MY_RADIO_NRF24
#define MY_NODE_ID BATT_VOLTAGE_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_02_NODE_ID
#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Battery Voltage"

#define DEFAULT_R1_VALUE 47.70F
#define DEFAULT_R2_VALUE 9.82F
#define DEFAULT_VOLTS 0.00F
#define DEFAULT_SCALE_FACTOR 0.175F //0.28163F

AlarmId heartbeatTimer;
AlarmId getSolarVoltageTimer;
AlarmId getBatteryVoltageTimer;
AlarmId requestSolarVoltageTimer;
AlarmId thingspeakMessageTimer;
AlarmId requestTimer;

byte solarNodeRequestCount;
byte solarVoltageRequestCount;
boolean solarNodeUp;
boolean sendSolarVoltageRequest;
boolean solarVoltageReceived;
boolean sendR1Request;
boolean sendR2Request;
boolean sendScaleFactorRequest;
boolean sendRelayStatusRequest;
boolean sendRawValues;

boolean resistorR1Received;
boolean resistorR2Received;
boolean scaleFactorReceived;
boolean relayStatusReceived;

byte resistorR1RequestCount;
byte resistorR2RequestCount;
byte scaleFactorRequestCount;
byte relayStatusRequestCount;

float voltsPerBit;
float solarVoltage;
float batteryVoltage;
float resistorR1Value;
float resistorR2Value;
float scaleFactor;

MyMessage solarVoltageMessage(SOLAR_VOLTAGE_ID, V_VOLTAGE);
MyMessage batteryVoltageMessage(BATTERY_VOLTAGE_ID, V_VOLTAGE);
MyMessage thingspeakMessage(WIFI_NODEMCU_ID, V_CUSTOM);
MyMessage lcdVoltageMessage;
MyMessage resetRelayMessage(RESET_RELAY_ID, V_STATUS);
MyMessage rawAnalogValueMessage;
MyMessage rawValuesMessage(SEND_RAW_VALUE_ID, V_STATUS);

void before()
{
	pinMode(VOLTAGE_SENSE_PIN, INPUT);
	pinMode(THRESHOLD_VOLTAGE_PIN, INPUT);
	pinMode(RELAY_PIN, OUTPUT);
}

void setup()
{
	digitalWrite(RELAY_PIN, LOW);
	resistorR1Value = DEFAULT_R1_VALUE;
	resistorR2Value = DEFAULT_R2_VALUE;
	scaleFactor = DEFAULT_SCALE_FACTOR;
	sendR1Request = true;
	sendR2Request = false;
	sendScaleFactorRequest = false;
	sendRelayStatusRequest = false;
	resistorR1Received = false;
	resistorR2Received = false;
	scaleFactorReceived = false;
	relayStatusReceived = false;
	sendRawValues = false;
	resistorR1RequestCount = 0;
	resistorR2RequestCount = 0;
	scaleFactorRequestCount = 0;
	relayStatusRequestCount = 0;

	solarVoltage = 0;
	batteryVoltage = 0;
	solarNodeRequestCount = 0;
	solarVoltageRequestCount = 0;
	solarNodeUp = false;
	sendSolarVoltageRequest = true;
	solarVoltageReceived = false;

	thingspeakMessage.setDestination(THINGSPEAK_NODE_ID);
	lcdVoltageMessage.setDestination(LCD_NODE_ID);
	lcdVoltageMessage.setType(V_VOLTAGE);

	thingspeakMessageTimer = Alarm.timerRepeat(QUATER_HOUR, sendThingspeakMessage);
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);

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
	present(BATTERY_VOLTAGE_ID, S_MULTIMETER, "Battery Voltage");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SOLAR_VOLTAGE_ID, S_MULTIMETER, "Solar Voltage");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(RESET_RELAY_ID, S_BINARY, "Reset Relay");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(REF_RAW_VALUE_ID, S_CUSTOM, "Reference Raw");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(INP_RAW_VALUE_ID, S_CUSTOM, "Input Raw");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SEND_RAW_VALUE_ID, S_BINARY, "Send Raw Values");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	send(resetRelayMessage.set(RELAY_OFF));
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	request(SOLAR_VOLTAGE_ID, V_VOLTAGE, SOLAR_VOLTAGE_NODE_ID);
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	if(sendRawValues)
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
	if (sendRelayStatusRequest)
	{
		sendRelayStatusRequest = false;
		request(RESET_RELAY_ID, V_STATUS);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkRelayStatusRequestStatus);
		relayStatusRequestCount++;
		if (relayStatusRequestCount == 10)
		{
			digitalWrite(RELAY_PIN, RELAY_OFF);
			send(resetRelayMessage.set(RELAY_OFF));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
	}
	if (sendSolarVoltageRequest)
	{
		sendSolarVoltageRequest = false;
		request(SOLAR_VOLTAGE_ID, V_VOLTAGE, SOLAR_VOLTAGE_NODE_ID);
		requestSolarVoltageTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkSolarVolategRequestStatus);
		solarVoltageRequestCount++;
		if (solarVoltageRequestCount == 10)
		{
			solarVoltage = DEFAULT_VOLTS;
			sendSolarVoltage();
			Alarm.free(requestSolarVoltageTimer);
			solarVoltageReceived = true;
			solarVoltageRequestCount = 0;
			solarNodeRequestCount++;
		}
		if (solarNodeRequestCount == 20)
		{
			solarNodeUp = false;
			Alarm.free(getSolarVoltageTimer);
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
			getBatteryVoltageTimer = Alarm.timerRepeat(FIVE_MINUTES, getBatteryVoltage);
			sendRelayStatusRequest = true;
		}
		break;
	case V_VAR4:
		if (message.getInt() == UP)
		{
			if (!solarNodeUp)
			{
				solarNodeUp = true;
				getSolarVoltageTimer = Alarm.timerRepeat(FIVE_MINUTES, getSolarVoltage);
			}
		}
		break;
	case V_VOLTAGE:
		solarVoltage = message.getFloat();
		sendSolarVoltage();

		if (!solarVoltageReceived)
		{
			Alarm.free(requestSolarVoltageTimer);
			solarVoltageReceived = true;
			solarNodeRequestCount = 0;
			solarVoltageRequestCount = 0;
		}
		break;
	case V_STATUS:
		switch (message.sensor)
		{
		case RESET_RELAY_ID:
			if (message.getInt())
			{
				digitalWrite(RELAY_PIN, RELAY_ON);
				send(resetRelayMessage.set(RELAY_ON));
				wait(WAIT_AFTER_SEND_MESSAGE);
			}
			else
			{
				digitalWrite(RELAY_PIN, RELAY_OFF);
				send(resetRelayMessage.set(RELAY_OFF));
				wait(WAIT_AFTER_SEND_MESSAGE);
			}
			if (!relayStatusReceived)
			{
				relayStatusReceived = true;
				Alarm.free(requestTimer);
				sendRelayStatusRequest = false;
				request(RESET_RELAY_ID, V_STATUS);
			}
			break;
		case SEND_RAW_VALUE_ID:
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
}

void getBatteryVoltage()
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

	if (scaleFactor < 0.25)
		batteryVoltage = (sensedInputVoltage * voltsPerBit) + scaleFactor;
	else
		batteryVoltage = (sensedInputVoltage * voltsPerBit);

	send(batteryVoltageMessage.set(batteryVoltage, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);
	lcdVoltageMessage.setSensor(BATTERY_VOLTAGE_ID);
	send(lcdVoltageMessage.set(batteryVoltage, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void getSolarVoltage()
{
	sendSolarVoltageRequest = true;
	solarVoltageReceived = false;
}

void checkSolarVolategRequestStatus()
{
	if (!solarVoltageReceived)
		sendSolarVoltageRequest = true;
}

void sendSolarVoltage()
{
	send(solarVoltageMessage.set(solarVoltage, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);
	lcdVoltageMessage.setSensor(SOLAR_VOLTAGE_ID);
	send(lcdVoltageMessage.set(solarVoltage, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void sendThingspeakMessage()
{
	thingspeakMessage.setSensor(SOLAR_VOLTAGE_ID);
	send(thingspeakMessage.set(solarVoltage, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);
	thingspeakMessage.setSensor(BATTERY_VOLTAGE_ID);
	send(thingspeakMessage.set(batteryVoltage, 5));
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

void checkRelayStatusRequestStatus()
{
	if (!relayStatusReceived)
		sendRelayStatusRequest = true;
}