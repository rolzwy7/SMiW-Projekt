/*
 void setup() {
  pinMode(D3, OUTPUT);
};

void loop() {
  digitalWrite(D3, HIGH);
  delay(750);
  digitalWrite(D3, LOW);
  delay(750);
};

*/
int val;

void setup() { // D6 -> D7
  pinMode(D6, OUTPUT);
  pinMode(D7, INPUT);
  Serial.begin(115200);
  digitalWrite(D6, HIGH);
};

void loop() {
  val = digitalRead(D7);
  Serial.println(val);
};
