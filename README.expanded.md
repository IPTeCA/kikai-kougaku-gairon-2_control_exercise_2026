# kikai-kougaku-gairon-2_control_exercise_2026

## Overview

このリポジトリは、Seeed Studio XIAO 系マイコンを使った制御演習用の教材とサンプルコードをまとめたものです。

- `SampleProgram/`
  前年度資料をもとにしたサンプルコード
- `10_Lesson/`
  今年度向けに再構成した Lesson
- `20_Exercise/`
  `Exercise.md` と解答例

今回の Lesson / Exercise は、最終的に `SampleProgram/60_example03/60_example03.ino` を読める、改造できる、段階的に再現できることを目標に構成しています。

## Boards

### Seeed Studio XIAO nRF52840

- 用途:
  LED, Serial, Servo, IMU, 姿勢推定
- IMU:
  LSM6DS3 を内蔵
- 主に使う Lesson:
  `Lesson01` から `Lesson10`, `Lesson13`, `Lesson15`

### Seeed Studio XIAO ESP32-C3

- 用途:
  `espnow-uart-passthrough` を使った UART over ESP-NOW
- 主に使う Lesson:
  `Lesson11` から `Lesson14`, `Exercise04`

## Servo Motor

使用するサーボモータ:

- `ES9251II`
- `ES9051`

どちらも Lesson では Arduino の `Servo` ライブラリで同様に扱います。

注意:

- サーボの電源は USB 直結だけでは不足する場合があります
- 複数サーボを同時に動かす場合は、電源容量と GND 共通化を確認してください
- 初回は必ず低負荷でテストしてください

## Libraries

### 共通

- `Wire`
  I2C 通信用
- `Servo`
  サーボ制御用

### XIAO nRF52840 で使用

- `LSM6DS3`
  加速度・ジャイロセンサ読み取り
- `MadgwickAHRS`
  姿勢推定用フィルタ

### XIAO ESP32-C3 で使用

- `WiFi`
  ESP-NOW 初期化のために使用
- `esp_now`
  ESP-NOW 通信用

## Recommended Board Packages

### XIAO nRF52840

- ボードマネージャ:
  `Seeed nRF52 mbed-enabled Boards`
- 代表的なボード選択:
  `Seeed XIAO nRF52840 Sense`

**出典（原文の引用）:** [Seeed Studio Wiki — *XIAO nRF52840 (Sense)* / Two Arduino Libraries](https://wiki.seeedstudio.com/XIAO_BLE/#two-arduino-libraries)

> **Two Arduino Libraries**  
> Seeed Studio XIAO nRF52840 assembles many functions in one tiny board and sometimes may not perform the best of them. Hence, Seeed has published two Arduino libraries to let it maximum the power of each function. Therefore:
>
> It is recommanded to use the Seeed nRF52 Boards library if you want to apply Bluetooth function and "Low Energy Cost Function".
>
> It is recommanded to use the Seeed nRF52 mbed-enabled Boards library if you want to use it in embedded Machine Learning Applications or apply "IMU & PDM advanced function".
>
> Both libraries support very well when it comes to the basic usage, such as LED, Digital, Analog, Serial, I2C, SPI.
>
> The Pin definition supported by these two libraries might be a little different and Seeed will keep update the wiki until it is clear.

### XIAO ESP32-C3

- ボードマネージャ:
  `esp32 by Espressif Systems`
- 代表的なボード選択:
  `XIAO_ESP32C3`

## Lesson Mapping

- `Lesson01`:
  単色 LED の点滅
- `Lesson02`:
  RGB LED の基本表示
- `Lesson03`:
  Serial の基本
- `Lesson04`:
  Serial で LED を制御
- `Exercise01`:
  RGB LED の応用
- `Lesson05`:
  サーボ 1 個の基本
- `Lesson06`:
  サーボ 3 個の基本
- `Lesson07`:
  Serial でサーボを制御
- `Exercise02`:
  サーボ操作のまとめ
- `Lesson08`:
  IMU の加速度・ジャイロ取得
- `Lesson09`:
  Madgwick による姿勢推定
- `Lesson10`:
  姿勢に応じた LED 表示
- `Exercise03`:
  姿勢に応じたサーボ制御
- `Lesson11`:
  `espnow-uart-passthrough` の導入と `/mac` での MAC 確認
- `Lesson12`:
  PC シリアルモニタ間で 1 行テキストを無線中継
- `Lesson13`:
  nRF52840 の姿勢情報を `Serial1` から ESP-NOW へ中継
- `Lesson14`:
  `/help`, `/mac`, `/stat` とブロードキャスト確認
- `Exercise04`:
  外部 UART 機器の文字列を ESP-NOW で中継
- `Lesson15`:
  `HzSleep` による一定周期ループ

## Notes

- `Lesson11` から `Exercise04` の ESP-NOW 関連教材は、`sassa4771/espnow-uart-passthrough` の `.h/.cpp` を Lesson/Exercise フォルダにコピーして同梱しています（スケッチは同じディレクトリ内の `espnow_uart_bridge.h` を参照します）
- `Lesson12`, `Lesson14`, `Exercise04` は ESP-NOW の相手側として、`XIAO ESP32-C3` を 2 台使う想定です
- `Lesson13` は 1 つの `.ino` に nRF52840 側と ESP32-C3 側の処理を同居させています
- `Lesson16` は `10_Lesson/Lesson.md` に見出しだけで詳細が無いため、今回の整備対象からは外しています
