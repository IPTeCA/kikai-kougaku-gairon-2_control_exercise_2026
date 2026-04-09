// Seeed XIAO nRF52840（20 Hz 固定）

#include "hz_sleep.h"

HzSleep rate;
bool ledOn = false;

static void variableFakeWork() {
  static uint8_t phase = 0;
  const uint32_t extra_us[] = {0, 4000, 8000, 12000};
  uint32_t t0 = micros();
  uint32_t target = extra_us[phase];
  phase = (phase + 1) % 4;
  while ((uint32_t)(micros() - t0) < target) {
  }
}

void setup() {
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  Serial.begin(115200);
  rate.sync();
}

void loop() {
  uint32_t periodUs;

  periodUs = rate.sleepMicros();
  variableFakeWork();
  ledOn = !ledOn;
  digitalWrite(LED_BLUE, ledOn ? LOW : HIGH);

  // delay 比較: 上 4 行をコメントし、下 6 行を有効化
  // uint32_t t0 = micros();
  // variableFakeWork();
  // ledOn = !ledOn;
  // digitalWrite(LED_BLUE, ledOn ? LOW : HIGH);
  // delay(50);
  // periodUs = micros() - t0;

  Serial.println(periodUs);
}
