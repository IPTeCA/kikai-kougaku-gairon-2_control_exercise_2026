// 対象ボード: Seeed XIAO ESP32-C3 を 2 台（各 PC に USB 接続し、同じスケッチを書き込む）

#include <Arduino.h>
#include "espnow_uart_bridge.h"

#define LED_PIN 2
#define UART_BAUD 115200
#define UART_PORT Serial

// 【必須】Lesson11 の /mac で調べた「相手機」の MAC を 6 バイトで書く（2 台とも相手側を指す）。
// 例: 相手が 12:34:56:78:9A:BC なら {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}
const uint8_t peerMac[6] = {0x94, 0xA9, 0x90, 0x6A, 0xE2, 0x20};

void setup() {
  espnow_uart_bridge::configure(peerMac, UART_PORT, LED_PIN);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  UART_PORT.begin(UART_BAUD);
  delay(200);

  espnow_uart_bridge::initializeEspNow();
  UART_PORT.println("Lesson12: wireless UART passthrough");
  UART_PORT.println("Enter one line and press Enter.");
  UART_PORT.println("Use /mac, /stat, /help as local commands.");
}

void loop() {
  espnow_uart_bridge::processUartLoop();
  delay(1);
}
