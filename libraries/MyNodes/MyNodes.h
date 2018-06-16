/***
 *  This file defines the Sensor ids used in MySensors network.
 */
#ifndef MyNodes_h
#define MyNodes_h

#define WAIT_AFTER_SEND_MESSAGE 500
#define WAIT_5MS 5
#define WAIT_10MS 10
#define WAIT_50MS 50
#define WAIT_1SEC 1000
#define TEN_SECS 10
#define TWENTY_SECS 20
#define THIRTY_SECS 30
#define ONE_MINUTE 60
#define FIVE_MINUTES 300
#define QUATER_HOUR 900
#define HALF_HOUR 1800
#define ONE_HOUR 3600
#define REQUEST_INTERVAL 60
#define HEARTBEAT_INTERVAL 600

char* getCodeVersion()
{
	char codeVersion[20];
	strcpy(codeVersion, "");
	strcat(codeVersion, __DATE__);
	strcat(codeVersion, " ");
	strcat(codeVersion, __TIME__);
	return codeVersion;
}
/*
The following are the Node Ids assigned, populated here as comments for easy reference.
0 - Gateway
1 - Balcony Lights
2 - Staircase Lights
3 - Gate Lights
4 - Tank 01
5 - Tank 02
6 - Tank 03
7 - Borewell Motor
8 - Sump Motor
9 - Tap Motor
10 - Repeater Node 01
11 - LCD
12 - Battery Voltage
13 - Solar Voltage
14 - Controller
15 - Repeater Node 02
16 - Inverter In
17 - Inverter Out
18 - 3Phase Wattmeter
19 - 1Phase Wattmeter
20 - Temperature & Humidity 
21 - Terrace Soil Sensor
22 - GFloor Soil Sensor
23 - SMS Node
250 - Default Firmware Node
254 - Thingspeak Node
*/

#define BALCONYLIGHT_NODE_ID 1
#define STAIRCASE_LIGHT_NODE_ID 2
#define GATELIGHT_NODE_ID 3
#define TANK_01_NODE_ID 4
#define TANK_02_NODE_ID 5
#define TANK_03_NODE_ID 6
#define BOREWELL_NODE_ID 7
#define SUMP_MOTOR_NODE_ID 8
#define TAP_MOTOR_NODE_ID 9
#define REPEATER_01_NODE_ID 10
#define LCD_NODE_ID 11
#define BATT_VOLTAGE_NODE_ID 12
#define SOLAR_VOLTAGE_NODE_ID 13
#define REMOTE_CONTROLLER_NODE_ID 14
#define REPEATER_02_NODE_ID 15
#define INV_IN_NODE_ID 16
#define INV_OUT_NODE_ID 17
#define PH3_NODE_ID 18
#define PH1_NODE_ID 19
#define TEMP_AND_HUMIDITY_NODE_ID 20
#define TERRACE_SOIL_SENSOR_NODE_ID 21
#define GFLOOR_SOIL_SENSOR_NODE_ID 22
#define SMS_NODE_ID 23


#define DEFAULT_FIRMWARE_NODE_ID 250
#define THINGSPEAK_NODE_ID 254
#define WIFI_NODEMCU_ID 1

#if defined MY_BUFFER_FEATURE
#define MY_RX_MESSAGE_BUFFER_FEATURE
#define MY_RF24_IRQ_PIN 2
#endif

#if defined LIGHT_NODE

#define LIGHT_RELAY_ID 1
#define LIGHT_DURATION_ID 2

#define LIGHT_RELAY_PIN 7
#define DEFAULT_LIGHT_ON_DURATION 60

#endif

// Overhead Tank 01
#if defined TANK_01_NODE
#define DRY_RUN_POLL_DURATION 900
#define BOREWELL_ADHOC_ID 3
#define BORE_ON_RELAY_ID 2
#define BORE_OFF_RELAY_ID 3
#endif

// Overhead Tank 02
#if defined TANK_02_NODE
#define SUMP_MOTOR_ADHOC_ID 3
#endif

//Underground Tank
#if defined TANK_03_NODE
#endif

#if defined BOREWELL_NODE
#define BOREWELL_MOTOR_STATUS_ID 1
#define BORE_ON_RELAY_ID 2
#define BORE_OFF_RELAY_ID 3
#define RELAY_TRIGGER_INTERVAL 2

#define DRY_RUN_POLL_DURATION 900
#define MOTOR_STATUS_PIN 3
#define BORE_ON_RELAY_PIN 7
#define BORE_OFF_RELAY_PIN 8
#endif

#if defined SUMP_RELATED_NODE
#define SUMP_MOTOR_RELAY_ID 1
#define RELAY_PIN 7

#define TANK02_LOW_LEVEL_ID 1
#define TANK02_HIGH_LEVEL_ID 2
#define TANK03_LOW_LEVEL_ID 3
#define TANK03_HIGH_LEVEL_ID 4
#define MOTOR_STATUS_PIN 3
#endif

#if defined STATUS_LEDS
#define MY_DEFAULT_LED_BLINK_PERIOD 300
#define MY_WITH_LEDS_BLINKING_INVERSE
#define MY_DEFAULT_ERR_LED_PIN 4  
#define MY_DEFAULT_RX_LED_PIN  5
#define MY_DEFAULT_TX_LED_PIN  6
#endif

#if defined STATUS_LEDS_MODEMCU
#define MY_DEFAULT_LED_BLINK_PERIOD 300
#define MY_WITH_LEDS_BLINKING_INVERSE
#define MY_DEFAULT_RX_LED_PIN  LED_BUILTIN
#endif

#if defined LCD_NODE
#define CURR_WATT_ID 1
#define BATTERY_VOLTAGE_ID 4
#define SOLAR_VOLTAGE_ID 5

#define LCD_I2C_ADDR 0x27
#define LCD_ROWS 4
#define LCD_COLUMNS 20
#define LCD_BACKLIGHT_ID 1
#define ROW_1 0
#define ROW_2 1
#define ROW_3 2
#define ROW_4 3

#endif

#if defined SOLAR_BATT_VOLTAGE_NODE
#define R1_VALUE_ID 1
#define R2_VALUE_ID 2
#define SCALE_FACTOR_ID 3
#define BATTERY_VOLTAGE_ID 4
#define SOLAR_VOLTAGE_ID 5
#define RESET_RELAY_ID 6
#define REF_RAW_VALUE_ID 7
#define INP_RAW_VALUE_ID 8
#define SEND_RAW_VALUE_ID 10

#define VOLTAGE_SENSE_PIN A0
#define THRESHOLD_VOLTAGE_PIN A1
#define RELAY_PIN 7
#define UP 1

#endif

#if defined REMOTE_CONTROLLER_NODE
#define KEYPAD_ID 1
#define SCL_PIN 3
#define SDA_PIN 4
#define KEYPAD_SIZE 16
#define KEYPAD_READ_INTERVAL 2

#define RELAY_ID 1
#define BORE_ON_RELAY_ID 2
#define BORE_OFF_RELAY_ID 3
#endif

#if defined WATT_METER_NODE
#define ZERO_PULSE 0
#define ACCUMULATION_FREQUENCY_SECS 30
#define MAX_WATT 6000
#define MAX_WATT_INVERTER 2000

#define CURR_WATT_ID 1
#define ACCUMULATED_WATT_CONSUMPTION_ID 2
#define HOURLY_KWATT_INIT_ID 3
#define HOURLY_WATT_CONSUMPTION_ID 4
#define DAILY_KWATT_INIT_ID 5
#define DAILY_WATT_CONSUMPTION_ID 6
#define MONTHLY_KWATT_INIT_ID 7
#define MONTHLY_WATT_CONSUMPTION_ID 8
#define DELTA_WATT_CONSUMPTION_ID 9
#define CURR_PULSE_COUNT_ID 10
#define RESET_TYPE_ID 11

#define PULSE_SENSOR_PIN 3
#define INTERRUPT_PULSE 1 // PULSE_SENSOR_PIN - 2

#define RESET_ALL 1
#define ZERO 0


#endif

#if defined WIFI_NODE

#define RELAY_ID 1
#define BORE_ON_RELAY_ID 2
#define BORE_OFF_RELAY_ID 3

#define SUNRISE_TIME_ID 1
#define SUNSET_TIME_ID 2
#define LAST_UPDATE_TIME_ID 3
#define SUNRISE_TRIGGER_TIME_ID 4
#define SUNSET_TRIGGER_TIME_ID 5

#define QUATER_HOUR_OFFSET 900
#define HALF_HOUR_OFFSET 1800
#define DEFAULT_SUNRISE_SUNSET_TIME 0
#define NIGHT_HH 23
#define NIGHT_MM 46
#define NIGHT_SS 00
#endif

#if defined SOIL_MOISTURE_NODE
#define SOIL_SENSOR_ID 1
#define SOIL_MOISTURE_THRESHOLD_ID 2
#define VALVE_RELAY_ID 3
#define DEFAULT_SOIL_MOISTURE_LEVEL 75.0F
#define MAX_SOIL_MOISTURE_LEVEL 90.0F
#define MAX_ANALOG_SENSOR_VALUE 840.00F
#define ZERO_VALUE 0.00F
#define CENT_VALUE 100.00F

#define SOIL_SENSOR_PIN A0
#define VALVE_RELAY_PIN 7
#define SENSOR_CONTROL_PIN 8
#endif


#if defined SMS_NODE

#define USER_ADMIN_ID 1
#define SMS_TEXT_ID 2

#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

#define MAXSMSLENGTH 30
#define MAXMSGLENGTH 22
#define MAXNUMBERLENGTH 13
#define MAXNODEID 254
#define MAXSENSORID 99
#define SIM800L_OK "SIM800L OK"
#define SIM800L_ERROR "SIM800L Error"

#define CONFIRM_MESSAGE "Admin request processed"
#define INVALID_MESSAGE "Invalid SMS from Admin"
#define UNKNOWN_MESSAGE "SMS from "

#define RELAY_ID 1
#define BORE_ON_RELAY_ID 2
#define BORE_OFF_RELAY_ID 3

#endif

#if defined NODE_INTERACTS_WITH_WIFI_AND_LCD

#define INV_IN_CURR_WATT_ID 1
#define INV_OUT_CURR_WATT_ID 2
#define PH3_CURR_WATT_ID 3
#define PH1_CURR_WATT_ID 4
#define INV_IN_OUT_DELTA_ID 5
#define PH3_PH1_DELTA_ID 6
#endif

#if defined NODE_HAS_RELAY
#define RELAY_ON 1
#define RELAY_OFF 0
#endif

#if defined NODE_INTERACTS_WITH_RELAY
#define RELAY_ON 1
#define RELAY_OFF 0
#endif

#if defined NODE_WITH_ON_OFF_FEATURE
#define TURN_ON 1
#define TURN_OFF 0
#endif

#if defined NODE_WITH_HIGH_LOW_FEATURE
#define HIGH_LEVEL 1
#define NOT_HIGH_LEVEL 0
#define LOW_LEVEL 1
#define NOT_LOW_LEVEL 0
#endif

#if defined NODE_INTERACTS_WITH_LCD
#define INV_IN_CURR_WATT_ID 1
#define INV_OUT_CURR_WATT_ID 2
#define PH3_CURR_WATT_ID 3
#define PH1_CURR_WATT_ID 4
#define INV_IN_OUT_DELTA_ID 5
#define PH3_PH1_DELTA_ID 6
#endif

#if defined KEYPAD_1R_2C
#define ROWS 1
#define COLS 2
#define ON '1'
#define OFF '2'

char keys[1][2] = { ON,OFF };
byte rowsPins[1] = { 6 };
byte colsPins[2] = { 4,5 };

#endif

#if defined NODE_HAS_TEMP_HUMIDITY
#define HUMIDITY_ID 1
#define TEMPERATURE_ID 2

#define DHT_SENSOR_PIN 2
#define SENSOR_TEMP_OFFSET 0
#endif

#if defined WATER_TANK_NODE
#define LOW_LEVEL_PIN 2
#define HIGH_LEVEL_PIN 3

#define FOR_LOW_LEVEL 0
#define FOR_HIGH_LEVEL 1
#define DEFAULT_SEND_STATUS 2

#define TANK_LOW_LEVEL_ID 1
#define TANK_HIGH_LEVEL_ID 2

#endif

#if defined NODE_HAS_ULTRASONIC_SENSOR
#define ECHO_PIN 5
#define TRIGGER_PIN 6
#define MAX_DISTANCE 300
#define WATER_LEVEL_SENSOR_ID 1

#endif

#if defined MOTION_SENSOR_WITH_LIGHT
#define MOTION_SENSOR_PIN 3
#define INTERRUPT_MOTION 1 // MOTION_SENSOR_PIN - 2
#define LIGHT_RELAY_PIN 7

#define MOTION_SENSOR_ID 1
#define LIGHT_RELAY_ID 2
#define LIGHT_DURATION_ID 4

#define STANDBY_MODE 0
#define DUSKLIGHT_MODE 1
#define SENSOR_MODE 2
#define ADHOC_MODE 3

#define MOTION_DETECTED 1
#define NO_MOTION_DETECTED 0

#define BALCONYLIGHT_WITH_PIR_NODE_ID 1
#define GATELIGHT_WITH_PIR_NODE_ID 3

#endif

void LOG(char *logmessage)
{
#if defined LOG_THIS_NODE_DATA
	Serial.println(logmessage);
#endif
}

#endif