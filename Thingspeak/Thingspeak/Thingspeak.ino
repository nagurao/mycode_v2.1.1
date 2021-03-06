#define PRINT_DEBUG_MESSAGES
#define PRINT_HTTP
#include <ESP8266WiFi.h>
#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <ThingSpeak.h>
#include <SPI.h>

#define OTA_UPDATE_FEATURE

#define WIFI_NODE
#define LIGHT_NODE
#define WATT_METER_NODE
#define SOLAR_BATT_VOLTAGE_NODE
#define NODE_INTERACTS_WITH_RELAY
#define NODE_INTERACTS_WITH_WIFI_AND_LCD
#define STATUS_LEDS_MODEMCU

#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_NODE_ID THINGSPEAK_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_01_NODE_ID

#include <MyNodes.h>
#include <MySensors.h>
#include <MyConfig.h>

#if defined OTA_UPDATE_FEATURE
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#endif

#include <MyWifi.h>

#define APPLICATION_NAME "Thingspeak Node"

//water level logging channel
#define TANK_01_FIELD 1
#define TANK_02_FILED 2
#define TANK_03_FIELD 3
//
#define TANK_01_IDX 0
#define TANK_02_IDX 1
#define TANK_03_IDX 2

//power hourly logging channel
#define INV_IN_HOURLY_FIELD 1
#define INV_OUT_HOURLY_FIELD 2
#define PH3_HOURLY_FIELD 3
#define PH1_HOURLY_FIELD 4
//
#define INV_IN_HOURLY_IDX 0
#define INV_OUT_HOURLY_IDX 1
#define PH3_HOURLY_IDX 2
#define PH1_HOURLY_IDX 3

//power daily logging channel
#define INV_IN_DAILY_FIELD 1
#define INV_OUT_DAILY_FIELD 2
#define PH3_DAILY_FIELD 3
#define PH1_DAILY_FIELD 4
#define INV_IN_OUT_DELTA_DAILY_FIELD 5
#define PH3_PH1_DELTA_DAILY_FIELD 6
//
#define INV_IN_DAILY_IDX 0
#define INV_OUT_DAILY_IDX 1
#define PH3_DAILY_IDX 2
#define PH1_DAILY_IDX 3
#define INV_IN_OUT_DELTA_DAILY_IDX 4
#define PH3_PH1_DELTA_DAILY_IDX 5

//power monthly logging channel
#define INV_IN_MONTHLY_FIELD 1
#define INV_OUT_MONTHLY_FIELD 2
#define PH3_MONTHLY_FIELD 3
#define PH1_MONTHLY_FIELD 4
//
#define INV_IN_MONTHLY_IDX 0
#define INV_OUT_MONTHLY_IDX 1
#define PH3_MONTHLY_IDX 2
#define PH1_MONTHLY_IDX 3

//power inverter realtime
#define INV_IN_CURR_WATT_FIELD 1
#define INV_OUT_CURR_WATT_FIELD 2
#define INV_IN_OUT_REAL_TIME_DELTA_FIELD 3
//
#define INV_IN_CURR_WATT_IDX 0
#define INV_OUT_CURR_WATT_IDX 1
#define INV_IN_OUT_REAL_TIME_DELTA_IDX 2

//power watt meter realtime
#define PH3_CURR_WATT_FIELD 1
#define PH1_CURR_WATT_FIELD 2
#define PH3_PH1_REAL_TIME_DELTA_FIELD 3
//
#define PH3_CURR_WATT_IDX 0
#define PH1_CURR_WATT_IDX 1
#define PH3_PH1_REAL_TIME_DELTA_IDX 2


//static data logging channel
#define BALCONY_LIGHTS_FIELD 1
#define STAIRCASE_LIGHTS_FIELD 2
#define GATE_LIGHTS_FIELD 3
#define BOREWELL_FIELD 4
#define SUMP_MOTOR_FIELD 5
//
#define BALCONY_LIGHTS_IDX 0
#define STAIRCASE_LIGHTS_IDX 1
#define GATE_LIGHTS_IDX 2
#define BOREWELL_IDX 3
#define SUMP_MOTOR_IDX 4

//battery & solar voltage
#define BATTERY_VOLT_FIELD 1
#define SOLAR_VOLT_FIELD 2
//
#define BATTERY_VOLT_IDX 0
#define SOLAR_VOLT_IDX 1

#define IN_BALCONY_LIGHT_IDX 0
#define IN_GATE_LIGHT_IDX 1
#define IN_BOREWELL_ON_IDX 2
#define IN_BOREWELL_OFF_IDX 3
#define IN_SUMP_MOTOR_IDX 4

#define THINGSPEAK_INTERVAL 60
#define DEFAULT_CHANNEL_VALUE -99.00
#define DEFAULT_CHANNEL_VALUE_INT 0

#define SEND_STATIC_DATA 0
#define SEND_WATER_LEVEL_DATA 1
#define SEND_VOLTAGE_DATA 2
#define SEND_POWER_HOURLY_DATA 3
#define SEND_POWER_DAILY_DATA 4
#define SEND_POWER_MONTHLY_DATA 5
#define SEND_REALTIME_INVERTER_DATA 6
#define SEND_REALTIME_WATTMETER_DATA 7
#define FETCH_AND_PROCESS_DATA 8
#define TYPES_OF_DATA 9

#define FIELDS_PER_CHANNEL 8
#define DEFAULT_QUEUE_VALUE 127

byte processQueue[TYPES_OF_DATA];
byte processQueueHead;
byte processQueueTail;

float waterLevelChannelData[FIELDS_PER_CHANNEL];
float staticChannelData[FIELDS_PER_CHANNEL];
float voltageChannelData[FIELDS_PER_CHANNEL];
float powerHourlyChannelData[FIELDS_PER_CHANNEL];
float powerDailyChannelData[FIELDS_PER_CHANNEL];
float powerMonthlyChannelData[FIELDS_PER_CHANNEL];
float powerRealtimeInverterChannelData[FIELDS_PER_CHANNEL];
float powerRealtimeWattmeterChannelData[FIELDS_PER_CHANNEL];
unsigned int incomingChannelData[FIELDS_PER_CHANNEL];

int status = WL_IDLE_STATUS;
WiFiClient  client;

boolean incomingDataFound;

AlarmId heartbeatTimer;
AlarmId thingspeakTimer;
AlarmId incomingDataTimer;

MyMessage lightNodeMessage(LIGHT_RELAY_ID, V_STATUS);
MyMessage borewellNodeMessage;
MyMessage sumpMotorMessage(RELAY_ID, V_STATUS);
MyMessage lcdNodeMessage;
MyMessage currWattMessage;
MyMessage currDeltaMessage;

AlarmId requestTimer;
AlarmId sunriseTimer;
AlarmId sunsetTimer;
AlarmId nightTimer;

byte currModeRequestCount;
boolean firstTime;
boolean sendSunriseRequest;
boolean sendSunsetRequest;
boolean sunriseTimeReceived;
boolean sunsetTimeReceived;
uint32_t sunriseSeconds;
uint32_t sunsetSeconds;
char mytime[6];
MyMessage sunriseTimeMessage(SUNRISE_TIME_ID, V_VAR1);
MyMessage sunsetTimeMessage(SUNSET_TIME_ID, V_VAR2);
MyMessage resetRelayMessage(RESET_RELAY_ID, V_STATUS);

char *codeVersion;

void before()
{
#if defined OTA_UPDATE_FEATURE
	WiFi.mode(WIFI_AP_STA);
#endif
	WiFi.begin(ssid, pass);
	ThingSpeak.begin(client);
#if defined OTA_UPDATE_FEATURE
	MDNS.begin(host);
	httpUpdater.setup(&httpServer, update_path, update_username, update_password);
	httpServer.begin();
	MDNS.addService("http", "tcp", 80);
#endif

}

void setup()
{
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, heartbeat);
	thingspeakTimer = Alarm.timerRepeat(THINGSPEAK_INTERVAL, processThingspeakData);
	incomingDataTimer = Alarm.timerRepeat(FIVE_MINUTES, insertFetchAndProcessDataRequest);
	Alarm.timerOnce(ONE_MINUTE, insertFetchAndProcessDataRequest);
	Alarm.timerRepeat(ONE_HOUR * 3, getTimeFromGateway);

	for (byte channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
	{
		waterLevelChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		staticChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		voltageChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		powerHourlyChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		powerDailyChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		powerMonthlyChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		powerRealtimeInverterChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		powerRealtimeWattmeterChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
		incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
	}
	incomingDataFound = false;
	lcdNodeMessage.setDestination(LCD_NODE_ID);
	processQueueHead = 0;
	processQueueTail = 0;
	for (byte index = 0; index < TYPES_OF_DATA; index++)
		processQueue[index] = DEFAULT_QUEUE_VALUE;

	currModeRequestCount = 0;
	sendSunriseRequest = true;
	sendSunsetRequest = false;
	sunriseTimeReceived = false;
	sunsetTimeReceived = false;
	sunriseSeconds = DEFAULT_SUNRISE_SUNSET_TIME;
	sunsetSeconds = DEFAULT_SUNRISE_SUNSET_TIME;
	sunriseSeconds = DEFAULT_SUNRISE_SUNSET_TIME;
	sunsetSeconds = DEFAULT_SUNRISE_SUNSET_TIME;
	firstTime = true;
	//Blynk.begin(auth);
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, __DATE__);
	wait(WAIT_AFTER_SEND_MESSAGE);
	present(SUNRISE_TIME_ID, S_CUSTOM, "Sunrise Time (sec)");
	wait(WAIT_AFTER_SEND_MESSAGE);
	present(SUNSET_TIME_ID, S_CUSTOM, "Sunset Time (sec)");
	wait(WAIT_AFTER_SEND_MESSAGE);
	present(LAST_UPDATE_TIME_ID, S_CUSTOM, "Last Update");
	wait(WAIT_AFTER_SEND_MESSAGE);
	present(SUNRISE_TRIGGER_TIME_ID, S_CUSTOM, "Sunrise Trigger Time");
	wait(WAIT_AFTER_SEND_MESSAGE);
	present(SUNSET_TRIGGER_TIME_ID, S_CUSTOM, "Sunset Trigger Time");
	wait(WAIT_AFTER_SEND_MESSAGE);
	requestTime();
}

void loop()
{
#if defined OTA_UPDATE_FEATURE
	httpServer.handleClient();
#endif
	if (sendSunriseRequest)
	{
		sendSunriseRequest = false;
		request(SUNRISE_TIME_ID, V_VAR1);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkSunRiseRequestStatus);
		currModeRequestCount++;
		if (currModeRequestCount == 10)
		{
			send(sunriseTimeMessage.set(DEFAULT_SUNRISE_SUNSET_TIME));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}

	}
	if (sendSunsetRequest)
	{
		sendSunsetRequest = false;
		request(SUNSET_TIME_ID, V_VAR2);
		requestTimer = Alarm.timerOnce(REQUEST_INTERVAL, checkSunSetRequestStatus);
		currModeRequestCount++;
		if (currModeRequestCount == 10)
		{
			send(sunsetTimeMessage.set(DEFAULT_SUNRISE_SUNSET_TIME));
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
	}
	//Blynk.run();
	Alarm.delay(1);
}

void receive(const MyMessage &message)
{
	switch (message.type)
	{
	case V_CUSTOM:
		switch (message.sender)
		{
		case BALCONYLIGHT_NODE_ID:
			staticChannelData[BALCONY_LIGHTS_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case STAIRCASE_LIGHT_NODE_ID:
			staticChannelData[STAIRCASE_LIGHTS_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case GATELIGHT_NODE_ID:
			staticChannelData[GATE_LIGHTS_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case TANK_01_NODE_ID:
			waterLevelChannelData[TANK_01_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case TANK_02_NODE_ID:
			waterLevelChannelData[TANK_02_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case TANK_03_NODE_ID:
			waterLevelChannelData[TANK_03_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case BOREWELL_NODE_ID:
			staticChannelData[BOREWELL_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case SUMP_MOTOR_NODE_ID:
			staticChannelData[SUMP_MOTOR_IDX] = message.getInt();
			insertQueue(SEND_STATIC_DATA);
			break;
		case BATT_VOLTAGE_NODE_ID:
			switch (message.sensor)
			{
			case BATTERY_VOLTAGE_ID:
				voltageChannelData[BATTERY_VOLT_IDX] = message.getFloat();
				insertQueue(SEND_VOLTAGE_DATA);
				lcdNodeMessage.setSensor(BATTERY_VOLTAGE_ID);
				lcdNodeMessage.setType(V_VOLTAGE);
				lcdNodeMessage.set(voltageChannelData[BATTERY_VOLT_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			case SOLAR_VOLTAGE_ID:
				voltageChannelData[SOLAR_VOLT_IDX] = message.getFloat();
				insertQueue(SEND_VOLTAGE_DATA);
				lcdNodeMessage.setSensor(SOLAR_VOLTAGE_ID);
				lcdNodeMessage.setType(V_VOLTAGE);
				lcdNodeMessage.set(voltageChannelData[SOLAR_VOLT_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			}
			break;
		case SOLAR_VOLTAGE_NODE_ID:
			break;
		case INV_IN_NODE_ID:
			switch (message.sensor)
			{
			case CURR_WATT_ID:
				powerRealtimeInverterChannelData[INV_IN_CURR_WATT_IDX] = message.getFloat();
				insertQueue(SEND_REALTIME_INVERTER_DATA);
				lcdNodeMessage.setSensor(INV_IN_CURR_WATT_ID);
				lcdNodeMessage.setType(V_WATT);
				lcdNodeMessage.set(powerRealtimeInverterChannelData[INV_IN_CURR_WATT_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			case HOURLY_WATT_CONSUMPTION_ID:
				powerHourlyChannelData[INV_IN_HOURLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_HOURLY_DATA);
				break;
			case DAILY_WATT_CONSUMPTION_ID:
				powerDailyChannelData[INV_IN_DAILY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_DAILY_DATA);
				break;
			case MONTHLY_WATT_CONSUMPTION_ID:
				powerMonthlyChannelData[INV_IN_MONTHLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_MONTHLY_DATA);
				break;
			case DELTA_WATT_CONSUMPTION_ID:
				powerDailyChannelData[INV_IN_OUT_DELTA_DAILY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_DAILY_DATA);
				break;
			}
			break;
		case INV_OUT_NODE_ID:
			switch (message.sensor)
			{
			case CURR_WATT_ID:
				powerRealtimeInverterChannelData[INV_OUT_CURR_WATT_IDX] = message.getFloat();
				insertQueue(SEND_REALTIME_INVERTER_DATA);
				lcdNodeMessage.setSensor(INV_OUT_CURR_WATT_ID);
				lcdNodeMessage.setType(V_WATT);
				lcdNodeMessage.set(powerRealtimeInverterChannelData[INV_OUT_CURR_WATT_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			case HOURLY_WATT_CONSUMPTION_ID:
				powerHourlyChannelData[INV_OUT_HOURLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_HOURLY_DATA);
				break;
			case DAILY_WATT_CONSUMPTION_ID:
				powerDailyChannelData[INV_OUT_DAILY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_DAILY_DATA);
				break;
			case MONTHLY_WATT_CONSUMPTION_ID:
				powerMonthlyChannelData[INV_OUT_MONTHLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_MONTHLY_DATA);
				break;
			case DELTA_WATT_CONSUMPTION_ID:
				powerRealtimeInverterChannelData[INV_IN_OUT_REAL_TIME_DELTA_IDX] = message.getFloat();
				insertQueue(SEND_REALTIME_INVERTER_DATA);
				lcdNodeMessage.setSensor(INV_IN_OUT_DELTA_ID);
				lcdNodeMessage.setType(V_KWH);
				lcdNodeMessage.set(powerRealtimeInverterChannelData[INV_IN_OUT_REAL_TIME_DELTA_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			}
			break;
		case PH3_NODE_ID:
			switch (message.sensor)
			{
			case CURR_WATT_ID:
				powerRealtimeWattmeterChannelData[PH3_CURR_WATT_IDX] = message.getFloat();
				insertQueue(SEND_REALTIME_WATTMETER_DATA);
				lcdNodeMessage.setSensor(PH3_CURR_WATT_ID);
				lcdNodeMessage.setType(V_WATT);
				lcdNodeMessage.set(powerRealtimeWattmeterChannelData[PH3_CURR_WATT_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			case HOURLY_WATT_CONSUMPTION_ID:
				powerHourlyChannelData[PH3_HOURLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_HOURLY_DATA);
				break;
			case DAILY_WATT_CONSUMPTION_ID:
				powerDailyChannelData[PH3_DAILY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_DAILY_DATA);
				break;
			case MONTHLY_WATT_CONSUMPTION_ID:
				powerMonthlyChannelData[PH3_MONTHLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_MONTHLY_DATA);
				break;
			case DELTA_WATT_CONSUMPTION_ID:
				powerDailyChannelData[PH3_PH1_DELTA_DAILY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_DAILY_DATA);
				break;
			}
			break;
		case PH1_NODE_ID:
			switch (message.sensor)
			{
			case CURR_WATT_ID:
				powerRealtimeWattmeterChannelData[PH1_CURR_WATT_IDX] = message.getFloat();
				insertQueue(SEND_REALTIME_WATTMETER_DATA);
				lcdNodeMessage.setSensor(PH1_CURR_WATT_ID);
				lcdNodeMessage.setType(V_WATT);
				lcdNodeMessage.set(powerRealtimeWattmeterChannelData[PH1_CURR_WATT_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			case HOURLY_WATT_CONSUMPTION_ID:
				powerHourlyChannelData[PH1_HOURLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_HOURLY_DATA);
				break;
			case DAILY_WATT_CONSUMPTION_ID:
				powerDailyChannelData[PH1_DAILY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_DAILY_DATA);
				break;
			case MONTHLY_WATT_CONSUMPTION_ID:
				powerMonthlyChannelData[PH1_MONTHLY_IDX] = message.getFloat();
				insertQueue(SEND_POWER_MONTHLY_DATA);
				break;
			case DELTA_WATT_CONSUMPTION_ID:
				powerRealtimeWattmeterChannelData[PH3_PH1_REAL_TIME_DELTA_IDX] = message.getFloat();
				insertQueue(SEND_REALTIME_WATTMETER_DATA);
				lcdNodeMessage.setSensor(PH3_PH1_DELTA_ID);
				lcdNodeMessage.setType(V_KWH);
				lcdNodeMessage.set(powerRealtimeWattmeterChannelData[PH3_PH1_REAL_TIME_DELTA_IDX], 2);
				send(lcdNodeMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			}
			break;
		}
		break;
	case V_VAR1:
		if (!sunriseTimeReceived)
		{
			sunriseTimeReceived = true;
			sendSunsetRequest = true;
			sunriseSeconds = message.getLong();
			Alarm.free(requestTimer);
			request(SUNRISE_TIME_ID, V_VAR1);
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
		else
			sunriseSeconds = message.getLong();

		send(sunriseTimeMessage.set(sunriseSeconds));
		wait(WAIT_AFTER_SEND_MESSAGE);
		if (sunriseSeconds != 0)
		{
			sunriseSeconds = (sunriseSeconds + QUATER_HOUR_OFFSET);
			int sunriseHr = hour(sunriseSeconds);
			int sunriseMin = minute(sunriseSeconds);
			int sunriseSecs = second(sunriseSeconds);
			if (Alarm.isAllocated(sunriseTimer))
				Alarm.free(sunriseTimer);
			sunriseTimer = Alarm.alarmRepeat(sunriseHr, sunriseMin, sunriseSecs, sunriseTriggerMessage);
			MyMessage sunriseTriggerTimeMessage(SUNRISE_TRIGGER_TIME_ID, V_VAR4);
			formatTimeForGateway(sunriseHr, sunriseMin);
			send(sunriseTriggerTimeMessage.set(mytime));
			wait(WAIT_AFTER_SEND_MESSAGE);
			resetTimeForGateway();
		}
		break;
	case V_VAR2:
		if (!sunsetTimeReceived)
		{
			sunsetTimeReceived = true;
			sunsetSeconds = message.getLong();
			Alarm.free(requestTimer);
			request(SUNSET_TIME_ID, V_VAR2);
			wait(WAIT_AFTER_SEND_MESSAGE);
		}
		else
			sunsetSeconds = message.getLong();
		send(sunsetTimeMessage.set(sunsetSeconds));
		wait(WAIT_AFTER_SEND_MESSAGE);
		if (sunsetSeconds != 0)
		{
			sunsetSeconds = (sunsetSeconds + QUATER_HOUR_OFFSET);
			int sunsetHr = hour(sunsetSeconds);
			int sunsetMin = minute(sunsetSeconds);
			int sunsetSecs = second(sunsetSeconds);
			if (Alarm.isAllocated(sunsetTimer))
				Alarm.free(sunsetTimer);
			sunsetTimer = Alarm.alarmRepeat(sunsetHr, sunsetMin, sunsetSecs, sunsetTriggerMessage);
			MyMessage sunsetTriggerTimeMessage(SUNSET_TRIGGER_TIME_ID, V_VAR5);
			formatTimeForGateway(sunsetHr, sunsetMin);
			send(sunsetTriggerTimeMessage.set(mytime));
			wait(WAIT_AFTER_SEND_MESSAGE);
			resetTimeForGateway();
		}

		break;
	}
}

void processThingspeakData()
{
	byte channelId;
	switch (processQueue[processQueueHead])
	{
	case SEND_STATIC_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (staticChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case BALCONY_LIGHTS_IDX:
					ThingSpeak.setField(BALCONY_LIGHTS_FIELD, staticChannelData[channelId]);
					break;
				case STAIRCASE_LIGHTS_IDX:
					ThingSpeak.setField(STAIRCASE_LIGHTS_FIELD, staticChannelData[channelId]);
					break;
				case GATE_LIGHTS_IDX:
					ThingSpeak.setField(GATE_LIGHTS_FIELD, staticChannelData[channelId]);
					break;
				case BOREWELL_IDX:
					ThingSpeak.setField(BOREWELL_FIELD, staticChannelData[channelId]);
					break;
				case SUMP_MOTOR_IDX:
					ThingSpeak.setField(SUMP_MOTOR_FIELD, staticChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(staticChannelNumber, staticWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (staticChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					staticChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case SEND_WATER_LEVEL_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (waterLevelChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case TANK_01_IDX:
					ThingSpeak.setField(TANK_01_FIELD, waterLevelChannelData[channelId]);
					break;
				case TANK_02_IDX:
					ThingSpeak.setField(TANK_02_FILED, waterLevelChannelData[channelId]);
					break;
				case TANK_03_IDX:
					ThingSpeak.setField(TANK_03_FIELD, waterLevelChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(waterLevelChannelNumber, waterLevelWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (waterLevelChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					waterLevelChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case SEND_VOLTAGE_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (voltageChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case BATTERY_VOLT_IDX:
					ThingSpeak.setField(BATTERY_VOLT_FIELD, voltageChannelData[channelId]);
					break;
				case SOLAR_VOLT_IDX:
					ThingSpeak.setField(SOLAR_VOLT_FIELD, voltageChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(voltageChannelNumber, voltageWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (voltageChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					voltageChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case SEND_POWER_HOURLY_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (powerHourlyChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case INV_IN_HOURLY_IDX:
					ThingSpeak.setField(INV_IN_HOURLY_FIELD, powerHourlyChannelData[channelId]);
					break;
				case INV_OUT_HOURLY_IDX:
					ThingSpeak.setField(INV_OUT_HOURLY_FIELD, powerHourlyChannelData[channelId]);
					break;
				case PH3_HOURLY_IDX:
					ThingSpeak.setField(PH3_HOURLY_FIELD, powerHourlyChannelData[channelId]);
					break;
				case PH1_HOURLY_IDX:
					ThingSpeak.setField(PH1_HOURLY_FIELD, powerHourlyChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(powerHourlyChannelNumber, powerHourlyWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (powerHourlyChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					powerHourlyChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case SEND_POWER_DAILY_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (powerDailyChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case INV_IN_DAILY_IDX:
					ThingSpeak.setField(INV_IN_HOURLY_FIELD, powerDailyChannelData[channelId]);
					break;
				case INV_OUT_DAILY_IDX:
					ThingSpeak.setField(INV_OUT_HOURLY_FIELD, powerDailyChannelData[channelId]);
					break;
				case PH3_DAILY_IDX:
					ThingSpeak.setField(PH3_HOURLY_FIELD, powerDailyChannelData[channelId]);
					break;
				case PH1_DAILY_IDX:
					ThingSpeak.setField(PH1_HOURLY_FIELD, powerDailyChannelData[channelId]);
					break;
				case INV_IN_OUT_DELTA_DAILY_IDX:
					ThingSpeak.setField(INV_IN_OUT_DELTA_DAILY_FIELD, powerDailyChannelData[channelId]);
					break;
				case PH3_PH1_DELTA_DAILY_IDX:
					ThingSpeak.setField(PH3_PH1_DELTA_DAILY_FIELD, powerDailyChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(powerDailyChannelNumber, powerDailyWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (powerDailyChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					powerDailyChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case SEND_POWER_MONTHLY_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (powerMonthlyChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case INV_IN_MONTHLY_IDX:
					ThingSpeak.setField(INV_IN_MONTHLY_FIELD, powerMonthlyChannelData[channelId]);
					break;
				case INV_OUT_MONTHLY_IDX:
					ThingSpeak.setField(INV_OUT_MONTHLY_FIELD, powerMonthlyChannelData[channelId]);
					break;
				case PH3_MONTHLY_IDX:
					ThingSpeak.setField(PH3_MONTHLY_FIELD, powerMonthlyChannelData[channelId]);
					break;
				case PH1_MONTHLY_IDX:
					ThingSpeak.setField(PH1_MONTHLY_FIELD, powerMonthlyChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(powerMonthlyChannelNumber, powerMonthlyWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (powerMonthlyChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					powerMonthlyChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case SEND_REALTIME_INVERTER_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (powerRealtimeInverterChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case INV_IN_CURR_WATT_IDX:
					ThingSpeak.setField(INV_IN_CURR_WATT_FIELD, powerRealtimeInverterChannelData[channelId]);
					break;
				case INV_OUT_CURR_WATT_IDX:
					ThingSpeak.setField(INV_OUT_CURR_WATT_FIELD, powerRealtimeInverterChannelData[channelId]);
					break;
				case INV_IN_OUT_REAL_TIME_DELTA_IDX:
					ThingSpeak.setField(INV_IN_OUT_REAL_TIME_DELTA_FIELD, powerRealtimeInverterChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(powerRealtimeInverterChannelNumber, powerRealtimeInverterWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (powerRealtimeInverterChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					powerRealtimeInverterChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case SEND_REALTIME_WATTMETER_DATA:
		for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
		{
			if (powerRealtimeWattmeterChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
			{
				switch (channelId)
				{
				case PH3_CURR_WATT_IDX:
					ThingSpeak.setField(PH3_CURR_WATT_FIELD, powerRealtimeWattmeterChannelData[channelId]);
					break;
				case PH1_CURR_WATT_IDX:
					ThingSpeak.setField(PH1_CURR_WATT_FIELD, powerRealtimeWattmeterChannelData[channelId]);
					break;
				case PH3_PH1_REAL_TIME_DELTA_IDX:
					ThingSpeak.setField(PH3_PH1_REAL_TIME_DELTA_FIELD, powerRealtimeWattmeterChannelData[channelId]);
					break;
				}
			}
		}
		if (ThingSpeak.writeFields(powerRealtimeWattmeterChannelNumber, powerRealtimeWattmeterWriteAPIKey) == OK_SUCCESS)
		{
			for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
			{
				if (powerRealtimeWattmeterChannelData[channelId] != DEFAULT_CHANNEL_VALUE)
					powerRealtimeWattmeterChannelData[channelId] = DEFAULT_CHANNEL_VALUE;
			}
		}
		break;
	case FETCH_AND_PROCESS_DATA:
		processIncomingData();
		break;
	}
	processQueue[processQueueHead] = DEFAULT_QUEUE_VALUE;
	processQueueHead = (processQueueHead + 1) % TYPES_OF_DATA;
}

void processIncomingData()
{
	byte channelId;
	incomingDataFound = false;
	/*
	for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
	{
	incomingChannelData[channelId] = (int)ThingSpeak.readIntField(incomingChannelNumber, channelId + 1, incomingReadAPIKey);
	if (ThingSpeak.getLastReadStatus() != OK_SUCCESS)
	incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
	if (ThingSpeak.getLastReadStatus() == OK_SUCCESS && (incomingChannelData[channelId] <= 0))// || incomingChannelData[channelId] > 2))
	incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
	if (isDigit(incomingChannelData[channelId]))
	{
	incomingChannelData[channelId] = incomingChannelData[channelId] + '0';
	if (incomingChannelData[channelId] > 2)
	incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
	}
	}
	*/

	for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
	{
		if (incomingChannelData[channelId] != DEFAULT_CHANNEL_VALUE_INT)
		{
			boolean inVal = incomingChannelData[channelId];
			incomingDataFound = true;
			switch (channelId)
			{
			case IN_BALCONY_LIGHT_IDX:
				lightNodeMessage.setDestination(BALCONYLIGHT_NODE_ID);
				lightNodeMessage.setSensor(LIGHT_RELAY_ID);
				lightNodeMessage.setType(V_STATUS);
				switch (incomingChannelData[channelId])
				{
				case 1:
					lightNodeMessage.set(RELAY_OFF);
					send(lightNodeMessage);
					break;
				case 2:
					lightNodeMessage.set(RELAY_ON);
					send(lightNodeMessage);
					break;
				}
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			case IN_GATE_LIGHT_IDX:
				lightNodeMessage.setDestination(GATELIGHT_NODE_ID);
				lightNodeMessage.setSensor(LIGHT_RELAY_ID);
				lightNodeMessage.setType(V_STATUS);
				switch (incomingChannelData[channelId])
				{
				case 1:
					lightNodeMessage.set(RELAY_OFF);
					send(lightNodeMessage);
					break;
				case 2:
					lightNodeMessage.set(RELAY_ON);
					send(lightNodeMessage);
					break;
				}
				wait(WAIT_AFTER_SEND_MESSAGE);
				//lightNodeMessage.set((incomingChannelData[channelId] == '1') ? 0 : 1);
				//send(lightNodeMessage);
				break;
			case IN_BOREWELL_ON_IDX:
				if (incomingChannelData[channelId] == 1)
				{
					borewellNodeMessage.setDestination(BOREWELL_NODE_ID);
					borewellNodeMessage.setSensor(BORE_ON_RELAY_ID);
					borewellNodeMessage.setType(V_STATUS);
					borewellNodeMessage.set(RELAY_ON);
					send(borewellNodeMessage);
					wait(WAIT_AFTER_SEND_MESSAGE);
				}
				/*else
				{
				borewellNodeMessage.setDestination(BOREWELL_NODE_ID);
				borewellNodeMessage.setSensor(BORE_ON_RELAY_ID);
				borewellNodeMessage.setType(V_STATUS);
				borewellNodeMessage.set(RELAY_OFF);
				send(borewellNodeMessage);
				}*/
				break;
			case IN_BOREWELL_OFF_IDX:
				if (incomingChannelData[channelId] == 1)
				{
					borewellNodeMessage.setDestination(BOREWELL_NODE_ID);
					borewellNodeMessage.setSensor(BORE_OFF_RELAY_ID);
					borewellNodeMessage.setType(V_STATUS);
					borewellNodeMessage.set(RELAY_ON);
					send(borewellNodeMessage);
					wait(WAIT_AFTER_SEND_MESSAGE);
				}
				break;
			case IN_SUMP_MOTOR_IDX:
				sumpMotorMessage.setDestination(SUMP_MOTOR_NODE_ID);
				sumpMotorMessage.setSensor(RELAY_ID);
				sumpMotorMessage.set((incomingChannelData[channelId] == 1) ? (byte)RELAY_ON : (byte)RELAY_OFF);
				send(sumpMotorMessage);
				wait(WAIT_AFTER_SEND_MESSAGE);
				break;
			}
			ThingSpeak.setField(channelId + 1, DEFAULT_CHANNEL_VALUE_INT);
			incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
		}
	}
	if (incomingDataFound)
		ThingSpeak.writeFields(incomingChannelNumber, incomingWriteAPIKey);
}

void insertQueue(byte data)
{
	boolean valueNotInQueue = true;
	for (byte index = 0; index < TYPES_OF_DATA; index++)
	{
		if (processQueue[index] == data)
			valueNotInQueue = false;
	}
	if (valueNotInQueue)
	{
		processQueue[processQueueTail] = data;
		processQueueTail = (processQueueTail + 1) % TYPES_OF_DATA;
	}
}

void insertFetchAndProcessDataRequest()
{
	byte channelId;
	int readValue;
	boolean dataToProcess = false;
	for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
	{
		if (incomingChannelData[channelId] == DEFAULT_CHANNEL_VALUE_INT)
		{
			readValue = (int)ThingSpeak.readIntField(incomingChannelNumber, channelId + 1, incomingReadAPIKey);
			//if (ThingSpeak.getLastReadStatus() != OK_SUCCESS)
			//	incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
			//if (ThingSpeak.getLastReadStatus() == OK_SUCCESS && (incomingChannelData[channelId] <= 0 || incomingChannelData[channelId] > 3))
			//	incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
			if (ThingSpeak.getLastReadStatus() != OK_SUCCESS)
				incomingChannelData[channelId] = DEFAULT_CHANNEL_VALUE_INT;
			else
			{
				switch (readValue)
				{
				case 1: incomingChannelData[channelId] = 1; break;
				case 2: incomingChannelData[channelId] = 2; break;
				case 3: incomingChannelData[channelId] = 3; break;
				default: incomingChannelData[channelId] = 0; break;
				}
			}
		}
	}
	/*for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
	{
	Serial.print("Channel Id - "); Serial.print(channelId); Serial.print(" - "); Serial.println(incomingChannelData[channelId]);
	}*/
	for (channelId = 0; channelId < FIELDS_PER_CHANNEL; channelId++)
	{
		if (incomingChannelData[channelId] != DEFAULT_CHANNEL_VALUE_INT)
		{
			dataToProcess = true;
			channelId = FIELDS_PER_CHANNEL;
		}
	}
	if (dataToProcess)
		//processIncomingData();
		insertQueue(FETCH_AND_PROCESS_DATA);
}

void checkSunRiseRequestStatus()
{
	if (!sunriseTimeReceived)
		sendSunriseRequest = true;
}

void checkSunSetRequestStatus()
{
	if (!sunsetTimeReceived)
		sendSunsetRequest = true;
}

void sunriseTriggerMessage()
{
	lightNodeMessage.setDestination(BALCONYLIGHT_NODE_ID);
	lightNodeMessage.setSensor(LIGHT_RELAY_ID);
	lightNodeMessage.setType(V_STATUS);
	lightNodeMessage.set(RELAY_OFF);
	send(lightNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	lightNodeMessage.setDestination(GATELIGHT_NODE_ID);
	lightNodeMessage.setSensor(LIGHT_RELAY_ID);
	lightNodeMessage.setType(V_STATUS);
	lightNodeMessage.set(RELAY_OFF);
	send(lightNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	resetRelayMessage.setDestination(BATT_VOLTAGE_NODE_ID);
	resetRelayMessage.setSensor(RESET_RELAY_ID);
	resetRelayMessage.setType(V_STATUS);
	resetRelayMessage.set(RELAY_ON);
	send(resetRelayMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	uint32_t valueToSend = sunriseSeconds - QUATER_HOUR_OFFSET;
	send(sunriseTimeMessage.set(valueToSend));
	wait(WAIT_AFTER_SEND_MESSAGE);
	formatTimeForGateway(hour(), minute());
	sendLastUpdateTime();
}

void sunsetTriggerMessage()
{
	lightNodeMessage.setDestination(BALCONYLIGHT_NODE_ID);
	lightNodeMessage.setSensor(LIGHT_RELAY_ID);
	lightNodeMessage.setType(V_STATUS);
	lightNodeMessage.set(RELAY_ON);
	send(lightNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	lightNodeMessage.setDestination(GATELIGHT_NODE_ID);
	lightNodeMessage.setSensor(LIGHT_RELAY_ID);
	lightNodeMessage.setType(V_STATUS);
	lightNodeMessage.set(RELAY_ON);
	send(lightNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	resetRelayMessage.setDestination(BATT_VOLTAGE_NODE_ID);
	resetRelayMessage.setSensor(RESET_RELAY_ID);
	resetRelayMessage.setType(V_STATUS);
	resetRelayMessage.set(RELAY_OFF);
	send(resetRelayMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	uint32_t valueToSend = sunsetSeconds - QUATER_HOUR_OFFSET;
	send(sunsetTimeMessage.set(valueToSend));
	wait(WAIT_AFTER_SEND_MESSAGE);
	formatTimeForGateway(hour(), minute());
	sendLastUpdateTime();
}

void nightTriggerMessage()
{
	lightNodeMessage.setDestination(BALCONYLIGHT_NODE_ID);
	lightNodeMessage.setSensor(LIGHT_RELAY_ID);
	lightNodeMessage.setType(V_STATUS);
	lightNodeMessage.set(RELAY_OFF);
	send(lightNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	lightNodeMessage.setDestination(GATELIGHT_NODE_ID);
	lightNodeMessage.setSensor(LIGHT_RELAY_ID);
	lightNodeMessage.setType(V_STATUS);
	lightNodeMessage.set(RELAY_OFF);
	send(lightNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);

	formatTimeForGateway(hour(), minute());
	sendLastUpdateTime();
}

void receiveTime(unsigned long controllerTime)
{
	setTime(controllerTime);
	formatTimeForGateway(hour(), minute());
	sendLastUpdateTime();
	if (firstTime)
	{
		firstTime = false;
		nightTimer = Alarm.alarmRepeat(NIGHT_HH, NIGHT_MM, NIGHT_SS, nightTriggerMessage);
	}
}

void formatTimeForGateway(byte hour, byte mins)
{
	mytime[0] = (hour / 10) + '0';
	mytime[1] = (hour % 10) + '0';
	mytime[2] = ':';
	mytime[3] = (mins / 10) + '0';
	mytime[4] = (mins % 10) + '0';
	mytime[5] = '\0';
}

void resetTimeForGateway()
{
	mytime[0] = 0 + '0';
	mytime[1] = 0 + '0';
	mytime[2] = ':';
	mytime[3] = 0 + '0';
	mytime[4] = 0 + '0';
	mytime[5] = '\0';

}

void sendLastUpdateTime()
{
	MyMessage gatewayTimeMessage(LAST_UPDATE_TIME_ID, V_VAR3);
	send(gatewayTimeMessage.set(mytime));
	wait(WAIT_AFTER_SEND_MESSAGE);
	resetTimeForGateway();
}

void heartbeat()
{
	sendHeartbeat();
}

void getTimeFromGateway()
{
	requestTime();
}

/*BLYNK_WRITE(V0)
{
	lightNodeMessage.setDestination(BALCONYLIGHT_NODE_ID);
	lightNodeMessage.setSensor(LIGHT_RELAY_ID);
	lightNodeMessage.setType(V_STATUS);
	lightNodeMessage.set(param.asInt());
	send(lightNodeMessage);
	wait(WAIT_AFTER_SEND_MESSAGE);
}*/