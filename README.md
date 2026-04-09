# kikai-kougaku-gairon-2_control_exercise_2026

機械工学概論2（制御演習）向けの教材・演習・サンプルコード集です。主に **Seeed Studio XIAO nRF52840** と **XIAO ESP32-C3** を使い、LED / Serial / サーボ / IMU（姿勢推定）/ ESP-NOW を段階的に扱います。

Lesson / Exercise は、最終的に `30_GliderSample/example03/60_example03.ino` を**読解・改造し、段階的に再現できる**ことを念頭に構成しています。

---

## 1. 必要なマイコン・機材とボード設定

### マイコン・用途の対応

| ボード | 主な用途 | 内蔵 IMU | 主に使うフォルダ |
|--------|----------|----------|------------------|
| **Seeed Studio XIAO nRF52840（Sense 推奨）** | LED / Serial / Servo / IMU / 姿勢推定 | LSM6DS3 | `Lesson01`〜`Lesson10`, `Lesson10_5`, `Lesson13`, `Lesson15`, `Exercise05` |
| **Seeed Studio XIAO ESP32-C3** | `espnow-uart-passthrough` による UART over ESP-NOW（2 台間でシリアル行を中継） | なし | `Lesson11`〜`Lesson14`, `Exercise04`, `Exercise05` |

### Arduino IDE：ボードマネージャとボード選択

まず Arduino IDE の **プリファレンス（Preferences）**で、ボードマネージャの追加URLを設定します。

1. Arduino IDE → `ファイル` → `環境設定...`（Preferences）
2. `追加のボードマネージャのURL` に、次を追加（複数ある場合は改行またはカンマ区切り）
   - Seeed（XIAO nRF52840）：`https://files.seeedstudio.com/arduino/package_seeeduino_boards_index.json`
   - Espressif（XIAO ESP32-C3）：`https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. `ツール` → `ボード` → `ボードマネージャ...` から必要なパッケージをインストールし、スケッチごとにボードを選びます。

| ボード | ボードマネージャ（追加パッケージ） | スケッチで選ぶボード例 |
|--------|-----------------------------------|------------------------|
| XIAO nRF52840 | **Seeed nRF52 mbed-enabled Boards** | `Seeed XIAO nRF52840 Sense` |
| XIAO ESP32-C3 | **esp32**（Espressif Systems） | `XIAO_ESP32C3` |

nRF52840 には Seeed が **2 種類の Arduino ボードパッケージ**を提供しています。IMU（LSM6DS3）や PDM などを使う本教材では **mbed-enabled** 側の利用が想定です。Bluetooth 中心・省電力用途では別パッケージの選択もあり得ます。詳細は [Seeed Wiki — XIAO nRF52840 / Two Arduino Libraries](https://wiki.seeedstudio.com/XIAO_BLE/#two-arduino-libraries) を参照してください。

出典（Wiki 原文・英語）: [Seeed Studio Wiki — *XIAO nRF52840 (Sense)* / Two Arduino Libraries](https://wiki.seeedstudio.com/XIAO_BLE/#two-arduino-libraries)

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

### サーボモータ

| 型番 | 備考 |
|------|------|
| ES9251II | `Servo` ライブラリで同様に扱います |
| ES9051 | 同上 |

- サーボの電源は **USB だけでは不足することがあります**（外部電源 + **GND 共通**）。
- 複数サーボ同時駆動時は電源容量と GND 共通化を確認してください。初回は低負荷でテストしてください。
- ソフト上のサーボ角は `0..180` に制限されるため、`constrain()` 等でクリップしてください。

### 主なライブラリ

| 分類 | ライブラリ | 用途 |
|------|------------|------|
| 共通 | `Wire` | I2C |
| 共通 | `Servo` | サーボ制御 |
| nRF52840 | `LSM6DS3` | 加速度・ジャイロ |
| nRF52840 | `MadgwickAHRS` | 姿勢推定 |
| ESP32-C3 | `WiFi` | ESP-NOW 初期化など |
| ESP32-C3 | `esp_now` | ESP-NOW 通信 |

---

## 2. リポジトリ構成

| パス | 説明 |
|------|------|
| `10_Lesson/` | 今年度向けに再構成した授業 Lesson（フォルダ別）と `Lesson.md` |
| `20_Exercise/` | `Exercise.md`、各演習フォルダ、**解答例** |
| `30_GliderSample/` | Lesson / Exercise 後に読む機体制御サンプル |
| `attachments/` | 授業用動画（`.mp4`）・画像（`.png` / `.jpg`） |

---

## 3. 取り組む順番

次の順で進めることを推奨します。

| 順番 | フォルダ | 内容 |
|:----:|----------|------|
| 1 | **`10_Lesson/`** | 各 Lesson フォルダと全体ノート `10_Lesson/Lesson.md` |
| 2 | **`20_Exercise/`** | 演習一覧 `20_Exercise/Exercise.md` と各 `Exercise01` など |
| 3 | **`30_GliderSample/`** | グライダー（機体制御）サンプル。まず `30_GliderSample/README.md` → 各 `example*/README.md` → `.ino` の順 |
| 4 | （本 README **§7**） | **`30_GliderSample` を踏まえた最終実装課題**（無線での PID・サーボ調整・テレメトリログ）。詳細は下記 §7 |

参考目標スケッチ（到達イメージの一例）: `30_GliderSample/example03/60_example03.ino`

---

## 4. Lesson と内容の対応（一覧）

| フォルダ | 内容 |
|----------|------|
| `Lesson01` | 単色 LED の点滅 |
| `Lesson02` | RGB LED の基本表示 |
| `Lesson03` | Serial の基本 |
| `Lesson04` | Serial で LED を制御 |
| `Exercise01` | RGB LED の応用 |
| `Lesson05` | サーボ 1 個の基本 |
| `Lesson06` | サーボ 3 個の基本 |
| `Lesson07` | Serial でサーボを制御 |
| `Exercise02` | サーボ操作のまとめ |
| `Lesson08` | IMU の加速度・ジャイロ取得 |
| `Lesson09` | Madgwick による姿勢推定 |
| `Lesson10` | 姿勢に応じた LED 表示 |
| `Lesson10_5` | X 軸加速度ピークの LED / Serial（Lesson 10.5 補足） |
| `Exercise03` | 姿勢に応じたサーボ制御 |
| `Lesson11` | `espnow-uart-passthrough` の導入と `/mac` での MAC 確認 |
| `Lesson12` | PC シリアルモニタ間で 1 行テキストを無線中継 |
| `Lesson13` | nRF52840 の姿勢情報を `Serial1` から ESP-NOW へ中継 |
| `Lesson14` | `/help`, `/mac`, `/stat` とブロードキャスト確認 |
| `Exercise04` | 外部 UART 機器の文字列を ESP-NOW で中継 |
| `Lesson15` | `HzSleep` による一定周期ループ |
| `Exercise05` | Lesson16 相当の離散 P/PD/PID（pitch→サーボ）を、`Serial1` と ESP32-C3×2・ESP-NOW で無線コンソール接続（3 枚構成） |

### 教材まわりの注意

- `Lesson11`〜`Exercise05` の ESP-NOW 教材は、`sassa4771/espnow-uart-passthrough` の `.h/.cpp` を Lesson/Exercise フォルダにコピーして同梱しています（スケッチは同ディレクトリの `espnow_uart_bridge.h` を参照します）。
- `Lesson12`, `Lesson14`, `Exercise04`, `Exercise05` は ESP-NOW の相手側として **XIAO ESP32-C3 を 2 台**使う想定です。
- `Lesson13` は 1 つの `.ino` に nRF52840 側と ESP32-C3 側の処理を同居させています。
- `Lesson16` は `10_Lesson/Lesson.md` に見出しのみで、今回の整備対象からは外しています。

---

## 5. Exercise の説明

演習の**課題全文・条件・解答例へのリンク**は `20_Exercise/Exercise.md` にまとめています。各フォルダ `20_Exercise/ExerciseNN/` に解答例スケッチ `ExerciseNN.ino` があります（**`Exercise05` は複数ボード用にサブフォルダ分け**）。

※ **Exercise は課題です。** 学習のねらいのため、**`30_GliderSample/` のサンプルや、解答例（`ExerciseNN.ino` など）を先に読んだり流用したりせず**、課題文とこれまでの Lesson をもとに**自分で取り組んでから**、必要に応じて参照する進め方を推奨します。

| フォルダ | 主に使う前提 Lesson | 課題の要約 |
|----------|---------------------|------------|
| `Exercise01` | `Lesson01`〜`Lesson04` | シリアル入力で RGB LED の色を切り替える（`g` / `c` / `m` / `w` / `o` など）。`115200 baud`、アクティブ Low 等の条件あり。 |
| `Exercise02` | `Lesson05`〜`Lesson07` | キーボードで 3 つのサーボを操作（キー割当・角度表示・`±90°` 制限など）。 |
| `Exercise03` | `Lesson08`〜`Lesson10` | IMU の姿勢に応じてサーボを駆動（`roll` / `pitch` 割当、`±45°` 制限、姿勢のシリアル出力など）。発展でサーボ3や `yaw` も可。 |
| `Exercise04` | `Lesson13`, `Lesson14` | nRF の IMU 行を `Serial1` で受けつつ、USB から `/help` 等と任意行（サーボ指示など）を ESP-NOW で送るブリッジ。同梱の `espnow_uart_bridge.*` 等を利用。 |
| `Exercise05` | `Lesson15`, **Lesson16**, `Exercise04` | Lesson16 と同等の離散 P/PD/PID を nRF で動かし、コンソールを `Serial1`＋ESP32-C3（機体・地上の 2 台）と ESP-NOW で接続する。解答例は `Exercise05_Glider_nrf52840` 等のサブフォルダ。 |

---

## 6. 30_GliderSample の説明

`10_Lesson/`・`20_Exercise/` のあとに読む **グライダー／機体制御のサンプル集**です。全体の狙い・読み方・ハード注意の詳細は **`30_GliderSample/README.md`** に書いています。

**このフォルダのねらい（要約）**

| ねらい | 内容 |
|--------|------|
| 流れを追う | IMU（姿勢推定）→ 誤差・操作量の計算 → サーボ、という **閉ループ** をコード上で通しで読む |
| 実装を比較する | 投げ上げ・加速度・ノイズ・サーボ制約など、**実機向けの工夫**がサンプルごとにどう違うかを見る |
| 改造の土台にする | 自分の課題に合わせて **出発点**として使う |

**進め方（推奨）**

1. `30_GliderSample/README.md` で全体像をつかむ  
2. 興味のある `exampleNN/` の **`README.md`** を読む  
3. 同フォルダの **`.ino`** を、`setup()` → `loop()` → 制御計算の順に追う  

**共通のハード注意（要約）**

- サーボ電源は USB だけでは不足しがち（外部電源 + **GND 共通**）
- サーボ角は `0..180` に制限され、ソフトでも `constrain()` 等が必要
- Madgwick の姿勢は、**強い加速度が乗る区間**では角度が乱れやすい

**サンプル一覧**

| フォルダ | 主な内容 |
|----------|----------|
| `example01` | 姿勢角からサーボへ至る **P 制御の最小例**（閉ループの骨格） |
| `example02` | **離散 PID**（積分・微分の更新形）をスケッチ上で揃えた例 |
| `example03` | 姿勢と **離散 PID**、モード（手動／自動／飛行など）。到達イメージの一例として `60_example03.ino` を参照 |
| `example04` | `example03` の発展（**キャリブレーション**、積分の扱いなど） |
| `example05` | 角度の平滑化、**D 項の安定化**など |
| `example-4pin_flight` | **4 サーボ**（エレベータ／エルロン／ラダー等）とミキシング |
| `example_tsuchiyama` | 投げ上げ条件を意識した例（**ジャイロを D に**、マルチレート、状態切替など） |

サンプルを読んだうえで、続けて **§7 の最終実装課題**（グライダー制御用プログラムとして満たす要件）に取り組みます。

---

## 7. 最終実装課題（グライダー制御用プログラム）

`30_GliderSample` までを踏まえたうえで、**グライダーの制御用プログラム**として次を満たすことを最終実装課題とします。

| 項目 | 要件 |
|------|------|
| **PID パラメータの遠隔変更** | 無線通信により **PID のパラメータ（少なくとも `kp`, `ki`, `kd` に対応する値）を変更できる**こと。 |
| **サーボの遠隔調整** | 無線通信により **サーボの調整**（トリム・目標角・チャンネル割当など、運用に必要な調整）ができること。 |
| **遠隔ログ** | 無線通信により、少なくとも次の項目を **ログとして取得できる**こと。 |

**ログに含めるフィールド（想定）**

| フィールド | 意味の目安 |
|------------|------------|
| `src_seq` | 送信側が付与する **サンプル／パケット通番**（下記「`src_seq` について」参照） |
| `dt_ms` | サンプル間隔 [ms] |
| `ax`, `ay`, `az` | 加速度 |
| `gx`, `gy`, `gz` | 角速度 |
| `roll`, `pitch`, `yaw` | 姿勢角 |
| `s0`, `s1`, `s2` | サーボ等の出力チャンネル |
| `kp`, `ki`, `kd` | PID ゲイン |

**`src_seq` について**  
「送信元シーケンス」とは、**テレメトリやログを送る側（多くは機体上のマイコン）が、そのデータに付ける通し番号**を指します。例として、制御ループや無線送信のたびに 0, 1, 2, … と増やす、といった使い方を想定します。受信側・記録側では、この番号で **パケット欠損や順序の逆転を検出**したり、加速度・姿勢・サーボ出力など **同一時刻のサンプル同士を対応付け**たりします。

（実装の都合で名前や単位はサンプルコードに合わせてよいが、**無線で上記相当のテレメトリ・PID 調整・サーボ調整が可能**であることを満たすこと。）
