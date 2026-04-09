// 対象ボード: Seeed XIAO nRF52840

#include <LSM6DS3.h>
#include <MadgwickAHRS.h>
#include <Wire.h>

#define MEASURING_FREQ 20

LSM6DS3 IMU(I2C_MODE, 0x6A);
Madgwick filter;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;
  while (IMU.begin() != 0) {
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

  String payload = String(filter.getRoll(), 2) + "," +
                   String(filter.getPitch(), 2) + "," +
                   String(filter.getYaw(), 2);

  Serial.println(payload);
  Serial1.println(payload);
  delay(1000 / MEASURING_FREQ);
}

