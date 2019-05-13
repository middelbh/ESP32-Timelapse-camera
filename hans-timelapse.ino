#include "esp_camera.h"
#include "esp_timer.h"
#include "Arduino.h"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "FS.h" //sd card esp32
#include "SD_MMC.h" //sd card esp32
#include "soc/soc.h" //disable brownout problems
#include "soc/rtc_cntl_reg.h"  //disable brownout problems 
//#include "BluetoothSerial.h"

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
#define SDA_pin            1
#define SCL_pin            3

int fileNumber = 1 ; 
int IntervalChoice = 0;
int Interval[] = {1, 3, 10, 30, 60, 300};
String Directory;
String fileName;
//String message = "";
//char incomingChar;

SSD1306Wire  Display(0x3c, SDA_pin, SCL_pin);
//BluetoothSerial SerialBT;

void ImageToSd()
{
  if (!SD_MMC.begin()) {
    Display.displayOn();
    printSmall("sdcard gone?",10,10);
  }
  else {
    fileName = "/"+String(fileNumber) + ".jpg";
    fileNumber+=1;
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get(); //take picture from cam
    if (!fb) {
      Display.displayOn();
      printSmall("Fail get",10,10);
      return ;
    }
    fs::FS &fs = SD_MMC;
    File file = fs.open(fileName, FILE_WRITE);
    if (!file) {
      Display.displayOn();
      printSmall("Fail save",10,10);      
    }
    else
    {
      file.write(fb->buf , fb->len);
    }
    esp_camera_fb_return(fb);//free buffer
  }
}

void setup() {
//  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout problems
  ConfigDisplay();
/*  SerialBT.begin("TimeLapse");
  Serial.println("pair bluetooth");
*/
  selectInterval();
  printSmall("Interval set",10,10);
  printSmall("Press key to continue",5,10);
  while(touchRead(0)==1){
//    checkBTInput();
  }
  Display.clear();
  Display.display();
  ConfigCam();
  printSmall("Camera ready",1,10);
  ConfigSD();
  printSmall("SD ready",2,10);
  printSmall("Setting white balance",3,10);  
  for(int i=0;i<5;i+=1){
    ImageToSd(); //take 5 pictures to set white balance...
  }
  fileNumber = 1;
  printSmall("Press key to start",10,10);
  while(touchRead(0)==1){}
  pinMode(XCLK_GPIO_NUM,OUTPUT);
  printSmall("Display to sleep",10,10);
  delay(1000);
  Display.displayOff();
}

void loop() {
  ImageToSd();
  delay(1000*Interval[IntervalChoice]); 
}  

void ConfigCam(){
  pinMode(XCLK_GPIO_NUM,OUTPUT); //otherwise pin 0 which is XCLK_GPIO_NUM for camera cannot be used  
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
    printSmall("Camera init failed",10,10);
    return;
  }
}

void ConfigSD(){
  bool sdcardenable = true ; 
  if (!SD_MMC.begin() and sdcardenable ) {
    printSmall("initCard Mount Failed",10,10);
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    printSmall("No SD_MMC card attached",10,10);
  }
}  

void ConfigDisplay(){
  pinMode(1,FUNCTION_3); 
  pinMode(3,FUNCTION_3); //Serial Tx and Rx pins now GPIO!
  Display.init();
  Display.flipScreenVertically();
  Display.clear();
  Display.display();
}

void printSmall(String text,int line,int xpos){
  if(line==10){
    Display.clear();
    Display.setFont(ArialMT_Plain_16);
    Display.setTextAlignment(TEXT_ALIGN_CENTER);
    Display.drawString(64, 20, text);
    Display.display();      
  }
  else {
    Display.setFont(ArialMT_Plain_10);//Plain_10 Plain_16 Plain_24 possible
    Display.setTextAlignment(TEXT_ALIGN_LEFT);
    Display.drawString(xpos, 10*line, text);
    Display.display();
  }  
}

void selectInterval(){
  Display.clear();
  Display.display();    
  for(int t=0;t<6;t+=1){
    printSmall(String(Interval[t]),t,30);
    printSmall("seconds",t,55);
  }
  printSmall("-->",IntervalChoice,1);
  delay(1000);
  int StartSelect=millis();
  while(1==1){
    while(touchRead(0)==1){ //"steal" pin 0 for input
      if(millis()>=StartSelect+1500){
        Display.clear();
        Display.display();
        return;
      }
    }
    StartSelect=millis();  
    Display.setColor(BLACK);
    printSmall("-->",IntervalChoice,1);
    Display.setColor(WHITE);
    IntervalChoice+=1;
    if(IntervalChoice==6){IntervalChoice=0;}
    printSmall("-->",IntervalChoice,1);
    delay(400); 
  }
}

/*
void checkBTInput(){
  if (SerialBT.available()){
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      message += String(incomingChar);
    }
    else{
      message = "";
    }
  }
// Check received message and control output accordingly
  if (message =="1_start"){
    IntervalChoice=0;
    break;
  }
  if (message =="3_start"){
    IntervalChoice=1;
    break;
  }
  if (message =="5_start"){
    IntervalChoice=2;
    break;
  } 
  if (message =="30_start"){
    IntervalChoice=3;
    break;
  }
  if (message =="60_start"){
    IntervalChoice=4;
    break;
  }           
  if (message =="300_start"){
    IntervalChoice=5;
    break;
  }
}
*/
