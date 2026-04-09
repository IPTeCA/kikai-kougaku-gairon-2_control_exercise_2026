#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Servo.h>

//#define MEASURING_FREQ (1660)
#define MEASURING_FREQ (20)
LSM6DS3 IMU(I2C_MODE, 0x6A);  //I2C device address 0x6A
Servo servo[3];

#define LED_OFF     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define RED_ON      digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define GREEN_ON    digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define BLUE_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define CYAN_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);
#define MAGENTA_ON  digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define YELLOW_ON   digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define WHITE_ON    digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);


/////////////////////制御変数・パラメータ///////////////////////////
#define ANGLE00 0
#define ANGLE01 0
#define ANGLE02 0

#define THETA00 0.0
#define THETA01 0.0
#define THETA02 0.0

#define KKK0  0.5
#define KKK1  0.5
#define KKK2  0.5

#define N 18
#define Acc(i)    p[0][i]
#define Acc0(i)   p[1][i]
#define Gyr(i)    p[2][i]
#define Gyr0(i)   p[3][i]
#define Q(i)      p[4][i]
#define Q0(i)     p[5][i]
#define A(i)      p[6][i]
#define A0(i)     p[7][i]
#define Kp(i)     p[8][i]
#define KI(i)     p[9][i]
#define KI0(i)    p[10][i]
#define Kd(i)     p[11][i]
#define E(i)      p[12][i]
#define E_(i)     p[13][i]
#define dE(i)     p[14][i]
#define dE_(i)    p[15][i]
#define SEdt(i)   p[16][i]
#define SEdt0(i)  p[17][i]
float p[N][3];

#define T_delay       20
#define T_calibration 50
#define T_integral    50
float err[3][T_integral];
int idx_e=0;

unsigned long t=0, t_=0, t000=0;

void reset_parameters() {
  int i, j;

  t000 =  millis();
  for(i=0; i<N; i++) {
    p[i][0] = p[i][0] = p[i][2] = 0;
    for(j=0; j<T_integral; j++) {
      err[0][j] = err[1][j] = err[1][j] = 0;
    }
  }
  A0(0) = ANGLE00;
  A0(1) = ANGLE01;
  A0(2) = ANGLE02;

  Q0(0) = THETA00;
  Q0(1) = THETA01;
  Q0(2) = THETA02;

  Kp(0) = KKK0;
  Kp(1) = KKK1;
  Kp(2) = KKK2;

}

/////////////////////センシング関係///////////////////////////
Madgwick m_;

void sensing() {
  t_ = t;
  t =  millis()-t000;
  read_acc();
  read_gyr();
  m_.updateIMU(Gyr(0), Gyr(1), Gyr(2), Acc(0), Acc(1), Acc(2));

  Q(0) = m_.getRoll();
  Q(1) = m_.getPitch();
  Q(2) = m_.getYaw();
}

void read_gyr() {
  Gyr(0) = IMU.readFloatGyroX() - Gyr0(0);
  Gyr(1) = IMU.readFloatGyroY() - Gyr0(1);
  Gyr(2) = IMU.readFloatGyroZ() - Gyr0(2);
//  Gyr(0) = IMU.readFloatGyroX();
//  Gyr(1) = IMU.readFloatGyroY();
//  Gyr(2) = IMU.readFloatGyroZ();
}

void read_acc() {
  Acc(0) = IMU.readFloatAccelX() - Acc0(0);
  Acc(1) = IMU.readFloatAccelY() - Acc0(1);
  Acc(2) = IMU.readFloatAccelZ() - Acc0(2);
//  Acc(0) = IMU.readFloatAccelX();
//  Acc(1) = IMU.readFloatAccelY();
//  Acc(2) = IMU.readFloatAccelZ();
}

void calibration() {
  int i;
  float tmp[6];

  YELLOW_ON;
  Acc0(0)=0;
  Acc0(1)=0;
  Acc0(2)=0;
  Gyr0(0)=0;
  Gyr0(1)=0;
  Gyr0(2)=0;

  for(i=0; i<6; i++) tmp[i]=0;
  for(i=0; i<T_calibration; i++) {
    read_acc();
    read_gyr();
    tmp[0] += Acc(0);  
    tmp[1] += Acc(1);  
    tmp[2] += Acc(2);  
    tmp[3] += Gyr(0);  
    tmp[4] += Gyr(1);  
    tmp[5] += Gyr(2);
    delay(T_delay);
  }
  Acc0(0) = tmp[0]/T_calibration;  
  Acc0(1) = tmp[1]/T_calibration;  
  Acc0(2) = tmp[2]/T_calibration;  
  Gyr0(0) = tmp[3]/T_calibration;  
  Gyr0(1) = tmp[4]/T_calibration;  
  Gyr0(2) = tmp[5]/T_calibration;
  LED_OFF;
}

/////////////////////制御関係///////////////////////////
void errors() {
  int i;
  float ccc[3];
  float dt = (t - t_)/1000.0;

  ccc[0] = 0;
  ccc[1] = 1/dt;
  ccc[2] = -ccc[1];
  for (i=0; i<3; i++) {
    E(i)  = Q0(i)-Q(i);
    SEdt0(i) +=  (E(i)+E_(i))/2*dt;
    SEdt(i)  +=  (E(i)-err[i][idx_e])*dt;
    dE(i) = ccc[0]*dE_(i) + ccc[1]*E(i) + ccc[2]*E_(i);
    E_(i) = E(i);
    dE_(i) = dE(i); 
    err[i][idx_e] = E(i);
  }
  if(++idx_e >= T_integral) idx_e=0;
}

void control01() {
  int i;

  errors();
  for (i=0; i<3; i++) {
    A(i)= (int) (A0(i) + Kp(i)*E(i)+KI(i)*SEdt(i)+Kd(i)*dE(i)+KI0(i)*SEdt0(i));
    if (A(i) >  90) A(i) = 90;
    if (A(i) < -90) A(i) =-90;
    servo[i].write(int(A(i)+ 90));
  }
}

void control_manual() {
  int i;

  for (i=0; i<3; i++) {
    A(i)= (int) (A0(i));
    if (A(i) >  90) A(i) = 90;
    if (A(i) < -90) A(i) =-90;
    servo[i].write(int(A(i)+ 90));
  }
}

/////////////////////動作モード関係///////////////////////////
#define MANUAL          0
#define AUTO            1
#define START           2
#define MODE3           3
#define MODE4           4
#define MODE5           5
#define REAL_FLIGHT     7
#define STOP            9

char mode=0;
char f_mode0=0;
unsigned long t_mode0=0;

void mode_select() {
  if(Acc(2) < -0.6) {
    if (f_mode0 == 0) {
      f_mode0 = 1;
      t_mode0 = t;
    } else {
      if (t - t_mode0 > 3000) {
        detect_mode();
        f_mode0 = 0;
      }
    }
  } else {
    f_mode0=0;
  }
}

void detect_mode() {
  float x, y, z;
  WHITE_ON delay(500); LED_OFF  delay(500);
  WHITE_ON delay(500); LED_OFF  delay(500);
  WHITE_ON delay(500); LED_OFF  delay(500);
  WHITE_ON delay(250); LED_OFF  delay(250);
  WHITE_ON delay(250); LED_OFF  delay(250);
  WHITE_ON delay(250); LED_OFF  delay(250);
  WHITE_ON delay(250); LED_OFF  delay(250);
  read_acc();

  x=Acc(0); if(x < 0) x =-x;
  y=Acc(1); if(y < 0) y =-y;
  z=Acc(2); if(z < 0) z =-z;

  if (x > y) {
    if(x > z) {
      if(Acc(0) > 0) mode=START; else mode=MODE3;
    } else {
      if(Acc(2) > 0) mode=MANUAL; else mode=STOP;
    }
  } else {
    if(y > z) {
      if(Acc(1) > 0) mode = MODE4; else mode=MODE5;
    } else {
      if(Acc(2) > 0) mode=MANUAL; else mode=STOP;
    }
  }
}


/////////////////////キーボード入力（シリアル通信：受信）///////////////////////////
int c;
char mes[100], buf[100];

void key_control() {
  if (Serial.available()) {
    c = Serial.read();
  }

  switch(c) {
    case 'm':
    case '1':
      mode = MANUAL;
      break;
    case 'M':
    case '2':
      mode = AUTO;
      break;
    case 'p':
      sprintf(buf, "A0=[%+3.0f,%+3.0f,%+3.0f], Kp=[%4.2f,%4.2f,%4.2f], roll=%4.2f, pitch=%4.2f, yaw=%4.2f", 
                    A0(0), A0(1), A0(2), Kp(0), Kp(1), Kp(2), Q(0), Q(1), Q(2));
      break;
  }
  if(mode==MANUAL) {
    key0(c); 
  } else {
    key1(c);
  }
  c = 0;
}

void key0(int c) {
  switch (c) {
    case 'a':
      A0(0) += 10;
      if (A0(0) > 90) A0(0) = 90;
      break;
    case 'z':
      A0(0) -= 10;
      if (A0(0) < -90) A0(0) = -90;
      break;
    case 'q':
      A0(0) = 0;
      break;
    case 's':
      A0(1) += 10;
      if (A0(1) > 90) A0(1) = 90;
      break;
    case 'x':
      A0(1) -= 10;
      if (A0(1) < -90) A0(1) = -90;
      break;
    case 'w':
      A0(1) = 0;
      break;
    case 'd':
      A0(2) += 10;
      if (A0(2) > 90) A0(2) = 90;
      break;
    case 'c':
      A0(2) -= 10;
      if (A0(2) < -90) A0(2) = -90;
      break;
    case 'e':
      A0(2) = 0;
      break;
  }
}

void key1(int c) {
  switch (c) {
    case 'a':
      Kp(0) += 0.1;
      if (Kp(0) > 3.0) Kp(0) = 3.0;
      break;
    case 'z':
      Kp(0) -= 0.1;
      if (Kp(0) < 0) Kp(0) = 0;
      break;
    case 'q':
      Kp(0) = 0.5;
      break;
    case 's':
      Kp(1) += 0.1;
      if (Kp(1) > 3.0) Kp(1) = 3.0;
      break;
    case 'x':
      Kp(1) -= 0.1;
      if (Kp(1) < 0) Kp(1) = 0;
      break;
    case 'w':
      Kp(1) = 0.5;
      break;
    case 'd':
      Kp(2) += 0.1;
      if (Kp(2) > 3.0) Kp(2) = 3.0;
      break;
    case 'c':
      Kp(2) -= 0.1;
      if (Kp(2) < 0) Kp(2) = 0;
      break;
    case 'e':
      Kp(2) = 0.5;
      break;
  }
}

/////////////////////モニタ出力（シリアル通信：送信）///////////////////////////
void report() {
  static unsigned long t_p=0;

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
}

void print_values() {
  Serial.print(Q(0));
  Serial.print(",");
  Serial.print(Q(1));
  Serial.print(",");
  Serial.print(Q(2));
  Serial.print(",");
  Serial.print(Acc(0));
  Serial.print(",");
  Serial.print(Acc(1));
  Serial.print(",");
  Serial.print(Acc(2));
  Serial.println("");
}

////////////////////////////////////////////////////////


void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);

  Serial.begin(115200);

  servo[0].attach(2);   //D2端子
  servo[1].attach(3);   //D3端子
  servo[2].attach(4);   //D4端子
//  servo[0].attach(8);   //D7端子
//  servo[1].attach(9);   //D8端子
//  servo[2].attach(10);  //D9端子

//  while (!Serial) {
//    RED_ON delay(500); LED_OFF  delay(500);
//  }

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

  mode = MANUAL;
  reset_parameters();
  calibration();
}


void loop() {
  static unsigned long t_p=0;

  sensing();
  mode_select();

  switch(mode) {
    case STOP:
      WHITE_ON;
    break;
    case MANUAL:
    case AUTO:
      key_control();
      if (mode==MANUAL) {
        GREEN_ON;
        control_manual();
      } else {
        BLUE_ON;
        control01();
      }
      report();
    break;
    case START:
      reset_parameters();
      RED_ON delay(500); LED_OFF  delay(500);
      RED_ON delay(500); LED_OFF  delay(500);
      RED_ON delay(500); LED_OFF  delay(500);
      RED_ON delay(250); LED_OFF  delay(250);
      RED_ON delay(250); LED_OFF  delay(250);
      RED_ON delay(250); LED_OFF  delay(250);
      RED_ON delay(250); LED_OFF  delay(250);
      reset_parameters();
      mode = REAL_FLIGHT;
    break;
    case REAL_FLIGHT:
      YELLOW_ON;
      control01();
    break;
  }
  delay(T_delay);
}
