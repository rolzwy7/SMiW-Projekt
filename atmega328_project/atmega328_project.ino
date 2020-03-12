#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3); // RX , TX

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Serial.println("UNO starting");
  delay(3000);
  Serial.println("started");
}

void loop() {
  String IncomingString = "";
  boolean StringReady = false;
  while(mySerial.available()) {
    IncomingString=mySerial.readString();
    StringReady = true;
  }

  if(StringReady) {
     Serial.println("Recv : " + IncomingString);
  }

  mySerial.write("FromUNO");
  delay(250);
}
