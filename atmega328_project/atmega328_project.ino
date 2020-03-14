#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX , TX

int WATER_SENSOR_AN_PIN = A3;

unsigned long time;

unsigned long ROOM_FLOODED_TIME;
bool ROOM_FLOODED;
int GT_FLOOD;


void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("UNO starting");
  delay(1000);
  Serial.println("started");

  ROOM_FLOODED = false;
  GT_FLOOD = 500;
  pinMode(7, OUTPUT);
}

void loop() {
  time = millis();
  int water_sensor_val = analogRead(WATER_SENSOR_AN_PIN);

  // Detect flood
  if(!ROOM_FLOODED && water_sensor_val > GT_FLOOD) {
    ROOM_FLOODED = true;
    ROOM_FLOODED_TIME = millis();
    digitalWrite(7, HIGH);
    mySerial.write("SEND_SMS_TO_RECIPIENT_LIST");
  } else {
    //
    if(time - ROOM_FLOODED_TIME > 3000) {
      ROOM_FLOODED = false;
      digitalWrite(7, LOW);
    }
    //
  }
 
  String IncomingString = "";
  boolean StringReady = false;
  while(mySerial.available()) {
    IncomingString=mySerial.readString();
    StringReady = true;
  }

  if(StringReady) {
     Serial.println("Recv : " + IncomingString);
  }


}
