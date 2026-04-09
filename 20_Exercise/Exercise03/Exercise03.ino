#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Servo.h>

#define MEASURING_FREQ 20

LSM6DS3 IMU(I2C_MODE, 0x6A);
Madgwick filter;
Servo servo[3];

void setup() {
  Serial.begin(115200);

  servo[0].attach(8);
  servo[1].attach(9);
  servo[2].attach(10);

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

  float roll = filter.getRoll();
  float pitch = filter.getPitch();
  float yaw = filter.getYaw();

  int servoRoll = constrain((int)roll, -45, 45);
  int servoPitch = constrain((int)pitch, -45, 45);
  int servoYaw = constrain((int)(yaw / 2.0), -45, 45);

  servo[0].write(servoRoll + 90);
  servo[1].write(-servoPitch + 90);
  servo[2].write(servoYaw + 90);

  Serial.print(roll);
  Serial.print(",");
  Serial.print(pitch);
  Serial.print(",");
  Serial.println(yaw);

  delay(1000 / MEASURING_FREQ);
}
