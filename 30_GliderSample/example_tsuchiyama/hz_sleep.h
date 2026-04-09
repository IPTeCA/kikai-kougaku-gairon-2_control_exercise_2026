#define LOGGER 1

class HzSleep{
  private:
   float hz;
   uint32_t previous_time_us;
  public:
  HzSleep(float hz) : hz(hz){
    previous_time_us = micros();
  }

  float sleep() {
    uint32_t dt;
    while(1) {
      dt = micros() - previous_time_us;

      if (dt > uint32_t(1000000.0/hz) + 1000){
        if(LOGGER) Serial.println("Over time");
      }

      if (dt > uint32_t(1000000.0/hz)) {
        break;
      }
    }
    previous_time_us = micros();

    return dt/1000000.0;
  }
};