#include <esp_now.h>
#include <WiFi.h>

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

// List of channels for hopping (user can modify this)
int channelList[] = {2, 3, 4, 5, 7, 8, 9, 10, 12, 13}; // Example: Only hop through these channels // perfect for a 4 atom board with 3 on 1,6,11 and that one that check other channel
int numChannels = sizeof(channelList) / sizeof(channelList[0]);

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

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(2, OUTPUT);  // Setup built-in LED
  WiFi.mode(WIFI_STA); // Set device as a Wi-Fi Station

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
  if ((millis() - lastTime) > timerDelay) {
    for (int i = 0; i < numChannels; i++) { // Loop through the selected channels
      int channel = channelList[i];          // Use user-defined channels
      int n = WiFi.scanNetworks(false, true, false, 500, channel); // Scan each channel
      if (n == 0) {
        Serial.print("No networks found on channel ");
        Serial.println(channel);
      } else {
        for (int8_t i = 0; i < n; i++) {
          if (seen_mac(WiFi.BSSID(i))) {
            Serial.println("Already seen this MAC");
            continue;
          }

          String MacString = WiFi.BSSIDstr(i);
          MacString.toCharArray(myData.bssid, 64);
          
          String AP = WiFi.SSID(i);
          AP.toCharArray(myData.ssid, 32);

          // Set encryption type
          String EncTy;
          switch (WiFi.encryptionType(i)) {
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

          myData.channel = channel;  // Store current channel
          myData.rssi = WiFi.RSSI(i);
          myData.boardID = channel;  // Set boardID equal to channel number for each iteration
          
          Serial.print("Sending data for channel ");
          Serial.println(channel);
          
          save_mac(WiFi.BSSID(i));
          esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
          delay(200);  // Short delay between each transmission
        }
      }
    }
    lastTime = millis();
  }
}
