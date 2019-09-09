# ESP32-Timelapse-camera

Based on the v12345vtm/timelapse-ESP32-CAM sketch I simplified his sketch on the one hand, and expanded on the other.
I decided to delete the web based interface and use an OLED display and a switch for the user interface. Hopefully somebody is able to add WiFi functionality to have the live image available to see where the camera is pointed at.
I also added OTA functionality, but the option does not yet show up in the Arduino IDE...

As the ESP32-Cam does not have unused GPIO's, I use RX and TX pins for both programming and display SDA and SCL by

pinMode(1,FUNCTION_3); 
pinMode(3,FUNCTION_3); 

Next to that I found that GPIO0 can be used with analogRead and touchRead, but not with digitalRead. I use touchRead.
Remark: before accessing the camera after GPIO0 (XCLK_GPIO_NUM) has been used as input, you must redefine this pin for output: 

pinMode(XCLK_GPIO_NUM,OUTPUT);

The use of the button is simple; press intermittendly to select the desired interval. Not pressing the button over 1.5 seconds 
sets the interval.

I also prepared the sketch for use with Bluetooth Classic, but that part does not function yet probably because of the antenna problem....

A 3D printed case is under development and will be available soon!
