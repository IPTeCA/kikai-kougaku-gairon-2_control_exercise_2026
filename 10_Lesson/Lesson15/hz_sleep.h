#ifndef HZ_SLEEP_H
#define HZ_SLEEP_H

#include <Arduino.h>

// 20 Hz 固定（50 ms = 50000 µs）
class HzSleep {
 private:
  uint32_t previousTimeUs;

 public:
  HzSleep() {
    previousTimeUs = micros();
  }

  void sync() {
    previousTimeUs = micros();
  }

  uint32_t sleepMicros() {
    uint32_t dt;
    while (true) {
      dt = micros() - previousTimeUs;
      if (dt >= 50000u) {
        break;
      }
    }
    previousTimeUs = micros();
    return dt;
  }

  float sleep() {
    return sleepMicros() / 1000000.0f;
  }
};

#endif
