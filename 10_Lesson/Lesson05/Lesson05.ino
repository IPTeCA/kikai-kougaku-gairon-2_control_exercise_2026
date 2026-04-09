// 対象ボード: Seeed XIAO nRF52840

#include <Servo.h>

Servo servo;

void setup() {
  servo.attach(0);  // D0
}

void loop() {
  servo.write(90);
  delay(1000);

  servo.write(30);
  delay(1000);

  servo.write(90);
  delay(1000);

  servo.write(150);
  delay(1000);
}
