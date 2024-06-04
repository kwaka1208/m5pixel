#include <M5Stack.h>
// #include <M5Unified.h>
#include <Adafruit_NeoPixel.h>
#define PIN 21        //INが接続されているピンを指定
#define NUMPIXELS 30  //LEDの数を指定

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);  //800kHzでNeoPixelを駆動

void setup() {
  M5.begin();
  pixels.begin();  //NeoPixelを開始
  SPIFFS.begin();

  M5.Lcd.fillScreen(BLACK);  //画面を黒で塗りつぶし
  M5.Lcd.setTextColor(NAVY, PINK);  //文字色を白に設定
  M5.Lcd.setTextSize(2);  //文字サイズを2に設定
  M5.Lcd.setCursor(5, 220);  //カーソルを(0, 0)に設定
  M5.Lcd.drawJpgFile(SPIFFS, "/01.jpg", 0, 0);
}

void loop() {
  for (int j = 0; j < NUMPIXELS; j++) {
    for (int i = 0; i < NUMPIXELS; i++) {
      int pos = (i + j) % NUMPIXELS;  //LEDの位置を計算
      pixels.setPixelColor(pos, pixels.ColorHSV((65535 / (NUMPIXELS / 3))* i , 255, 50));  //全てのLEDを消灯
      pixels.show();  //LEDに色を反映
      M5.update();
      if (M5.BtnA.isPressed()) {
        M5.Lcd.drawJpgFile(SPIFFS, "/01.jpg", 0, 0);
      }
      if (M5.BtnB.isPressed()) {
        M5.Lcd.drawJpgFile(SPIFFS, "/02.jpg", 0, 0);
      }
      if (M5.BtnC.isPressed()) {
        M5.Lcd.drawJpgFile(SPIFFS, "/03.jpg", 0, 0);
      }
    }
  }
}

