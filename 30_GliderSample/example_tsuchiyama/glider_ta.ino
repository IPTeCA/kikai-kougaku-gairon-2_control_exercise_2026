#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include "hz_sleep.h"
#include <Servo.h>

//#define MEASURING_FREQ (1660)
#define MEASURING_FREQ (200)
#define LIMIT_ACCX 1.0
#define SERVO_SKIP 5

#define SERVO_PIN_1 2
#define SERVO_PIN_2 3
#define SERVO_PIN_3 4

#define OFFSET_SERVO_1 0
#define OFFSET_SERVO_2 -10
#define OFFSET_SERVO_3 24

#define KP_ROLL 8.0
#define KD_ROLL 0.1
#define KI_ROLL 0.0

// #define KP_ROLL 0.0
// #define KD_ROLL 0.0
// #define KI_ROLL 0.0

#define KP_PITCH -2.5
#define KD_PITCH 0.0
#define KI_PITCH 1.0
float limit_i = 0.0;
bool gone = false;

// #define KP_PITCH 0.0
// #define KD_PITCH 0.0
// #define KI_PITCH 0.0

//Create a instance of class LSM6DS3
LSM6DS3 IMU(I2C_MODE, 0x6A);  //I2C device address 0x6A
HzSleep r(MEASURING_FREQ);
Servo sv1;
Servo sv2;
Servo sv3;
Madgwick m_;

float pre_roll = 0.0;
float pre_pitch = 0.0;

typedef struct {
  float x;
  float y;
  float z;
} pos3d_t;

pos3d_t gyr_ = { 0 };
pos3d_t acc_ = { 0 };
pos3d_t ang_ = { 0 };
pos3d_t ang_offset_ = { 0 };
double mes_time_ = 1.0 / (double) MEASURING_FREQ * 1000.0;

void blink(int pin, uint16_t delay_time = 5000) {
  digitalWrite(pin, HIGH);
  delay(delay_time);
  digitalWrite(pin, LOW);
  delay(delay_time);
}

void read_gyr() {
  gyr_.x = IMU.readFloatGyroX();
  gyr_.y = IMU.readFloatGyroY();
  gyr_.z = IMU.readFloatGyroZ();
}

void read_acc() {
  acc_.x = IMU.readFloatAccelX();
  acc_.y = IMU.readFloatAccelY();
  acc_.z = IMU.readFloatAccelZ();
}

void calc_offset(int iter){
  digitalWrite(LED_BLUE, HIGH);
  for(int i; i < iter; i++){
    Serial.print("Calc offset");
    Serial.println(i);

    read_acc();
    read_gyr();
    m_.updateIMU(gyr_.x, gyr_.y, gyr_.z, acc_.x, acc_.y, acc_.z);
    ang_offset_.x += m_.getRoll();
    ang_offset_.y += m_.getPitch();
    ang_offset_.z += m_.getYaw();
  
    r.sleep();
  }

  ang_offset_.x /= float(iter);
  ang_offset_.y /= float(iter);
  ang_offset_.z /= float(iter);
}

void setup() {
  // 初期化
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  sv1.attach(SERVO_PIN_1);
  sv2.attach(SERVO_PIN_2);
  sv3.attach(SERVO_PIN_3);

  Serial.begin(115200);
  Serial.println("Start serial");

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;  
  while (IMU.begin() != 0) {
    Serial.print("Begin IMU...");
    blink(LED_GREEN);
  }

  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL2_G,  0x8C);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x8A);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL7_G,  0x00);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL8_XL, 0x09);
  // 測定周波数  
  m_.begin((int)MEASURING_FREQ);

  delay(1000);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);

  calc_offset(500);

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);
}

void loop() {
  digitalWrite(LED_RED, HIGH);

  // update
  read_acc();
  read_gyr();
  m_.updateIMU(gyr_.x, gyr_.y, gyr_.z, acc_.x, acc_.y, acc_.z);

  // get
  ang_.x = m_.getRoll()  - ang_offset_.x;
  ang_.y = m_.getPitch() - ang_offset_.y;
  ang_.z = m_.getYaw()   - ang_offset_.z;

  // accx limitation
  if(IMU.readFloatAccelX()>LIMIT_ACCX) {
    gone = true;
  }
  if(gone) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);
    limit_i = 5.0;
  }
  else {
    digitalWrite(LED_RED, HIGH);
    ang_.y = 0.0;
  }

  // pid
  float pid_roll  = ang_.x*KP_ROLL  + gyr_.x*KD_ROLL  + pre_roll*KI_ROLL;
  float pid_pitch = ang_.y*KP_PITCH + gyr_.y*KD_PITCH + constrain(pre_pitch*KI_PITCH, -limit_i, limit_i);
  pre_roll  = pid_roll;
  pre_pitch = pid_pitch;

  // write servo angle
  static int i = 0;
  i++;
  if(i>=SERVO_SKIP){
    sv1.write(constrain(int(pid_roll)  + 90 + OFFSET_SERVO_1, 0, 180));
    sv2.write(constrain(int(pid_roll)  + 90 + OFFSET_SERVO_2, 0, 180));
    sv3.write(constrain(-int(pid_pitch) + 90 + OFFSET_SERVO_3, 75, 175));
    i = 0;
  }

  // log
  // Serial.print(pid_roll);
  // Serial.print(",");
  // Serial.print(pid_pitch);
  // Serial.println();

  // Serial.print(acc_.x);
  // Serial.print(",");
  // Serial.print(acc_.y);
  // Serial.print(",");
  Serial.print(acc_.x);
  Serial.println();

  r.sleep();
}