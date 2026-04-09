// Seeed XIAO ESP32-C3 (PC side)
// USB Serial -> ESP-NOW -> (Glider ESP32-C3) -> Serial1 -> nRF52840
//
// Send commands like:
//   kp 1.0
//   ki 0.2
//   kd 0.02
//   status
// And watch responses forwarded back over ESP-NOW.

#include <Arduino.h>

#include "espnow_uart_bridge.h"

#define LED_PIN 2
#define UART_BAUD 115200
#define CMD_PORT Serial

// Set to the Glider ESP32-C3 STA MAC.
const uint8_t peerMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  CMD_PORT.begin(UART_BAUD);
  delay(200);

  espnow_uart_bridge::configure(peerMac, CMD_PORT, LED_PIN, nullptr);
  espnow_uart_bridge::initializeEspNow();

  CMD_PORT.println("Exercise05 (PC/ESP32-C3): wireless console (Lesson16-style: 1/2/3, kp/ki/kd, status)");
  CMD_PORT.println("USB: /help, /mac, /stat - other lines forwarded to Glider");
}

void loop() {
  espnow_uart_bridge::processUartLoop();
  delay(1);
}
