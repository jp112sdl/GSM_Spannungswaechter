#include <SoftwareSerial.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#define A6_RX D4
#define A6_TX D3

SoftwareSerial A6board(A6_RX, A6_TX);

#define OLED_RESET  D5
Adafruit_SSD1306 display(OLED_RESET);

#define OK 1
#define NOTOK 2
#define TIMEOUT 3

#define Taster      D6
#define DebugSwitch D7

#define LEFT   0
#define CENTER 1
#define RIGHT  2
#define CLEARLINE  1

#define maxSMSReceiver 5
String SMSreceiver[maxSMSReceiver];

#define A0LowValue                    700 //analog Input LOW
#define A0Hysterese                   100 //analog Input Hysterese from LOW -> HIGH
#define checkSignalStrengthSeconds    120
#define DEBUGOutputSeconds            20
#define DisplaySMSsentTimeoutSeconds  20

boolean DEBUG   = true;
boolean A6DEBUG = true;

byte GSMRXLevel = 0;

String configFilename = "config.txt";

unsigned long lastMillis = 0;
unsigned long oldMillis = 0;
unsigned long debugMillis = 0;
unsigned long DisplaySMSsentTimeoutMillis = 0;
unsigned long noACPowerSince = 0;
unsigned long hasACPowerSince = 0;

byte ACPowerDownMins = 0;
byte ACPowerUpMins   = 0;
byte costPerSMS      = 9;

String lastReply;
String oldDisplayLine[8];

volatile byte interruptCounter = 0;
bool gsmActive = false;
bool hasACPower = true;

bool ACPowerDownAlertSent = false;
bool ACPowerUpAlertSent   = true;
String ACPowerDownMessage = "ACHTUNG Stromausfall!";
String ACPowerUpMessage = "Strom wieder da!";

char end_c[2];

boolean configMode = false;
extern const unsigned char toolBMP[];
extern const unsigned char checkBMP[];
extern const unsigned char cancelBMP[];
extern const unsigned char simErrorBMP[];

DNSServer dnsServer;
ESP8266WebServer webServer(80);

void setup() {
  end_c[0] = 0x1a;
  end_c[1] = '\0';
  pinMode(Taster, INPUT_PULLUP);
  pinMode(DebugSwitch, INPUT_PULLUP);
  pinMode(A0, INPUT);

  DEBUG = (digitalRead(DebugSwitch) == HIGH);
  A6DEBUG = (digitalRead(DebugSwitch) == HIGH);

  Serial.begin(115200);
  A6board.begin(9600);
  Serial.println("Start. DEBUG = " + String(DEBUG));


  initDisplay();

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    printDisplayLine(5, false, "SPIFFS ERROR!");
  } else {
    if (!loadConfig()) {
      printDisplayLine(5, false, "loadConfig ERROR!");
      Serial.println("Failed to load config");
      while (1 == 1);
    } else {
      Serial.println("Config loaded");
    }
  }

  if (digitalRead(Taster) == LOW) {
    Serial.println("Starte Config-Mode");
    configMode = true;
    startAPmode();
  } else {

    WiFi.mode(WIFI_OFF);
    drawGSMRXLevel();

    printDisplayLine(1, false, "Lade Config");

    gsmActive = true;
    printDisplayLine(1, false, "Warte auf GSM");
    for (int i = 0; i < maxSMSReceiver; i++) {
      printDisplayLine(i + 3, false, "SMS " + String(i + 1) + ": " + SMSreceiver[i]);
    }
    if (A6begin() != OK) {
      Serial.println("Error");
      printDisplayLine(1, false, "GSM ERROR!");
      while (1 == 1);
    }
    gsmActive = false;
    drawBalance();
    checkSignalStrength();
    printDisplayLine(1, false, "GSM OK");
    printDisplayLine(2, true,   "                     ");

    for (int i = 0; i < maxSMSReceiver; i++) {
      printDisplayLine(i + 3, false, " ");
    }
    attachInterrupt(digitalPinToInterrupt(Taster), handleInterruptCHANGE, CHANGE);
  }
}

void loop() {
  if (configMode) {
    dnsServer.processNextRequest();
    webServer.handleClient();
  } else {

    checkMillis();

    int analogValue = analogRead(A0);
    if (analogValue < A0LowValue) {
      if (hasACPower) {
        noACPowerSince = millis();
        hasACPowerSince = 0;
        Serial.println("Analogeingang < " + String(A0LowValue) + "!");
        hasACPower = false;
      }
    } else {
      if (analogValue > A0LowValue + A0Hysterese) {
        if (!hasACPower) {
          noACPowerSince = 0;
          hasACPowerSince = millis();
          Serial.println("Analogeingang > " + String(A0LowValue + 100) + "!");
          hasACPower = true;
        }
      }
    }

    if (!hasACPower) {
      printDisplayLine(4, false, "+++ Stromausfall! +++");
      String timeString = (((millis() - noACPowerSince) / 1000) < 60) ? String((millis() - noACPowerSince) / 1000) + " Sek." : String((millis() - noACPowerSince) / 1000 / 60) + " Min.";
      printDisplayLine(5, false, "seit: " + timeString, CENTER);

      if (!ACPowerDownAlertSent) {
        printDisplayLine(6, false, "Warte " + String(ACPowerDownMins) + " Minute(n)...");
      }
      else {
        printDisplayLine(6, false, "SMS versendet!", CENTER);
      }

    } else {
      printDisplayLine(4, false, " Eingangsspannung OK");
      printDisplayLine(5, CLEARLINE);
      if (!ACPowerUpAlertSent) {
        printDisplayLine(6, false, "Warte " + String(ACPowerUpMins) + " Minute(n)...");
      } else {
        if (millis() - DisplaySMSsentTimeoutMillis > DisplaySMSsentTimeoutSeconds * 1000) {
          if (DisplaySMSsentTimeoutMillis > 0)
            drawBalance();
          printDisplayLine(6, CLEARLINE);
        }
      }
    }

    if ((!ACPowerDownAlertSent)
        && (ACPowerDownMins > 0)
        && ((millis() - noACPowerSince) / 1000 / 60) >= ACPowerDownMins
        && !hasACPower) {
      Serial.println("Stromausfallbenachrichtigung wird gesendet!");
      sendSMStoAll(ACPowerDownMessage);
      ACPowerDownAlertSent = true;
      ACPowerUpAlertSent = false;
      saveConfig();
      delay(2000);
      drawBalance();
    }

    if ((!ACPowerUpAlertSent)
        && (ACPowerUpMins > 0)
        && ((millis() - hasACPowerSince) / 1000 / 60) >= ACPowerUpMins
        && hasACPower) {
      Serial.println("Strom OK Meldung wird gesendet!");
      sendSMStoAll(ACPowerUpMessage);
      printDisplayLine(6, false, " SMS versendet!", CENTER);
      ACPowerUpAlertSent = true;
      ACPowerDownAlertSent = false;
      saveConfig();
      DisplaySMSsentTimeoutMillis = millis();
    }

    if (millis() - debugMillis > DEBUGOutputSeconds * 1000 || debugMillis == 0) {
      if (DEBUG) {
        SerialDebug();
      }
    }

    if (interruptCounter > 0) {
      printDisplayLine(7, false, "- manuelle Abfrage -");
      drawBalance();
      checkSignalStrength();
      interruptCounter = 0;
      printDisplayLine(7, false, "");
    }

    if (A6board.available()) {
      Serial.write(A6board.read());
    }

    if ((millis() - lastMillis > checkSignalStrengthSeconds * 1000 || lastMillis == 0)) {
      checkSignalStrength();
      lastMillis = millis();
    }

    if (Serial.available()) {
      String serread = Serial.readStringUntil('\n');
      if (serread.startsWith("SET:") || serread.startsWith("GET:")) {
        processLocalConfig(serread);
      } else if (serread.startsWith("AT"))
        A6board.println(serread);
    }
  }
}

void SerialDebug() {
  Serial.println();
  Serial.println("DEBUG: ACPowerDownMins      = " + String(ACPowerDownMins));
  Serial.println("DEBUG: ACPowerUpMins        = " + String(ACPowerUpMins));
  Serial.println("DEBUG: noACPowerSince       = " + String(noACPowerSince));
  Serial.println("DEBUG: hasACPowerSince      = " + String(hasACPowerSince));
  Serial.println("DEBUG: hasACPower           = " + String(hasACPower ? "JA" : "NEIN"));
  Serial.println("DEBUG: ACPowerUpAlertSent   = " + String(ACPowerUpAlertSent ? "JA" : "NEIN"));
  Serial.println("DEBUG: ACPowerDownAlertSent = " + String(ACPowerDownAlertSent ? "JA" : "NEIN"));
  Serial.println();
  debugMillis = millis();
}

void handleInterruptCHANGE() {
  if (digitalRead(Taster) == LOW) {
    if (oldMillis == 0 || (millis() - oldMillis > 50)) {
      oldMillis = millis();
    }
  } else {
    interruptCounter++;
  }
}
