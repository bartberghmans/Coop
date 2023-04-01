#include <Arduino.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
//#include <SoftwareSerial.h>
//#include <TinyGPS.h>
#include <WiFiServer.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <string.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "AstroTime.h"
#include "MotorControl.h"
#include "Climate.h"
#include "RFID.h"

#include "index.h"
#include "style_css.h"
#include "scripts_js.h"

#define STATUS_GATE_OPENED		0b00000001
#define STATUS_GATE_CLOSED		0b00000010
#define STATUS_TIME_CALCULATED	0b00000100
#define STATUS_SYNCH_TIME		0b00001000
#define STATUS_TIME_SYNCHED		0b00010000

#define SETTING_SYNCH_NTP		0b00100000
#define SETTING_SYNCH_GPS		0b01000000

#define FLAG_RFID_STATUS		0b10000000

#define TIME_UPDATE_INTERVAL 180 // seconds
#define TIME_SYNC_INTERVAL 720	// [0..60] minutes
#define TIME_DOOR_OPENING 30	// [0..60] seconds
#define SUNRISE_OFFSET 0
#define SUNSET_OFFSET 1800

#define RFIDIRQPIN D5
#define RFID_WINDOW 500

//#define GPSPIN_RX D6
//#define GPSPIN_TX D7
#define NTP_SERVER "pool.ntp.org"

byte status = STATUS_SYNCH_TIME | SETTING_SYNCH_NTP;
int i = 0;

String names[3] = {"DoorState", "OpenTime", "CloseTime"};
String types[3] = {"text/doorstate", "time/UTC", "time/UTC"};
String values[3] = {"", "", ""};

AstroTimeClass astro;
RFIDClass rfid;
ClimateClass climate;

//SoftwareSerial serialGPS(GPSPIN_RX, GPSPIN_TX);
//TinyGPS gps;

AlarmId sunriseTimer = dtINVALID_ALARM_ID;
AlarmId sunsetTimer = dtINVALID_ALARM_ID;
AlarmId recalculateTimer = dtINVALID_ALARM_ID;
AlarmId synchTimer = dtINVALID_ALARM_ID;
AlarmId stopTimer = dtINVALID_ALARM_ID;
AlarmId updateClimateTimer = dtINVALID_ALARM_ID;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, 0, 60000);
ESP8266WebServer server(80);

boolean getStatusByte(byte mask)
{
	return (status & mask) != 0;
}

void setStatusByte(byte mask, boolean value)
{
	status = value ? status | mask : status & (~mask);
}

void clearAlarm(AlarmId *id)
{
	if (*id == dtINVALID_ALARM_ID)
		return;
	Alarm.disable(*id);
	Alarm.free(*id);
	*id = dtINVALID_ALARM_ID;
}

AlarmID_t setAlarm(time_t alarmtime, OnTick_t onTickHandler)
{
	AlarmID_t id = Alarm.triggerOnce(alarmtime, onTickHandler);
	Alarm.enable(id);
	return id;
}

void StopDoor()
{
	clearAlarm(&stopTimer);
	MotorControlClass::Stop();
}

void MoveDoor(bool open)
{
	setStatusByte(STATUS_GATE_OPENED, open);
	setStatusByte(STATUS_GATE_CLOSED, !open);

	stopTimer = Alarm.timerOnce(0, 0, TIME_DOOR_OPENING, StopDoor);
	Alarm.enable(stopTimer);

	MotorControlClass::MoveDoor(open);
}

void OpenDoor()
{
	MoveDoor(true);
	values[0] = "open";
}

void CloseDoor()
{
	MoveDoor(false);
	values[0] = "closed";
}

void recalculate()
{
	if (!getStatusByte(STATUS_TIME_SYNCHED))
		return;

	time_t midnight = astro.getMidnight();
	uint16_t day_of_year = astro.getDayOfYear(day(), month());

	time_t sunrisetime = astro.adjustTime(astro.getSunrise(day_of_year) + SUNRISE_OFFSET + midnight);
	clearAlarm(&sunriseTimer);
	sunriseTimer = setAlarm(sunrisetime, OpenDoor);
	values[1] = astro.getTimeString(sunrisetime);

	time_t sunsettime = astro.adjustTime(astro.getSunset(day_of_year) + SUNSET_OFFSET + midnight);
	clearAlarm(&sunsetTimer);
	sunsetTimer = setAlarm(sunsettime, CloseDoor);
	values[2] = astro.getTimeString(sunsettime);

	clearAlarm(&recalculateTimer);
	recalculateTimer = setAlarm(astro.adjustTime(astro.getMidnight()), recalculate);

	if (!getStatusByte(STATUS_TIME_CALCULATED))
	{
		if (sunsettime < sunrisetime)
			OpenDoor();
		else
			CloseDoor();
	}

	setStatusByte(STATUS_TIME_CALCULATED, true);
}

bool checkStatusByte()
{
	if (!getStatusByte(STATUS_TIME_CALCULATED))
		recalculate();
	return true;
}

void updateClimate()
{
	climate.Update();
}

void setSynchFlag()
{
	setStatusByte(STATUS_SYNCH_TIME, true);
}

bool updateTime()
{
	// too soon since last synch
	if (!getStatusByte(STATUS_SYNCH_TIME))
		return false;

	if (getStatusByte(SETTING_SYNCH_NTP))
	{
		// not possible to synch time with ntp server
		if (!timeClient.update())
			return false;

		// let's get on with it
		setTime(timeClient.getEpochTime());
	}

	

	setStatusByte(STATUS_TIME_SYNCHED, true);
	setStatusByte(STATUS_SYNCH_TIME, false);

	return true;
}

// push static pages
void handleRoot() { server.send(200, "text/html", MAIN_PAGE); }
void handleCSS() { server.send(200, "text/css", STYLE_CSS); }
void handleJS() { server.send(200, "text/javascript", SCRIPTS_JS); }

// create dynamic XML status page
void handleStatus()
{
	String xmldata =  "<?xml version=\"1.0\" encoding=\"UTF-8\"?><kiekenkot>";
	
	for (size_t i = 0; i < 3; i++)
	{
		xmldata += "<status name='" + names[i] + "' type = '" + types[i] + "'>" + values[i] + "</status>";
	}

	server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	server.sendHeader("Access-Control-Allow-Origin", "*");
	server.send(200, "text/xml", xmldata);

	climate.SendXML(&server);
	rfid.SendXML(&server);

	server.sendContent("</kiekenkot>");
	server.sendContent("");
}

void handleChangeDoor()
{
	String doorstate = server.arg("doorState");
	bool result = false;

	if (doorstate == "open")
		OpenDoor();
	else if (doorstate == "close")
		CloseDoor();

	handleStatus();
}

ICACHE_RAM_ATTR void readRFID()
{
	setStatusByte(FLAG_RFID_STATUS, true);
}

void setup()
{
	MotorControlClass::Init();
	Serial.begin(9600);
	WiFi.hostname("kiekenkot-test");

	WiFiManager wifimanager;
	wifimanager.autoConnect("KiekenkotAP");



	server.on("/", handleRoot);
	server.on("/style.css", handleCSS);
	server.on("/scripts.js", handleJS);
	server.on("/changeDoor", handleChangeDoor);
	server.on("/status", handleStatus);
	server.begin();

	timeClient.begin();
	synchTimer = Alarm.timerRepeat(0, TIME_SYNC_INTERVAL, 0, setSynchFlag);
	updateClimateTimer = Alarm.timerRepeat(0, 0, TIME_UPDATE_INTERVAL, updateClimate);

	attachInterrupt(digitalPinToInterrupt(RFIDIRQPIN), readRFID, RISING);
}

void loop()
{
	updateTime();
	checkStatusByte();

	if (getStatusByte(FLAG_RFID_STATUS)) {
		while (Serial.available())
			if (rfid.Parse(Serial.read()))
				rfid.Register();

		setStatusByte(FLAG_RFID_STATUS, false);
	}

	server.handleClient();
	Alarm.delay(0);
}
