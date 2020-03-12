#define G_DEBUG 1

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>


// Wifi Soft AP
const char *g_SOFT_AP_SSID     = "SMIW_PROJ_AP_BN";
const char *g_SOFT_AP_PASSWORD = "smiw_password";

// Wifi Station
const char *g_WIFI_STATION_SSID     = "FunBox3-1482";
const char *g_WIFI_STATION_PASSWORD = "T-y9*yS528";    

void setup() {
  // ===================== SETUP =====================
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("\n[*** ESP Serial begin ***]\n");
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(250);
  }
  // ===================== Soft AP =====================
  Serial.print("[Wifi Soft AP] Starting, ");
  Serial.print(", SSID :=");
  Serial.print(g_SOFT_AP_SSID);
  WiFi.softAP(g_SOFT_AP_SSID, g_SOFT_AP_PASSWORD);
  Serial.print(", IP := ");
  Serial.println(WiFi.softAPIP()); 
  // ===================== WiFi Station =====================
  Serial.print("[Wifi Station] Connecting to ");
  Serial.print(g_WIFI_STATION_SSID);
  Serial.println(" ...");
  WiFi.begin(g_WIFI_STATION_SSID, g_WIFI_STATION_PASSWORD);
}

void loop() {
  // Czytaj komende
  String IncomingString = "";
  boolean StringReady = false;
  while(Serial.available() > 0) {
    IncomingString = Serial.readString();
    StringReady = true;
  }
  //
  if(StringReady) {
      Serial.println("Command : " + IncomingString);
      if(IncomingString == "SEND_SMS_TO_RECIPIENT_LIST") {
        Serial.println("Executing: SEND_SMS_TO_RECIPIENT_LIST");
        Serial.write("executing SEND_SMS_TO_RECIPIENT_LIST");
      }
    }


  Serial.println("delay");
  delay(10000);
}
