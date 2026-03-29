#if defined(DESKTOP_REPLAY)
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

typedef uint8_t byte;
#define OUTPUT 1
#define HIGH 1
#define LOW 0
#define FILE_READ 0
#define FILE_WRITE 1
#define F(x) x

struct xyzFloat {
  float x;
  float y;
  float z;
};

class SerialShim {
public:
  void begin(unsigned long) {}
  void print(const char *s) { std::cout << s; }
  void print(float v) { std::cout << v; }
  void print(float v, int) { std::cout << v; }
  void print(int v) { std::cout << v; }
  void println(const char *s) { std::cout << s << std::endl; }
  void println(float v) { std::cout << v << std::endl; }
  void println(int v) { std::cout << v << std::endl; }
};

SerialShim Serial;

class WireShim {
public:
  void begin() {}
  void setClock(unsigned long) {}
};

WireShim Wire;

void pinMode(int, int) {}
void digitalWrite(int, int) {}

unsigned long gMillis = 0;
unsigned long millis() {
  gMillis += 50;
  return gMillis;
}

void delay(unsigned long ms) {
  gMillis += ms;
}

class File {
public:
  File() : writeMode(false) {}
  operator bool() const { return false; }
  bool available() const { return false; }
  std::string readStringUntil(char) { return ""; }
  bool seek(unsigned long) { return false; }
  void print(const char *) {}
  void print(int) {}
  void println(const char *) {}
  void println(int) {}
  void close() {}

private:
  bool writeMode;
};

class SDShim {
public:
  bool begin(int) { return true; }
  File open(const char *, int) { return File(); }
};

SDShim SD;
struct Sd2Card {};

#else
// include the SD library:
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MS5611.h>
#include <ICM20948_WE.h>
#endif
#include "flight_state.h"

#ifndef USE_DATALOG_REPLAY
#define USE_DATALOG_REPLAY 0
#endif

// set up variables using the SD utility library functions:
Sd2Card card;   // sd card containing memory and logs

#if defined(DESKTOP_REPLAY)
static std::ofstream gDesktopDatalogOut;
static std::string gDesktopDatalogOutPath = "SIM_DATALOG.TXT";

void desktopOpenDatalogOut() {
  const char* envOutPath = std::getenv("SIM_DATALOG_OUT");
  if (envOutPath && envOutPath[0] != '\0') {
    gDesktopDatalogOutPath = envOutPath;
  }

  gDesktopDatalogOut.open(gDesktopDatalogOutPath, std::ios::out | std::ios::trunc);
  if (gDesktopDatalogOut.is_open()) {
    gDesktopDatalogOut << "state,alt,accX,temp" << std::endl;
  }
}

void desktopWriteMarker(const char* marker) {
  if (gDesktopDatalogOut.is_open()) {
    gDesktopDatalogOut << marker << std::endl;
  }
}

void desktopCloseDatalogOut() {
  if (gDesktopDatalogOut.is_open()) {
    gDesktopDatalogOut.close();
  }
}
#endif

#if USE_DATALOG_REPLAY
class DatalogReplaySource {
public:
  bool begin(const char* path) {
#if defined(DESKTOP_REPLAY)
    const char* envPath = std::getenv("DATALOG_PATH");
    const char* selectedPath = (envPath && envPath[0] != '\0') ? envPath : path;
    stream.open(selectedPath);
    if (!stream.is_open()) {
      stream.clear();
      stream.open("DATALOG.TXT");
    }
    if (!stream.is_open()) {
      stream.clear();
      stream.open("c:/Users/ebagu/Downloads/DATALOG.TXT");
    }
    if (stream.is_open()) {
      sourcePath = selectedPath;
    }
    return stream.is_open();
#else
    file = SD.open(path, FILE_READ);
    return file;
#endif
  }

  bool advance() {
#if defined(DESKTOP_REPLAY)
    if (!stream.is_open()) {
      return false;
    }

    std::string line;
    while (true) {
      while (std::getline(stream, line)) {
        if (line.empty()) {
          continue;
        }

        if (line.find("PRE_LAUNCH->ASCENT") != std::string::npos || line.find("DEPLOY") != std::string::npos) {
          continue;
        }

        int a = 0;
        int b = 0;
        int c = 0;
        int d = 0;

        int parsed = std::sscanf(line.c_str(), "%d,%d,%d,%d", &a, &b, &c, &d);
        if (parsed == 4) {
          altitudeM = (float)b / 100.0f;
          accG = (float)c / 100.0f;
          tempC = (float)d / 100.0f;
          hasSample = true;
          return true;
        }

        parsed = std::sscanf(line.c_str(), "%d,%d,%d", &b, &c, &d);
        if (parsed == 3) {
          altitudeM = (float)b / 100.0f;
          accG = (float)c / 100.0f;
          tempC = (float)d / 100.0f;
          hasSample = true;
          return true;
        }
      }

      stream.clear();
      stream.seekg(0, std::ios::beg);
      if (!stream.good()) {
        return hasSample;
      }
      std::cout << "REPLAY: rewinding source file" << std::endl;
    }
#else
    if (!file) {
      return false;
    }

    while (true) {
      while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) {
          continue;
        }

        if (line.indexOf("PRE_LAUNCH->ASCENT") >= 0 || line.indexOf("DEPLOY") >= 0) {
          continue;
        }

        int a = 0;
        int b = 0;
        int c = 0;
        int d = 0;

        int parsed = sscanf(line.c_str(), "%d,%d,%d,%d", &a, &b, &c, &d);
        if (parsed == 4) {
          // state,alt(dm),acc(cg),temp(cC)
          altitudeM = (float)b / 100.0f;
          accG = (float)c / 100.0f;
          tempC = (float)d / 100.0f;
          hasSample = true;
          return true;
        }

        parsed = sscanf(line.c_str(), "%d,%d,%d", &b, &c, &d);
        if (parsed == 3) {
          // alt(dm),acc(cg),temp(cC)
          altitudeM = (float)b / 100.0f;
          accG = (float)c / 100.0f;
          tempC = (float)d / 100.0f;
          hasSample = true;
          return true;
        }
      }

      // Loop the replay file to keep feeding values.
      if (!file.seek(0)) {
        return hasSample;
      }
    }
#endif
  }

  float getAltitudeM() const { return altitudeM; }
  float getAccG() const { return accG; }
  float getTempC() const { return tempC; }

private:
#if defined(DESKTOP_REPLAY)
  std::ifstream stream;
  std::string sourcePath;
#else
  File file;
#endif
  float altitudeM = 0.0f;
  float accG = 0.0f;
  float tempC = 20.0f;
  bool hasSample = false;
};

class ReplayMS5611 {
public:
  explicit ReplayMS5611(DatalogReplaySource &src) : source(src) {}

  bool begin() { return true; }
  void read() { source.advance(); }
  float getAltitude() { return source.getAltitudeM(); }
  float getTemperature() { return source.getTempC(); }

private:
  DatalogReplaySource &source;
};

class ReplayICM20948 {
public:
  explicit ReplayICM20948(DatalogReplaySource &src) : source(src) {}

  bool init() { return source.begin("DATALOG.TXT"); }
  void readSensor() {}

  void getGValues(xyzFloat *gValue) {
    gValue->x = 0.0f;
    gValue->y = 0.0f;
    gValue->z = source.getAccG();
  }

private:
  DatalogReplaySource &source;
};

DatalogReplaySource replaySource;
ReplayMS5611 ms5611(replaySource);
ReplayICM20948 IMU(replaySource);
#else
MS5611 ms5611;  // tracks altitude and temperature

#define ICM_ADDR 0x68                             // ICM20948 default I2C address (does accelerometer and gyroscope)
ICM20948_WE IMU = ICM20948_WE(ICM_ADDR);          // gyroscope and accelerometer
#endif

#define CHIP_SELECT 10   // SD card chip select pin
#define MOSFET_PIN1 8    // Connect to MOSFET Gate (Pin 1)
#define MOSFET_PIN2 6    // Connect to MOSFET Gate (Pin 2)
float maxAlt = 0;
int counter = 0;         // counter to track how many consecutive readings have been in a certain state (e.g. descent or landing)
float groundlevel = 0;
RocketState currentState = PRE_LAUNCH;

unsigned long lastLogTime = 0;  // writing data in sd card
unsigned long loopTime = 0;     // how fast to run loop (e.g. how fast to read sensors and check state)
#define logInterval 200         // 200ms = 5 times per second
#define preLaunchInterval 1000  // 1 second

#define VerticalGThreshold 4.0f // Threshold for vertical acceleration to detect launch (in g's)

#if USE_DATALOG_REPLAY || defined(DESKTOP_REPLAY)
const char* stateName(RocketState state) {
  switch (state) {
    case PRE_LAUNCH:
      return "PRE_LAUNCH";
    case ASCENT:
      return "ASCENT";
    case DESCENT:
      return "DESCENT";
    case LAND:
      return "LAND";
    default:
      return "UNKNOWN";
  }
}
#endif

void setup() {
  // Open serial communications :
  Serial.begin(115200);                  // Serial only works when bay is connected to computer by USB
  Wire.begin();                          // data bus for ICM20948 and MS5611
  Wire.setClock(100000);                 // set I2C clock speed to 100kHz (standard mode)

  pinMode(MOSFET_PIN1, OUTPUT);
  pinMode(MOSFET_PIN2, OUTPUT);

  digitalWrite(MOSFET_PIN1, LOW);        // Ensure MOSFETs are off at the start
  digitalWrite(MOSFET_PIN2, LOW);

  delay(100);                            // 100ms = 0.1s delay to let everything power up

#if USE_DATALOG_REPLAY
#if defined(DESKTOP_REPLAY)
  desktopOpenDatalogOut();
#endif

  Serial.print(F("Init SD..."));
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println(F("sd failed"));
  }

  delay(100);

  Serial.print(F("Init replay source..."));
  if (!IMU.init()) {
    Serial.println(F("replay init failed"));
  } else {
    Serial.println(F("replay OK"));
  }

  groundlevel = 0.0f;
  Serial.println(F("Replay mode enabled: using DATALOG.TXT as sensor input"));
#else
  Serial.println(F("Init ms5611..."));
  if (!ms5611.begin()) {
    Serial.println(F("MS5611 failed"));
    //while (1);
  }

  delay(100);

  Serial.print(F("Init SD..."));
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println(F("sd failed"));
    //while (1); // Halt if card fails
  }

  delay(100);

  Serial.print(F("Init ICM..."));
  
  if (!IMU.init()) {
      Serial.println(F("ICM failed"));
      // while(1); // Uncomment this for actual flight!
  } else {
      Serial.println(F("ICM OK"));
  }

  // use median to eliminate bad readings (outliers)
  const int groundSamples = 20;
  float altitudeSamples[groundSamples];

  for (int i = 0; i < groundSamples; i++) {
    ms5611.read();
    altitudeSamples[i] = ms5611.getAltitude();
    delay(50);
  }

  // simple selection sort
  for (int i = 0; i < groundSamples - 1; i++) {
    for (int j = i + 1; j < groundSamples; j++) {
      if (altitudeSamples[j] < altitudeSamples[i]) {
        float temp = altitudeSamples[i];
        altitudeSamples[i] = altitudeSamples[j];
        altitudeSamples[j] = temp;
      }
    }
  }

  // Even-sized median = average of two middle values
  groundlevel = (altitudeSamples[groundSamples / 2 - 1] + altitudeSamples[groundSamples / 2]) / 2.0;

  // Clear sample buffer once calibration is done
  memset(altitudeSamples, 0, sizeof(altitudeSamples));

  Serial.print(F("Ground Level: "));
  Serial.println(groundlevel);
#endif
}

void log_data(int16_t altDm, int16_t tempCentiC, int16_t accZCentiG, uint8_t state){
#if USE_DATALOG_REPLAY
#if defined(DESKTOP_REPLAY)
  if (gDesktopDatalogOut.is_open()) {
    gDesktopDatalogOut << (int)state << ','
      << (int)altDm << ','
      << (int)accZCentiG << ','
      << (int)tempCentiC << std::endl;
  }
#endif

  (void)altDm;
  (void)tempCentiC;
  (void)accZCentiG;
  (void)state;
  return;
#else
  File file = SD.open("DATALOG.TXT", FILE_WRITE);
  if (file){
    // Compact CSV line: state,alt(dm),accZ(cg),temp(cC)
    file.print(state);
    file.print(',');
    file.print(altDm);
    file.print(',');
    file.print(accZCentiG);
    file.print(',');
    file.println(tempCentiC);
    file.close();
  } else{
    Serial.println(F("Datalog error"));
  }
#endif
}

void temuReading() {
    ms5611.read();
    IMU.readSensor();
    xyzFloat gValue;
    IMU.getGValues(&gValue);

    log_data(
      (int16_t)((ms5611.getAltitude() - groundlevel) * 100.0f),
      (int16_t)(ms5611.getTemperature() * 100.0f),
      (int16_t)(gValue.z * 100.0f),
      (uint8_t)currentState
    );
    delay(200); // Small delay so we don't spam the SD card
}

void trigger() {
#if USE_DATALOG_REPLAY
#if defined(DESKTOP_REPLAY)
  desktopWriteMarker("DEPLOY side chute,,,");
#endif
  Serial.println(F("Replay mode: trigger skipped"));
  return;
#else
  unsigned long triggerStart = millis();

  digitalWrite(MOSFET_PIN2, HIGH); // FIRE IGNITER 2 (small chute)
  while (millis()- triggerStart < 1500.0f) {
    temuReading();
  }
  digitalWrite(MOSFET_PIN2, LOW); // stop IGNITER
  Serial.println(F("DEPLOY side"));
  File file = SD.open("DATALOG.TXT", FILE_WRITE);
    if (file){
      file.println(F("DEPLOY side chute,,,"));
      file.close();
    }

  while (millis() - triggerStart < 4500.0f) {
    temuReading();
  }
  digitalWrite(MOSFET_PIN1, HIGH); // FIRE main chute

  while (millis() - triggerStart < 6000.0f) {
    temuReading();
  } 
  digitalWrite(MOSFET_PIN1, LOW);  // Turn off main chute
#endif
}

void loop() {
  unsigned long currentTime = millis();
#if USE_DATALOG_REPLAY
  static unsigned long lastReplayPrint = 0;
#endif
  if (currentTime - loopTime >= 50 ) {
    loopTime = currentTime;

    ms5611.read();
    float currTemp = ms5611.getTemperature();
    float currAlt = ms5611.getAltitude() - groundlevel;

    IMU.readSensor();
    xyzFloat gValue;
    IMU.getGValues(&gValue);
    float currAccZ = gValue.z;

    StateUpdateResult stateUpdate = updateRocketState(
      currAlt,
      currAccZ,
      currentState,
      maxAlt,
      counter,
      VerticalGThreshold
    );

    if (stateUpdate.launched) {
      Serial.println(F("LAUNCHED"));

#if defined(DESKTOP_REPLAY)
      desktopWriteMarker("PRE_LAUNCH->ASCENT,,,");
#else
      File file = SD.open("DATALOG.TXT", FILE_WRITE);
      if (file) {
        file.println(F("PRE_LAUNCH->ASCENT,,,"));
        file.close();
      }
#endif
    }

    if (stateUpdate.deployTriggered) {
      trigger();
    }

#if USE_DATALOG_REPLAY
    if (currentTime - lastReplayPrint >= 200) {
      lastReplayPrint = currentTime;
      Serial.print(F("REPLAY,"));
      Serial.print(currAlt, 2);
      Serial.print(F(","));
      Serial.print(currAccZ, 2);
      Serial.print(F(","));
      Serial.print(currTemp, 2);
      Serial.print(F(",state="));
      Serial.println(stateName(currentState));
    }
#endif

    switch (currentState) {
      case PRE_LAUNCH:
        if (currentTime - lastLogTime >= preLaunchInterval) {
          lastLogTime = currentTime;
          Serial.print(F("station:"));
          Serial.println(currAlt);
          Serial.println(currAccZ);
          log_data(
            (int16_t)(currAlt * 100.0f),
            (int16_t)(currTemp * 100.0f),
            (int16_t)(currAccZ * 100.0f),
            (uint8_t)currentState
          );
        }
        break;

      case ASCENT:
        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime;
          Serial.print(F("ASCENT"));
          log_data(
            (int16_t)(currAlt * 100.0f),
            (int16_t)(currTemp * 100.0f),
            (int16_t)(currAccZ * 100.0f),
            (uint8_t)currentState
          );
        }
        break;

      case DESCENT:
        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime;
          Serial.print(F("DESCENT"));
          log_data(
            (int16_t)(currAlt * 100.0f),
            (int16_t)(currTemp * 100.0f),
            (int16_t)(currAccZ * 100.0f),
            (uint8_t)currentState
          );
        }
        break;

      case LAND:
        Serial.println(F("Landed"));
        break;
    }
  }
}

#if defined(DESKTOP_REPLAY)
int main() {
  setup();

  // Run enough iterations to visualize state progression from replayed log data.
  for (int i = 0; i < 4000; i++) {
    loop();
    if (currentState == LAND) {
      break;
    }
  }

  desktopCloseDatalogOut();
  std::cout << "Desktop replay finished in state=" << stateName(currentState) << std::endl;
  std::cout << "Simulation datalog written to " << gDesktopDatalogOutPath << std::endl;
  return 0;
}
#endif
