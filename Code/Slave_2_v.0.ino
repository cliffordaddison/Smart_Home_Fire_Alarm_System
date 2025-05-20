#include <Wire.h>

#define SLAVE_ID 2

#define GAS_PIN A0
#define TEMP_PIN A1
#define PIR_PIN 8
#define LDR_PIN A2
#define BUZZER_PIN 6
#define RED_LED 7
#define GREEN_LED 9

#define GAS_THRESHOLD 650
#define TEMP_THRESHOLD 50
#define MOTION_DEBOUNCE 200

unsigned long lastMotionTime = 0;
bool motionDetected = false;

void setup() {
  Wire.begin(SLAVE_ID);
  Wire.onRequest(sendData);
  Wire.onReceive(receiveCommand);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  digitalWrite(GREEN_LED, HIGH);
}

void loop() {
  bool currentState = digitalRead(PIR_PIN);
  static bool lastState = false;
  
  if(currentState && !lastState) {
    if(millis() - lastMotionTime > MOTION_DEBOUNCE) {
      motionDetected = true;
      lastMotionTime = millis();
    }
  } 
  else if(motionDetected && millis() - lastMotionTime > 2000) {
    motionDetected = false;
  }
  
  lastState = currentState;
  delay(50);
}

void sendData() {
  int gas = analogRead(GAS_PIN);
  float temp = (analogRead(TEMP_PIN) * 0.488) - 50;
  int light = analogRead(LDR_PIN);
  
  Wire.write((byte*)&gas, 2);
  Wire.write((byte*)&temp, 4);
  Wire.write((byte*)&light, 2);
  Wire.write(motionDetected);
}

void receiveCommand(int bytes) {
  if(Wire.available()) {
    byte cmd = Wire.read();
    digitalWrite(RED_LED, cmd == 1);
    digitalWrite(GREEN_LED, cmd != 1);
    if(cmd == 1) tone(BUZZER_PIN, 1000);
    else noTone(BUZZER_PIN);
  }
}
