#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define SOIL_MOISTURE_NODE
#define NODE_INTERACTS_WITH_RELAY

#define MY_RADIO_NRF24
#define MY_NODE_ID TERRACE_SOIL_SENSOR_NODE_ID

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Terrace Soil Sensor"

AlarmId heartbeatTimer;
AlarmId soilMoistureTimer;
AlarmId requestTimer;

float soilMoisture;
float soilMoistureThreshold;
byte soilMoistureThresholdRequestCount;
boolean sendSoilMoistureThresholdRequest;
boolean soilMoistureThresholdReceived;

MyMessage soilMoistureMessage(SOIL_SENSOR_ID, V_LEVEL);
MyMessage valveMessage(VALVE_RELAY_ID, V_STATUS);

void before()
{
	pinMode(SOIL_SENSOR_PIN, INPUT);
	pinMode(VALVE_RELAY_PIN, OUTPUT);
	pinMode(SENSOR_CONTROL_PIN, OUTPUT);
}

void setup()
{
	soilMoisture = 0;
	soilMoistureThresholdRequestCount = 0;
	sendSoilMoistureThresholdRequest = true;
	soilMoistureThresholdReceived = false;

	digitalWrite(VALVE_RELAY_PIN, LOW);
	digitalWrite(SENSOR_CONTROL_PIN, LOW);
	soilMoistureTimer = Alarm.timerRepeat(QUATER_HOUR, initSoilSensor);
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
	Alarm.timerOnce(ONE_MINUTE, initSoilSensor);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SOIL_SENSOR_ID, S_MOISTURE, "Soil Moisture");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(SOIL_MOISTURE_THRESHOLD_ID, S_MOISTURE, "Soil Moisture Threshold");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(VALVE_RELAY_ID, S_BINARY, "Flow Valve");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{
	if (sendSoilMoistureThresholdRequest)
	{
		sendSoilMoistureThresholdRequest = false;
		request(SOIL_MOISTURE_THRESHOLD_ID, V_LEVEL);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkSoilMoistureThresholdStatus);
		soilMoistureThresholdRequestCount++;
		if (soilMoistureThresholdRequestCount == 10)
		{
			MyMessage soilMoistureThresholdMessage(SOIL_MOISTURE_THRESHOLD_ID, V_LEVEL);
			send(soilMoistureThresholdMessage.set(DEFAULT_SOIL_MOISTURE_LEVEL,2));
			wait(WAIT_AFTER_SEND_MESSAGE);
			request(SOIL_MOISTURE_THRESHOLD_ID, V_LEVEL);
			wait(WAIT_AFTER_SEND_MESSAGE);
			soilMoistureThresholdRequestCount = 0;
		}
	}
	Alarm.delay(1);
}

void receive(const MyMessage &message)
{
	switch (message.type)
	{
	case V_LEVEL:
		if (!isnan(message.getFloat()))
		{
			soilMoistureThreshold = message.getFloat();
			if (!soilMoistureThresholdReceived)
				soilMoistureThresholdReceived = true;
		}
		break;
	case V_STATUS:
		if(message.getInt())
			digitalWrite(VALVE_RELAY_PIN, HIGH);
		else
			digitalWrite(VALVE_RELAY_PIN, LOW);
		send(valveMessage.set(digitalRead(VALVE_RELAY_PIN)));
		wait(WAIT_AFTER_SEND_MESSAGE);
		break;
	}
}

void initSoilSensor()
{
	digitalWrite(SENSOR_CONTROL_PIN, HIGH);
	Alarm.timerOnce(TEN_SECS, sendSoilMoisture);
}

void sendSoilMoisture()
{
	soilMoisture = analogRead(SOIL_SENSOR_PIN);
	soilMoisture = map(soilMoisture, ZERO_VALUE, MAX_ANALOG_SENSOR_VALUE, ZERO_VALUE, CENT_VALUE);
	send(soilMoistureMessage.set(soilMoisture,2));
	wait(WAIT_AFTER_SEND_MESSAGE);
	digitalWrite(SENSOR_CONTROL_PIN, LOW);
	if (soilMoisture < soilMoistureThreshold)
		digitalWrite(VALVE_RELAY_PIN, HIGH);
	if (soilMoisture > MAX_SOIL_MOISTURE_LEVEL)
		digitalWrite(VALVE_RELAY_PIN, LOW);
	send(valveMessage.set(digitalRead(VALVE_RELAY_PIN)));
	wait(WAIT_AFTER_SEND_MESSAGE);
}

void checkSoilMoistureThresholdStatus()
{
	if (!soilMoistureThresholdReceived)
		sendSoilMoistureThresholdRequest = true;
}