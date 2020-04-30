#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <EEPROM.h>

#define BAUD 115200

unsigned long timestamp_http = millis();

// Wifi Soft AP
const char *g_SOFT_AP_SSID     = "SMIW_PROJ_AP_BN";
const char *g_SOFT_AP_PASSWORD = "smiw_password";

// Alarm Message
String DEFAULT_MESSAGE = "Pomieszczenie%20zalane!!!";
String DEFAULT_PHONE_NUMBER = "517174906";

// HTTP server
const int HTTP_PORT = 80;
WiFiServer server(HTTP_PORT);
String HTTP_HEADER;
const long HTTP_TIMEOUT = 2000;

// Wifi Station
const char *g_WIFI_STATION_SSID     = "FunBox3-1482";
const char *g_WIFI_STATION_PASSWORD = "T-y9*yS528";

void wifDisconnect() { WiFi.disconnect(); }
void wifiConnect() { WiFi.begin(g_WIFI_STATION_SSID, g_WIFI_STATION_PASSWORD); }
void wifiBlockUntilConnected() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
   Serial.println("");
}

void setup() {
  // ===================== EEPROM =====================
  // EEPROM.begin(512);
  // ===================== SETUP =====================
  Serial.begin(BAUD);
  while (!Serial) { continue; }
  Serial.println("Serial begin");
  Serial.println();
  Serial.println("\n[*** ESP Serial Begin ***]\n");
  for (uint8_t t = 1; t < 4; t++) {
    Serial.printf("[Setup] Waiting %d/3\n", t);
    Serial.flush();
    delay(1000);
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
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  // ===================== HTTP Server =====================
  Serial.print("Port:");
  Serial.println(HTTP_PORT);
  server.begin();
}

void extend_template(WiFiClient & client, String view) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html lang='pl'>");
    client.println("<head>");
    client.println("<meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>Czujnik zalania pomieszczenia - Projekt SMIW</title>");
    client.println("</head>");
    client.println("<body>");
    client.println("<a href='/'>Homepage Link</a><br>");
    client.println(view);
    client.println("</body>");
    client.println("</html>");
}

void config_view(WiFiClient & client) {
  String content = "<b>Config</b>";
  extend_template(client, content);
}

void homepage_view(WiFiClient & client) {
  String content = "<b>Homepage</b><br><a href='/config'>config link</a>";
  extend_template(client, content);
}


void router(WiFiClient & client) {
  // Configuration View
  if (HTTP_HEADER.indexOf("GET /config") >= 0) {
    config_view(client);
  // Homepage View
  } else if (HTTP_HEADER.indexOf("GET /") >= 0) {
    homepage_view(client);
  // 404 View
  } else {
    extend_template(client, "<b>Homepage</b><br><a href='/config'>config link</a>");
  }
}

void loop() {
  WiFiClient client = server.available();
  if(client) {
    String line = "";
  
    timestamp_http = millis();
    while (client.connected() && millis() - timestamp_http <= HTTP_TIMEOUT) { // Request loop
      if (client.available()) { // if data available
        char c = client.read();
        Serial.write(c);
        HTTP_HEADER += c;
        if(c == '\n') {
          if (line.length() == 0) {
            // Response - Start
            router(client);
            // Response - End
            break;
          } else {
            line = "";
          }
        } else if (c != '\r') {
          line += c;
        }
        
      }
      
    }
 
    HTTP_HEADER = "";
    client.stop();
    Serial.println("New Client.");
    Serial.println("Client disconnected.");
  }
}
