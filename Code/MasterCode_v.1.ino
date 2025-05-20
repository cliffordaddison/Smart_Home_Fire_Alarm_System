#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>

#define SLAVE_COUNT 4
#define ALERT_DURATION 4000

Adafruit_LiquidCrystal lcdA(0x20);
Adafruit_LiquidCrystal lcdB(0x21);

struct RoomData {
  int gas;
  float temp;
  int light;
  bool motion;
  bool fire;
  unsigned long lastAlert;
};

RoomData rooms[SLAVE_COUNT];

void setup() {
  Wire.begin();
  Serial.begin(9600);

  lcdA.begin(16, 2);
  lcdB.begin(16, 2);
  lcdA.print("System Ready");
  lcdB.print("System Ready");
  delay(2000);
}

void loop() {
  bool anyFire = false;
  int fireRoom = -1;
  int motionRoom = -1;

  for (int i = 0; i < SLAVE_COUNT; i++) {
    if(Wire.requestFrom(i+1, 9) == 9) {
      rooms[i].gas = Wire.read() | (Wire.read() << 8);
      byte* tempBytes = (byte*)&rooms[i].temp;
      for(int j=0; j<4; j++) tempBytes[j] = Wire.read();
      rooms[i].light = Wire.read() | (Wire.read() << 8);
      rooms[i].motion = Wire.read();
      
      bool smoke = rooms[i].gas > 650;
      bool highTemp = rooms[i].temp > 50;
      rooms[i].fire = (smoke && highTemp) || (rooms[i].motion && highTemp && smoke);

      if(rooms[i].fire) fireRoom = i+1;
      else if(rooms[i].motion) motionRoom = i+1;
    }
    delay(10);
  }

  for(int i=1; i<=SLAVE_COUNT; i++) {
    Wire.beginTransmission(i);
    Wire.write(fireRoom > 0 ? 1 : 2);
    Wire.endTransmission();
  }

  if(fireRoom > 0) {
    lcdA.clear(); lcdB.clear();
    lcdA.print("Fire Room " + String(fireRoom));
    lcdA.setCursor(0,1); lcdA.print("EVACUATE NOW!");
    lcdB.print("Fire Room " + String(fireRoom));
    lcdB.setCursor(0,1); lcdB.print("EVACUATE NOW!");
    Serial.print(">>>>>>>>>>>>>>>>>>>>>>> FIRE ROOM " + String(fireRoom) + " <<<<<<<<<<<<<<<<<<<<<<<\n");
  } 
  else if(motionRoom > 0) {
    lcdA.clear(); lcdB.clear();
    lcdA.print("Motion Room " + String(motionRoom));
    lcdA.setCursor(0,1); lcdA.print("Detected");
    lcdB.print("Motion Room " + String(motionRoom));
    lcdB.setCursor(0,1); lcdB.print("Detected");
    Serial.print(">>>>>>>>>>>>>>>>>>>>> MOTION ROOM " + String(motionRoom) + " <<<<<<<<<<<<<<<<<<<<<\n");
  } 
  else {
    lcdA.clear(); lcdB.clear();
    lcdA.print("R1:" + statusText(0));
    lcdA.setCursor(0,1); lcdA.print("R2:" + statusText(1));
    lcdB.print("R3:" + statusText(2));
    lcdB.setCursor(0,1); lcdB.print("R4:" + statusText(3));
    Serial.print("======================= READINGS =======================\n");
  }

  for(int i=0; i<SLAVE_COUNT; i++) {
    Serial.print("Room "); Serial.print(i+1); Serial.print(" | "); 
    Serial.print(" Gas:"); Serial.print(rooms[i].gas); Serial.print(" | "); 
    Serial.print(" Temp:"); Serial.print(rooms[i].temp,1); Serial.print(" | "); 
    Serial.print(" Light:"); Serial.print(rooms[i].light); Serial.print(" | "); 
    Serial.print(" Motion:"); Serial.println(rooms[i].motion ? "YES" : "NO");
    }
  
  delay(250);
}

String statusText(int i) {
  if(rooms[i].fire) return "FIRE!";
  if(rooms[i].light > 250) return "BRIGHT!";
  if(rooms[i].temp > 50) return "HOT!";
  if(rooms[i].gas > 650) return "SMOKE!";
  if(rooms[i].motion) return "MOTION";
  return "Normal";
}
