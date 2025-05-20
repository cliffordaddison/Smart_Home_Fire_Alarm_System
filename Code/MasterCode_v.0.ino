#include <Wire.h>
#include <Adafruit_LiquidCrystal.h>
Adafruit_LiquidCrystal lcd_1(0);
Adafruit_LiquidCrystal lcd_2(1);

const int timePotPin = A0; // Potentiometer connected here

void setup() {
  
  lcd_1.begin(16,2);
  lcd_2.begin(16,2);

  lcd_1.setCursor(0, 0);
  lcd_2.setCursor(0, 0);
  
  Wire.begin(); // Start as I2C master
  Serial.begin(9600);
}

void loop() {
  int potValue = analogRead(timePotPin);
  String command;

  if (potValue <= 400) {
    command = "NORMAL";  // Daytime mode
  } else if (potValue <= 700) {
    command = "DIM";     // After 9 PM
  } else {
    command = "OFF";     // After 11 PM
  }
  lcd_1.setCursor(0, 0);
  //lcd_1.print(command);
  lcd_2.setCursor(0, 0);
  // Send command to all room Arduinos with addresses 0x01 to 0x04
  for (byte addr = 0x01; addr <= 0x04; addr++) {
    sendCommand(addr, command);
  }
  
  // Receive command from all room Arduinos with addresses 0x01 to 0x04
  for (byte addr = 0x01; addr <= 0x04; addr++) {
  	String req_data = requestCommand(addr);
  	Serial.print("Response from ");
  	Serial.print(addr);
	Serial.print(": ");
	Serial.println(req_data);
    if(addr == 0x01){
    	lcd_1.print(req_data);
    }else if(addr == 0x02){
      lcd_1.setCursor(0, 1);
      lcd_1.print(req_data);
    }else if(addr == 0x03){
    	lcd_2.print(req_data);
    }else if(addr == 0x04){
      lcd_2.setCursor(0, 1);
      lcd_2.print(req_data);
    }

  }

  Serial.print("Potentiometer: ");
  Serial.print(potValue);
  Serial.print(" | Mode: ");
  Serial.println(command);

  //delay(100);  // Update every 0.1 seconds
}


//To send commands to All Slaves
void sendCommand(byte address, String cmd) {
  Wire.beginTransmission(address);
  Wire.write(cmd.c_str());
  Wire.endTransmission();
}

//To receive commands from All Slaves
String requestCommand(byte address) {
  Wire.requestFrom(address,29); // Request up to 20 bytes
  String response = "";
  while (Wire.available()) {
    char c = Wire.read();
    if (isPrintable(c)) {  // Skip weird chars
    	response += c;
  	}
  }
  return response;
}
