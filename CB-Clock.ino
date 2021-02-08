/*******************************************************************************************
 **                                www.bastelgarage.ch                                    **
 ** Der Onlineshop mit Videoanleitungen und kompletten Bausätzen für Anfänger und Profis! **
 *******************************************************************************************
 ** Autor: Alf Müller                                                                     **
 ** Datum: Mai 2018                                                                       **
 ** Version: 2.0                                                                          ** 
 ** Lizenz: CC BY-NC-ND 2.5 CH                                                            **
 *******************************************************************************************
*/

/************************( Importieren der genutzten Bibliotheken )************************/

#include "Arduino.h"
#include <ESP8266WiFi.h>                    // https://www.youtube.com/watch?v=vhm09S15toI
#include "libs\ArduinoJson\ArduinoJson.h"   // included in this download - do not update
#include "libs\Timezone\src\Timezone.h"         //https://github.com/JChristensen/Timezone - inc.
#include "libs\Time\TimeLib.h"              //https://github.com/PaulStoffregen/Time - inc.
#include <WiFiUdp.h>
#include <WiFiClientSecure.h>


/************************************(Wifi Einstellungen)**********************************/
const char* ssid     = "**************";     // SSID of local network
const char* password = "**************";   // Password on network

/**************************( Chaturbate Details )*******************************/
// Chaturbate Followers and token Balance with convert to dollars
String cbusername = "**************";  // < < < Enter in YOUR Chaturbate username
String cbtoken = "**************"; // Enter in YOUR Chaturbate token from https://chaturbate.com/statsapi/authtoken/


// NTP Serverpool für die Schweiz:
static const char ntpServerName[] = "pool.ntp.org";
//===========================================
// change this to your timezone - 0 is the UK
const int timeZone = 0;
//===========================================

WiFiUDP Udp;
unsigned int localPort = 8888;              
time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);
String dateString;

WiFiClient client; // Dont touch this line

/********************( Chaturbate variables and settings )********************/
// Chaturbate Site Fingerprint renews Oct 2021
#define CHATURBATE_FINGERPRINT "16 a5 01 0d a1 67 f5 f1 4f 99 9c dd 00 16 18 c5 97 9b b9 44"

// Connect to host Secure
WiFiClientSecure modelClient; // Dont touch this line - makes CB work

int numFollowers;
float tokenBalance;
String cbFollowerString;
float resultPayout;

/********************( Define the global variables and settings )********************/

#define NUM_MAX 8

// for D1 Mini = ESP8266 CONNECTINNG TO THE DISPLAY
#define DIN_PIN 13  // D7
#define CS_PIN  12  // D6
#define CLK_PIN 14  // D5

#include "max7219.h"
#include "fonts.h"


bool bNTPStarted = false;
bool gbIsConnecting = false;

#define MAX_DIGITS 16
byte dig[MAX_DIGITS] = {0};
byte digold[MAX_DIGITS] = {0};
byte digtrans[MAX_DIGITS] = {0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx = 0;
int dy = 0;
byte del = 0;
int h, m, s, w, mo, ye, d;
const char* wochenstring;
float utcOffset = 0;
long localEpoc = 0;
long localMillisAtUpdate = 0;


/*****************************************( Setup )****************************************/
void setup(){

  Serial.begin(115200);
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN, 1);
  sendCmdAll(CMD_INTENSITY, 0);
  Serial.print("Connecting to WiFi ");
  WiFi.begin(ssid, password);
  printStringWithShift(ssid, 15);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected: "); Serial.println(WiFi.localIP());
  printStringWithShift("         Connected", 15);

  // Chaturbate Fingerprint updates October 2021
  modelClient.setFingerprint(CHATURBATE_FINGERPRINT);

  Udp.begin(localPort);
  Serial.print("local port: ");
  Serial.println(Udp.localPort());
  Serial.println("Waiting for synchronization");
  setSyncProvider(getNtpTime);
  setSyncInterval(86400);                    // Syncs clock every 1 Day


    Serial.println("Connecting to Chaturbate...");
    getCBFollowerData();
    Serial.println("Chaturbate Data Loaded");

}

/*************************************(Hauptprogramm)**************************************/
void loop()
{
  if (updCnt <= 0) { // every 10 scrolls, ~450s = 7.5m
    updCnt = 10;
    Serial.println("Sync Time..");
    getNtpTime();
    Serial.println("Time Synced");
    clkTime = millis();
  } else if (updCnt <= 0) { 
    updCnt = 40; // every 40 scrolls, ~1800s = 60m = 1hr
    Serial.println("Updating CB...");
    getCBFollowerData();
    Serial.println("CB Updated");
    clkTime = millis();
  } 

  if (millis() - clkTime > 120000 && !del && dots) { // clock for 2 minutes, then scrolls for about 30s
    printStringWithShift(dateString.c_str(), 25);
    printStringWithShift(cbFollowerString.c_str(), 25);
    delay(3000);
    updCnt--;
    clkTime = millis();
  }

  if (millis() - dotTime > 1500) {
    dotTime = millis();
    dots = !dots;
  }
  getTimeLocal();
  showAnimClock();
}
