// 対象ボード: Seeed XIAO nRF52840

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  if (Serial.available())
  {
    char c = Serial.read();
    // 受信バイトをそのまま返す（エコーバック）
    Serial.write(c);
    // 1文字でも print で送れる（表示はほぼ同じ）
    // Serial.print(c);
    // 1文字のあとに改行したいとき
    // Serial.println(c);
  }
}
