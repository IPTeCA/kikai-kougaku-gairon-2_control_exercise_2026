// 対象ボード: Seeed XIAO nRF52840

#include <LSM6DS3.h>
#include <Wire.h>

#define LED_OFF digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, HIGH)
#define RED_ON digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, HIGH)
#define GREEN_ON digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, HIGH)
#define BLUE_ON digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, LOW)
#define CYAN_ON digitalWrite(LED_RED, HIGH); digitalWrite(LED_GREEN, LOW); digitalWrite(LED_BLUE, LOW)
#define MAGENTA_ON digitalWrite(LED_RED, LOW); digitalWrite(LED_GREEN, HIGH); digitalWrite(LED_BLUE, LOW)

LSM6DS3 IMU(I2C_MODE, 0x6A);

void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  LED_OFF;

  Serial.begin(115200);

  IMU.settings.gyroRange = 2000;
  IMU.settings.accelRange = 4;
  while (IMU.begin() != 0) {
    GREEN_ON;
    delay(200);
    LED_OFF;
    delay(200);
  }
}

void loop() {
  float ax = IMU.readFloatAccelX();
  float ay = IMU.readFloatAccelY();
  float az = IMU.readFloatAccelZ();

  float absX = abs(ax);
  float absY = abs(ay);
  float absZ = abs(az);

  if (absX > absY && absX > absZ) {
    if (ax > 0) {
      RED_ON;
    } else {
      BLUE_ON;
    }
  } else if (absY > absZ) {
    if (ay > 0) {
      MAGENTA_ON;
    } else {
      GREEN_ON;
    }
  } else {
    if (az > 0) {
      CYAN_ON;
    } else {
      LED_OFF;
    }
  }

  Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.println(az);

  delay(50);
}
