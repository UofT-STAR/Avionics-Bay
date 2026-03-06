/*
    SD card attached to SPI bus as follows:
 ** SDO - pin 11 on Arduino Uno/Duemilanove/Diecimila
 ** SDI - pin 12 on Arduino Uno/Duemilanove/Diecimila
 ** CLK - pin 13 on Arduino Uno/Duemilanove/Diecimila
 ** CS - 	Pin 10 used here for consistency
*/
// include the SD library:
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MS5611.h>

MS5611 ms5611;

#define CHIP_SELECT 10
#define MOSFET_PIN1 9
#define MOSFET_PIN2 8
#define LOG_INTERVAL 200
#define PRE_LAUNCH_INTERVAL 1000

float maxAlt = 0.0f;
int descent = 0;
float groundlevel = 0.0f;

enum RocketState { PRE_LAUNCH, ASCENT, DESCENT, LAND };
RocketState currentState = PRE_LAUNCH;

unsigned long lastLogTime = 0;

// Forward declarations (required in .cpp files)
void log_data(float currAlt);
void trigger();

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(MOSFET_PIN1, OUTPUT);
  pinMode(MOSFET_PIN2, OUTPUT);
  digitalWrite(MOSFET_PIN1, LOW);
  digitalWrite(MOSFET_PIN2, LOW);

  Serial.println("Initializing ms5611");
  if (!ms5611.begin()) {
    Serial.println("MS5611 not found!");
    while (1) {}
  }

  Serial.print("Initializing SD card...");
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("initialization failed!");
    while (1) {}
  }
  Serial.println("SD init OK");

  float sum = 0.0f;
  for (int i = 0; i < 10; i++) {
    ms5611.read();
    sum += ms5611.getAltitude();
    delay(50);
  }
  groundlevel = sum / 10.0f;

  Serial.print("Ground Level set to: ");
  Serial.println(groundlevel);
}

void loop() {
  ms5611.read();
  float currAlt = ms5611.getAltitude() - groundlevel;
  unsigned long currentTime = millis();

  switch (currentState) {
    case PRE_LAUNCH:
      Serial.print("not launched, altitude: ");
      Serial.println(currAlt);

      if (currAlt > 10.0f) {
        currentState = ASCENT;
        Serial.println("LAUNCH DETECTED");

        File file = SD.open("DATALOG.TXT", FILE_WRITE);
        if (file) {
          file.println("LAUNCH DETECTED");
          file.close();
        }
      }

      // Fixed macro name
      if (currentTime - lastLogTime >= PRE_LAUNCH_INTERVAL) {
        lastLogTime = currentTime;
        Serial.print("GROUND ");
        log_data(currAlt);
      }
      break;

    case ASCENT:
      if (currAlt > maxAlt) {
        maxAlt = currAlt;
        descent = 0;
      }

      if (currAlt < maxAlt - 5.0f) {
        descent++;
      }

      if (descent >= 20) {
        trigger();
        currentState = DESCENT;
      }

      if (currentTime - lastLogTime >= LOG_INTERVAL) {
        lastLogTime = currentTime;
        Serial.print("ASCENT ");
        log_data(currAlt);
      }
      break;

    case DESCENT:
      if (currentTime - lastLogTime >= LOG_INTERVAL) {
        lastLogTime = currentTime;
        Serial.print("DESCENT ");
        log_data(currAlt);
      }

      if (currAlt <= 7.0f) {
        currentState = LAND;
      }
      break;

    case LAND:
      Serial.println("Landed");
      break;
  }

  delay(20);
}

void log_data(float currAlt) {
  File file = SD.open("DATALOG.TXT", FILE_WRITE);
  if (file) {
    Serial.println(currAlt);
    file.println(currAlt);
    file.close();
  } else {
    Serial.println("Error opening datalog");
  }
}

void trigger() {
  digitalWrite(MOSFET_PIN2, HIGH);
  Serial.println("DEPLOYMENT TRIGGERED");

  File file = SD.open("DATALOG.TXT", FILE_WRITE);
  if (file) {
    file.println("DEPLOYMENT TRIGGERED");
    file.close();
  }

  delay(1500);
  digitalWrite(MOSFET_PIN2, LOW);
  delay(500);
  digitalWrite(MOSFET_PIN1, HIGH);
  delay(2000);
  digitalWrite(MOSFET_PIN1, LOW);
}