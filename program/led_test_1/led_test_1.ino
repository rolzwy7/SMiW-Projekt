int val;

unsigned long _start;
bool toggle;

void setup() {
  Serial.begin(115200);

  pinMode(D2, INPUT);

  pinMode(D1, OUTPUT);  
  _start = millis();
  toggle = true;
};

void loop() {
  val = digitalRead(D2);
  Serial.println(val);

  if( millis() - _start > 1000) {
    _start = millis();
    if(toggle) {
      toggle = false;
      digitalWrite(D1, HIGH);
    } else {
      toggle = true;
      digitalWrite(D1, LOW);
    }
  }
 
};
