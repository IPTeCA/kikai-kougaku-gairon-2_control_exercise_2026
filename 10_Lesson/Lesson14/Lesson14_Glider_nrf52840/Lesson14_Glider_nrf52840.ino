// 対象ボード: Seeed XIAO nRF52840（Glider側 / サーボ制御）
//
// 目的:
// - `Serial1` から受け取ったコマンド（1文字）でサーボ角を操作する
// - コマンドは Lesson07 と同じ（a/z/q, s/x/w, d/c/e）
//
// 想定:
// - Glider側ESP32-C3がESP-NOWで受信した文字列をSerial1へ吐き出す
// - nRF52840はSerial1から文字を読み取り、サーボを動かす

#include <Servo.h>

Servo servo[3];
int angleDeg[3] = {0, 0, 0};

void writeServoAngles() {
  servo[0].write(angleDeg[0] + 90);
  servo[1].write(angleDeg[1] + 90);
  servo[2].write(angleDeg[2] + 90);
}

void setup() {
  Serial.begin(115200);   // debug
  Serial1.begin(115200);  // command input from ESP32-C3

  // Lesson07 と同じピン配置（必要に応じて変更）
  servo[0].attach(0);  // D0
  servo[1].attach(1);  // D1
  servo[2].attach(2);  // D2
  writeServoAngles();

  Serial.println("Lesson14 (Glider/nRF52840): waiting commands on Serial1");
}

void loop() {
  if (!Serial1.available()) {
    return;
  }

  char c = Serial1.read();
  switch (c) {
    case 'a':
      angleDeg[0] += 10;
      break;
    case 'z':
      angleDeg[0] -= 10;
      break;
    case 'q':
      angleDeg[0] = 0;
      break;
    case 's':
      angleDeg[1] += 10;
      break;
    case 'x':
      angleDeg[1] -= 10;
      break;
    case 'w':
      angleDeg[1] = 0;
      break;
    case 'd':
      angleDeg[2] += 10;
      break;
    case 'c':
      angleDeg[2] -= 10;
      break;
    case 'e':
      angleDeg[2] = 0;
      break;
    default:
      // ignore '\r', '\n', and other chars
      return;
  }

  for (int i = 0; i < 3; i++) {
    angleDeg[i] = constrain(angleDeg[i], -90, 90);
  }

  writeServoAngles();

  Serial.print("servo = ");
  Serial.print(angleDeg[0]);
  Serial.print(", ");
  Serial.print(angleDeg[1]);
  Serial.print(", ");
  Serial.println(angleDeg[2]);
}

