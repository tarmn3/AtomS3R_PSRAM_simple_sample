// src/main.cpp
#include <M5Unified.h>
#include <LittleFS.h>
#include <esp_heap_caps.h>

using namespace m5;

// Image buffers to load into PSRAM
static uint8_t* imgBuf[2];
static size_t   imgSz[2];
static int      currentImg = 0;

void setup() {
  // 1) Initialize M5AtomS3R and USB CDC serial
  M5.begin();
  M5.Lcd.println("Hello");
  Serial.begin(115200);
  //while (!Serial) { delay(10); }
  //Serial.println("Serial OK");

  // 2) LCD test
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.println("LCD OK");
  delay(500);

  // 3) Mount LittleFS (with formatting fallback)
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed, formatting...");
    if (!LittleFS.format()) {
      Serial.println("LittleFS format failed");
      while (true) { M5.update(); delay(100); }
    }
    if (!LittleFS.begin()) {
      Serial.println("LittleFS mount failed again");
      while (true) { M5.update(); delay(100); }
    }
  }
  Serial.println("FS OK");

  // 3a) Check LittleFS total and used space
  size_t total = LittleFS.totalBytes();
  size_t used  = LittleFS.usedBytes();
  Serial.printf("LittleFS total: %u bytes\n", total);
  Serial.printf("LittleFS used:  %u bytes\n", used);
  M5.Lcd.setCursor(10, 30);
  M5.Lcd.printf("LFS total: %u", total);
  M5.Lcd.setCursor(10, 50);
  M5.Lcd.printf("LFS used:  %u", used);
  delay(1000);

  // 4) Load images into PSRAM
  const char* files[2] = {"/1.png", "/2.png"};
  for (int i = 0; i < 2; i++) {
    File f = LittleFS.open(files[i], "r");
    if (!f) {
      Serial.printf("Failed to open %s\n", files[i]);
      while (true) { M5.update(); delay(100); }
    }
    imgSz[i] = f.size();
    imgBuf[i] = (uint8_t*)heap_caps_malloc(imgSz[i], MALLOC_CAP_SPIRAM);
    if (!imgBuf[i]) {
      Serial.printf("PSRAM alloc fail for %s\n", files[i]);
      while (true) { M5.update(); delay(100); }
    }
    f.read(imgBuf[i], imgSz[i]);
    f.close();
    Serial.printf("Loaded %s (%u bytes)\n", files[i], (unsigned)imgSz[i]);
  }

  // 5) Display first image
  M5.Lcd.clear();
  M5.Lcd.drawPng(imgBuf[0], imgSz[0], 0, 0);
  Serial.println("Displayed image 1");
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    currentImg = 1 - currentImg;
    M5.Lcd.clear();
    M5.Lcd.drawPng(imgBuf[currentImg], imgSz[currentImg], 0, 0);
    Serial.printf("Displayed image %d\n", currentImg + 1);
  }
}
