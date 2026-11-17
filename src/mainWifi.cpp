#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MS5611.h>
#include <ICM20948_WE.h>

const char* ssid = "Rocket_Telemetry_AP";
const char* password = "LaunchPassword123";
WebServer server(80);

MS5611 ms5611;  
#define ICM_ADDR 0x68                             
ICM20948_WE IMU = ICM20948_WE(ICM_ADDR);          

#define CHIP_SELECT 10   
#define MOSFET_PIN1 8    
#define MOSFET_PIN2 6    
float maxAlt = 0;
int counter = 0;         
float groundlevel = 0;

enum RocketState {PRE_LAUNCH, ASCENT, DESCENT, LAND};
RocketState currentState = PRE_LAUNCH;

unsigned long lastLogTime = 0;  
unsigned long loopTime = 0;     
#define logInterval 200         
#define preLaunchInterval 1000  
#define VerticalGThreshold -4.0f 

String flightStateStr = "PRE_LAUNCH";
float liveAlt = 0, liveAccX = 0, liveTemp = 0;

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='0.2'>";
  html += "<title>Rocket Telemetry</title><style>body{font-family:Arial; text-align:center; background:#1e1e24; color:#fff;}";
  html += ".card{background:#2a2a35; padding:20px; margin:15px auto; width:300px; border-radius:10px;}";
  html += "h1{color:#ff4a4a;} .val{font-size:24px; font-weight:bold; color:#4af3ff;}</style></head><body>";
  html += "<h1>ROCKET MISSION CONTROL</h1>";
  html += "<div class='card'><h3>Flight Status</h3><div class='val'>" + flightStateStr + "</div></div>";
  html += "<div class='card'><h3>Relative Altitude</h3><div class='val'>" + String(liveAlt) + " m</div></div>";
  html += "<div class='card'><h3>Vertical Accel (X)</h3><div class='val'>" + String(liveAccX) + " g</div></div>";
  html += "<div class='card'><h3>Temperature</h3><div class='val'>" + String(liveTemp) + " C</div></div></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);                  
  Wire.begin();                          
  Wire.setClock(100000);                 

  pinMode(MOSFET_PIN1, OUTPUT);
  pinMode(MOSFET_PIN2, OUTPUT);
  digitalWrite(MOSFET_PIN1, LOW);        
  digitalWrite(MOSFET_PIN2, LOW);

  delay(100);                            
  if (!ms5611.begin()) Serial.println(F("MS5611 failed"));
  if (!SD.begin(CHIP_SELECT)) Serial.println(F("sd failed"));
  if (!IMU.init()) Serial.println(F("ICM failed"));

  const int groundSamples = 20;
  float altitudeSamples[groundSamples];
  for (int i = 0; i < groundSamples; i++) {
    ms5611.read();
    altitudeSamples[i] = ms5611.getAltitude();
    delay(50);
  }
  for (int i = 0; i < groundSamples - 1; i++) {
    for (int j = i + 1; j < groundSamples; j++) {
      if (altitudeSamples[j] < altitudeSamples[i]) {
        float temp = altitudeSamples[i];
        altitudeSamples[i] = altitudeSamples[j];
        altitudeSamples[j] = temp;
      }
    }
  }
  groundlevel = (altitudeSamples[groundSamples / 2 - 1] + altitudeSamples[groundSamples / 2]) / 2.0;
  memset(altitudeSamples, 0, sizeof(altitudeSamples));

  // Initialize Wi-Fi Access Point directly on the Nano ESP32
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.begin();
}

const char* getStateName(uint8_t state) {
  switch(state) {
    case PRE_LAUNCH: return "PRE_LAUNCH";
    case ASCENT:     return "ASCENT";
    case DESCENT:    return "DESCENT";
    case LAND:       return "LANDED";
    default:         return "UNKNOWN";
  }
}

void log_data(int16_t altDm, int16_t tempCentiC, int16_t accXCentiG, uint8_t state){
  liveAlt = altDm / 100.0f;
  liveTemp = tempCentiC / 100.0f;
  liveAccX = accXCentiG / 100.0f;
  flightStateStr = getStateName(state);

  File file = SD.open("DATALOG.TXT", FILE_WRITE);
  if (file){
    file.print(state); file.print(','); file.print(altDm); file.print(',');
    file.print(accXCentiG); file.print(','); file.println(tempCentiC);
    file.close();
  }
}

void temuReading() {
    ms5611.read();
    IMU.readSensor();
    xyzFloat gValue;
    IMU.getGValues(&gValue);
    log_data((int16_t)((ms5611.getAltitude() - groundlevel) * 100.0f), (int16_t)(ms5611.getTemperature() * 100.0f), (int16_t)(gValue.x * 100.0f), (uint8_t)currentState);
    delay(200); 
}

void trigger() {
  unsigned long triggerStart = millis();
  digitalWrite(MOSFET_PIN2, HIGH); 
  while (millis()- triggerStart < 1500.0f) temuReading();
  digitalWrite(MOSFET_PIN2, LOW); 
  
  File file = SD.open("DATALOG.TXT", FILE_WRITE);
  if (file) { file.println(F("DEPLOY side chute,,,")); file.close(); }

  while (millis() - triggerStart < 4500.0f) temuReading();
  digitalWrite(MOSFET_PIN1, HIGH); 
  while (millis() - triggerStart < 6000.0f) temuReading();
  digitalWrite(MOSFET_PIN1, LOW);  
}

void loop() {
  server.handleClient(); // Handle inbound Mac requests
  unsigned long currentTime = millis();
  if (currentTime - loopTime >= 50 ) {
    loopTime = currentTime;

    ms5611.read();
    float currTemp = ms5611.getTemperature();
    float currAlt = ms5611.getAltitude() - groundlevel;
    IMU.readSensor();
    xyzFloat gValue;
    IMU.getGValues(&gValue);
    float currAccX = gValue.x;

    switch (currentState) {
      case PRE_LAUNCH:
        if (currAlt > 40.0f || ( currAlt > 20.0f and currAccX < VerticalGThreshold ) || currAccX < -5.0f ) counter++;
        else counter = 0;

        if (counter >= 10) { 
          counter = 0; currentState = ASCENT;
          File file = SD.open("DATALOG.TXT", FILE_WRITE);
          if (file) { file.println(F("PRE_LAUNCH->ASCENT,,,")); file.close(); } 
        }
        if (currentTime - lastLogTime >= preLaunchInterval) {
          lastLogTime = currentTime; 
          log_data((int16_t)(currAlt * 100.0f), (int16_t)(currTemp * 100.0f), (int16_t)(currAccX * 100.0f), (uint8_t)currentState); 
        }
        break;

      case ASCENT:
        if (currAlt > maxAlt) { maxAlt = currAlt; counter = 0; }
        if (currAlt < maxAlt - 10.0f ) counter++;
        if (counter >= 10) { counter = 0; trigger(); currentState = DESCENT; }

        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime; 
          log_data((int16_t)(currAlt * 100.0f), (int16_t)(currTemp * 100.0f), (int16_t)(currAccX * 100.0f), (uint8_t)currentState); 
        }
        break;

      case DESCENT:
        if (currAlt <= 10) counter++;
        if (counter >= 20) { counter = 0; currentState = LAND; }

        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime; 
          log_data((int16_t)(currAlt * 100.0f), (int16_t)(currTemp * 100.0f), (int16_t)(currAccX * 100.0f), (uint8_t)currentState); 
        }
        break;

      case LAND:
        if (currentTime - lastLogTime >= logInterval) {
          lastLogTime = currentTime;
          log_data((int16_t)(currAlt * 100.0f), (int16_t)(currTemp * 100.0f), (int16_t)(currAccX * 100.0f), (uint8_t)currentState);
        }
        break;
    }
  }
}
