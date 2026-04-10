# Exercise

## Exercise01

Lesson01 から Lesson04 で学んだ内容を使って、キーボード入力で次の色を切り替えられるプログラムを作成してください。

- `g` : GREEN
- `c` : CYAN
- `m` : MAGENTA
- `w` : WHITE
- `o` : OFF

条件:

- シリアルモニタは `115200 baud` を使用すること
- RGB LED はアクティブ Low として扱うこと
- 入力した文字をシリアルモニタに表示すること

解答例:

- [Exercise01.ino](D:/GitHub/kikai-kougaku-gairon-2_control_exercise_2026/20_Exercise/Exercise01/Exercise01.ino)

## Exercise02

Lesson05 から Lesson07 を使って、3つのサーボをキーボードで操作できるプログラムを作成してください。

仕様:

- `a/z/q` でサーボ1を `+10 / -10 / 0` 度
- `s/x/w` でサーボ2を `+10 / -10 / 0` 度
- `d/c/e` でサーボ3を `+10 / -10 / 0` 度
- `p` を押したら現在角度をまとめて表示すること
- 角度範囲は `-90` 度から `+90` 度に制限すること

解答例:

- [Exercise02.ino](D:/GitHub/kikai-kougaku-gairon-2_control_exercise_2026/20_Exercise/Exercise02/Exercise02.ino)

## Exercise03

Lesson08 から Lesson10 を使って、姿勢に応じてサーボモータを動かすプログラムを作成してください。

仕様:

- IMU から姿勢情報を取得すること
- `roll` に応じてサーボ1、`pitch` に応じてサーボ2 を動かすこと
- サーボ角は `-45` 度から `+45` 度の範囲に制限すること
- 現在の `roll, pitch, yaw` をシリアルモニタへ出力すること

発展:

- 余裕があればサーボ3も `yaw` や姿勢状態に応じて動かしてください

解答例:

- [Exercise03.ino](D:/GitHub/kikai-kougaku-gairon-2_control_exercise_2026/20_Exercise/Exercise03/Exercise03.ino)

## Exercise04

Lesson13（nRF の IMU 行を `Serial1` で受け取る）と Lesson14（USB でローカルコマンドと任意行を送る）を合体し、**IMU の姿勢行を流しつつ USB からサーボ変更などの指示行を出せる**ブリッジを作ってください。

条件:

- ESP-NOW の共通処理は、`20_Exercise/Exercise04/` に同梱している `espnow_uart_bridge.*` / `uart_line_protocol.*` を使うこと（`espnow_uart_bridge.h` を `#include` して利用する）
- **USB `Serial`**: `/help` `/mac` `/stat` と、それ以外の行は ESP-NOW で送信（サーボ用の文字列など）
- **`Serial1`**: `UART_RX_PIN` / `UART_TX_PIN`（例: D7/D6）を設定し、nRF からの 1 行（姿勢テキスト）をそのまま ESP-NOW で送信する
- 送信先 `peerMac` を相手 ESP32-C3 の MAC アドレスに設定すること（検証時はブロードキャスト可）
- UART ボーレートは `115200` とすること
- nRF 側は Lesson13 の IMU スケッチを想定すること
- `/mac` と `/stat` を使って動作確認すること

解答例:

- [Exercise04.ino](D:/GitHub/kikai-kougaku-gairon-2_control_exercise_2026/20_Exercise/Exercise04/Exercise04.ino)

## Exercise05

Lesson15（一定周期ループ）、Lesson16（P/PD/PID）、Exercise04（ESP-NOW と UART のブリッジ）を前提として、**XIAO nRF52840 上で Lesson16 と同等の P/PD/PID（IMU の pitch → サーボ）**を実装してください。Lesson16 では USB `Serial` からコマンドを受けますが、本課題では **コンソールを `Serial1` に置き、機体側・地上側の XIAO ESP32-C3 各 1 台と ESP-NOW により無線接続する**構成（計 3 枚）とします。

条件:

- **nRF52840 側**: 無線コンソールは `Serial1`（解答例では `RADIO_SERIAL`）。キー `1` / `2` / `3` で P / PD / PID、行コマンドで `kp <値>` / `ki <値>` / `kd <値>` および `help` / `status` を扱えること（仕様の詳細は解答例のヘルプ出力を参照してよい）
- **ESP32-C3 側（2 台）**: ESP-NOW の共通処理は、`20_Exercise/Exercise05/` の各サブフォルダに同梱している `espnow_uart_bridge.*` / `uart_line_protocol.*` を使うこと（`espnow_uart_bridge.h` を `#include` して利用する。Exercise04 と同様の方針）
- UART ボーレートは `115200` とすること
- 送信先 `peerMac` を相手 ESP32-C3 の MAC アドレスに設定すること（検証時はブロードキャスト可）

解答例（ボードごとにサブフォルダが分かれています）:

- [Exercise05_Glider_nrf52840.ino](D:/GitHub/kikai-kougaku-gairon-2_control_exercise_2026/20_Exercise/Exercise05/Exercise05_Glider_nrf52840/Exercise05_Glider_nrf52840.ino)
- [Exercise05_Glider_esp32c3.ino](D:/GitHub/kikai-kougaku-gairon-2_control_exercise_2026/20_Exercise/Exercise05/Exercise05_Glider_esp32c3/Exercise05_Glider_esp32c3.ino)
- [Exercise05_PC_esp32c3.ino](D:/GitHub/kikai-kougaku-gairon-2_control_exercise_2026/20_Exercise/Exercise05/Exercise05_PC_esp32c3/Exercise05_PC_esp32c3.ino)
