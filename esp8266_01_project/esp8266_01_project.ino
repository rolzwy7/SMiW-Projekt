#define G_DEBUG 1
#define BAUD 9600

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

// Alarm Message
String DEFAULT_MESSAGE = "Pomieszczenie%20zalane!!!";
String DEFAULT_PHONE_NUMBER = "517174906";

// IFTTT Url & Fingerprint
// IFTTT_URL = "https://maker.ifttt.com/trigger/sms_notify_574235896/with/key/qqGPpslrMv-cNGVOFdXJa?value1=519128819&value2=Wow"
String IFTTT_URL = "https://maker.ifttt.com/trigger/sms_notify_574235896/with/key/qqGPpslrMv-cNGVOFdXJa";
String IFTTT_FINGERPRINT = "aa 75 cb 41 2e d5 f9 97 ff 5d a0 8b 7d ac 12 21 08 4b 00 8c";

// Wifi Soft AP
const char *g_SOFT_AP_SSID     = "SMIW_PROJ_AP_BN";
const char *g_SOFT_AP_PASSWORD = "smiw_password";

// Wifi Station
const char *g_WIFI_STATION_SSID     = "FunBox3-1482";
const char *g_WIFI_STATION_PASSWORD = "T-y9*yS528";

// HTTP Client
HTTPClient http;

String getAlarmLink(String phone_number, String alarm_message) {
  return IFTTT_URL + "?value1=" + phone_number + "&value2=" + DEFAULT_MESSAGE;
}

void wifDisconnect() {
  WiFi.disconnect();
}

void wifiConnect() {
  WiFi.begin(g_WIFI_STATION_SSID, g_WIFI_STATION_PASSWORD);
}

void wifiBlockUntilConnected() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
   Serial.println("");
}

void sendAlarmSMS(String phone_number, String alarm_message) {
  wifiBlockUntilConnected();
  String ifttt_url_local = getAlarmLink(phone_number, alarm_message);
  Serial.println("Sending to : " + ifttt_url_local);
  http.begin(ifttt_url_local, IFTTT_FINGERPRINT);
  int httpCode = http.GET();
  Serial.println(httpCode);
  //Send the request
  if (httpCode > 0)
  {
    String payload = http.getString();
    Serial.println(payload);
    /////////////////
    /*
    StaticJsonDocument<200> doc;
    String payload = http.getString();
    Serial.println(payload);
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      return;
    }
    bool json_response = doc["success"];
    Serial.print("JSON success : ");
    Serial.println(json_response);
    */
    /////////////////
  }
  http.end();
}

void setup() {
  // ===================== SETUP =====================
  Serial.begin(BAUD);
  while (!Serial) { continue; }
  Serial.println("Serial begin");

  Serial.println();
  Serial.println("\n[*** ESP Serial Begin ***]\n");
  for (uint8_t t = 1; t < 11; t++) {
    Serial.printf("[Setup] Waiting %d/10\n", t);
    Serial.flush();
    delay(100);
  }
  // ===================== Soft AP =====================
  Serial.print("[Wifi Soft AP] Starting, ");
  Serial.print(", SSID :=");
  Serial.print(g_SOFT_AP_SSID);
  WiFi.softAP(g_SOFT_AP_SSID, g_SOFT_AP_PASSWORD);
  Serial.print(", IP := ");
  Serial.println(WiFi.softAPIP()); 
  // ===================== WiFi Station =====================
  wifDisconnect();
  Serial.print("[Wifi Station] Connecting to ");
  Serial.print(g_WIFI_STATION_SSID);
  wifiConnect();
  wifiBlockUntilConnected();
  Serial.println(" done");
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
      #if G_DEBUG
        Serial.println("Command : " + IncomingString);
      #endif
      if(IncomingString == "SEND_SMS_TO_RECIPIENT_LIST") {
        Serial.write("executing SEND_SMS_TO_RECIPIENT_LIST");
        sendAlarmSMS(DEFAULT_PHONE_NUMBER, DEFAULT_MESSAGE);
      }
    }
  ////////////////////////////
  ////////////////////////////
}
