#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <NewPing.h>
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
#define DHTPIN 7
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
float temp = 0;
float hum = 0;

//Ultrasonic sensor
#define TRIGGER_PIN  4  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     5  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
float distance = 0;
const int distrange = 20; //set distance range in inches
const int diststart = 100; //set distance start in inches

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

//Change I2c ports
void PortChange(uint8_t bus) //function of TCA9548A
{
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);  
  Wire.read();        // send byte to select bus
  Wire.endTransmission();
  
}

boolean getDistance() {
  //get distance from sensor
  distance = sonar.ping_cm(); // Send ping, get ping time in microseconds (uS).
  float speedOfSound = 331.4 + (0.6 * temp) + (0.0124 * hum); // Calculate speed of sound in m/s
  float duration = sonar.ping_median(10); // 10 interations - returns duration in microseconds
  duration = duration/1000000; // Convert mircroseconds to seconds
  distance = (speedOfSound * duration)/2;
  distance = distance * 100; // meters to centimeters
  Serial.print(F("Distance: "));
  Serial.print(distance);
  //set distance to 1-5
  if (distance <= diststart) {distance = 1;}
  else if (distance <= diststart+distrange) {distance = 2;}
  else if (distance <= diststart+distrange*2) {distance = 3;}
  else if (distance <= diststart+distrange*3) {distance = 4;}
  else if (distance >= diststart+distrange*4) {distance = 5;}
  
  if (distance == 0) {
    return false;
  } else {
    return true;
  }
}

//get temp from sensor
boolean getTemp() {
  sensors_event_t event;
  if (isnan(dht.readTemperature())) {
    return false;
  } else {
    temp = dht.readTemperature()*9/5+32;
    hum = dht.readHumidity();
    Serial.print(F(" Temp: "));
    Serial.print(temp);
    Serial.print(F(" Humidity: "));
    Serial.print(hum);
    return true;
  }
}

int updateTempLights() {
  //turn on leds based on temp. Output 
  if (getTemp()) {
    if (temp <77) {
      return 1;
    }
    else if (temp < 78) {
      return 1;
    }
    else if (temp < 79) {
      return 2;
    }
    else if (temp < 80) {
      return 3;
    }
    else if (temp < 81) {
      return 4;
    }
    else if (temp < 82) {
      return 5;
    }
    else if (temp < 83) {
      return 6;
    }
    else if (temp < 84) {
      return 7;
    }
    else{
      return 8;
    }
  } else {
    Serial.println(F("Failed to read from DHT sensor!"));
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

//Display Display on OLED
void displayDistance() {
  if (getDistance()) {
    PortChange(7);
    display.clearDisplay();
    if (distance == 1 || distance == 0) {
      display.drawRect(40, 64-12-12-12-12-13, 48, 9, SSD1306_WHITE);
    }
    else if (distance == 2) {
      display.drawRect(30, 64-12-12-12-13, 68, 9, SSD1306_WHITE);
      display.drawRect(40, 64-12-12-12-12-13, 48, 9, SSD1306_WHITE);
    }
    else if (distance ==3) {
      display.fillRect(20, 64-12-12-13, 88, 9, SSD1306_WHITE);
      display.drawRect(30, 64-12-12-12-13, 68, 9, SSD1306_WHITE);
      display.drawRect(40, 64-12-12-12-12-13, 48, 9, SSD1306_WHITE);    
    }
    else if (distance == 4) {
      display.drawRect(10, 64-12-13, 108, 9, SSD1306_WHITE);
      display.fillRect(20, 64-12-12-13, 88, 9, SSD1306_WHITE);
      display.drawRect(30, 64-12-12-12-13, 68, 9, SSD1306_WHITE);
      display.drawRect(40, 64-12-12-12-12-13, 48, 9, SSD1306_WHITE);
    }
    else if (distance == 5) {
      display.drawRect(0, 64-12, 128, 9, SSD1306_WHITE);
      display.drawRect(10, 64-12-13, 108, 9, SSD1306_WHITE);
      display.fillRect(20, 64-12-12-13, 88, 9, SSD1306_WHITE);
      display.drawRect(30, 64-12-12-12-13, 68, 9, SSD1306_WHITE);
      display.drawRect(40, 64-12-12-12-12-13, 48, 9, SSD1306_WHITE);
    }
    display.display();
    Serial.println(distance);
    
  } else {
    Serial.println(F("Failed to read from sensor!"));
  }
}

void setup() {
  Wire.begin();
  Serial.begin(9600);

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);


  //init humitemp sensor
  dht.begin();

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
  //display distance on OLED
  displayDistance();
  //delay 
  
  delay(1000);
  
}


 

