// include the SD library:
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MS5611.h>
#include <ICM20948_WE.h>

// set up variables using the SD utility library functions:
Sd2Card card;
MS5611 ms5611;

#define ICM_ADDR 0x68
ICM20948_WE IMU = ICM20948_WE(ICM_ADDR);

#define CHIP_SELECT 10
#define MOSFET_PIN1 8    // Connect to MOSFET Gate (Pin 1)
#define MOSFET_PIN2 6    // Connect to MOSFET Gate (Pin 2)
float maxAlt = 0;
int counter = 0;
float groundlevel = 0;

enum RocketState {PRE_LAUNCH, ASCENT, DESCENT, LAND};
RocketState currentState = PRE_LAUNCH;

unsigned long lastLogTime = 0;
unsigned long loopTime = 0;
#define logInterval 200 // 200ms = 5 times per second
#define preLaunchInterval 1000 // 10 second

void setup() {
  // Open serial communications :
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);

  pinMode(MOSFET_PIN1, OUTPUT);
  pinMode(MOSFET_PIN2, OUTPUT);

  digitalWrite(MOSFET_PIN1, LOW);
  digitalWrite(MOSFET_PIN2, LOW);

  delay(100);

  Serial.println(F("Init ms5611..."));
  if (!ms5611.begin()) {
    Serial.println(F("MS5611 failed"));
    //while (1);
  }

  delay(100);

  Serial.print(F("Init SD..."));
  if (!SD.begin(CHIP_SELECT)) {
    Serial.println("sd failed");
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

  //set up ground level
  for(int i = 0; i < 20; i++) {
    ms5611.read();
    groundlevel += ms5611.getAltitude();
    delay(50);
  }
  groundlevel = groundlevel / 20.0;
  
  Serial.print("Ground Level: ");
  Serial.println(groundlevel);
}

void log_data(float currAlt, float currTemp, float currAccZ, int state){
  File file = SD.open("DATALOG.TXT", FILE_WRITE);
  if (file){
    // write stuff into it
    file.print(F("state: "));
    file.print(state);
    file.print(F(" alt: "));
    file.print(currAlt);
    file.print(F(" accel: "));
    file.print(currAccZ);
    file.print(F(" temp: "));
    file.println(currTemp); // .println finishes the line
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

    float alt = ms5611.getAltitude() - groundlevel;
    float temp = ms5611.getTemperature();
    float accZ = gValue.z;
    log_data(alt, temp, accZ, currentState); 
    delay(200); // Small delay so we don't spam the SD card
}

void trigger() {
  unsigned long triggerStart = millis();

  digitalWrite(MOSFET_PIN2, HIGH); // FIRE IGNITER 2 (small chute)
  while (millis()- triggerStart < 1500) {
    temuReading();
  }
  digitalWrite(MOSFET_PIN2, LOW); // stop IGNITER
  Serial.println("DEPLOY side");
  File file = SD.open("DATALOG.TXT", FILE_WRITE);
    if (file){
      file.println("DEPLOY side chute");
      file.close();
    } 

  while (millis() - triggerStart < 4500) {
    temuReading();
  }
  digitalWrite(MOSFET_PIN1, HIGH); // FIRE main chute

  while (millis() - triggerStart < 6000) {
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
        if (currAlt > 40 || ( currAlt > 20 and currAccZ > 3 ) || currAccZ > 5 ) {
          counter++;
        } else {
          counter = 0;
        }

        if (counter >= 10) { 
          counter = 0;
          currentState = ASCENT;
          Serial.println("LAUNCHED");

          File file = SD.open("DATALOG.TXT", FILE_WRITE);
          if (file){
            file.println("LAUNCH!");
            file.close();
          } 
        }

        if (currentTime - lastLogTime >= preLaunchInterval) {
          lastLogTime = currentTime; 
          Serial.print("station:");
          Serial.println(currAlt);
          Serial.println(currAccZ);
          log_data(currAlt, currTemp, currAccZ, currentState); 
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
          Serial.print("ASCENT");
          log_data(currAlt, currTemp, currAccZ, currentState); // This method runs exactly 5 times per second
        }
        break;

      case DESCENT:
        // timer to log data
        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime; 
          Serial.print("DESCENT");
          log_data(currAlt, currTemp, currAccZ, currentState); // This method runs exactly 5 times per second
        }
        if (currAlt <= 10) {
          counter++;
        }

        if (counter >= 200) {
          counter = 0;
          currentState = LAND;
        }
        break;

      case LAND:
        Serial.println("Landed");
        break;
    }
  }
}
