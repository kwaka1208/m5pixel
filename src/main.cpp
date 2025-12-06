/**
 * M5Stack Basic + NeoPixel Christmas Illumination
 * * 機能 (ユーザー指定のボタン配置: 上からC, B, A):
 * - ボタンC (上): 明るさアップ
 * - ボタンB (中): 明るさダウン
 * - ボタンA (下): モード切り替え
 * * 画面設定: Rotation 2 (縦画面、ガイド右側)
 * * 更新: 明るさバーを縦方向に変更（画面左端）
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
  MODE_MERRY_XMAS = 0,      // 赤と緑
  MODE_MERRY_XMAS_BLUE,     // 赤と緑と青
  MODE_MERRY_XMAS_MULTI,    // カラフル
  MODE_SNOW_SPARKLE,        // 白ベースにキラキラ（ふわっとVer）
  MODE_CANDLE,              // 暖色のゆらぎ
  MODE_OFF,                 // 消灯
  MODE_COUNT                // モード数カウント用
};

int currentMode = MODE_MERRY_XMAS;
int currentBrightness = DEFAULT_BRIGHT;
unsigned long lastUpdate = 0;

// Snow Sparkle用の状態管理配列
float sparkleLevel[NUM_LEDS];
float sparkleStep[NUM_LEDS];

// 関数プロトタイプ宣言
void drawScreen();
void effectMerryXmas();
void effectMerryXmasBlue();
void effectMerryXmasMulti();
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
    case MODE_MERRY_XMAS_BLUE:
      effectMerryXmasBlue();
      break;
    case MODE_MERRY_XMAS_MULTI:
      effectMerryXmasMulti();
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

// モード1: 赤と緑が流れる
void effectMerryXmas() {
  static int offset = 0;
  if (millis() - lastUpdate < 200) return;
  lastUpdate = millis();

  for (int i = 0; i < NUM_LEDS; i++) {
    if ((i + offset) % 3 == 0) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // 赤
    } else if ((i + offset) % 3 == 1) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // 緑
    } else {
      pixels.setPixelColor(i, pixels.Color(200, 200, 200)); // 白
    }
  }
  pixels.show();
  offset++;
  if (offset >= 3) offset = 0;
}

// モード2: 赤と緑と青が流れる
void effectMerryXmasBlue() {
  static int offset = 0;
  if (millis() - lastUpdate < 200) return;
  lastUpdate = millis();

  for (int i = 0; i < NUM_LEDS; i++) {
    // 3色のローテーション (赤 -> 緑 -> 青)
    if ((i + offset) % 3 == 0) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // 赤
    } else if ((i + offset) % 3 == 1) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // 緑
    } else {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // 青
    }
  }
  pixels.show();
  offset++;
  if (offset >= 3) offset = 0;
}

// モード3: マルチカラー (赤,緑,青,黄,紫,橙)
void effectMerryXmasMulti() {
  static int offset = 0;
  if (millis() - lastUpdate < 200) return;
  lastUpdate = millis();

  // 6色のパレット
  uint32_t colors[6] = {
    pixels.Color(255, 0, 0),   // 赤
    pixels.Color(255, 165, 0), // オレンジ
    pixels.Color(255, 255, 0), // 黄
    pixels.Color(0, 255, 0),   // 緑
    pixels.Color(0, 0, 255),   // 青
    pixels.Color(128, 0, 128)  // 紫
  };

  for (int i = 0; i < NUM_LEDS; i++) {
    int colorIndex = (i + offset) % 6;
    pixels.setPixelColor(i, colors[colorIndex]);
  }
  pixels.show();
  offset++;
  if (offset >= 6) offset = 0;
}

// モード4: 雪のきらめき（ふわっとVer）
void effectSnowSparkle() {
  if (millis() - lastUpdate < 20) return; 
  lastUpdate = millis();

  if (random(100) < 2) { 
    int i = random(NUM_LEDS);
    if (sparkleStep[i] == 0) { 
      sparkleStep[i] = 3.0; 
    }
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    int baseR = 10;
    int baseG = 10;
    int baseB = 20;

    if (sparkleStep[i] != 0) {
      sparkleLevel[i] += sparkleStep[i];
      if (sparkleLevel[i] >= 255.0) {
        sparkleLevel[i] = 255.0;
        sparkleStep[i] = -3.0; 
      }
      else if (sparkleLevel[i] <= 0.0) {
        sparkleLevel[i] = 0.0;
        sparkleStep[i] = 0;
      }
    }
    int r = constrain(baseR + (int)sparkleLevel[i], 0, 255);
    int g = constrain(baseG + (int)sparkleLevel[i], 0, 255);
    int b = constrain(baseB + (int)sparkleLevel[i], 0, 255);

    pixels.setPixelColor(i, pixels.Color(r, g, b));
  }
  pixels.show();
}

// モード5: キャンドル（暖色のゆらぎ）
void effectCandle() {
  if (millis() - lastUpdate < 100) return;
  lastUpdate = millis();

  for(int i=0; i<NUM_LEDS; i++) {
    int r = random(200, 255);
    int g = random(50, 120); 
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
  M5.Lcd.setCursor(20, 10); // 左端にバーが入るので少し右へ
  M5.Lcd.println("Christmas");
  M5.Lcd.setCursor(20, 35);
  M5.Lcd.println("Lights");
  
  // 現在のモード表示
  M5.Lcd.setTextSize(3);
  int yMode = 90;
  int xMode = 20; // 左マージン確保
  M5.Lcd.setCursor(xMode, yMode);
  
  switch (currentMode) {
    case MODE_MERRY_XMAS:
      M5.Lcd.setTextColor(RED);
      M5.Lcd.println("Merry");
      M5.Lcd.setCursor(xMode, yMode + 30);
      M5.Lcd.println("Xmas");
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(xMode, yMode + 60);
      M5.Lcd.println("(Red & Green)");
      break;
    case MODE_MERRY_XMAS_BLUE:
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.println("Merry");
      M5.Lcd.setCursor(xMode, yMode + 30);
      M5.Lcd.println("Xmas");
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(xMode, yMode + 60);
      M5.Lcd.println("(+ Blue)");
      break;
    case MODE_MERRY_XMAS_MULTI:
      M5.Lcd.setTextColor(MAGENTA);
      M5.Lcd.println("Merry");
      M5.Lcd.setCursor(xMode, yMode + 30);
      M5.Lcd.println("Multi");
      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(xMode, yMode + 60);
      M5.Lcd.println("(Colorful)");
      break;
    case MODE_SNOW_SPARKLE:
      M5.Lcd.setTextColor(CYAN);
      M5.Lcd.println("Snow");
      M5.Lcd.setCursor(xMode, yMode + 30);
      M5.Lcd.println("Sparkle");
      break;
    case MODE_CANDLE:
      M5.Lcd.setTextColor(ORANGE);
      M5.Lcd.println("Candle");
      M5.Lcd.setCursor(xMode, yMode + 30);
      M5.Lcd.println("Light");
      break;
    case MODE_OFF:
      M5.Lcd.setTextColor(DARKGREY);
      M5.Lcd.println("OFF");
      break;
  }

  // 操作ガイド
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  
  // 右寄せのためのX座標
  int guideX = 130; 
  
  // 一番上 (BtnC) -> Bright
  M5.Lcd.setCursor(guideX, 200);
  M5.Lcd.print("Bright >");
  
  // 真ん中 (BtnB) -> Dark
  M5.Lcd.setCursor(guideX + 24, 240); 
  M5.Lcd.print("Dark >");

  // 一番下 (BtnA) -> Mode
  M5.Lcd.setCursor(guideX + 24, 280);
  M5.Lcd.print("Mode >");
  
  // 明るさバー (縦方向・左端)
  // 幅8px, 高さ最大320px
  int barHeight = map(currentBrightness, 0, 255, 0, 320);
  
  // 枠線（目安）
  M5.Lcd.drawRect(0, 0, 10, 320, DARKGREY);
  
  // 中身（下から上へ）
  // 描画開始Y座標 = 320(下端) - 高さ
  if (barHeight > 0) {
    M5.Lcd.fillRect(1, 320 - barHeight, 8, barHeight, YELLOW);
  }
}