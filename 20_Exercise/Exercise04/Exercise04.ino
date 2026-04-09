// 対象ボード: Seeed XIAO ESP32-C3
// Lesson13（nRF の IMU 行を Serial1 で受信して ESP-NOW へ）と
// Lesson14（USB Serial でローカルコマンドと任意行送信）を合体した例。
//
// XIAO nRF52840 側は Lesson13 の IMU スケッチのまま（Serial1 で roll,pitch,yaw を送出）。
// 受信側 ESP32-C3 では Exercise02 相当などで行を解釈してサーボを動かす想定。

#include <Arduino.h>
#include "espnow_uart_bridge.h"

#define LED_PIN 2
#define UART_BAUD 115200
#define CMD_PORT Serial
#define RELAY_PORT Serial1
#define D7 20
#define D6 21
#define UART_RX_PIN D7
#define UART_TX_PIN D6

// 受信側 ESP32-C3 の STA MAC に合わせる（ブロードキャストのままでも可）。
const uint8_t peerMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  CMD_PORT.begin(UART_BAUD);
  RELAY_PORT.begin(UART_BAUD, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  delay(200);

  espnow_uart_bridge::configure(peerMac, CMD_PORT, LED_PIN, &RELAY_PORT);
  espnow_uart_bridge::initializeEspNow();

  CMD_PORT.println("Exercise04: IMU (Serial1) + USB commands -> ESP-NOW");
  CMD_PORT.println("Serial1: posture lines from nRF (Lesson13 firmware)");
  CMD_PORT.println("USB: /help, /mac, /stat — or send servo command lines to peer");
}

void loop() {
  espnow_uart_bridge::processUartLoop();
  delay(1);
}
