#include <EEPROM.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define BAUD 115200

unsigned long timestamp_http = millis();
int EEPROM_IT = 9;

bool is_connected = false;
String connected_to = "<nie połączono>";
bool added_ap = false;

// Wifi Soft AP
const char *g_SOFT_AP_SSID     = "SMIW_PROJ_AP_BN";
const char *g_SOFT_AP_PASSWORD = "smiw_password";

String SSIDs[][2] = {{"", ""},{"", ""},{"", ""},{"", ""},{"", ""},{"", ""},{"", ""},{"", ""},{"", ""},{"", ""}};

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
void wifiConnect(String ssid, String password) {
  Serial.print("[Wifi Station] Trying to connect to: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
}
bool wifiBlockUntilConnected() {
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    ++count;
    if(count > 8) { return false; }
  }
   Serial.println("");
   return true;
}

void connection_routine() {

  for(int i=0;i<10;++i){
    String ssid = SSIDs[i][0];
    String passwd = SSIDs[i][1];
    if(ssid == "") { continue; }
    wifDisconnect();
    Serial.println("[Wifi Station]");
    wifiConnect(ssid, passwd);
    if(wifiBlockUntilConnected()) {
      Serial.println(" done");
      Serial.print("IP address:\t");
      Serial.println(WiFi.localIP());
      connected_to = ssid;
      is_connected = true;
      break;
    } else {
      Serial.println("failed");
    }
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
  byte eeprom_value;
  for(int i=9;i<EEPROM.length();++i){
    eeprom_value = EEPROM.read(i);
    if (eeprom_value == 0) {
      EEPROM_IT = i;
      break;
    }
  }
  // ===================== Soft AP =====================
  Serial.print("[Wifi Soft AP] Starting, ");
  Serial.print(", SSID :=");
  Serial.print(g_SOFT_AP_SSID);
  WiFi.softAP(g_SOFT_AP_SSID, g_SOFT_AP_PASSWORD);
  Serial.print(", IP := ");
  Serial.println(WiFi.softAPIP());
  delay(1500);
  // ===================== Make pass arr ===================== 
  ssid_make_table();
  // ===================== WiFi Station =====================
  connection_routine();
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
  if(added_ap) {
    client.println("<div class='alert alert-danger'>Tablica punktów dostępowych została zmodyfikowana. Aby zmiany były uwzględniane należy <a href='/esp-restart'>zrestartować</a> urządzenie</div>");
  }
  push_message(client, "pwds-too-short", "danger", "Hasło jest zbyt krótkie (<8 znaków)");
  push_message(client, "pwds-too-long", "danger", "Hasło jest zbyt długie (>63 znaki)");
  push_message(client, "pwds-bad-seq", "danger", "Żadna podana wartość nie może zawierać znaku '='");
  push_message(client, "pwds-not-same", "danger", "Podane hasła nie są takie same");
  push_message(client, "add-ap-missing-params", "danger", "Żadna podana wartość nie może zawierać znaku '='");
  push_message(client, "success-ap-added", "success", "Dodano punkt dostępowy pomyślnie");
  push_message(client, "ssid-too-short", "danger", "SSID jest zbyt krótkie");
  push_message(client, "memory-exceeded", "danger", "Pamięć EEPROM została wyczerpana i nie może pomieścić tego punktu dostępowego");
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

  style += "td {width:50%;}";

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

void ssid_make_table() {
  String content="";
  byte value;
  for(int address=0;address<EEPROM.length();++address){
    value = EEPROM.read(address);
    if(value==0){break;}
    content += char(value);
  }

  int arr_it = 0;
  while(content.indexOf("==") > 0) {
    int _from = content.indexOf("=") + 1;
    int _to = content.indexOf("=", _from);
    String ssid = content.substring(_from, _to);
   
    _from = content.indexOf("=", _to) + 1;
    _to = content.indexOf("==", _from);
    String passwd = content.substring(_from, _to);

    content = content.substring(_to+2, content.length());

    Serial.print(ssid);
    Serial.print(" - ");
    Serial.println(passwd);
    SSIDs[arr_it][0] = ssid;
    SSIDs[arr_it][1] = passwd;
    arr_it++;

  }
}

void config_view(WiFiClient & client) {
  String content = "";
  if(is_connected) {
    content += "<h2>Status: połączono</h2>";
  } else {
    content += "<h2>Status: nie połączono</h2>";
  }
  content += "<h2>Konfiguracja</h2>";
  content += "<table class='table table-striped'><tbody>";
  content += "<tr><td>MAC Address</td><td>" + String(WiFi.macAddress()) + "</td></tr>";
  content += "<tr><td>Soft AP SSID</td><td>" + String(g_SOFT_AP_SSID) + " (" + WiFi.softAPIP().toString() + ")</td></tr>";
  content += "<tr><td>Local IP</td><td>" + connected_to + " (" + WiFi.localIP().toString() + ")</td></tr>";
  content += "<tr><td>EEPROM</td><td>" + String(EEPROM_IT) + "/" + String(EEPROM.length()) + "</td></tr>";
  content += "</tbody></table>";
  content += "<hr>";
  content += "<h2>Punkty dostępowe</h2>";
  content += "<form action='/add-ap' method='GET'>";
  content += "<table>";
  content += "<tr><td><b>SSID</b></td><td><input type='text' name='ssid' value='" + remember_ssid + "' required></td></tr>";
  content += "<tr><td><b>Hasło</b></td><td><input type='password' name='pwd' value='" + remember_pwd + "' required></td></tr>";
  content += "<tr><td><b>Powtórz hasło</b></td><td><input type='password' name='pwd2' value='" + remember_pwd2 + "' required></td></tr>";
  content += "<tr><td></td><td><input type='submit' name='submit' value='Dodaj punkt dostępowy'></td></tr>";
  content += "</table>";
  content += "</form>";
  content += "<table class='table table-striped'><thead><tr><th>SSID</th><th>Hasło</th></tr></thead><tbody>";
  for(int i=0;i<10;++i){
    content += "<tr>";
    if(connected_to == SSIDs[i][0]) {
      content += "<td>" + SSIDs[i][0] + " (connected)</td>";
    } else {
      content += "<td>" + SSIDs[i][0] + "</td>";
    }
    content += "<td>" + SSIDs[i][1] + "</td>";
    content += "</tr>";
  }
  content += "</tbody></table>";
  
  extend_template(client, content);
}

void homepage_view(WiFiClient & client) {
  String content = "<b>Homepage</b>";
  content += "<br><a href='/config'>Konfiguracja</a>";
  content += "<br><a href='/eeprom-show'>Pokaż EEPROM</a>";
  content += "<hr>";
  content += "<br><a href='/wyczysc-pamiec'>Wyczyść EEPROM</a>";
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

  if(var_ssid.length() <=0) { event = "ssid-too-short"; }
  
  if(var_pwd.length() > 63) { { event = "pwds-too-long"; } }
  if(var_pwd.length() < 8) { { event = "pwds-too-short"; } }

  Serial.println(event);
  if(event == "success-ap-added") {
    String new_ap = "="+var_ssid+"="+var_pwd+"==";
    bool memory_error = false;
    int i;
    int ch=0;
    for(i=EEPROM_IT;i<EEPROM_IT+new_ap.length();++i) {
      if(i >= EEPROM.length()) {
        break;
        memory_error=true;
      }
      EEPROM.write(i, new_ap.charAt(ch++));
    }
    EEPROM_IT=i;
    if(!memory_error) {
      EEPROM.commit();
      remember_ssid = "";
      remember_pwd = "";
      remember_pwd2  = "";
      added_ap = true;
      ssid_make_table();
    } else {
      event="memory-exceeded";
     }
  } else {
    remember_ssid = var_ssid;
    remember_pwd = var_pwd;
    remember_pwd2  = var_pwd2;
  }
 
  client.println("HTTP/1.1 307 Temporary Redirect");
  client.println("Location: /config?event=" + event);
  client.println();
}


void pokaz_eeprom_view(WiFiClient & client) {
  String content = "";
  byte value;
  for(int address=0;address<EEPROM.length();++address){
    value = EEPROM.read(address);
    content += char(value);
  }
  content = "<pre>" + content + "</pre>";
  extend_template(client, content);
}

void wyczysc_pamiec_view(WiFiClient & client) {
  for (int ad = 0; ad < EEPROM.length(); ad++) {
    EEPROM.write(ad, 0);
  }
  EEPROM.commit();
  EEPROM_IT = 0;
  extend_template(client, "Pamięć została wyczyszczona.");
}

void esp_restart_view(WiFiClient & client) {
  String content = "";
  content += "<p>Czujniki zalania jest w fazie restartu.</p><p>Przekierowanie na stronę główną nastąpi za <span id='msg'>20</span></p>";
  content += "<script>var ttr = 20;setInterval(function() {ttr = ttr - 1;document.getElementById('msg').innerText = ttr;if(ttr == 0) {window.location = '/';}},1000);</script>";
  extend_template(client, content);
  ESP.restart();
}

void router(WiFiClient & client) {
  // Configuration View
  if (HTTP_HEADER.indexOf("GET /config") >= 0) {
    config_view(client);
  // Homepage View
  } else if (HTTP_HEADER.indexOf("GET /add-ap") >= 0) {
    add_ap_view(client); 
  } else if (HTTP_HEADER.indexOf("GET /eeprom-show") >= 0) {
    pokaz_eeprom_view(client); 
  } else if (HTTP_HEADER.indexOf("GET /wyczysc-pamiec") >= 0) {
    wyczysc_pamiec_view(client);
  } else if (HTTP_HEADER.indexOf("GET /esp-restart") >= 0) {
    esp_restart_view(client);
  } else if (HTTP_HEADER.indexOf("GET /") >= 0) {
    homepage_view(client);
  } else {
    extend_template(client, "Unknown route");
  }

}

void loop() {
  is_connected = WiFi.status() == WL_CONNECTED;
  
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
