#include <RS232.h>
#include <SPI.h>
#include <RTClib.h>
#include <Adafruit_ILI9341.h>
#include <SD.h>
#include <Wire.h>
#include <HoneywellZephyrI2C.h>
#include <HoneywellTruStabilitySPI.h>

#define TFT_DC 3
#define TFT_CS 2

const int ENTER_BUTTON = I0_5;
const int SV1_PIN = R0_7;
const int SV2_PIN = R0_6;
const int SV3_PIN = R0_5;
const int SV4_PIN = R0_4;

Adafruit_ILI9341 tft(TFT_CS, TFT_DC);

TruStabilityPressureSensor PSRsensor(Q0_0, 0, 150);
TruStabilityPressureSensor PS1sensor(Q0_1, 0, 150);
TruStabilityPressureSensor PS2sensor(Q0_2, 0, 150);

ZephyrFlowRateSensor FS1sensor(0x49, 15, ZephyrFlowRateSensor::SLPM);
ZephyrFlowRateSensor FS2sensor(0x49, 750, ZephyrFlowRateSensor::SCCM);

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

float getAveragePressure(String sens){
  float Sensor;

  if(sens == "PS1sensor"){
    Sensor = readPressureSensor(PS1sensor);
  }

  if(sens == "PS2sensor"){
    Sensor = readPressureSensor(PS2sensor);
  }

  if(sens == "PSRsensor"){
    Sensor = readPressureSensor(PSRsensor);
  }

  if(sens == "FS1sensor"){
   Sensor = readFlowSensor(FS1sensor, 0);
  }

  if(sens == "FS2sensor"){
    Sensor = readFlowSensor(FS2sensor, 1, true);
  }

  while(readIndex < numReadings)
  {
    // Subtract the last reading
    total = total - readings[readIndex];
      // Read the current pressure
    float pressure = Sensor;
      // Store the reading
    readings[readIndex] = pressure;
      // Add the reading to the total
    total = total + readings[readIndex];
      // Advance to the next position in the array
    readIndex = readIndex + 1;
      // If we're at the end of the array, wrap around to the beginning
    delay(10);
  }
  if (readIndex >= numReadings) 
  {
    readIndex = 0;
  }
  // Calculate the average
  float average = total / numReadings;
  return average;
}

float readPressureSensor(TruStabilityPressureSensor& sensor) {
  //sensor.readSensor();
  getAveragePressure(sensor);
  return getAveragePressure() / 14.504; // Convert to desired units
}

float readFlowSensor(ZephyrFlowRateSensor& sensor, uint8_t bus, bool convertToLPM = false) {
  TCA9548A(bus);
  //sensor.readSensor();
  getAveragePressure(sensor);
  float flow = getAveragePressure(sensor)
  //float flow = sensor.flow();
  return convertToLPM ? flow / 1000 : flow;
}

void readAllSensors() {
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  tft.setCursor(0, 120);
  tft.print("PSR = ");
  tft.print(readPressureSensor(PSRsensor),3);
  tft.print("     ");

  tft.setCursor(0, 150);
  tft.print("PS1 = ");
  tft.print(readPressureSensor(PS1sensor),3);
  tft.print("     ");

  tft.setCursor(0, 180);
  tft.print("PS2 = ");
  tft.print(readPressureSensor(PS2sensor),3);
  tft.print("     ");

  tft.setCursor(0, 210);
  tft.print("FS1 = ");
  tft.print(readFlowSensor(FS1sensor, 0),3);
  tft.print("     ");

  tft.setCursor(0, 240);
  tft.print("FS2 = ");
  tft.print(readFlowSensor(FS2sensor, 1, true),3);
  tft.print("     ");
}

void setup() {
  RS232.begin(9600);
  SPI.begin();
  Wire.begin();
  FS1sensor.begin();
  FS2sensor.begin();
  PSRsensor.begin();
  PS1sensor.begin();
  PS2sensor.begin();
  tft.begin();

  pinMode(ENTER_BUTTON, INPUT);
  pinMode(SV1_PIN, OUTPUT);
  pinMode(SV2_PIN, OUTPUT);
  pinMode(SV3_PIN, OUTPUT);
  pinMode(SV4_PIN, OUTPUT);

  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5, 5);
  tft.println("NOxBOx Change Over");
  tft.setCursor(5, 55);
  tft.println("TEST RIG");
  tft.setCursor(5, 105);
  tft.println("THE WEST GROUP");
  tft.setCursor(5, 155);
  tft.println("DTV 2023");
  tft.setCursor(5, 200);
  tft.println("CALIBRATION MODE");

  delay(5000);
}

void loop() {
  tft.fillScreen(ILI9341_BLACK);

  digitalWrite(SV1_PIN, LOW);
  digitalWrite(SV2_PIN, LOW);
  digitalWrite(SV3_PIN, LOW);
  digitalWrite(SV4_PIN, LOW);
  delay(200);

  while (digitalRead(ENTER_BUTTON) == LOW) {
    digitalWrite(SV1_PIN, HIGH);
    tft.setTextSize(3);
    tft.setCursor(5, 5);
    tft.print("Port 1 Fed");

    while (digitalRead(ENTER_BUTTON) == LOW) {
      readAllSensors();
    }

    delay(200);
    digitalWrite(SV2_PIN, HIGH);
    digitalWrite(SV1_PIN, LOW);
    tft.setTextSize(3);
    tft.setCursor(5, 5);
    tft.println("Port 2 Fed");

    while (digitalRead(ENTER_BUTTON) == LOW) {
      readAllSensors();
    }

    delay(200);
    digitalWrite(SV1_PIN, LOW);
    digitalWrite(SV2_PIN, HIGH);
    digitalWrite(SV3_PIN, HIGH);
    tft.setTextSize(3);
    tft.setCursor(5, 5);
    tft.println("Low Flow Open");

    while (digitalRead(ENTER_BUTTON) == LOW) {
      readAllSensors();
    }

    tft.fillScreen(ILI9341_BLACK);
    delay(200);
    digitalWrite(SV3_PIN, LOW);
    digitalWrite(SV4_PIN, HIGH);
    tft.setTextSize(3);
    tft.setCursor(5, 5);
    tft.println("High Flow");
    tft.setCursor(5, 30);
    tft.println("Open");

    while (digitalRead(ENTER_BUTTON) == LOW) {
      readAllSensors();
    }
  }
}
