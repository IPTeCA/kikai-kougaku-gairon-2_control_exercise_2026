#include <Servo.h>

#if defined(SERIAL_PORT_MONITOR)
#define LESSON_SERIAL SERIAL_PORT_MONITOR
#else
#define LESSON_SERIAL Serial
#endif

Servo servo[3];
int angleDeg[3] = {0, 0, 0};

void writeServoAngles() {
  for (int i = 0; i < 3; i++) {
    servo[i].write(angleDeg[i] + 90);
  }
}

void printServoAngles() {
  LESSON_SERIAL.print("angles = [");
  LESSON_SERIAL.print(angleDeg[0]);
  LESSON_SERIAL.print(", ");
  LESSON_SERIAL.print(angleDeg[1]);
  LESSON_SERIAL.print(", ");
  LESSON_SERIAL.print(angleDeg[2]);
  LESSON_SERIAL.println("]");
}

void setup() {
  LESSON_SERIAL.begin(115200);
  servo[0].attach(8);
  servo[1].attach(9);
  servo[2].attach(10);
  writeServoAngles();
  printServoAngles();
}

void loop() {
  if (!LESSON_SERIAL.available()) {
    return;
  }

  char c = LESSON_SERIAL.read();
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
    case 'p':
      printServoAngles();
      return;
  }

  for (int i = 0; i < 3; i++) {
    angleDeg[i] = constrain(angleDeg[i], -90, 90);
  }

  writeServoAngles();
  printServoAngles();
}
