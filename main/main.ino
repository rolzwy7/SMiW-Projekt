#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <EEPROM.h>

#define BAUD 115200

unsigned long timestamp_http = millis();
int EEPROM_MAX = 1024;

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
String remember_ssid = "";
String remember_pwd = "";
String remember_pwd2 = "";

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
void eeprom_commit() {
  if (EEPROM.commit()) {
    Serial.println("EEPROM successfully committed");
  } else {
    Serial.println("ERROR! EEPROM commit failed");
  }
}

void setup() {
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
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  // ===================== EEPROM =====================
  EEPROM.begin(512);
  EEPROM.write(0, 110);
  EEPROM.write(1, 111);
  EEPROM.write(2, 119);
  EEPROM.write(3, 97);
  EEPROM.write(4, 107);
  eeprom_commit();
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

unsigned char h2int(char c) {
    if (c >= '0' && c <='9') return((unsigned char)c - '0');
    if (c >= 'a' && c <='f') return((unsigned char)c - 'a' + 10);
    if (c >= 'A' && c <='F') return((unsigned char)c - 'A' + 10);
    return(0);
}

String urldecode(String str) {
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (int i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        encodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        encodedString+=c;
      } else{
        encodedString+=c;  
      }
    }
   return encodedString;
}

void push_message(WiFiClient & client, String get_param, String alert_type, String msg) {
  bool is_present = (HTTP_HEADER.indexOf("event="+get_param + " HTTP/1.1") >= 0) ? true:false;
  if(is_present) {
    String alert = "<div class='alert alert-" + alert_type + "'>" + msg + "</div>";
    client.println(alert);
  }
}

void messages(WiFiClient & client) {
  push_message(client, "pwds-too-short", "danger", "Hasło jest zbyt krótkie (<8 znaków)");
  push_message(client, "pwds-too-long", "danger", "Hasło jest zbyt długie (>63 znaki)");
  push_message(client, "pwds-bad-seq", "danger", "Żadna podana wartość nie może zawierać znaku '='");
  push_message(client, "pwds-not-same", "danger", "Podane hasła nie są takie same");
  push_message(client, "add-ap-missing-params", "danger", "Żadna podana wartość nie może zawierać znaku '='");
  push_message(client, "success-ap-added", "success", "Dodano punkt dostępowy pomyślnie");
  push_message(client, "eeprom-cleared", "success", "EEPROM został wyczyszczony");
}

void extend_style(WiFiClient & client) {
  String style = "<style>";
  style += ".navbar {position: relative;display: flex;flex-wrap: wrap;align-items: center;justify-content: space-between;padding: 0.5rem 1rem;}";
  style += ".alert {position: relative;padding: 0.75rem 1.25rem;margin-bottom: 1rem;border: 1px solid transparent;border-radius: 0.25rem;}";
  style += ".alert-success {color: #155724;background-color: #d4edda;border-color: #c3e6cb;}";
  style += ".alert-warning {color: #856404; background-color: #fff3cd; border-color: #ffeeba;}";
  style += ".alert-warning .alert-link {color: #533f03;}";
  style += ".alert-danger {color: #721c24;background-color: #f8d7da;border-color: #f5c6cb;}";
  style += ".alert-danger .alert-link {color: #491217;}";
  style += ".alert-success {color: #155724;background-color: #d4edda;border-color: #c3e6cb;}";
  style += ".alert-success hr {border-top-color: #b1dfbb;}";
  style += ".alert-success .alert-link {color: #0b2e13;}";

  style += ".table {width: 100%;margin-bottom: 1rem;color: #212529;}";
  style += ".table th, .table td {padding: 0.75rem;vertical-align: top;border-top: 1px solid #dee2e6;}";
  style += ".table thead th {vertical-align: bottom;border-bottom: 2px solid #dee2e6;}";
  style += ".table-striped tbody tr:nth-of-type(odd) {background-color: rgba(0, 0, 0, 0.05);}";

  style += "</style>";
  client.println(style);
}

void extend_template(WiFiClient & client, String view) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println("Connection: close");
    client.println();
    client.println("<!DOCTYPE html>");
    client.println("<html lang='pl'>");
    client.println("<head>");
    extend_style(client);
    client.println("<meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>Czujnik zalania pomieszczenia - Projekt SMIW</title>");
    client.println("</head>");
    client.println("<body>");
    client.println("<nav class='navbar'>");
    client.println("<a href='/'>Strona główna</a>");
    client.println("</nav>");
    messages(client);
    client.println(view);
    client.println("</body>");
    client.println("</html>");
}

void config_view(WiFiClient & client) {
  String content = "<h2>Konfiguracja</h2>";
  content += "<table class='table table-striped'><tbody>";
  content += "<tr><td>MAC Address</td><td>" + String(WiFi.macAddress()) + "</td></tr>";
  content += "<tr><td>Soft AP SSID</td><td>" + String(g_SOFT_AP_SSID) + "</td></tr>";
  content += "<tr><td>Local IP</td><td>" + WiFi.localIP().toString() + "</td></tr>";
  content += "</tbody></table>";
  content += "<br>";
  content += "<form action='/add-ap' method='GET'>";
  content += "<input type='text' name='ssid' value='" + remember_ssid + "' required><br>";
  content += "<input type='password' name='pwd' value='" + remember_pwd + "' required><br>";
  content += "<input type='password' name='pwd2' value='" + remember_pwd2 + "' required><br>";
  content += "<input type='submit' name='submit' value='Dodaj punkt dostępowy'><br>";
  content += "</form>";
  extend_template(client, content);
}

void homepage_view(WiFiClient & client) {
  String content = "<b>Homepage</b>";
  content += "<br><a href='/config'>Konfiguracja</a>";
  content += "<br><a href='/eeprom-show'>Pokaż EEPROM</a>";
  content += "<br><a href='/eeprom-clear'>Wyczyść EEPROM</a>";
  extend_template(client, content);
}

void add_ap_view(WiFiClient & client) {
  Serial.println("[ ADD AP VIEW ]");
  String var_ssid = "";
  String var_pwd = "";
  String var_pwd2 = "";
  String event = "success-ap-added";
 
  if(HTTP_HEADER.indexOf("ssid=") >= 0) {
    int _from = HTTP_HEADER.indexOf("ssid=") + 5;
    int _to = HTTP_HEADER.indexOf("&", _from);
    var_ssid = HTTP_HEADER.substring(_from, _to);
  }
  if(HTTP_HEADER.indexOf("pwd=") >= 0) {
    int _from = HTTP_HEADER.indexOf("pwd=") + 4;
    int _to = HTTP_HEADER.indexOf("&", _from);
    var_pwd = HTTP_HEADER.substring(_from, _to);
  }
  if(HTTP_HEADER.indexOf("pwd2=") >= 0) {
    int _from = HTTP_HEADER.indexOf("pwd2=") + 5;
    int _to = HTTP_HEADER.indexOf("&", _from);
    var_pwd2 = HTTP_HEADER.substring(_from, _to);
  }

  if(var_ssid == "" || var_pwd == "" || var_pwd2 == "") { event = "add-ap-missing-params"; }
  var_ssid = urldecode(var_ssid);
  var_pwd = urldecode(var_pwd);
  var_pwd2 = urldecode(var_pwd2);
  Serial.println(var_ssid);
  Serial.println(var_pwd2);
  Serial.println(var_pwd);

  if(var_pwd != var_pwd2){ event = "pwds-not-same"; }
  if(var_pwd.indexOf("=") >= 0) { event = "pwds-bad-seq"; }
  if(var_pwd2.indexOf("=") >= 0) { event = "pwds-bad-seq"; }
  if(var_ssid.indexOf("=") >= 0) { event = "pwds-bad-seq"; }
  if(var_pwd.length() > 63) { { event = "pwds-too-long"; } }
  if(var_pwd.length() < 8) { { event = "pwds-too-short"; } }


  Serial.println(event);
  if(event == "success-ap-added") {
    Serial.println("YEY");
    remember_ssid = "";
    remember_pwd = "";
    remember_pwd2  = "";
  } else {
    remember_ssid = var_ssid;
    remember_pwd = var_pwd;
    remember_pwd2  = var_pwd2;
  }

  client.println("HTTP/1.1 301 Moved Permanently");
  client.println("Location: /config?event=" + event);
  client.println();
}

void dump_eeprom(WiFiClient & client) {
  String content = "";
  byte value;
  for(int address=0;address<EEPROM_MAX;++address){
    value = EEPROM.read(address);
    content += char(value);
  }
  extend_template(client, content);
}

void clear_eeprom(WiFiClient & client) {
  for (int i = 0; i < EEPROM_MAX; i++) {
    EEPROM.write(i, 0);
  }
  eeprom_commit();
  client.println("HTTP/1.1 301 Moved Permanently");
  client.println("Location: /?event=eeprom-cleared");
  client.println();
}

void router(WiFiClient & client) {
  // Configuration View
  if (HTTP_HEADER.indexOf("GET /config") >= 0) {
    config_view(client);
  // Homepage View
  } else if (HTTP_HEADER.indexOf("GET /add-ap") >= 0) {
    add_ap_view(client); 
  } else if (HTTP_HEADER.indexOf("GET /eeprom-show") >= 0) {
    dump_eeprom(client); 
  } else if (HTTP_HEADER.indexOf("GET /eeprom-clear") >= 0) {
    clear_eeprom(client); 
  } else if (HTTP_HEADER.indexOf("GET /") >= 0) {
    homepage_view(client);
  } else {
    extend_template(client, "Unknown route");
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
