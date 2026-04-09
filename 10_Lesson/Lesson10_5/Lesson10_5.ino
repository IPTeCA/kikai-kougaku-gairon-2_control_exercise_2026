// 対象ボード: Seeed XIAO nRF52840（Sense）
// Lesson 10.5 補足: X 軸加速度のピークを LED 段階表示・Serial 出力

#include <LSM6DS3.h>  // Seeed Arduino LSM6DS3
#include <Wire.h>
#include <MadgwickAHRS.h>

#define N 3
float sig, sig_m, sig_s, sig_max, sig_[N];
int i_sig;
unsigned long t_max;
#define MEASURING_FREQ (20)

#define LED_OFF     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define RED_ON      digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define GREEN_ON    digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define BLUE_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define CYAN_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);
#define MAGENTA_ON  digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define YELLOW_ON   digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define WHITE_ON    digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);

LSM6DS3 IMU(I2C_MODE, 0x6A);

typedef struct {
  float x;
  float y;
  float z;
} pos3d_t;

unsigned long t = 0, t_ = 0;
pos3d_t acc_ = { 0 };
double mes_time_ = 1.0 / (double)MEASURING_FREQ * 1000.0;
Madgwick m_;

void print_values() {
  Serial.print(sig_max);
  Serial.print(",");
  Serial.print(t_max);
  Serial.print(",");
  Serial.print(sig);
  Serial.print(",");
  Serial.print(sig_m);
  Serial.println("");
}

void read_acc() {
  acc_.x = IMU.readFloatAccelX();
  acc_.y = IMU.readFloatAccelY();
  acc_.z = IMU.readFloatAccelZ();
}

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  Serial.begin(115200);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;

  while (IMU.begin() != 0) {
    GREEN_ON delay(500);
    LED_OFF delay(500);
  }

  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL2_G, 0x8C);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x8A);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL7_G, 0x00);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL8_XL, 0x09);

  m_.begin((int)MEASURING_FREQ);

  reset_sigs();
}

void reset_sigs() {
  int i;
  sig = sig_m = sig_s = sig_max = 0;
  for (i = 0; i < N; i++) sig_[i] = 0;
  i_sig = 0;
  t_max = 0;
}

void sensing() {
  t_ = t;
  t = millis();
  read_acc();
}

void max_signal() {
  sig_s += sig - sig_[i_sig];
  sig_[i_sig] = sig;
  if (++i_sig > N) i_sig = 0;
  sig_[i_sig] = sig;

  sig_m = sig_s / N;
  if (sig_m >= sig_max) {
    sig_max = sig_m;
    t_max = t;
  }
}

unsigned char mode = 0, f_mode0 = 0;
unsigned long t_mode0 = 0;

void mode_select() {
  if (acc_.z < -0.6) {
    if (f_mode0 == 0) {
      f_mode0 = 1;
      t_mode0 = t;
    } else {
      if (t - t_mode0 > 3000) {
        mode = 55;
        f_mode0 = 0;
      }
    }
  } else {
    f_mode0 = 0;
  }

  switch (mode) {
    case 55:
      WHITE_ON delay(500);
      LED_OFF delay(500);
      WHITE_ON delay(500);
      LED_OFF delay(500);
      WHITE_ON delay(500);
      LED_OFF delay(500);
      WHITE_ON delay(250);
      LED_OFF delay(250);
      WHITE_ON delay(250);
      LED_OFF delay(250);
      WHITE_ON delay(250);
      LED_OFF delay(250);
      WHITE_ON delay(250);
      LED_OFF delay(250);
      reset_sigs();
      mode = 0;
      break;
  }
}

#define VAL5 2.5
#define VAL4 2.0
#define VAL3 1.5
#define VAL2 1.0
#define VAL1 0.5

void loop() {
  sensing();
  mode_select();

  sig = acc_.x;
  max_signal();

  if (sig_max > VAL5) {
    RED_ON;
  } else if (sig_max > VAL4) {
    MAGENTA_ON;
  } else if (sig_max > VAL3) {
    YELLOW_ON;
  } else if (sig_max > VAL2) {
    CYAN_ON;
  } else if (sig_max > VAL1) {
    BLUE_ON;
  } else {
    GREEN_ON;
  }
  print_values();
}
