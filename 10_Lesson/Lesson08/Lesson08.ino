// 対象ボード: Seeed XIAO nRF52840

#include <LSM6DS3.h>  // Seeed Arduino LSM6DS3 by Seeed Studio 2.0.3
#include <Wire.h>

LSM6DS3 IMU(I2C_MODE, 0x6A);

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  Serial.begin(115200);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;

  while (IMU.begin() != 0) {
    digitalWrite(LED_GREEN, LOW);
    delay(200);
    digitalWrite(LED_GREEN, HIGH);
    delay(200);
  }
}

void loop() {
  float ax = IMU.readFloatAccelX();
  float ay = IMU.readFloatAccelY();
  float az = IMU.readFloatAccelZ();
  float gx = IMU.readFloatGyroX();
  float gy = IMU.readFloatGyroY();
  float gz = IMU.readFloatGyroZ();

  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.print(az);
  Serial.print(",");
  Serial.print(gx);
  Serial.print(",");
  Serial.print(gy);
  Serial.print(",");
  Serial.println(gz);

  delay(50);
}
