#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define WATT_METER_NODE
#define NODE_INTERACTS_WITH_LCD

#define MY_RADIO_NRF24
#define MY_NODE_ID INV_IN_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_02_NODE_ID
#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#define APPLICATION_NAME "Inverter In"

#define DEFAULT_BLINKS_PER_KWH 6400 // value from energy meter
AlarmId heartbeatTimer;
AlarmId requestTimer;
AlarmId accumulationTimer;
AlarmId updateConsumptionTimer;

boolean sendPulseCountRequest;
boolean pulseCountReceived;
byte pulseCountRequestCount;

float pulseFactor;

volatile unsigned long currPulseCount;
volatile unsigned long prevPulseCount;
volatile float currWatt;
volatile float prevWatt;
volatile unsigned long lastBlink;

float accumulatedKWH;
float hourlyConsumptionInitKWH;
float unitsPerHour;
float dailyConsumptionInitKWH;
float unitsPerDay;
float monthlyConsumptionInitKWH;
float unitsPerMonth;
float deltaUnitsTillDay;

MyMessage currentConsumptionMessage(CURR_WATT_ID, V_WATT);
MyMessage accumulatedKWMessage(ACCUMULATED_WATT_CONSUMPTION_ID, V_KWH);
MyMessage pulseCountMessage(CURR_PULSE_COUNT_ID, V_VAR1);
MyMessage thingspeakMessage(WIFI_NODEMCU_ID, V_CUSTOM);

void before()
{
	pulseFactor = DEFAULT_BLINKS_PER_KWH / 1000;
	resetAll();
	attachInterrupt(INTERRUPT_PULSE, onPulse, RISING);
}

void setup()
{
	sendPulseCountRequest = true;
	pulseCountReceived = false;
	pulseCountRequestCount = 0;
	lastBlink = 0;
	prevWatt = 0.00;
	currWatt = 0.00;

	thingspeakMessage.setDestination(THINGSPEAK_NODE_ID);
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(CURR_WATT_ID, S_POWER, "Current Consumption");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(ACCUMULATED_WATT_CONSUMPTION_ID, S_POWER, "Accumulated Consumption");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(HOURLY_KWATT_INIT_ID, S_POWER, "Hourly kWh Init");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(HOURLY_WATT_CONSUMPTION_ID, S_POWER, "Hourly Consumption");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(DAILY_KWATT_INIT_ID, S_POWER, "Daily kWh Init");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(DAILY_WATT_CONSUMPTION_ID, S_POWER, "Daily Consumption");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(MONTHLY_KWATT_INIT_ID, S_POWER, "Monthly kWh Init");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(MONTHLY_WATT_CONSUMPTION_ID, S_POWER, "Monthly Consumption");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(DELTA_WATT_CONSUMPTION_ID, S_POWER, "Delta Consumption");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(CURR_PULSE_COUNT_ID, S_CUSTOM, "Pulse Count");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(RESET_TYPE_ID, S_CUSTOM, "Reset Consumption");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{
	if (sendPulseCountRequest)
	{
		sendPulseCountRequest = false;
		request(CURR_PULSE_COUNT_ID, V_VAR1);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkPulseCountRequestStatus);
		pulseCountRequestCount++;
		if (pulseCountRequestCount == 10)
		{
			send(pulseCountMessage.set(ZERO_PULSE));
		}
	}
	Alarm.delay(1);
}
void receive(const MyMessage &message)
{
	switch (message.type)
	{
	case V_VAR1:
		currPulseCount = currPulseCount + message.getLong();
		if (!pulseCountReceived)
		{
			pulseCountReceived = true;
			Alarm.free(requestTimer);
			accumulationTimer = Alarm.timerRepeat(ACCUMULATION_FREQUENCY_SECS, updateConsumptionData);
			updateConsumptionTimer = Alarm.timerRepeat(FIVE_MINUTES, sendAccumulationData);
		}
		break;
	case V_VAR2:
		switch (message.getInt())
		{
		case RESET_ALL:
			resetAll();
			break;
		}
	case V_KWH:
		switch (message.sensor)
		{
		case ACCUMULATED_WATT_CONSUMPTION_ID:
			accumulatedKWH = message.getFloat();
			break;
		case HOURLY_KWATT_INIT_ID:
			hourlyConsumptionInitKWH = message.getFloat();
			break;
		case HOURLY_WATT_CONSUMPTION_ID:
			unitsPerHour = message.getFloat();
			thingspeakMessage.setSensor(HOURLY_WATT_CONSUMPTION_ID);
			send(thingspeakMessage.set(unitsPerHour, 5));
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		case DAILY_KWATT_INIT_ID:
			dailyConsumptionInitKWH = message.getFloat();
			break;
		case DAILY_WATT_CONSUMPTION_ID:
			unitsPerDay = message.getFloat();
			thingspeakMessage.setSensor(DAILY_WATT_CONSUMPTION_ID);
			send(thingspeakMessage.set(unitsPerDay, 5));
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		case MONTHLY_KWATT_INIT_ID:
			monthlyConsumptionInitKWH = message.getFloat();
			break;
		case MONTHLY_WATT_CONSUMPTION_ID:
			unitsPerMonth = message.getFloat();
			thingspeakMessage.setSensor(DAILY_WATT_CONSUMPTION_ID);
			send(thingspeakMessage.set(unitsPerDay, 5));
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		case DELTA_WATT_CONSUMPTION_ID:
			deltaUnitsTillDay = message.getFloat();
			thingspeakMessage.setSensor(DELTA_WATT_CONSUMPTION_ID);
			send(thingspeakMessage.set(deltaUnitsTillDay, 5));
			wait(WAIT_AFTER_SEND_MESSAGE);
			break;
		}
		break;
	}
}
void onPulse()
{
	unsigned long newBlink = micros();
	unsigned long interval = newBlink - lastBlink;
	if (interval < 10000L)
	{
		return;
	}
	currWatt = (3600000000.0 / interval) / pulseFactor;
	lastBlink = newBlink;
	currPulseCount++;
}

void updateConsumptionData()
{
	if (currWatt != prevWatt)
	{
		if (currWatt < MAX_WATT_INVERTER)
			send(currentConsumptionMessage.set(currWatt, 2));
		prevWatt = currWatt;
	}
	if (currPulseCount != prevPulseCount)
	{
		send(pulseCountMessage.set(currPulseCount));
		prevPulseCount = currPulseCount;
		float currAccumulatedKWH = ((float)currPulseCount / ((float)DEFAULT_BLINKS_PER_KWH));
		if (currAccumulatedKWH != accumulatedKWH)
		{
			send(accumulatedKWMessage.set(currAccumulatedKWH, 5));
			accumulatedKWH = currAccumulatedKWH;
		}
	}
}

void resetAll()
{
	currPulseCount = 0;
	accumulatedKWH = 0.00;
	hourlyConsumptionInitKWH = 0.00;
	unitsPerHour = 0.00;
	dailyConsumptionInitKWH = 0.00;
	unitsPerDay = 0.00;
	monthlyConsumptionInitKWH = 0.00;
	unitsPerMonth = 0.00;
	deltaUnitsTillDay = 0.00;
}

void checkPulseCountRequestStatus()
{
	if (!pulseCountReceived)
		sendPulseCountRequest = true;
}

void sendAccumulationData()
{
	thingspeakMessage.setSensor(CURR_WATT_ID);
	send(thingspeakMessage.set(currWatt, 5));
	wait(WAIT_AFTER_SEND_MESSAGE);

	MyMessage lcdNodeMessage;
	lcdNodeMessage.setSensor(INV_IN_CURR_WATT_ID);
	lcdNodeMessage.setType(V_WATT);
	lcdNodeMessage.set(currWatt, 2);
	send(lcdNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);
}