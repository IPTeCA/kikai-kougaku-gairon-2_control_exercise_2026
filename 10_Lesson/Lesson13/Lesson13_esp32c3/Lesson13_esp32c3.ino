// 対象ボード: Seeed XIAO ESP32-C3

#include <Arduino.h>
#include "espnow_uart_bridge.h"

#define LED_PIN 2
#define UART_BAUD 115200
#define UART_PORT Serial1
#define D7 20
#define D6 21
#define UART_RX_PIN D7
#define UART_TX_PIN D6

// Replace this with the MAC address of the wireless receiver.
const uint8_t peerMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setup() {
  espnow_uart_bridge::configure(peerMac, UART_PORT, LED_PIN);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(115200);
  Serial.println("Lesson13 (ESP32-C3): bridge posture text from Serial1 to ESP-NOW");

  UART_PORT.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  delay(200);

  espnow_uart_bridge::initializeEspNow();
}

void loop() {
  espnow_uart_bridge::processUartLoop();
  delay(1);
}

