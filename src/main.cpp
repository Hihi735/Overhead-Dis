#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
//Display 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
//74hc595 pins
const int latchPin = 8;
const int clockPin = 12;
const int dataPin = 11;

//HumiTemp sensor
#define DHTPIN 2
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);
uint32_t delayMS;
int temp = 0;

//Change I2c ports
void PortChange(uint8_t bus) //function of TCA9548A
{
  Wire.end();
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);  
  Wire.read();        // send byte to select bus
  Wire.endTransmission();
  
}

boolean getTemp() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    return false;
  } else {
    temp = event.temperature;
    return true;
  }
}

int updateTempLights() {
  //turn on leds based on temp. Output 
  if (getTemp()) {
    if (temp <0) {
      return 0;
    }
    else if (temp < 10) {
      return 1;
    }
    else if (temp < 20) {
      return 2;
    }
    else if (temp < 30) {
      return 3;
    }
    else if (temp < 40) {
      return 4;
    }
    else if (temp < 50) {
      return 5;
    }
    else if (temp < 60) {
      return 6;
    }
    else if (temp < 70) {
      return 7;
    }
    else{
      return 8;
    }
  }

  }

// Function to display the level on the LEDs
void displayLevel(int levelR, int levelL) {
  // Create two bytes to control the 16 LEDs
  byte highByte = 0;
  byte lowByte = 0;
  
  // Set bits for LEDs based on the level
  for (int i = 0; i < levelL; i++) {
      lowByte |= (1 << i);  } // Set bit for the lower 8 LEDs
  for (int i = 0; i < levelR; i++) {
      highByte |= (1 << (i)); // Set bit for the upper 8 LEDs
    }
  
  
  // Send the bytes to the shift registers
  digitalWrite(latchPin, LOW);    // Start sending data
  shiftOut(dataPin, clockPin, MSBFIRST, highByte);  // Send high byte first
  shiftOut(dataPin, clockPin, MSBFIRST, lowByte);   // Send low byte second
  digitalWrite(latchPin, HIGH);   // Latch the data to output
}

void setup() {
  Wire.begin();
  Serial.begin(9600);

  //init humitemp sensor
  dht.begin();
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;

  //init display
  PortChange(7);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();

  PortChange(6);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
}

void loop() {
  //updates status of lights
  int tempNum = updateTempLights();
  //sends data to 74hc595
  displayLevel(tempNum, tempNum);
  //delay 
  Serial.println(temp);
  delay(500);
  
}


 

