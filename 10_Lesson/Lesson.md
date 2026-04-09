# LED と Serial 通信について

この `Lesson.md` は **各 Lesson の目的・学習内容・サンプルコード・補足**をまとめたノートです（UTF-8）。

---

## Lesson01: LED 点滅（XIAO nRF52840）
**目標** `pinMode` / `digitalWrite` / `delay` の基本を学ぶ

### 学習内容
- RGB LED の各ピンを出力に設定する
- `loop()` で赤LEDを一定周期で点滅させる
- `delay(ms)` で時間を作る

### サンプルコード
`10_Lesson/Lesson01/Lesson01.ino`

```cpp
void setup() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
}

void loop() {
  digitalWrite(LED_RED, LOW);
  delay(500);
  digitalWrite(LED_RED, HIGH);
  delay(500);
}
```

### 補足：アクティブ LOW とは
このボードの RGB LED は **アクティブ LOW** です。

- `LOW`：点灯
- `HIGH`：消灯

そのため、`setup()` でいったん全部 `HIGH`（消灯）にしてから、点灯したい色だけ `LOW` にします。

---

## Lesson02: RGB LED の3色表示（XIAO nRF52840）
**目標** RGB LED の基本表示（R/G/B）を学ぶ

### 学習内容
- 3色（R/G/B）を順番に点灯する
- 最後に全消灯して区切りをつける
- `delay()` で切り替え間隔を一定にする

### サンプルコード
`10_Lesson/Lesson02/Lesson02.ino`

```cpp
void loop() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  delay(400);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, HIGH);
  delay(400);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, LOW);
  delay(400);

  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  delay(400);
}
```

---

## Lesson03: キーボードで Serial 通信（XIAO nRF52840）
**目標** Serial 通信の基本（受信・送信）を学ぶ

### 学習内容
- `Serial.begin(115200)` で通信速度を合わせる
- `Serial.available()` で受信の有無を確認する
- `Serial.read()` で1文字読む
- `Serial.write()` / `Serial.print()` / `Serial.println()` の使い分け

### `Serial.write` と `print` / `println` の違い（目安）
| 関数 | 何を送るか |
|---|---|
| `write(c)` | 受信した **バイトをそのまま** 送る（エコーに便利） |
| `print(x)` | 人が読むための **文字列** に変換して送る |
| `println(x)` | `print` + 改行（行ごとに見やすい） |

### サンプルコード（エコーバック）
`10_Lesson/Lesson03/Lesson03.ino`

```cpp
void loop()
{
  if (Serial.available())
  {
    char c = Serial.read();
    Serial.write(c);
  }
}
```

---

## Lesson04: Serial 入力で LED 制御（XIAO nRF52840）
**目標** Serial 入力（コマンド）で状態を切り替える

### 学習内容
- 1文字コマンド（`r/g/b/0`）で LED を切り替える
- LED 操作をマクロにして読みやすくする

### サンプルコード
`10_Lesson/Lesson04/Lesson04.ino`

```cpp
if (Serial.available()) {
  char c = Serial.read();
  if (c == 'r') {
    RED_ON;
    Serial.println("RED");
  } else if (c == '0') {
    LED_OFF;
    Serial.println("OFF");
  }
}
```

---

## Lesson05: サーボ 1 個の基本（XIAO nRF52840）
**目標** `Servo` ライブラリでサーボを動かす

### 学習内容
- `servo.attach(pin)` でピンに接続
- `servo.write(angle)` で角度指定（0〜180）

### サンプルコード
`10_Lesson/Lesson05/Lesson05.ino`

```cpp
Servo servo;

void setup() {
  servo.attach(0);  // D0
}

void loop() {
  servo.write(90);
  delay(1000);
  servo.write(30);
  delay(1000);
}
```

### 補足：電源
サーボは USB だけだと電流が足りないことがあります。動作が不安定なら外部電源を検討し、**GND 共通**にしてください。

---

## Lesson06: サーボ 3 個の基本（XIAO nRF52840）
**目標** 複数サーボを配列で扱う

### 学習内容
- `Servo servo[3];` でまとめる
- 複数の角度を同時に切り替える

### サンプルコード
`10_Lesson/Lesson06/Lesson06.ino`

```cpp
Servo servo[3];

void setup() {
  servo[0].attach(0);
  servo[1].attach(1);
  servo[2].attach(2);
}
```

---

## Lesson07: Serial でサーボを制御（XIAO nRF52840）
**目標** キーボード操作でサーボ角を増減・リセットする

### 学習内容
- コマンド例：`a/z/q`, `s/x/w`, `d/c/e`
- 角度を `constrain()` で制限する
- 角度配列を保持して `writeServoAngles()` で反映する

### サンプルコード
`10_Lesson/Lesson07/Lesson07.ino`

---

## Lesson08: IMU（LSM6DS3）で加速度・ジャイロ取得（XIAO nRF52840）
**目標** IMU の基本データを Serial に出す

### 学習内容
- `IMU.begin()` が成功するまで待つ（失敗時は LED 点滅など）
- `readFloatAccel*()` / `readFloatGyro*()` を読む
- CSV で出してプロットできるようにする

### サンプルコード
`10_Lesson/Lesson08/Lesson08.ino`

```cpp
float ax = IMU.readFloatAccelX();
float ay = IMU.readFloatAccelY();
float az = IMU.readFloatAccelZ();
float gx = IMU.readFloatGyroX();
float gy = IMU.readFloatGyroY();
float gz = IMU.readFloatGyroZ();

Serial.print(ax); Serial.print(",");
Serial.print(ay); Serial.print(",");
Serial.print(az); Serial.print(",");
Serial.print(gx); Serial.print(",");
Serial.print(gy); Serial.print(",");
Serial.println(gz);
```

---

## Lesson09: Madgwick で姿勢推定（XIAO nRF52840）
**目標** roll/pitch/yaw を推定して出力する

### 学習内容
- `filter.begin(MEASURING_FREQ)`
- `filter.updateIMU(gx,gy,gz, ax,ay,az)`
- `getRoll/getPitch/getYaw`

### サンプルコード
`10_Lesson/Lesson09/Lesson09.ino`

---

## Lesson10: 姿勢に応じた LED 表示（XIAO nRF52840）
**目標** センサ値から状態を作って出力（LED）で見える化する

### 学習内容
- 加速度の絶対値を比較して「どの軸が一番立っているか」を判定
- 状態ごとに LED の色を変える

### サンプルコード
`10_Lesson/Lesson10/Lesson10.ino`

---

## Lesson11: ESP-NOW の導入（XIAO ESP32-C3）
**目標** ESP-NOW を初期化して `/mac` で MAC を確認する

### 学習内容
- `espnow_uart_bridge::configure(...)`
- `espnow_uart_bridge::initializeEspNow()`
- `/help`, `/mac` などのローカルコマンド

### サンプルコード
`10_Lesson/Lesson11/Lesson11.ino`

---

## Lesson12: 1 行テキストを無線中継（XIAO ESP32-C3 ×2）
**目標** Enter で確定した 1 行を ESP-NOW で相手へ送る

### 学習内容
- `peerMac` を相手の MAC に設定する
- 1 行（`\n`）単位で送信・受信する

### サンプルコード
`10_Lesson/Lesson12/Lesson12.ino`

---

## Lesson13: nRF52840 の姿勢を Serial1 → ESP-NOW（nRF + ESP32-C3）
**目標** センサ側（nRF）と中継側（ESP32）を分けて動かす

### 学習内容
- nRF 側：姿勢を計算して `Serial1.println(...)` で送る
- ESP32 側：`Serial1` を受けて ESP-NOW に流す

### サンプルコード
- `10_Lesson/Lesson13/Lesson13_nrf52840/Lesson13_nrf52840.ino`
- `10_Lesson/Lesson13/Lesson13_esp32c3/Lesson13_esp32c3.ino`

---

## Lesson14: グライダー側 nRF（Serial1 から舵コマンドでサーボ）
**目標** 無線経由のコマンド（最終的には Serial1）で舵を動かす

### 学習内容
- Lesson07 と同じキー操作を Serial1 入力で受ける
- コマンド → 角度更新 → `servo.write()` の流れ

### サンプルコード
`10_Lesson/Lesson14/Lesson14_Glider_nrf52840/Lesson14_Glider_nrf52840.ino`

---

## Lesson15: 一定周期ループ（HzSleep）
**目標** `dt`（周期）が制御に効くことを体感する

### 学習内容
- `HzSleep` で一定周期に同期する
- 処理時間が変動しても周期を一定に近づける

### サンプルコード
`10_Lesson/Lesson15/Lesson15.ino`

---

## Lesson15_ServoTarget: 目標角入力でサーボを動かす（PIDの前段）
**目標** ステップ入力（大きい目標変化）を簡単に作る

### 学習内容
- Serial で角度を入力してサーボを動かす
- A/B の目標を用意して `toggle` で切り替える

### サンプルコード
`10_Lesson/Lesson15_ServoTarget/Lesson15_ServoTarget.ino`

---

## Lesson16: 離散 P / PD / PID（IMU pitch → サーボ）
**目標** PID の各項（P/I/D）が挙動にどう効くかを観察する

### 学習内容
- `e = target - pitch`
- `I += e*dt`（PID のときだけ）
- `dE = (e - prevE)/dt`（PD/PID のときだけ）
- `u = Kp*e`（P）
- `u = Kp*e + Kd*dE`（PD）
- `u = Kp*e + Ki*I + Kd*dE`（PID）
- `servo = 90 + u`（0..180 に制限）

### 操作（Serial）
- モード切替：`1`=P, `2`=PD, `3`=PID
- ゲイン変更：`kp <v>` / `ki <v>` / `kd <v>`
- `status` / `help`（`?`）

### 補足：PD の注意点
PD は I を持たないのでワインドアップは起きませんが、D（`dE/dt`）はノイズに敏感で、`Kd` を上げすぎると動きが荒くなることがあります。

---

## Lesson17: 実機向けの工夫（D にジャイロ、I の開始条件、サーボ間引き）
**目標** Lesson16 を実機で使いやすくする工夫を知る

### 学習内容
- D項に `dE/dt` ではなく **ジャイロ `gy`** を使う（ノイズに強い）
- `started` 条件を満たすまで I を溜めない
- `SERVO_SKIP` でサーボ更新を間引く

### サンプルコード
`10_Lesson/Lesson17/Lesson17.ino`

---

## Lesson18: PID を調整しながら理解する（Serial コマンド付き）
**目標** 目標角・ゲイン・ログ間引き等を動かしながら、PID の挙動を掴む

### 学習内容
- P/PD/PID の切替（`1/2/3`）
- `kp/ki/kd` の変更
- `target`, `ilim`（積分上限）, `log`（ログ間引き）, `reset`

### サンプルコード
`10_Lesson/Lesson18/Lesson18.ino`
