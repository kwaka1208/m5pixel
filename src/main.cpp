/**
 * M5Stack Basic + NeoPixel Christmas Illumination
 * * 機能 (ユーザー指定のボタン配置: 上からC, B, A):
 * - ボタンC (上): 明るさアップ
 * - ボタンB (中): 明るさダウン
 * - ボタンA (下): モード切り替え
 * * 画面設定: Rotation 2 (縦画面、ガイド右側)
 * * 更新: Snow Sparkleをふわっとした動作に変更
 */

#include <M5Stack.h>
#include <Adafruit_NeoPixel.h>

// ==========================================
// 設定エリア
// ==========================================
// GPIO 21（Port Aの赤いコネクタのピンを使用可能）
#define PIN            21   
#define NUM_LEDS       30   // LEDの数 (ここで定数指定)
#define DEFAULT_BRIGHT 20   // 初期の明るさ (0-255)

// NeoPixelインスタンスの作成
Adafruit_NeoPixel pixels(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

// モード定義
enum Mode {
  MODE_MERRY_XMAS = 0, // 赤と緑の交互移動
  MODE_SNOW_SPARKLE,   // 白ベースにキラキラ（ふわっとVer）
  MODE_CANDLE,         // 暖色のゆらぎ
  MODE_OFF,            // 消灯
  MODE_COUNT           // モード数カウント用
};

int currentMode = MODE_MERRY_XMAS;
int currentBrightness = DEFAULT_BRIGHT;
unsigned long lastUpdate = 0;

// Snow Sparkle用の状態管理配列
// sparkleLevel: 現在の追加輝度 (0.0 - 255.0)
// sparkleStep: 変化量 (+なら明るくなる、-なら暗くなる、0なら待機)
float sparkleLevel[NUM_LEDS];
float sparkleStep[NUM_LEDS];

// 関数プロトタイプ宣言
void drawScreen();
void effectMerryXmas();
void effectSnowSparkle();
void effectCandle();
void clearLeds();
void resetSparkleVars();

void setup() {
  // LCD, SD, Serialは有効、I2Cは無効(false)にして干渉を避ける
  M5.begin(true, true, true, false); 
  M5.Power.begin();
  
  // スピーカーのDAC出力無効化
  dacWrite(25, 0); 

  // 画面の向きを設定: 2
  M5.Lcd.setRotation(2);

  // ピンを出力モードに設定
  pinMode(PIN, OUTPUT);
  
  pixels.begin();
  pixels.setBrightness(currentBrightness);
  pixels.clear();
  pixels.show(); // 初期化時は消灯
  
  // 変数初期化
  resetSparkleVars();

  drawScreen();
}

void loop() {
  M5.update();

  // --- ボタン操作 ---
  // 物理配置: 上からBtnC, BtnB, BtnAの順
  
  // ボタンC (一番上): 明るくする
  if (M5.BtnC.wasPressed()) {
    currentBrightness = min(255, currentBrightness + 5);
    pixels.setBrightness(currentBrightness);
    pixels.show();
    drawScreen();
  }

  // ボタンB (真ん中): 暗くする
  if (M5.BtnB.wasPressed()) {
    currentBrightness = max(5, currentBrightness - 5);
    pixels.setBrightness(currentBrightness);
    pixels.show(); 
    drawScreen();
  }

  // ボタンA (一番下): モード変更
  if (M5.BtnA.wasPressed()) {
    currentMode++;
    if (currentMode >= MODE_COUNT) {
      currentMode = 0;
    }
    drawScreen();
    // モード切り替え時に一度リセット・変数のクリア
    clearLeds();
    if (currentMode == MODE_SNOW_SPARKLE) {
      resetSparkleVars();
    }
  }

  // --- エフェクト処理 ---
  switch (currentMode) {
    case MODE_MERRY_XMAS:
      effectMerryXmas();
      break;
    case MODE_SNOW_SPARKLE:
      effectSnowSparkle();
      break;
    case MODE_CANDLE:
      effectCandle();
      break;
    case MODE_OFF:
      // 何もしない
      break;
  }
}

// --------------------------------------------------
// エフェクトロジック
// --------------------------------------------------

void resetSparkleVars() {
  for (int i = 0; i < NUM_LEDS; i++) {
    sparkleLevel[i] = 0;
    sparkleStep[i] = 0;
  }
}

// モード1: 赤と緑が流れる（キャンディケイン風）
void effectMerryXmas() {
  static int offset = 0;
  // 更新速度調整
  if (millis() - lastUpdate < 200) return;
  lastUpdate = millis();

  for (int i = 0; i < NUM_LEDS; i++) {
    // オフセットを使って動きをつける
    if ((i + offset) % 3 == 0) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // 赤
    } else if ((i + offset) % 3 == 1) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // 緑
    } else {
      pixels.setPixelColor(i, pixels.Color(200, 200, 200)); // 白（少し控えめ）
    }
  }
  pixels.show();
  
  offset++;
  if (offset >= 3) offset = 0;
}

// モード2: 雪のきらめき（ふわっとVer）
void effectSnowSparkle() {
  // 更新速度: アニメーションを滑らかにするため20msごとに更新
  if (millis() - lastUpdate < 20) return; 
  lastUpdate = millis();

  // 1. ランダムに新しいきらめきを発生させる抽選
  // 確率: 1/100 (1%) くらいで発生。
  // ゆっくり見せたい場合はこの確率を下げたり、stepの値を小さくします。
  if (random(100) < 2) { 
    int i = random(NUM_LEDS);
    // 今光っていないLEDならフェードイン開始
    if (sparkleStep[i] == 0) { 
      sparkleStep[i] = 3.0; // 明るくなる速度 (小さいほどゆっくり)
    }
  }

  // 2. 全ピクセルの更新と描画
  for (int i = 0; i < NUM_LEDS; i++) {
    // 背景色（冷たい白・青白）
    int baseR = 10;
    int baseG = 10;
    int baseB = 20;

    // きらめきアニメーションの計算
    if (sparkleStep[i] != 0) {
      sparkleLevel[i] += sparkleStep[i];

      // 輝度がピーク(255)に達したら -> フェードアウトへ反転
      if (sparkleLevel[i] >= 255.0) {
        sparkleLevel[i] = 255.0;
        sparkleStep[i] = -3.0; // 暗くなる速度 (符号をマイナスに)
      }
      // 輝度が0に戻ったら -> 終了
      else if (sparkleLevel[i] <= 0.0) {
        sparkleLevel[i] = 0.0;
        sparkleStep[i] = 0;
      }
    }

    // 背景色 + きらめき成分 を合成
    int r = constrain(baseR + (int)sparkleLevel[i], 0, 255);
    int g = constrain(baseG + (int)sparkleLevel[i], 0, 255);
    int b = constrain(baseB + (int)sparkleLevel[i], 0, 255);

    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

// モード3: キャンドル（暖色のゆらぎ）
void effectCandle() {
  if (millis() - lastUpdate < 100) return;
  lastUpdate = millis();

  for(int i=0; i<NUM_LEDS; i++) {
    // 赤～オレンジ～黄色の範囲でランダムに揺らぐ
    int r = random(200, 255);
    int g = random(50, 120); // 緑成分で黄色みを調整
    int b = 0;
    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

void clearLeds() {
  pixels.clear();
  pixels.show();
}

// --------------------------------------------------
// UI描画
// --------------------------------------------------
void drawScreen() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  
  // 縦画面レイアウト (240x320)
  
  // タイトル
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("Christmas");
  M5.Lcd.setCursor(10, 35);
  M5.Lcd.println("Lights");
  
  // 現在のモード表示
  M5.Lcd.setTextSize(3);
  int yMode = 90;
  M5.Lcd.setCursor(10, yMode);
  
  switch (currentMode) {
    case MODE_MERRY_XMAS:
      M5.Lcd.setTextColor(RED);
      M5.Lcd.println("Merry");
      M5.Lcd.setCursor(10, yMode + 30);
      M5.Lcd.println("Xmas");
      break;
    case MODE_SNOW_SPARKLE:
      M5.Lcd.setTextColor(CYAN);
      M5.Lcd.println("Snow");
      M5.Lcd.setCursor(10, yMode + 30);
      M5.Lcd.println("Sparkle");
      break;
    case MODE_CANDLE:
      M5.Lcd.setTextColor(ORANGE);
      M5.Lcd.println("Candle");
      M5.Lcd.setCursor(10, yMode + 30);
      M5.Lcd.println("Light");
      break;
    case MODE_OFF:
      M5.Lcd.setTextColor(DARKGREY);
      M5.Lcd.println("OFF");
      break;
  }

  // 操作ガイド
  // 画面上の位置（上・中・下）に合わせてテキストを表示
  // 機能割り当ては loop() で変更済み
  
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  
  // 右寄せのためのX座標
  int guideX = 130; 
  
  // 一番上 (BtnC) -> Bright
  M5.Lcd.setCursor(guideX, 200);
  M5.Lcd.print("Bright >");
  
  // 真ん中 (BtnB) -> Dark
  M5.Lcd.setCursor(guideX + 24, 240); // 右揃え調整
  M5.Lcd.print("Dark >");

  // 一番下 (BtnA) -> Mode
  M5.Lcd.setCursor(guideX + 24, 280);
  M5.Lcd.print("Mode >");
  
  // 明るさバー (下端)
  int barWidth = map(currentBrightness, 0, 255, 0, 240);
  M5.Lcd.fillRect(0, 315, barWidth, 5, YELLOW);
}