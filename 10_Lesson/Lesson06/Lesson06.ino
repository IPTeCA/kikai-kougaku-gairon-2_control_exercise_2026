// 対象ボード: Seeed XIAO nRF52840

#include <Servo.h>

Servo servo[3];

void setup() {
  servo[0].attach(0); // D0
  servo[1].attach(1); // D1
  servo[2].attach(2); // D2
}

void loop() {
  servo[0].write(90);
  servo[1].write(90);
  servo[2].write(90);
  delay(1000);

  servo[0].write(0);
  servo[1].write(30);
  servo[2].write(30);
  delay(1000);

  servo[0].write(90);
  servo[1].write(90);
  servo[2].write(90);
  delay(1000);

  servo[0].write(180);
  servo[1].write(150);
  servo[2].write(150);
  delay(1000);
}
