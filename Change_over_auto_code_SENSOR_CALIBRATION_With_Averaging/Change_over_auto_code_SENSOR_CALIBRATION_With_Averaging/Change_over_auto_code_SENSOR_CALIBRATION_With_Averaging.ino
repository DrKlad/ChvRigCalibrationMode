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

// Define calibration factors and offsets for each sensor
float PSR_calibrationFactor = 1.0;
float PSR_calibrationOffset = 0.0;

float PS1_calibrationFactor = 1.0;
float PS1_calibrationOffset = 0.0;

float PS2_calibrationFactor = 1.0;
float PS2_calibrationOffset = 0.0;

float FS1_calibrationFactor = 1.0;
float FS1_calibrationOffset = 0.0;

float FS2_calibrationFactor = 1.0;
float FS2_calibrationOffset = 0.0;

void TCA9548A(uint8_t bus) {
  Wire.beginTransmission(0x70);
  Wire.write(1 << bus);
  Wire.endTransmission();
}

// Function to apply calibration to pressure sensor readings
float calibratePressure(float rawValue, float calibrationFactor, float calibrationOffset) {
  return (rawValue * calibrationFactor) + calibrationOffset;
}

// Function to apply calibration to flow sensor readings
float calibrateFlow(float rawValue, float calibrationFactor, float calibrationOffset) {
  return (rawValue * calibrationFactor) + calibrationOffset;
}

// Function to take 10 readings and return the average for pressure sensors
float averageSensorReadings(float (*readFunc)(TruStabilityPressureSensor&, float, float), TruStabilityPressureSensor& sensor, float calibrationFactor, float calibrationOffset, int numReadings = 10, int delayMs = 100) {
  float total = 0.0;
  for (int i = 0; i < numReadings; ++i) {
    total += readFunc(sensor, calibrationFactor, calibrationOffset);
    delay(delayMs / numReadings);
  }
  return total / numReadings;
}

// Function to take 10 readings and return the average for flow sensors
float averageFlowReadings(float (*readFunc)(ZephyrFlowRateSensor&, uint8_t, bool, float, float), ZephyrFlowRateSensor& sensor, uint8_t bus, bool convertToLPM, float calibrationFactor, float calibrationOffset, int numReadings = 10, int delayMs = 100) {
  float total = 0.0;
  for (int i = 0; i < numReadings; ++i) {
    total += readFunc(sensor, bus, convertToLPM, calibrationFactor, calibrationOffset);
    delay(delayMs / numReadings);
  }
  return total / numReadings;
}

// Modified function to read and calibrate pressure sensor data
float readPressureSensor(TruStabilityPressureSensor& sensor, float calibrationFactor, float calibrationOffset) {
  sensor.readSensor();
  float rawPressure = sensor.pressure() / 14.504; // Convert to desired units
  return calibratePressure(rawPressure, calibrationFactor, calibrationOffset);
}

// Modified function to read and calibrate flow sensor data
float readFlowSensor(ZephyrFlowRateSensor& sensor, uint8_t bus, bool convertToLPM, float calibrationFactor, float calibrationOffset) {
  TCA9548A(bus);
  sensor.readSensor();
  float rawFlow = sensor.flow();
  if (convertToLPM) rawFlow /= 1000; // Convert to Liters per Minute if needed
  return calibrateFlow(rawFlow, calibrationFactor, calibrationOffset);
}

void readAllSensors() {
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);

  tft.setCursor(0, 120);
  tft.print("PSR = ");
  tft.print(averageSensorReadings(readPressureSensor, PSRsensor, PSR_calibrationFactor, PSR_calibrationOffset));
  tft.print("     ");

  tft.setCursor(0, 150);
  tft.print("PS1 = ");
  tft.print(averageSensorReadings(readPressureSensor, PS1sensor, PS1_calibrationFactor, PS1_calibrationOffset));
  tft.print("     ");

  tft.setCursor(0, 180);
  tft.print("PS2 = ");
  tft.print(averageSensorReadings(readPressureSensor, PS2sensor, PS2_calibrationFactor, PS2_calibrationOffset));
  tft.print("     ");

  tft.setCursor(0, 210);
  tft.print("FS1 = ");
  tft.print(averageFlowReadings(readFlowSensor, FS1sensor, 0, false, FS1_calibrationFactor, FS1_calibrationOffset));
  tft.print("     ");

  tft.setCursor(0, 240);
  tft.print("FS2 = ");
  tft.print(averageFlowReadings(readFlowSensor, FS2sensor, 1, true, FS2_calibrationFactor, FS2_calibrationOffset));
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
  
  pinMode(3,OUTPUT);
  pinMode(2,OUTPUT); 

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
