#include <ICM_20948.h> // Click here to get the library: http://librarymanager/All#SparkFun_ICM_20948_9DoF_IMU
#define AD0_VAL 0
ICM_20948_I2C myICM; 

void setup() {
  Serial.begin(115200);
  Wire.begin();
  myICM.begin(Wire, AD0_VAL);
}

void loop() {
  if (myICM.dataReady()) {
    myICM.getAGMT(); // Read Accelerometer, Gyro, Magnetometer, Temp
    Serial.print(myICM.accX());
    Serial.print("              ");
    Serial.println(myICM.accY());

    // ... add more print statements
  }

  delay(30);
}
