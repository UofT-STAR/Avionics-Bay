// include the SD library:
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MS5611.h>
#include <ICM20948_WE.h>

// set up variables using the SD utility library functions:
Sd2Card card;   // sd card containing memory and logs
MS5611 ms5611;  // tracks altitude and temperature

#define ICM_ADDR 0x68                             // ICM20948 default I2C address (does accelerometer and gyroscope)
ICM20948_WE IMU = ICM20948_WE(ICM_ADDR);          // gyroscope and accelerometer

#define CHIP_SELECT 10   // SD card chip select pin
#define MOSFET_PIN1 8    // Connect to MOSFET Gate (Pin 1)
#define MOSFET_PIN2 6    // Connect to MOSFET Gate (Pin 2)
float maxAlt = 0;
int counter = 0;         // counter to track how many consecutive readings have been in a certain state (e.g. descent or landing)
float groundlevel = 0;

enum RocketState {PRE_LAUNCH, ASCENT, DESCENT, LAND};
RocketState currentState = PRE_LAUNCH;

unsigned long lastLogTime = 0;  // writing data in sd card
unsigned long loopTime = 0;     // how fast to run loop (e.g. how fast to read sensors and check state)
#define logInterval 200         // 200ms = 5 times per second
#define preLaunchInterval 1000  // 1 second

#define VerticalGThreshold 4.0f // Threshold for vertical acceleration to detect launch (in g's)

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
}

void log_data(int16_t altDm, int16_t tempCentiC, int16_t accZCentiG, uint8_t state){
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
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - loopTime >= 50 ) {
    loopTime = currentTime;

    ms5611.read();
    float currTemp = ms5611.getTemperature();
    float currAlt = ms5611.getAltitude() - groundlevel;

    IMU.readSensor();
    xyzFloat gValue;
    IMU.getGValues(&gValue);
    float currAccZ = gValue.z;

    switch (currentState) {

      case PRE_LAUNCH:
        // check if we are in the air
        if (currAlt > 40.0f || ( currAlt > 20.0f and currAccZ > VerticalGThreshold ) || currAccZ > 5.0f ) {  // this is very subjective, there could be other better conditions
          counter++;
        } else {
          counter = 0;
        }

        if (counter >= 10) { 
          counter = 0;
          currentState = ASCENT;
          Serial.println(F("LAUNCHED"));

          File file = SD.open("DATALOG.TXT", FILE_WRITE);
          if (file){
            file.println(F("PRE_LAUNCH->ASCENT,,,"));
            file.close();
          } 
        }

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

        // if not descending reset counter
        if (currAlt > maxAlt) {
          maxAlt = currAlt;
          counter = 0; // Reset counter
        }

        // increase counter if descending
        if (currAlt < maxAlt - 10.0 || currAccZ < -1 ) { // 10 meter buffer 
          counter++;
        }

        // Trigger after 10 consistent descent readings
        if (counter >= 10) {
          counter = 0;
          trigger();
          currentState = DESCENT;
        }

        // timer to log data
        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime; 
          Serial.print(F("ASCENT"));
          log_data(
            (int16_t)(currAlt * 100.0f),
            (int16_t)(currTemp * 100.0f),
            (int16_t)(currAccZ * 100.0f),
            (uint8_t)currentState
          ); // This method runs exactly 5 times per second
        }
        break;

      case DESCENT:
        // timer to log data
        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime; 
          Serial.print(F("DESCENT"));
          log_data(
            (int16_t)(currAlt * 100.0f),
            (int16_t)(currTemp * 100.0f),
            (int16_t)(currAccZ * 100.0f),
            (uint8_t)currentState
          ); // This method runs exactly 5 times per second
        }
        if (currAlt <= 10) {
          counter++;
        }

        if (counter >= 20) {
          counter = 0;
          currentState = LAND;
        }
        break;

      case LAND:
        Serial.println(F("Landed"));
        break;
    }
  }
}
