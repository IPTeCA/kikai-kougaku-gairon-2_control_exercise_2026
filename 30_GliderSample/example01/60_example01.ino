#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Servo.h>

//#define MEASURING_FREQ (1660)
#define MEASURING_FREQ (20)

#define LED_OFF     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define RED_ON      digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define GREEN_ON    digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define BLUE_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define CYAN_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);
#define MAGENTA_ON  digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define YELLOW_ON   digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define WHITE_ON    digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);

#define ANGLE00 0
#define ANGLE01 0
#define ANGLE02 0

#define THETA00 0.0
#define THETA01 0.0
#define THETA02 0.0

#define KKK0  1.5
#define KKK1  1.5
#define KKK2  1.5

LSM6DS3 IMU(I2C_MODE, 0x6A);  //I2C device address 0x6A
Servo servo[3];

unsigned long t=0, t_=0;
int ang[3], ang0[3];
float theta[3], theta0[3], kkk[3];

int c;
char mes[100], buf[100];
char mode=0, mode_p=0;

typedef struct {
  float x;
  float y;
  float z;
} pos3d_t;

pos3d_t gyr_ = { 0 };
pos3d_t acc_ = { 0 };
pos3d_t ang_ = { 0 };
double mes_time_ = 1.0 / (double) MEASURING_FREQ * 1000.0;
Madgwick m_;

void sensing() {
  t_ = t;
  t =  millis();
  read_acc();
  read_gyr();
  m_.updateIMU(gyr_.x, gyr_.y, gyr_.z, acc_.x, acc_.y, acc_.z);
  // get
  theta[0] = m_.getRoll();
  theta[1] = m_.getPitch();
  theta[2] = m_.getYaw();
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

void set_angle() {
  int i;
  for (i=0; i<3; i++) {
    ang[i]= (int) (ang0[i] + mode*kkk[i]*(theta0[i]-theta[i])*0.3);
    if (ang[i] >  90) ang[i]= 90;
    if (ang[i] < -90) ang[i]=-90;
    servo[i].write(ang[i] + 90);
  }
}

void key_control() {
  if (Serial.available()) {
    c = Serial.read();
  }
  if(mode==0) key0(c); else key1(c);
  c = 0;
}

void key0(int c) {
  switch (c) {
    case 'a':
      ang0[0] += 10;
      if (ang0[0] > 90) ang0[0] = 90;
      break;
    case 'z':
      ang0[0] -= 10;
      if (ang0[0] < -90) ang0[0] = -90;
      break;
    case 'q':
      ang0[0] = 0;
      break;
    case 's':
      ang0[1] += 10;
      if (ang0[1] > 90) ang0[1] = 90;
      break;
    case 'x':
      ang0[1] -= 10;
      if (ang0[1] < -90) ang0[1] = -90;
      break;
    case 'w':
      ang0[1] = 0;
      break;
    case 'd':
      ang0[2] += 10;
      if (ang0[2] > 90) ang0[2] = 90;
      break;
    case 'c':
      ang0[2] -= 10;
      if (ang0[2] < -90) ang0[2] = -90;
      break;
    case 'e':
      ang0[2] = 0;
      break;
    case 'm':
    case '1':
      mode = 0;
      break;
    case 'M':
    case '2':
      mode = 1;
      break;
    case 'p':
      sprintf(buf, "ang0=[%3d,%3d,%3d], K=[%4.2f,%4.2f,%4.2f], roll=%4.2f, pitch=%4.2f, yaw=%4.2f", 
                    ang0[0], ang0[1], ang0[2], kkk[0], kkk[1], kkk[2], theta[0], theta[1], theta[2]);
      break;
  }
}

void key1(int c) {
  switch (c) {
    case 'a':
      kkk[0] += 0.1;
      if (kkk[0] > 3.0) kkk[0] = 3.0;
      break;
    case 'z':
      kkk[0] -= 0.1;
      if (kkk[0] < 0) kkk[0] = 0;
      break;
    case 'q':
      kkk[0] = 1.5;
      break;
    case 's':
      kkk[1] += 0.1;
      if (kkk[1] > 3.0) kkk[1] = 3.0;
      break;
    case 'x':
      kkk[1] -= 0.1;
      if (kkk[1] < 0) kkk[1] = 0;
      break;
    case 'w':
      kkk[1] = 1.5;
      break;
    case 'd':
      kkk[2] += 0.1;
      if (kkk[2] > 3.0) kkk[2] = 3.0;
      break;
    case 'c':
      kkk[2] -= 0.1;
      if (kkk[2] < 0) kkk[2] = 0;
      break;
    case 'e':
      kkk[2] = 1.5;
      break;
    case 'm':
    case '1':
      mode = 0;
      break;
    case 'M':
    case '2':
      mode = 1;
      break;    
    case '0':
      if (mode_p == 1) mode_p=0; else mode_p=1;
      break;
    case 'p':
      sprintf(buf, "ang0=[%3d,%3d,%3d], K=[%4.2f,%4.2f,%4.2f], roll=%4.2f, pitch=%4.2f, yaw=%4.2f", 
                    ang0[0], ang0[1], ang0[2], kkk[0], kkk[1], kkk[2], theta[0], theta[1], theta[2]);
      break;
  }
}

void setup() {
  // 初期化
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  Serial.begin(115200);
  servo[0].attach(8);   //D7端子
  servo[1].attach(9);   //D8端子
  servo[2].attach(10);  //D9端子

  while (!Serial) {
    RED_ON delay(500); LED_OFF  delay(500);
  }

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;
  while (IMU.begin() != 0) {
    GREEN_ON delay(500); LED_OFF  delay(500);
  }
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL2_G,  0x8C);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x8A);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL7_G,  0x00);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL8_XL, 0x09);
  m_.begin((int)MEASURING_FREQ);

  ang[0] = ang[1] = ang[2] = 0;
  ang0[0] = ANGLE00;
  ang0[1] = ANGLE01;
  ang0[2] = ANGLE02;

  theta0[0] = THETA00;
  theta0[1] = THETA01;
  theta0[2] = THETA02;

  kkk[0] = KKK0;
  kkk[1] = KKK1;
  kkk[2] = KKK2;
}

void loop() {
  static unsigned long t_p=0;
  sensing();
  key_control();
  set_angle();
  if (mode_p == 0) {
    if(t - t_p > 1000) {
      if(mode) {
        sprintf(mes, "t=%6d **Auto** %s\n", t, buf);
      } else {
        sprintf(mes, "t=%6d *Manual* %s\n", t, buf);      
      }
      sprintf(buf, "\0");
      Serial.print(mes);
      t_p = t;
    }
  } else {
      print_values();
  }
  delay(10);
}

void print_values() {
  Serial.print(theta[0]);
  Serial.print(",");
  Serial.print(theta[1]);
  Serial.print(",");
  Serial.print(theta[2]);
  Serial.print(",");
  Serial.print(acc_.x);
  Serial.print(",");
  Serial.print(acc_.y);
  Serial.print(",");
  Serial.print(acc_.z);
  Serial.println("");
}
