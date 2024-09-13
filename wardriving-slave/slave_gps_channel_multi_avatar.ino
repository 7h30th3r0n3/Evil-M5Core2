#include <esp_now.h>
#include <WiFi.h>
#include <M5Unified.h>
#include <Avatar.h>

// Keep as all zeroes so we do a broadcast
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

// Number of MAC addresses to track
#define mac_history_len 512

struct mac_addr {
  unsigned char bytes[6];
};
struct mac_addr mac_history[mac_history_len];
unsigned int mac_history_cursor = 0;

// Structure example to send data
typedef struct struct_message {
  char bssid[64];
  char ssid[32];
  char encryptionType[16];
  int32_t channel;
  int32_t rssi;
  int boardID;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 200;  // send readings timer

// Define channel hopping list
int channelList[] = { 2, 3, 4, 5, 7, 8, 9, 10, 12, 13}; // List of channels to hop through
int numChannels = sizeof(channelList) / sizeof(channelList[0]);
int currentChannelIndex = 0;  // Track the index in channelList
int scanIndex = 0;            // Current network being processed in the scan result
int networksFound = 0;        // Number of networks found in the current scan

// Callback when data is sent
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void save_mac(unsigned char* mac) {
  if (mac_history_cursor >= mac_history_len) {
    mac_history_cursor = 0;
  }
  struct mac_addr tmp;
  for (int x = 0; x < 6; x++) {
    tmp.bytes[x] = mac[x];
  }

  mac_history[mac_history_cursor] = tmp;
  mac_history_cursor++;
  Serial.print("Mac history length: ");
  Serial.println(mac_history_cursor);
}

boolean seen_mac(unsigned char* mac) {
  struct mac_addr tmp;
  for (int x = 0; x < 6; x++) {
    tmp.bytes[x] = mac[x];
  }

  for (int x = 0; x < mac_history_len; x++) {
    if (mac_cmp(tmp, mac_history[x])) {
      return true;
    }
  }
  return false;
}

boolean mac_cmp(struct mac_addr addr1, struct mac_addr addr2) {
  for (int y = 0; y < 6; y++) {
    if (addr1.bytes[y] != addr2.bytes[y]) {
      return false;
    }
  }
  return true;
}

// Face part
using namespace m5avatar;
Avatar avatar;
unsigned long angryExpressionStartTime = 0;
unsigned long lastExpressionChangeTime = 0;
const unsigned long angryDuration = 3000;  // 3 seconds angry
const unsigned long expressionDuration = 20000;  // 20 seconds for each expression
const float someThreshold = 8;  // Sensitivity for reaction to movement

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
uint8_t display_rotation = 0;  // Orientation for M5AtomS3

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Initialize M5Core2 (Screen)
  M5.begin();
  M5.Display.setRotation(display_rotation);
  avatar.setScale(scale);
  avatar.setPosition(position_top, position_left);
  avatar.init();

  // Setup built-in LED and Wi-Fi
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback when sending data
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  M5.update();
  avatar.setMouthOpenRatio(0);
  // Manage avatar expressions based on IMU data
  if (M5.Imu.update()) {
    auto data = M5.Imu.getImuData();
    avatar.setRotation(data.accel.x * 35);

    if ((abs(data.accel.x) + abs(data.accel.y) + abs(data.accel.z)) > someThreshold) {
      avatar.setMouthOpenRatio(0.8);
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
  }

  // Wi-Fi scanning and ESP-NOW sending part (non-blocking)
  if ((millis() - lastTime) > timerDelay) {
    int currentChannel = channelList[currentChannelIndex]; // Get current channel from the list

    // If no scan is in progress, initiate a new scan
    if (scanIndex == 0) {
      networksFound = WiFi.scanNetworks(false, true, false, 500, currentChannel);
      Serial.print("Scanning channel ");
      Serial.println(currentChannel);
    }

    // If there are networks found, process each one in a non-blocking manner
    if (scanIndex < networksFound) {
      if (!seen_mac(WiFi.BSSID(scanIndex))) {
        String MacString = WiFi.BSSIDstr(scanIndex);
        MacString.toCharArray(myData.bssid, 64);
        
        String AP = WiFi.SSID(scanIndex);
        AP.toCharArray(myData.ssid, 32);

        // Set encryption type
        String EncTy;
        switch (WiFi.encryptionType(scanIndex)) {
          case WIFI_AUTH_OPEN:
            EncTy = "Open";
            break;
          case WIFI_AUTH_WEP:
            EncTy = "WEP";
            break;
          case WIFI_AUTH_WPA_PSK:
            EncTy = "WPA PSK";
            break;
          case WIFI_AUTH_WPA2_PSK:
            EncTy = "WPA2 PSK";
            break;
          case WIFI_AUTH_WPA_WPA2_PSK:
            EncTy = "WPA/WPA2 PSK";
            break;
          case WIFI_AUTH_WPA2_ENTERPRISE:
            EncTy = "WPA2 Enterprise";
            break;
          default:
            EncTy = "Unknown";
            break;
        }
        EncTy.toCharArray(myData.encryptionType, 16);

        myData.channel = currentChannel;  // Store current channel
        myData.rssi = WiFi.RSSI(scanIndex);
        myData.boardID = currentChannel;  // Set boardID equal to channel number for each iteration

        Serial.print("Sending data for network ");
        Serial.println(scanIndex);
        save_mac(WiFi.BSSID(scanIndex));
        esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
        avatar.setMouthOpenRatio(0.9);
        delay(30);
      }
      scanIndex++;  // Process the next network in the next loop iteration
    } else {
      // If no more networks are left to process, move to the next channel
      scanIndex = 0;
      currentChannelIndex++;
      if (currentChannelIndex >= numChannels) currentChannelIndex = 0;  // Cycle through channels in channelList
    }

    lastTime = millis();  // Reset the timer
  }
}
