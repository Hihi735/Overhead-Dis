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
#define TRIGGER_PIN  3  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     2  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 500 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
float distance = 0;
const int distrange = 20; //set distance range in inches
const int diststart = 110; //set distance start in inches
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

//Spectral sensor
#define STROBE 4
#define RESET 5
#define DC_One A0
#define DC_Two A1
//Define spectrum variables
int freq_amp;
int Frequencies_One[7][3];
int Frequencies_Two[7][3];
int isPeak_One[7];
int isPeak_Two[7];
int i;

//Change I2c ports
void PortChange(uint8_t bus) //function of TCA9548A
{
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);  
  Wire.read();        // send byte to select bus
  Wire.endTransmission();
  
}

//Convert list to byte
byte convertListToByte(int bits[]) {
    byte result = 0;
    for (int i = 0; i < sizeof(bits); ++i) {
        if (bits[i] == 1) {
            result |= (1 << i);
        }
    }
    return result;
}
boolean getDistance() {
  //get distance from sensor
  distance = sonar.ping_cm(); // Send ping, get ping time in microseconds (uS).
  float speedOfSound = 331.4 + (0.6 * temp) + (0.0124 * hum); // Calculate speed of sound in m/s
  float duration = sonar.ping_median(10); // 10 interations - returns duration in microseconds
  duration = duration/1000000; // Convert mircroseconds to seconds
  distance = (speedOfSound * duration)/2;
  distance = distance * 100; // meters to centimeters
  /*set distance to 1-5
  if (distance <= diststart) {distance = 1;}
  else if (distance <= diststart+distrange) {distance = 2;}
  else if (distance <= diststart+distrange*2) {distance = 3;}
  else if (distance <= diststart+distrange*3) {distance = 4;}
  else if (distance >= diststart+distrange*4) {distance = 5;}
  */
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
void displayDistTemp() {
  if (getDistance()) {
    PortChange(7);
    display.clearDisplay();
    display.setCursor(8, 16);
    display.setTextSize(6);
    display.setTextColor(SSD1306_WHITE);
    display.print((int)distance);
    display.setTextSize(2);
    display.setCursor(102, 46);
    display.print("cm");

    //Status bar
    display.drawRect(0, 0, 128, 16, SSD1306_WHITE);
    int mapdistance = map(distance, 40, diststart+distrange*5, 0, 122);
    display.fillRect(3, 3, mapdistance, 10, SSD1306_WHITE);
    int mapTar = map(diststart+distrange*2, 0, diststart+distrange*5, 0, 122);
    display.drawLine(mapTar, 0, mapTar, 16, SSD1306_WHITE);
    display.fillCircle(mapTar, 16, 4, SSD1306_WHITE);
    display.display();
  }
    /*display.clearDisplay();
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
    PortChange(7);
    Serial.println(F("Failed to read from sensor!"));
    display.setCursor(0, 0);
    display.setTextSize(5);
    display.setTextColor(SSD1306_WHITE);
    display.println(F("*"));
    display.display();
  } */
  PortChange(6);
  display.clearDisplay();
  display.setTextSize(6);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(32, 16);
  display.print((int)temp);
  display.setTextSize(2);
  display.setCursor(102, 46);
  display.print("F");
  display.setCursor(56, 0);
  display.setTextSize(2);
  display.print((int)hum);
  display.setTextSize(1);
  display.setCursor(80, 8);
  display.print("%");
  display.display();

}

//Read spectral sensor
void Read_Frequencies() {
  digitalWrite(RESET, HIGH);
  delayMicroseconds(50);
  digitalWrite(RESET, LOW);
  delayMicroseconds(50);

  //Read frequencies for each band
  for (freq_amp = 0; freq_amp < 7; freq_amp++)
  {
    digitalWrite(STROBE, HIGH);
    delayMicroseconds(20);
    digitalWrite(STROBE, LOW);
    delayMicroseconds(20);
    
    //Read the first DC value
    Frequencies_One[freq_amp][2] = Frequencies_One[freq_amp][1];
    Frequencies_One[freq_amp][1] =  Frequencies_One[freq_amp][0];
    Frequencies_One[freq_amp][0] = analogRead(DC_One);

    //Read the second DC value
    Frequencies_Two[freq_amp][2] = Frequencies_Two[freq_amp][1];
    Frequencies_Two[freq_amp][1] =  Frequencies_Two[freq_amp][0];
    Frequencies_Two[freq_amp][0] = analogRead(DC_Two);
    if (Frequencies_One[freq_amp][0] < 90)
    {
      isPeak_One[freq_amp] = 0;
    }
    else
    {
      //Check for peaks (if the difference between the first and second DC values is greater than 10 and if the second and third DC values are less than the first and second DC values)
      if ((Frequencies_One[freq_amp][0] - Frequencies_One[freq_amp][1] > 50) || (Frequencies_One[freq_amp][2]+Frequencies_One[freq_amp][1] < Frequencies_One[freq_amp][1]+Frequencies_One[freq_amp][0]))
      {
        //Display the peak
        isPeak_One[freq_amp] = 1;
      }
      else
      {
        isPeak_One[freq_amp] = 0;
      }
    }
    if (Frequencies_Two[freq_amp][0] < 90)
    {
      isPeak_Two[freq_amp] = 0;
    }
    else
    {
      if ((Frequencies_Two[freq_amp][0] - Frequencies_Two[freq_amp][1] > 50) || (Frequencies_Two[freq_amp][2]+Frequencies_Two[freq_amp][1] < Frequencies_Two[freq_amp][1]+Frequencies_Two[freq_amp][0]))
      {
        isPeak_Two[freq_amp] = 1;
      }
      else
      {
        isPeak_Two[freq_amp] = 0;
      }
    }
  }
}

//display spectral sensor
void displaySpectral() {
  Read_Frequencies();
  byte highByte = 0;
  byte lowByte = 0;
  for (int i = 0; i < 6; i++) { // Loop through the first 6 elements
    if (isPeak_One[i] == 1) {
      highByte |= (1 << 6-i); // Reverse position: set bit in lowByte instead of highByte
    }
    if (isPeak_Two[i] == 1) {
      lowByte |= (1 << 6-i); // Reverse position: set bit in highByte instead of lowByte
    }
  }

  // Handle the last element separately to light up 2 LEDs
  if (isPeak_One[7] == 1) {
    highByte |= (1 << 7); // Reverse position: set the 7th bit in lowByte
  }
  if (isPeak_Two[7] == 1) {
    lowByte |= (1 << 7); // Reverse position: set the 8th bit in highByte
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

  //Spectral sensor
  //Set spectrum Shield pin configurations
  pinMode(STROBE, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(DC_One, INPUT);
  pinMode(DC_Two, INPUT);
  //Initialize Spectrum Analyzers
  digitalWrite(STROBE, LOW);
  digitalWrite(RESET, LOW);
}

void loop() {
  //updates status of lights
  //int tempNum = updateTempLights();
  //sends data to 74hc595
  //displayLevel(tempNum, tempNum);
  displaySpectral();
  //display distance on OLED
  //displayDistTemp();
  //delay 
  
  
}


 

