#ifndef DEFVAR_H
#define DEFVAR_H
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
HardwareSerial SerialAT(1);
#define SerialMon Serial

bool FLAG_2G = true;  // true = 2G | false = NB-IoT
bool FLAG_LAN = true; // true = WIFI | false = GSM

String firmware_version = "0.8.0";

// #define WRITE_EEPROM
// #define DUMMY_DATA
#define DEBUG_MON

#ifdef DEBUG_MON
#define DEBUGPRINTLN(x) Serial.println(x)
#define DEBUGPRINT(x) Serial.print(x)
#else
#define DEBUGPRINTLN(x)
#define DEBUGPRINT(x)
#endif

// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_BG96_NBIOT
#define TINY_GSM_MODEM_BC92
// #define TINY_GSM_MODEM_BC65_NBIOT
// #define TINY_GSM_MODEM_BC65

#if defined TINY_GSM_MODEM_BC92 || defined TINY_GSM_MODEM_BC65 || defined TINY_GSM_MODEM_BC65_NBIOT
#define BAUDRATE 9600
#else
#define BAUDRATE 115200
#endif

#define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG_DEEP SerialMon
#define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200

// Define how you're planning to connect to the internet
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false

// set GSM PIN, if any
#define GSM_PIN ""

#define DEPLOY

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// MQTT details
const char *port = "1883";
const char *broker = "iot.cht.com.tw";
// const char *broker = "iot.cht.com.tw";

// String device_id = "36828155029";
// String device_key = "DKWP9U1CY3HBCHF5KH";
char device_id[17] = "36828155029";
char device_key[33] = "DKWP9U1CY3HBCHF5KH";

// const char* broker = "mqtt.flespi.io"; //"broker.hivemq.com";
// const char* MQTTuser = "J5s3JhlHuTIZgBaf0uLHmfVJEdjYJEFObLeF5BGghdBPkuSQqsgy7OvOZJCzu8T6";
// const char* MQTTpasw = "";
char adyaconnectid[17] = "123123123";
// String s_adyaconnectid;
// String adyaconnectid = "1231231231231231";
char adyaconnectkey[33] = "123123123";
// String s_adyaconnectkey;
// String adyaconnectkey = "123456789ABCDEFGHIJKLMNOPQRSTUVW";

uint16_t Periode;
int valuebr = 100;
int counteronoff = 0;

char topicpubbuf[50];
char topicsubbuf[90];
char topicsubbuf2[90];

unsigned long lastMsg = 0;

#include <TinyGsmClient.h>

#include <EEPROM.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif
TinyGsmClient client(modem);

// #include <ArduinoHttpClient.h>
#include <Update.h>

// Your GPRS credentials, if any
// const char apn[] = "NB1INTERNET";
// const char apn2g[] = "M2MINTERNET";//"M2MINTERNET";
char apn[17] = "NB1INTERNET";
char apn2g[17] = "M2MINTERNET";
char oper[10] = "51010";
char last_apn[17] = "NB1INTERNET";
char last_apn2g[17] = "M2MINTERNET";
char last_oper[10] = "51010";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_GPRS && not defined TINY_GSM_MODEM_HAS_GPRS
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_GPRS
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_GPRS true
#define TINY_GSM_USE_WIFI false
#endif

// #define DUMP_AT_COMMANDS

// PINOUT DEFINITION

#define BL0940_RX 25
#define BL0940_TX 26
#define EM_CF_PIN 32
#define EM_LED_PIN 12
#define RELAY_PIN 15
#define PWRKEY_PIN 17
#define RESET_PIN 27
#define AT_TX_PIN 19
#define AT_RX_PIN 23
#define DHT_PIN 10
#define BUTTON_PIN 33

String lpwan = "NBIoT";
int stateConn = 0;

bool FLAG_RESTART_MODEM = false;
bool SUB_STATE = false;
uint32_t lastReconnectAttempt = 0;
bool buttonAlarm = LOW;
int counter = 0;
char msg1[150];
char msg2[128];
char msg3[128];
char telemetry[312];
char clid[50];
char buffAck[50];
char msgPubAktif[128];
int counterpub = 0;
int counterLOS = 0;
int counterLOSRestartModem;
int sensorValue = 4095;
int ldrThreshold = 0;
bool autoBrightness = false;

unsigned long timeStart = 0;
unsigned long hitungStart = 0;
unsigned long periodePub = 10000;
unsigned long periodeSampling = 5000;
unsigned long lastLOSTime = 0;
unsigned long timeAutoBrightness = 0;
bool pubStatusAktif = 0;
byte sttPeriode;
byte ACTIVATION_FLAG = 1;
int counterAPNloss;
int counterLOSTooShort = 0;
// String clientid = "";
int pubAck = 0;
String apnString = "";
int YearLastPub, dayLastPub, hourLastPub, minuteLastPub;
int hourNextPub, minuteNextPub;
byte TIME_FLAG = 0;
bool FLAG_COPS = 0;
char custidbuf[10];
char imeibuf[20];
char csqbuf[20];

String csq;
uint8_t mux = 0;
uint8_t urc_res = 0;
int counterMqttConn, counterMqttOpen, counterSub, counterTryConnect, counterCheckMqtt = 0;

int year, month, day, hour, minute, second;
float timezone;

#include <esp_task_wdt.h>
// 3 seconds WDT
#define WDT_TIMEOUT 200
unsigned long lastWdtModem = 0;
unsigned long lastWdtWifi = 0;
unsigned long lastWdtEnergy = 0;
int wdtPeriodModemMs = 120000;
int wdtPeriodWifiMs = 100000;

/*---------------------------------------------
// Energy Meter BL0940 variable and definition
--------------------------------------------*/

#include <bl0940.h>
HardwareSerial SerialEM(2);

bl0940 em_bl0940(&SerialEM, &Serial);
unsigned long lastHandleBL0940 = 0;
int handleBL0940Ms = 3000;

bl0940Config_t configbl0940;
float voltage;
float lastVoltage = 0.0;
float current, lastCurrent;
double activePower, lastActivePower;
double activePower2;
double reactivePower;
double apparentPower;
float Energy;
float EnergyCF;
float EnergyDelta;
float PhaseAngle;
float PowerFactor, PowerFactorMod, PowerFactor3, lastPowerFactor;
uint32_t c_f = 0;
String lastStandKwhBuff, lastMonthlyKwhBuff;
bool FLAG_ADD_ENERGY_CF = false;
int counterCF = 0;

/*---------------------------------------------
//       RTC variable and definition
---------------------------------------------*/

#include <Wire.h>
#include "PCF85063TP.h"
#include "time.h"
#include "timestamp32bits.h"
timestamp32bits stamp = timestamp32bits();

PCD85063TP clockrtc; // define a object of PCD85063TP class

uint16_t calibrated_blink_second = 5900;
unsigned long unixTimestamp;

/*---------------------------------------------
//            0-10V Controller
-----------------[---------------------------*/
#include "DFRobot_GP8403.h"
DFRobot_GP8403 dacZeroTen(&Wire, 0x58);

int nilaiDAC;

/*---------------------------------------------
//      LIGHT SENSOR variable and definition
---------------------------------------------*/
#include <LTR308.h>

LTR308 light(&Wire1);
double lux; // Resulting lux value

// Global variables:

// If gain = 0, device is set to 1X gain
// If gain = 1, device is set to 3X gain (default)
// If gain = 2, device is set to 6X gain
// If gain = 3, device is set to 9X gain
// If gain = 4, device is set to 18X gain
unsigned char gain = 1; // Gain setting, values = 0-4

//------------------------------------------------------
// If integrationTime = 0, integrationTime will be 400ms / 20 bits result
// If integrationTime = 1, integrationTime will be 200ms / 19 bits result
// If integrationTime = 2, integrationTime will be 100ms / 18 bits result (default)
// If integrationTime = 3, integrationTime will be 50ms / 17 bits result
// If integrationTime = 4, integrationTime will be 25ms / 16 bits result
unsigned char integrationTime = 0; // Integration ("shutter") time, values 0 - 4

//------------------------------------------------------
// If measurementRate = 0, measurementRate will be 25ms
// If measurementRate = 1, measurementRate will be 50ms
// If measurementRate = 2, measurementRate will be 100ms (default)
// If measurementRate = 3, measurementRate will be 500ms
// If measurementRate = 5, measurementRate will be 1000ms
// If measurementRate = 6, measurementRate will be 2000ms
// If measurementRate = 7, measurementRate will be 2000ms
unsigned char measurementRate = 3; // Interval between DATA_REGISTERS update, values 0 - 7, except 4

//------------------------------------------------------
// Chip ID - should be 0xB1 for all LTR-308
unsigned char ID;

//------------------------------------------------------
// Main Control Register
unsigned char control;

/*---------------------------------------------
//      EEPROM variable and definition
---------------------------------------------*/
#include <I2C_EEPROM.h>
AT24C64<> eep;
uint16_t FLASH_MARKING_ADDR = 0;
uint16_t FLAG_TURN_ON_WIFI_ADD = 5;
uint16_t ID_LAMP_ADDR = 10;
uint16_t DEV_KEY_ADDR = 40;
// uint16_t PASS_ADDR = 68;
uint16_t PERIODE_ADDR = 105;
uint16_t BRIGHTNESS_ADDR = 120;
uint16_t VOLTAGE_ADDR = 130;

uint16_t FLAG_WIFI_CONF_ADDR = 180;

uint16_t TS_SSID_ADDR = 230;
uint16_t TS_DEVID_ADDR = 260;
uint16_t TS_CONN_MODE_ADDR = 290;
uint16_t ADYACONN_ID_ADDR = 330;
uint16_t ADYACONN_KEY_ADDR = 360;
uint16_t TS_OTALOCAL_ADDR = 400;

uint16_t SAFE_MODE_ADDR = 520;

struct
{
  uint16_t CONN_MODE_ADDR = 440;
  uint16_t SSID_ADDR = 190;
  uint16_t SSID_PASS_ADDR = 210;
  uint16_t APN_ADDR = 465;
  uint16_t APN2G_ADDR = 480;
  uint16_t OPER_ADDR = 500;
} last_conf_addr;

struct
{
  uint16_t CONN_MODE_ADDR = 525; // size 20
  uint16_t SSID_ADDR = 140;
  uint16_t SSID_PASS_ADDR = 160;
  uint16_t APN_ADDR = 550;
  uint16_t APN2G_ADDR = 565;
  uint16_t OPER_ADDR = 585;
} curr_conf_addr;

/*---------------------------------------------
//      DHT variable and definition
---------------------------------------------*/

#include <DHT.h>
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);
unsigned long lastReadDht = 0;
int periodDhtMs = 10000;
float temp = 0.0;

/*================================
            WIFI
  ================================*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

WiFiClient espClient;
PubSubClient clientwf(espClient);
// char ssid[20] = "XTI-IoT";
// char ssid_pass[20] = "10Th!ng5";
// char ssid_last[20] = "XTI-IoT";
// char ssid_pass_last[20] = "10Th!ng5";
char ssid[20] = "SMARTLAMPSSID";
char ssid_pass[20] = "password1234";
char ssid_last[20] = "SMARTLAMPSSID";
char ssid_pass_last[20] = "password1234";
uint8_t safe_mode = 0;
char ts_ssid[28] = "2024010010T090280150529Z";
char ts_devid[28] = "2024010010T090280150529Z";
char ts_conn_mode[28] = "2024010010T090280150529Z";
char ts_otalocal[28] = "2024010010T090280150529Z";
char conn_mode[20] = "WIFI";      // WIFI, NB, GSM
char last_conn_mode[20] = "WIFI"; // WIFI, NB, GSM
bool FLAG_WIFI_NOT_CONNECTED = 1;
unsigned long lastWifiRetry = 0;
uint32_t wifi_config_periode = 300000;
uint32_t WifiRetryPeriod = 180000;
bool FLAG_WIFI_CONF = false;

AsyncWebServer server(80);

int qc_EEPROM = 0;
int qc_RTC = 0;
int qc_sensor_cahaya = 0;
int qc_sensor_suhu = 0;
int qc_sensor_elektrikal = 0;
int qc_relay = 1;
int qc_dimming = 0;
int qc_wifi = 0;
int qc_gsm = 1;
String boardid = "ABCDEFGHIJKLMNOPQRSTU";

#endif

/*=================================
//          PINOUT
=================================*/

#define TFT_CS 36
#define TFT_RST 19 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 2
#define BACKLIGHT 25
#define TFT_SCK 18
#define TFT_MOSI 23
#define RS485_TX_PIN 16
#define RS485_RX_PIN 22

#include <Arduino_GFX_Library.h>
Arduino_ESP32SPI bus = Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI);
Arduino_ST7735 display = Arduino_ST7735(&bus, TFT_RST);

const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

#define TEXT_COLOR WHITE
#define HEADER_COLOR YELLOW
#define HIGHLIGHT_COLOR GREEN
#define BACKGROUND_COLOR BLACK
#define BOX_COLOR 0x3A0E