#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "eepy.h"
#include "quotes.h"
#include "rain.h"
#include "dance1.h"
#include "bean.h"
#include "grinch.h"
#include "images.h"
#include "homieani.h"
#include "border.h"
#include "mask.h"
#include "hellokity.h"

#define BTN 14
#define RST_ENABLE 16

#define I2C_SDA 2 
#define I2C_SCL 0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 
#define OLED_RESET    -1 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BTN_TIMEOUT 500 
bool btn_state_1, btn_flg_1, hld_flg_1;
unsigned btn_tmr_1;
int btn_counter_1;

int frame = 0;
unsigned long last_update = 0;

int current_media = 0;
bool showed_once = false;

#define time_to_sleep 60000
unsigned long last_activity = 0;

void setupOTA(const char* nameprefix, const char* ssid, const char* password) {

  // Configure the hostname
  uint16_t maxlen = strlen(nameprefix) + 7;
  char *fullhostname = new char[maxlen];
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(fullhostname, maxlen, "%s-%02x%02x%02x", nameprefix, mac[3], mac[4], mac[5]);
  ArduinoOTA.setHostname(fullhostname);
  delete[] fullhostname;

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setFont(NULL);
  display.setCursor(0, 0);
  display.print("-OTA Init..");
  display.setCursor(0, 10);
  display.print("-Searching for WiFi..");
  display.display();

  // Configure and start the WiFi station
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  display.setCursor(0, 20);
  display.print("-Connected to WiFi..");
  display.display();
  //Serial.println(ssid);
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232); // Use 8266 port if you are working in Sloeber IDE, it is fixed there and not adjustable

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() 
  {
	//NOTE: make .detach() here for all functions called by Ticker.h library - not to interrupt transfer process in any way.
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    //Serial.println("Start updating " + type);
    display.setCursor(0, 40);
    display.print("-Update Started..");
    display.display();

  });
  
  ArduinoOTA.onEnd([]() {
    //Serial.println("\nEnd");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    //Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    display.fillRect(0, 48, 180, 11, 0);
    display.setCursor(0, 50);
    display.printf("-Progress: %u%%\r", (progress / (total / 100)));
    display.print("..");
    display.display();
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    //Serial.printf("Error[%u]: ", error);
    //if (error == OTA_AUTH_ERROR) Serial.println("\nAuth Failed");
    //else if (error == OTA_BEGIN_ERROR) Serial.println("\nBegin Failed");
    //else if (error == OTA_CONNECT_ERROR) Serial.println("\nConnect Failed");
    //else if (error == OTA_RECEIVE_ERROR) Serial.println("\nReceive Failed");
    //else if (error == OTA_END_ERROR) Serial.println("\nEnd Failed");
  });

  ArduinoOTA.begin();

  display.setCursor(0, 30);
  display.print("-OTA Server Active..");
  display.display();

  while(true)
  {
    ArduinoOTA.handle();
    button_handler(true);
    delay(25);
  }
}

void button_handler(bool hold_disable) {

  btn_state_1 = !digitalRead(BTN);

  if(btn_state_1 && !btn_flg_1) 
  {
    btn_flg_1 = true;
    btn_counter_1++;
    btn_tmr_1 = millis();
  }

  if(btn_flg_1 && btn_state_1 && (millis() - btn_tmr_1 > BTN_TIMEOUT + 1500) && !hld_flg_1 && !hold_disable) //button held 
  {
    hld_flg_1 = true;
    btn_counter_1 = 0;
    display.clearDisplay();
    display.setCursor(10, 7);
    display.println("OTA STARTED!");
    display.display();

    setupOTA("Keychain", "ssid", "password");
  }

  if((millis() - btn_tmr_1 > BTN_TIMEOUT) && (btn_counter_1 != 0) && !btn_state_1) 
  {
    last_activity = millis();

    if (btn_counter_1 == 1) // 1 press count
    {
      frame = 0;
      showed_once = false;
      if(current_media != 10)current_media++;
      else current_media = 0;
    }
    if (btn_counter_1 == 2) // 2 press count
    { 
    }
    if (btn_counter_1 == 3) // 3 press count
    {
      enter_deep_sleep();
    } 
    if (btn_counter_1 == 4) // 4 press count
    {
    } 
    btn_counter_1 = 0;
  }

  if(!btn_state_1 && btn_flg_1) 
  {
    btn_flg_1 = false;
    hld_flg_1 = false;
    if(millis() - btn_tmr_1 <= BTN_TIMEOUT) //button released (single press)
    {
      last_activity = millis();
    }
  }
}

void display_animation(bool no_block, int frame_delay, const unsigned char* array[], int frames) {

  if(no_block)
  {
    if(millis() - last_update > frame_delay)
    {
      display.drawBitmap(0, 0, array[frame], 128, 64, 1);
      if(frame < frames - 1) frame++;
      else frame = 0;

      last_update = millis();
    }
    else
    {
      //print same frame during delay
      display.drawBitmap(0, 0, array[frame], 128, 64, 1);
    }
  }
  else
  {
    for(int i = 0; i < frames - 1; i++)
    {
      display.clearDisplay();
      display.drawBitmap(0, 0, array[i], 128, 64, 1);
      display.display();
      delay(frame_delay);
    }
  }
}

void text_scroll() {

  delay(2000);

  int chosen = random(1, 77);

  String text = quotes[chosen];
  int len = text.length();

  int x = display.width();
  int minX = -12 * len;

  display.setTextWrap(false);
  display.setTextSize(2);

  while(true)
  {
    display.clearDisplay();
    display_animation(true, 50, rainArray, rainArray_LEN);
    display.fillRect(-1, 18, 130, 26, 0);
    display.drawRect(-1, 18, 130, 26, 1);
    display.setCursor(x, 24);
    display.print(text);
    display.display();

    x -= 2; // scroll speed, make more positive to slow down the scroll

    if(x < minX)
    {
      delay(2000);
      break;
    }

    if(digitalRead(BTN) == LOW)
    {
      delay(500);
      break;
    }
  }
  display.setTextSize(1);
  display.clearDisplay();
  display.display();
}

void enter_deep_sleep() {

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setFont(NULL);
  display.setCursor(55, 25);
  display.println("Going");
  display.setCursor(60, 35);
  display.print("to sleep..");
  display.drawBitmap(0, 0, eepy, 128, 64, 1);
  display.display();
  delay(5000);

  display.ssd1306_command(SSD1306_DISPLAYOFF);
  WiFi.mode(WIFI_OFF);
  ESP.deepSleep(0); 
}

void setup() {

  pinMode(BTN, INPUT);
  pinMode(RST_ENABLE, OUTPUT);
  digitalWrite(RST_ENABLE, LOW);

  delay(500);

  Wire.begin(I2C_SDA, I2C_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setTextWrap(false);

  display.clearDisplay();
  display.drawBitmap(0, 0, border, 128, 64, 1);
  display.setCursor(20, 22);
  display.print("Adi's 46th");
  display.setCursor(32, 35);
  display.print("Project HEHEH");
  display.display();

  delay(3000);

  text_scroll();
}

void loop() {

  if(current_media == 0)
  {
    display.clearDisplay();
    display_animation(true, 12, dance1Array, dance1Array_LEN);
    display.display();
  }
  else if(current_media == 1 && !showed_once)
  {
    display.clearDisplay();
    display.drawBitmap(0, 0, myBitmap, 128, 64, 1);
    display.display();
    showed_once = true;
  }
  else if(current_media == 2 && !showed_once)
  {
    display.clearDisplay();
    display.drawBitmap(0, 0, gym, 128, 64, 1);
    display.display();
    showed_once = true;
  }
  else if(current_media == 3 && !showed_once)
  {
    display.clearDisplay();
    display.drawBitmap(0, 0, skibidi, 128, 64, 1);
    display.display();
    showed_once = true;
  }
  else if(current_media == 4 && !showed_once)
  {
    display.clearDisplay();
    display_animation(true, 30, beanallArray, beanallArray_LEN);
    display.display();
  }
  else if(current_media == 5 && !showed_once)
  {
    display.clearDisplay();
    display_animation(true, 65, grinchallArray, grinchallArray_LEN);
    display.display();
  }
  else if(current_media == 6 && !showed_once)
  {
    display.clearDisplay();
    display_animation(true, 42, homelanderallArray, 60);
    display.display();
  }  
  else if(current_media == 7 && !showed_once)//frontdb
  {
    display.clearDisplay();
    display_animation(true, 35, maskallArray, maskallArray_LEN);
    display.display();
  }
  else if(current_media == 8 && !showed_once)
  {
    display.clearDisplay();
    display.drawBitmap(0, 0, cat, 128, 64, 1);
    display.display();
    showed_once = true;
  }
  else if(current_media == 8 && !showed_once)
  {
    display.clearDisplay();
    display.drawBitmap(0, 0, dog, 128, 64, 1);
    display.display();
    showed_once = true;
  }
  else if(current_media == 9 && !showed_once)
  {
    display.clearDisplay();
    display.drawBitmap(0, 0, scooby, 128, 64, 1);
    display.display();
    showed_once = true;
  }
  else if(current_media == 10 && !showed_once)
  {
    display.clearDisplay();
    display.drawBitmap(0, 0, raccoon, 128, 64, 1);
    display.display();
    showed_once = true;
  }

  button_handler(false);

  if(millis() - last_activity > time_to_sleep) enter_deep_sleep();
}