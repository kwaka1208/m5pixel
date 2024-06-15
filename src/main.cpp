#include <M5Stack.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include "wifi_setting.h"
// #include <HTTPClient.h>

#define PIN 21        //INが接続されているピンを指定
#define NUMPIXELS 60  //LEDの数を指定


void ShowTime(struct tm *timeInfo);
// void ShowURLImage(const char* url);

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);  //800kHzでNeoPixelを駆動
struct tm timeInfo;  //時刻を格納するオブジェクト

void setup() {
  M5.begin();
  M5.Lcd.fillScreen(BLACK);  // 塗りつぶし
  M5.Lcd.setTextColor(WHITE,BLACK); //文字色、背景色
  M5.Lcd.setTextSize(2);  //文字サイズ
  M5.Lcd.setRotation(2);
  M5.Lcd.setCursor(10, 160);  //カーソル位置指定
  M5.Lcd.println("Welcome to MStack");

  pixels.begin();  //NeoPixelを開始

#ifdef SSID
  WiFi.mode(WIFI_AP_STA);

  bool is_connected = false;
  WiFi.begin(SSID, PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
    // M5.update();
    // if (M5.BtnA.isPressed()) break;
  }
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
    is_connected = true;
    M5.Lcd.println("Wi-Fi Connected");
    configTime(9 * 3600L, 0, "ntp.nict.jp", "time.google.com", "ntp.jst.mfeed.ad.jp");  //NTPの設定
    getLocalTime(&timeInfo);  //tmオブジェクトのtimeInfoに現在時刻を入れ込む
  }
  // M5.Lcd.drawJpgFile(SD, "/320x240/s/coderdojo02.jpg", 0, 0);
  if(is_connected) {
    ShowTime(&timeInfo);
  }
#endif
}

void loop() {
  for (int j = 0; j < NUMPIXELS; j++) {
    for (int i = 0; i < NUMPIXELS; i++) {
      int pos = (i + j) % NUMPIXELS;  //LEDの位置を計算
      pixels.setPixelColor(pos, pixels.ColorHSV((65535 / (NUMPIXELS / 3))* i , 255, 50));  //全てのLEDを消灯
      pixels.show();  //LEDに色を反映
      M5.update();
      if (M5.BtnA.isPressed()) {
        M5.Lcd.drawJpgFile(SD, "/240x320/dojo/dojo01.jpg", 0, 0);
      }
      if (M5.BtnB.isPressed()) {
        M5.Lcd.drawJpgFile(SD, "/240x320/dojo/dojo02.jpg", 0, 0);
      }
      if (M5.BtnC.isPressed()) {
        M5.Lcd.drawJpgFile(SD, "/240x320/dojo/dojo03.jpg", 0, 0);
      }
    }
  }
}

void ShowTime(struct tm *timeInfo) {
  String time_str = String(timeInfo->tm_year + 1900) + "/" + String(timeInfo->tm_mon + 1) + "/" + String(timeInfo->tm_mday);
  M5.Lcd.setCursor(60, 180);  //カーソル位置指定
  M5.Lcd.println(time_str);
  }
