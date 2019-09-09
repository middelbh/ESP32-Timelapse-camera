//based on sketch from v12345vtm

#include "esp_camera.h"
#include "esp_timer.h"
#include <WiFi.h>
#include "Arduino.h"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "FS.h" //sd card esp32
#include "SD_MMC.h" //sd card esp32
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems 
#include <ArduinoOTA.h>
const char* ssid = "XXXXXXXXXXXXXXXX";
const char* password = "YYYYYYYYYYYYYYYY";
const char* answer[] = {"YES", "NO"};
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define SDA_pin            3
#define SCL_pin            1

int fileNumber = 1 ;
int IntervalChoice = 0;
int Interval[] = {1, 3, 10, 30, 60, 300};
int progress;
int maxtry = 0;
boolean NoSD = true;
String fileName;
boolean retry;
SSD1306Wire  Display(0x3c, SDA_pin, SCL_pin);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout problems
  ConfigDisplay();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED && maxtry <= 20) {
    delay(250);
    printSmall("              ", 10, 10);
    delay(250);
    printSmall("Connect to WiFi", 10, 10);
    maxtry += 1;
  }
  if (maxtry <= 20) {
    printSmall(WiFi.localIP().toString(), 10, 10);
  }
  else {
    printSmall("No WiFi", 10, 10);
    delay(1500);
    askretry();
    if (retry) {
      ESP.restart();
    }
  }

  delay(1500);
  selectInterval();
  printSmall("Interval set", 10, 10);
  printSmall("Press key to continue", 5, 10);
  Display.clear();
  Display.display();
  ConfigCam();
  printSmall("Camera ready", 1, 10);
  ConfigSD();
  if (NoSD == false) {
    printSmall("SD ready", 2, 10);
    printSmall("Setting white balance", 3, 10);
    for (int i = 0; i < 5; i += 1) {
      ImageToSd(); //take 5 pictures to set white balance...
    }
    fileNumber = 1;
    printSmall("Press key to start", 10, 10);
    while (touchRead(0) == 1) {}
    pinMode(XCLK_GPIO_NUM, OUTPUT);
    printSmall("Display to sleep", 10, 10);
    delay(1000);
    Display.displayOff();
    esp_sleep_enable_timer_wakeup(Interval[IntervalChoice] * 1000 * 1000);
  }
  else {
    while (1 == 1) {}
  }
  // nu het OTA stuk....
  ArduinoOTA.onStart([]() {
  });
  ArduinoOTA.onEnd([]() {
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  });
  ArduinoOTA.onError([](ota_error_t error) {
/*
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
*/
  });
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  ImageToSd();
  esp_light_sleep_start();
}

void ConfigCam() {
  pinMode(XCLK_GPIO_NUM, OUTPUT); //otherwise pin 0 which is XCLK_GPIO_NUM for camera cannot be used
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    printSmall("Camera init failed", 10, 10);
    return;
  }
}

void ConfigSD() {
  bool sdcardenable = true ;
  if (!SD_MMC.begin() and sdcardenable ) {
    printSmall("SD Mount Fail", 10, 10);
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    printSmall("No SD card", 10, 10);
    NoSD = true;
  }
  else {

    NoSD = false;
  }
}

void ConfigDisplay() {
  pinMode(1, FUNCTION_3);
  pinMode(3, FUNCTION_3); //Serial Tx and Rx pins now GPIO!
  Display.init();
  Display.flipScreenVertically();
  Display.clear();
  Display.display();
}

void printSmall(String text, int line, int xpos) {
  if (line == 10) {
    Display.clear();
    Display.setFont(ArialMT_Plain_16);
    Display.setTextAlignment(TEXT_ALIGN_CENTER);
    Display.drawString(64, 20, text);
    Display.display();
  }
  else {
    Display.setFont(ArialMT_Plain_10);//Plain_10 Plain_16 Plain_24 possible
    Display.setTextAlignment(TEXT_ALIGN_LEFT);
    Display.drawString(xpos, 8 * line, text);
    Display.display();
  }
}

void selectInterval() {
  Display.clear();
  Display.display();
  for (int t = 0; t < 6; t += 1) {
    printSmall(String(Interval[t]), t, 30);
    if (t == 0) {
      printSmall("second", t, 55);
    }
    else {
      printSmall("seconds", t, 55);
    }
  }
  printSmall("-->", IntervalChoice, 1);
  int StartSelect = millis();
  while (1 == 1) {
    while (touchRead(0) == 1) { //"steal" pin 0 for input; only analogRead and touchRead work on pin 0
      progress = StartSelect + 3000 - millis();
      progress = map(progress, 3000, 0, 0, 100);
      Display.drawProgressBar(0, 54, 127, 9, progress);
      Display.display();
      if (millis() >= StartSelect + 3000) {
        Display.clear();
        Display.display();
        return;
      }
    }
    Display.setColor(BLACK);
    Display.fillRect(0, 54, 128, 64);
    printSmall("-->", IntervalChoice, 1);
    Display.setColor(WHITE);
    IntervalChoice += 1;
    if (IntervalChoice == 6) {
      IntervalChoice = 0;
    }
    printSmall("-->", IntervalChoice, 1);
    delay(500);
    StartSelect = millis();
  }
}

void ImageToSd()
{
  if (!SD_MMC.begin()) {
    Display.displayOn();
    printSmall("sdcard gone?", 10, 10);
  }
  else {
    fileName = "/" + String(fileNumber) + ".jpg";
    fileNumber += 1;
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get(); //take picture from cam
    if (!fb) {
      Display.displayOn();
      printSmall("Fail get picture", 10, 10);
      return ;
    }
    fs::FS &fs = SD_MMC;
    File file = fs.open(fileName, FILE_WRITE);
    if (!file) {
      Display.displayOn();
      printSmall("Fail save picture", 10, 10);
    }
    else
    {
      file.write(fb->buf , fb->len);
    }
    esp_camera_fb_return(fb);//free buffer
  }
}

void askretry()
{
  int choice;
  Display.clear();
  printSmall("Retry?", 1, 20);
  Display.display();
  for (choice = 0; choice < 2; choice += 1) {
    printSmall(answer[choice], choice + 3, 20);
  }
  printSmall("-->", 3, 1);
  int StartSelect = millis();
  while (1 == 1) {
    while (touchRead(0) == 1) { //"steal" pin 0 for input; only analogRead and touchRead work on pin 0
      
      progress = StartSelect + 3000 - millis();
      progress = map(progress, 3000, 0, 0, 100);
      Display.drawProgressBar(0, 54, 127, 9, progress);
      Display.display();
      if (millis() >= StartSelect + 3000) {
        Display.clear();
        Display.display();
        if (choice == 0) {
          retry = true;
        }
        else {
          retry = false;
        }
        return;
      }
    }
    Display.setColor(BLACK);
    Display.fillRect(0, 54, 128, 64);
    printSmall("-->", choice + 3, 1);
    Display.setColor(WHITE);
    choice += 1;
    if (choice >= 2) {
      choice = 0;
    }
    printSmall("-->", choice + 3, 1);
    delay(500);
    StartSelect = millis();
  }
}
