#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Servo.h>

#define MEASURING_FREQ 100
#define SERVO_PIN 2
#define SERVO_SKIP 5

const float Kp = 1.0f;
const float Ki = 0.5f;
const float KdGyro = 0.05f;  // D uses gyro (deg/s)

const float targetPitchDeg = 0.0f;
const float accelXStartThreshold = 1.0f;  // "started" condition example
const float integralLimitAfterStart = 50.0f;

LSM6DS3 IMU(I2C_MODE, 0x6A);
Madgwick filter;
Servo servo;

bool started = false;
float integralE = 0.0f;
uint32_t prevUs = 0;

void setup() {
  Serial.begin(115200);

  servo.attach(SERVO_PIN);
  servo.write(90);

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

  prevUs = micros();
}

void loop() {
  uint32_t nowUs = micros();
  float dt = (nowUs - prevUs) / 1000000.0f;
  if (dt <= 0.0f) dt = 1.0f / (float)MEASURING_FREQ;
  prevUs = nowUs;

  float ax = IMU.readFloatAccelX();
  float ay = IMU.readFloatAccelY();
  float az = IMU.readFloatAccelZ();
  float gx = IMU.readFloatGyroX();
  float gy = IMU.readFloatGyroY();  // pitch rate (example)
  float gz = IMU.readFloatGyroZ();
  filter.updateIMU(gx, gy, gz, ax, ay, az);
  float pitchDeg = filter.getPitch();

  if (!started && ax > accelXStartThreshold) {
    started = true;
  }

  float e = targetPitchDeg - pitchDeg;

  // I: enabled only after "started"
  if (started) {
    integralE += e * dt;
    if (integralE > integralLimitAfterStart) integralE = integralLimitAfterStart;
    if (integralE < -integralLimitAfterStart) integralE = -integralLimitAfterStart;
  } else {
    integralE = 0.0f;
  }

  // D: use gyro directly (avoid noisy (e - e_prev)/dt)
  float u = Kp * e + Ki * integralE - KdGyro * gy;

  static int skip = 0;
  skip++;
  if (skip >= SERVO_SKIP) {
    int servoAngle = (int)(90.0f + u);
    if (servoAngle > 180) servoAngle = 180;
    if (servoAngle < 0) servoAngle = 0;
    servo.write(servoAngle);
    skip = 0;
  }

  Serial.print("started=");
  Serial.print(started ? 1 : 0);
  Serial.print(",pitch=");
  Serial.print(pitchDeg);
  Serial.print(",ax=");
  Serial.print(ax);
  Serial.print(",I=");
  Serial.print(integralE);
  Serial.print(",u=");
  Serial.println(u);

  delay(1000 / MEASURING_FREQ);
}

