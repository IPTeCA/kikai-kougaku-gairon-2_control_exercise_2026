// 対象ボード: Seeed XIAO nRF52840

#include <LSM6DS3.h>  // Seeed Arduino LSM6DS3 by Seeed Studio 2.0.3
#include <Wire.h>
#include <MadgwickAHRS.h>

#define MEASURING_FREQ 20

LSM6DS3 IMU(I2C_MODE, 0x6A);
Madgwick filter;

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

  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL2_G, 0x8C);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x8A);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL7_G, 0x00);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL8_XL, 0x09);
  filter.begin(MEASURING_FREQ);
}

void loop() {
  float ax = IMU.readFloatAccelX();
  float ay = IMU.readFloatAccelY();
  float az = IMU.readFloatAccelZ();
  float gx = IMU.readFloatGyroX();
  float gy = IMU.readFloatGyroY();
  float gz = IMU.readFloatGyroZ();

  filter.updateIMU(gx, gy, gz, ax, ay, az);

  Serial.print(filter.getRoll());
  Serial.print(",");
  Serial.print(filter.getPitch());
  Serial.print(",");
  Serial.print(filter.getYaw());
  Serial.print(",");
  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.println(az);

  delay(1000 / MEASURING_FREQ);
}
