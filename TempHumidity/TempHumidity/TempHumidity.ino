#include <DHT.h>
#include <TimeAlarms.h>
#include <TimeLib.h>
#include <Time.h>
#include <SPI.h>

#define NODE_HAS_TEMP_HUMIDITY
#define MY_RADIO_NRF24
#define MY_NODE_ID TEMP_AND_HUMIDITY_NODE_ID
#define MY_PARENT_NODE_ID REPEATER_02_NODE_ID
#define MY_PARENT_NODE_IS_STATIC

#include <MyNodes.h>
#include <MySensors.h>

#define APPLICATION_NAME "Temp & Humidity"

MyMessage humidityMessage(HUMIDITY_ID, V_HUM);
MyMessage temperatureMessage(TEMPERATURE_ID, V_TEMP);
DHT dht;

AlarmId heartbeatTimer;
AlarmId humidityTemperatureTimer;

boolean firstTime;
void before()
{
	firstTime = true;
}

void setup()
{
	dht.setup(DHT_SENSOR_PIN);
	heartbeatTimer = Alarm.timerRepeat(HEARTBEAT_INTERVAL, sendHeartbeat);
	humidityTemperatureTimer = Alarm.timerRepeat(FIVE_MINUTES, sendHumidityAndTemperature);
	sleep(dht.getMinimumSamplingPeriod());
}

void presentation()
{
	sendSketchInfo(APPLICATION_NAME, getCodeVersion());
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(HUMIDITY_ID, S_HUM, "Humidity");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
	present(TEMPERATURE_ID, S_TEMP, "Temperature");
	Alarm.delay(WAIT_AFTER_SEND_MESSAGE);
}

void loop()
{
	if (firstTime)
	{
		sendHumidityAndTemperature();
		firstTime = false;
	}
	Alarm.delay(1);
}
void receive(const MyMessage &message)
{

}

void sendHumidityAndTemperature()
{
	float humidity = 0.0;
	float temperature = 0.0;
	humidity = dht.getHumidity();
	temperature = dht.getTemperature();
	if (!isnan(humidity))
		send(humidityMessage.set(humidity, 2));
	wait(WAIT_AFTER_SEND_MESSAGE);
	if (!isnan(temperature))
		send(temperatureMessage.set(temperature, 2));
	wait(WAIT_AFTER_SEND_MESSAGE);
}