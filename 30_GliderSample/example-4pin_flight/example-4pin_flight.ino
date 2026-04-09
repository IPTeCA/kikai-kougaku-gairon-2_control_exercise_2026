/* プログラム概要（読了後、この欄は左タブから閉じておくことを推奨）

 これは滑空機の4ピン対応プログラムコードです。
 エレベータ、左右エルロン、ラダーを制御します。
 ピン番号は84行のdefine文で設定してください。
 キーボードによって以下のモードを切り替え可能です。
 ・MANUALモード：キーボード操作によって各飛行段階の目標モーター角度を変更することができます。
 ・AUTO_TESTモード：自律飛行します。キーボード操作によって各ゲインを変更することができます。
 ・AUTOモード：自律飛行します。本番飛行を想定。

 ** 操作方法 **
 シリアル通信を通じてキー入力を送信します。
 "0": AUTO モード
    これを押した約1秒後に飛行スタンバイモードに移行します。
    これまでに調整したハイパーパラメータは初期化されません。
    初期化される情報：
      飛行段階、積分値、6軸値などの内部パラメータ。
      詳細は reset_parameters() 参照

 "m": MANUALモード
    エレベータ
      "q": +1 (deg)
      "a": -1
      "z": reset (ハイパーパラメータで設定した初期値になります)
    エルロン左
      "w": +1
      "s": -1
      "x": reset
    エルロン右
      "e": +1
      "d": -1
      "c": reset
    ラダー
      "r": +1
      "f": -1
      "v": reset

 "M": AUTO_TEST モード
    各段階対応の自律飛行モードです。
    "1": フライト直後姿勢に変更
    "2": 安定飛行姿勢に変更（初期状態はこれです）
    "3": 落下飛行姿勢に変更

    "4": roll 操作に変更（初期状態はこれです）
    "5": pitch 操作に変更
    "6": yaw 操作に変更
    Pゲイン
      "q": +0.1
      "a": -0.1
      "z": reset
    Iゲイン
      "w": +0.01
      "s": -0.01
      "x": reset
    Dゲイン 
      "e": +0.01
      "d": -0.01
      "c": reset

 ** 備考 **
 フライト段階による理想姿勢の切り替えは調整が面倒なだけだから不要だぜ！という方は
 各段階のゲインをすべて同じ値にすることでフライト段階ごとの調整が不要になります。

 不具合や要望は満仲まで
 mitsunaka.kanae.z9@s.gifu-u.ac.jp

@autor mitsunaka
*/

#include <LSM6DS3.h>
#include <Wire.h>
#include <MadgwickAHRS.h>
#include <Servo.h>
#define AUTO            0
#define MANUAL          1
#define AUTO_TEST       2

///////////////////// ハイパーパラメータ //////////////////////////
// 初期モード(↑上のdefine参照)
#define MODE 2
// 応答速度(ms) フィーリングの値。推奨 15ms ~ 60fps
#define delay_ms 15

// サーボ位置修正 (滑空機の構造に応じてpin番号を割り当ててください。デフォルト：0~3)
#define servo_elev    0   // エレベータのピン番号
#define servo_aile_L  1   // エルロン左
#define servo_aile_R  2   // エルロン右
#define servo_rudd    3   // ラダー

// モーター初期角度 0:エレベータ, 1:左, 2:右, 3:ラダー
#define ANGLE0_0  0
#define ANGLE0_1  0
#define ANGLE0_2  0
#define ANGLE0_3  0

// フライト直後の目標姿勢 0:roll, 1:pitch, 2:yaw
#define THETA0_0  0.0
#define THETA0_1  4.0  // 機首上げ
#define THETA0_2  0.0  // yaw は相対値のみ利用が理想。デフォルトだと使用しないので0固定で可
#define TIME_0    1500  // このモードを維持する時間(ms)

// 安定飛行時の目標姿勢 0:roll, 1:pitch, 2:yaw
#define THETA1_0  0.0
#define THETA1_1  0.0
#define THETA1_2  0.0   // yaw は相対値のみ利用が理想。0固定で可
#define TIME_1    1000  // このモードを維持する時間(ms)

// 落下飛行時の目標姿勢 0:roll, 1:pitch, 2:yaw
#define THETA2_0  0.0
#define THETA2_1  0.0
#define THETA2_2  0.0   // yaw は相対値のみ利用が理想。0固定で可

// ゲイン調整：数値が大きいほどモータ変化に影響大
// 計算式を弄りたい場合は control_auto() 参照
// デフォルトは elve,aile,rudd がそれぞれ roll,pitch,yaw のみを参照
// 0:roll
#define Kp0   1.0  // P:差に対するゲイン
#define KI0   0.0  // I:積分
#define Kd0   0.0  // D:微分

// 1:pitch
#define Kp1   2.7
#define KI1   0.0
#define Kd1   0.02

// 2:yaw
#define Kp2   0.0
#define KI2   0.0
#define Kd2   0.1

// AUTO 時のモーター角度の限界値 0:エレベータ, 1:左, 2:右, 3:ラダー
// MANUALモードで許容角度を探してください。デフォルトは -90~90
// elve
#define ANGLE0_min  -90
#define ANGLE0_MAX  90
// aile_L
#define ANGLE1_min  -90
#define ANGLE1_MAX  90
// aile_R
#define ANGLE2_min  -90
#define ANGLE2_MAX  90
// rudd
#define ANGLE3_min  -90
#define ANGLE3_MAX  90

//////// ユーザー設定ここまで ////////


///////////////////// 以下内部処理 ///////////////////////////
#define LIMIT_ACCX 1.0

//#define MEASURING_FREQ (1660)
#define MEASURING_FREQ (30)
LSM6DS3 IMU(I2C_MODE, 0x6A);  //I2C device address 0x6A
Servo servo[4];

#define LED_OFF     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define RED_ON      digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, HIGH);
#define GREEN_ON    digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define BLUE_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define CYAN_ON     digitalWrite(LED_RED, HIGH);digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);
#define MAGENTA_ON  digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH);digitalWrite(LED_BLUE, LOW);
#define YELLOW_ON   digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH);
#define WHITE_ON    digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW);

#define N_p 13
#define Acc(i)    p[0][i]
#define Gyr(i)    p[1][i]
#define Q(i)      p[2][i]
#define Q0(i)     p[3][i]
#define Kp(i)     p[4][i]
#define KI(i)     p[5][i]
#define Kd(i)     p[6][i]
#define E(i)      p[7][i]
#define E_(i)     p[8][i]
#define dE(i)     p[9][i]
#define dE_(i)    p[10][i]
#define SEdt(i)   p[11][i]
#define temp(i)   p[12][i]

#define N_q 4
#define A(j)      q[0][j]
#define A0(j)     q[1][j]
#define servo_min(j) q[2][j]
#define servo_MAX(j) q[3][j]

unsigned long t=0, t_=0, t000=0;
float p[N_p][3];
float q[N_q][4];
char mode = MODE;
int flight_mode = 0;
long count_start;
int auto_test_rpy = 0;

void reset_parameters() {
  int i;
  RED_ON;

  t000 =  millis();
  for(i=0; i<N_p; i++) {
    p[i][0] = p[i][1] = p[i][2] = 0;
  }
  for(i=0; i<N_q; i++) {
    p[i][0] = p[i][1] = p[i][2] = p[i][3] = 0;
  }
  A0(0) = ANGLE0_0;
  A0(1) = ANGLE0_1;
  A0(2) = ANGLE0_2;
  A0(3) = ANGLE0_3;

  Q0(0) = THETA0_0;
  Q0(1) = THETA0_1;
  Q0(2) = THETA0_2;

  Kp(0) = Kp0;
  Kp(1) = Kp1;
  Kp(2) = Kp2;

  KI(0) = KI0;
  KI(1) = KI1; 
  KI(2) = KI2;

  Kd(0) = Kd0;
  Kd(1) = Kd1;
  Kd(2) = Kd2;

  servo_min(0) = ANGLE0_min;
  servo_min(1) = ANGLE1_min;
  servo_min(2) = ANGLE2_min;
  servo_min(3) = ANGLE3_min;

  servo_MAX(0) = ANGLE0_MAX;
  servo_MAX(1) = ANGLE1_MAX;
  servo_MAX(2) = ANGLE2_MAX;
  servo_MAX(3) = ANGLE3_MAX;

  delay(1000);
  LED_OFF;
  
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
  Gyr(0) = IMU.readFloatGyroX();
  Gyr(1) = IMU.readFloatGyroY();
  Gyr(2) = IMU.readFloatGyroZ();
}

void read_acc() {
  Acc(0) = IMU.readFloatAccelX();
  Acc(1) = IMU.readFloatAccelY();
  Acc(2) = IMU.readFloatAccelZ();
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
    E(i) = Q0(i)-Q(i);
    SEdt(i) += (E(i)+E_(i))/2*dt;
    dE(i) = ccc[0]*dE_(i) + ccc[1]*E(i) + ccc[2]*E_(i);
    E_(i) = E(i);
    dE_(i) = dE(i); 
  }
}

void control_auto() {
  int i;
  errors();

  // A,A0...0:エレベータ 1:右 2:左 3:ラダー
  // others...0:roll 1:pitch 2:yaw
  A(0)= (int) (A0(0) + Kp(1)*E(1) + KI(1)*SEdt(1) + Kd(1)*dE(1)); // pitch:PID
  A(1)= (int) (A0(1) + Kp(0)*E(0) + KI(0)*SEdt(0) + Kd(0)*dE(0)); // roll:PID
  A(2)= (int) (A0(2) + Kp(0)*E(0) + KI(0)*SEdt(0) + Kd(0)*dE(0)); // 同上
  A(3)= (int) (A0(3) + Kp(2)*(E(2)-E_(2)) + KI(2)*SEdt(2) + Kd(2)*dE(2)); // yaw:PID
  
  // 調整
  for (i=0; i<4; i++) { // servo roop
    if (A(i) < servo_min(i)) A(i) = servo_min(i);
    if (A(i) > servo_MAX(i)) A(i) = servo_MAX(i);

    servo[i].write(int(A(i)+ 90));
  }
}

void control_manual() {
  int i;

  for (i=0; i<4; i++) {
    A(i)= (int) (A0(i));
    if (A(i) >  90) A(i) = 90;
    if (A(i) < -90) A(i) =-90;
    servo[i].write(int(A(i)+ 90));
  }
}

void update_Q0(){
  switch(flight_mode){
    case 0:
    case 1:
      Q0(0) = THETA0_0;
      Q0(1) = THETA0_1;
      Q0(2) = THETA0_2;
      break;
    case 2:
      Q0(0) = THETA1_0;
      Q0(1) = THETA1_1;
      Q0(2) = THETA1_2;
      break;
    case 3:
      Q0(0) = THETA2_0;
      Q0(1) = THETA2_1;
      Q0(2) = THETA2_2;
      break;
  }
}

/////////////////////キーボード入力（シリアル通信：受信）///////////////////////////
int c;
char mes[100], buf[100];

void key_control() {
  if (Serial.available()) {
    c = Serial.read();
  }
  // AUTO:0, MANUAL:1m, AUTO_TEST:2M
  switch(c) {
    case '0':
      mode = AUTO;
      flight_mode = 0;
      // このモードのキーが押されるたびに飛行スタンバイモードに入り直します
      reset_parameters();
      break;
    case 'm':
      mode = MANUAL;
      flight_mode = 0;
      break;
    case 'M':
      mode = AUTO_TEST;
      flight_mode = 2;
      break;
  }
  
  if(mode == MANUAL) {
    key_manual(c); 
  }
  else if(mode == AUTO_TEST) {
    key_auto(c);
  }
  c = 0;
}

void key_manual(int c) {
  /* メモ
    エレベータ
      "q": +1 (deg)
      "a": -1
      "z": reset
    エルロン左
      "w": +1
      "s": -1
      "x": reset
    エルロン右
      "e": +1
      "d": -1
      "c": reset
    ラダー
      "r": +1
      "f": -1
      "v": reset
  */
  switch (c) {
    case 'q':
      A0(0) += 1;
      break;
    case 'a':
      A0(0) -= 1;
      break;
    case 'z':
      A0(0) = ANGLE0_0;
      break;

    case 'w':
      A0(1) += 1;
      break;
    case 's':
      A0(1) -= 1;
      break;
    case 'x':
      A0(1) = ANGLE0_1;
      break;

    case 'e':
      A0(2) += 1;
      break;
    case 'd':
      A0(2) -= 1;
      break;
    case 'c':
      A0(2) = ANGLE0_2;
      break;

    case 'r':
      A0(3) += 1;
      break;
    case 'f':
      A0(3) -= 1;
      break;
    case 'v':
      A0(3) = ANGLE0_3;
      break;
  }
}

void key_auto(int c) {
  /* memo
  各段階対応の自律飛行モードです。
    "1": フライト直後姿勢に変更
    "2": 安定飛行姿勢に変更（初期状態はこれです）
    "3": 落下飛行姿勢に変更

    "4": P ゲイン操作に変更（初期状態はこれです）
    "5": I ゲイン操作に変更
    "6": D ゲイン操作に変更
    ロール
      "q": +0.1
      "a": -0.1
      "z": reset
    ピッチ
      "w": +0.1
      "s": -0.1
      "x": reset
    ヨー 
      "e": +0.1
      "d": -0.1
      "c": reset
  */

  // 初期値に戻すための初期値格納用配列を用意 012=PID
  if(auto_test_rpy==0){
    temp(0)=Kp0;
    temp(1)=KI0;
    temp(2)=Kd0;
  }
  else if(auto_test_rpy==1){
    temp(0)=Kp1;
    temp(1)=KI1;
    temp(2)=Kd1;
  }
  else {
    temp(0)=Kp2;
    temp(1)=KI2;
    temp(2)=Kd2;
  }
  switch (c) {
    case '1':
      flight_mode = 1;
      update_Q0();
      break;
    case '2':
      flight_mode = 2;
      update_Q0();
      break;
    case '3':
      flight_mode = 3;
      update_Q0();
      break;
    
    case '4':
      auto_test_rpy = 0;
      break;
    case '5':
      auto_test_rpy = 1;
      break;
    case '6':
      auto_test_rpy = 2;
      break;
    
    case 'q':
      Kp(auto_test_rpy) += 0.1;
      break;
    case 'a':
      Kp(auto_test_rpy) -= 0.1;
      break;
    case 'z':
      Kp(auto_test_rpy) = temp(auto_test_rpy);
      break;

    case 'w':
      KI(auto_test_rpy) += 0.01;
      break;
    case 's':
      KI(auto_test_rpy) -= 0.01;
      break;
    case 'x':
      KI(auto_test_rpy) = temp(auto_test_rpy);
      break;

    case 'e':
      Kd(auto_test_rpy) += 0.01;
      break;
    case 'd':
      Kd(auto_test_rpy) -= 0.01;
      break;
    case 'c':
      Kd(auto_test_rpy) = temp(auto_test_rpy);
      break;
  }
}

/////////////////////モニタ出力（シリアル通信：送信）///////////////////////////
void report() {
  static unsigned long t_p=0;

  if(t - t_p > 1000) {
    if(mode==AUTO) {
      sprintf(mes, "t=%6d ** Auto ** %s\n", t, buf);
      sprintf(buf, "flight_mode=%d, roll=%4.2f, pitch=%4.2f, yaw=%4.2f", flight_mode, Q(0), Q(1), Q(2));
    }
    else if(mode==MANUAL) {
      sprintf(mes, "t=%6d * Manual * %s\n", t, buf);
      sprintf(buf, "A0=[%+3.0f,%+3.0f,%+3.0f,%+3.0f], roll=%4.2f, pitch=%4.2f, yaw=%4.2f", A0(0), A0(1), A0(2), A0(3), Q(0), Q(1), Q(2));
    } else {
      sprintf(mes, "t=%6d *AutoTest* %s\n", t, buf);
      if(auto_test_rpy==0){
        sprintf(buf, "mode=%d, roll, PID=[%4.2f,%4.3f,%4.3f], roll=%4.2f, pitch=%4.2f, yaw=%4.2f", flight_mode, Kp(0), KI(0), Kd(0), Q(0), Q(1), Q(2));
      }
      else if(auto_test_rpy==1){
        sprintf(buf, "mode=%d, pitch, PID=[%4.2f,%4.3f,%4.3f], roll=%4.2f, pitch=%4.2f, yaw=%4.2f", flight_mode, Kp(1), KI(1), Kd(1), Q(0), Q(1), Q(2));
      } else {
        sprintf(buf, "mode=%d, yaw, PID=[%4.2f,%4.3f,%4.3f], roll=%4.2f, pitch=%4.2f, yaw=%4.2f", flight_mode, Kp(2), KI(2), Kd(2), Q(0), Q(1), Q(2));
      }
    }
    Serial.print(mes);
    t_p = t;
  }
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

  servo[0].attach(servo_elev);
  servo[1].attach(servo_aile_L);
  servo[2].attach(servo_aile_R);
  servo[3].attach(servo_rudd);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;
  while (IMU.begin() != 0) {
    RED_ON delay(500); LED_OFF delay(500);
  }
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL2_G,  0x8C);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x8A);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL7_G,  0x00);
  IMU.writeRegister(LSM6DS3_ACC_GYRO_CTRL8_XL, 0x09);
  m_.begin((int)MEASURING_FREQ);

  reset_parameters();
}


void loop() {
  static unsigned long t_p=0;

  sensing();
  key_control();

  if (mode == AUTO){
    // 本番飛行
    WHITE_ON;

    if (flight_mode==1 && (millis() - count_start > TIME_0)){
      // 発射フラグが立ってからの時間（count_start）が指定秒数超えたとき目標姿勢を更新
      count_start = millis(); // カウントし直す
      flight_mode = 2;
      update_Q0();
    }
    else if (flight_mode==2 && (millis() - count_start > TIME_1)){
      // 安定姿勢から落下姿勢に更新
      flight_mode = 3;
      update_Q0();
    }
    else if (flight_mode == 0){
      // フライトスタンバイ
      // x成分（進行方向）の加速度を見て、一定の値より高いとフラグを立てる
      if(IMU.readFloatAccelX()>LIMIT_ACCX) {
        flight_mode = 1;
        update_Q0();
        RED_ON;
      } else {
        // 今か今かと待ち続ける
        count_start = millis();
      }
    }
    control_auto();
  }
  else if (mode == MANUAL) {
    GREEN_ON;
    control_manual();
  } else {
    // AUTO_TEST,
    BLUE_ON;
    control_auto();
  }

  report();
  delay(delay_ms);
}

