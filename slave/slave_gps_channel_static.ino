#include <esp_now.h>
#include <WiFi.h>


uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

//say how many macs we should keep in the buffer to compare for uniqueness
#define mac_history_len 512

struct mac_addr {
  unsigned char bytes[6];
};
struct mac_addr mac_history[mac_history_len];
unsigned int mac_history_cursor = 0;

typedef struct struct_message {
  char bssid[64];
  char ssid[32];
  char encryptionType[16];
  int32_t channel;
  int32_t rssi;
  int boardID;
} struct_message;


//**********
//**********
//change the boardID for every unit you flash, this also sets the channel that the board will scan on
int boardID = 11;


String AP;
String BSSIDchar;
String ENC;
String EncTy;

struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 200; 


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
  Serial.print("Mac len ");
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

void print_mac(struct mac_addr mac) {
  for (int x = 0; x < 6; x++) {
    Serial.print(mac.bytes[x], HEX);
    Serial.print(":");
  }
}

boolean mac_cmp(struct mac_addr addr1, struct mac_addr addr2) {
  for (int y = 0; y < 6; y++) {
    if (addr1.bytes[y] != addr2.bytes[y]) {
      return false;
    }
  }
  return true;
}

String security_int_to_string(int security_type) {
  String authtype = "";
  switch (security_type) {
    case WIFI_AUTH_OPEN:
      authtype = "[OPEN]";
      break;

    case WIFI_AUTH_WEP:
      authtype = "[WEP]";
      break;

    case WIFI_AUTH_WPA_PSK:
      authtype = "[WPA_PSK]";
      break;

    case WIFI_AUTH_WPA2_PSK:
      authtype = "[WPA2_PSK]";
      break;

    case WIFI_AUTH_WPA_WPA2_PSK:
      authtype = "[WPA_WPA2_PSK]";
      break;

    case WIFI_AUTH_WPA2_ENTERPRISE:
      authtype = "[WPA2]";
      break;

    case WIFI_AUTH_WPA3_PSK:
      authtype = "[WPA3_PSK]";
      break;

    case WIFI_AUTH_WPA2_WPA3_PSK:
      authtype = "[WPA2_WPA3_PSK]";

    default:
      authtype = "";
  }

  return authtype;
}



void setup() {

  Serial.begin(115200);

  pinMode(2, OUTPUT); 
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

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
  char Buf[50];
  char bufBSSID[64];
  char BufEnc[50];
  if ((millis() - lastTime) > timerDelay) {

    int n = WiFi.scanNetworks(false, true, false, 500, boardID);
    if (n == 0) {
      Serial.println("No networks found");
    } else {
      for (int8_t i = 0; i < n; i++) {
        if (seen_mac(WiFi.BSSID(i))) {
          Serial.println("We've already seen it");
          Serial.println(myData.boardID);
          continue;
        }
        Serial.println("We havent seen it");
        String MacString = WiFi.BSSIDstr(i).c_str();
        
        MacString.toCharArray(bufBSSID, 64);
        strcpy(myData.bssid, bufBSSID);
        Serial.println(myData.bssid);



        String AP = WiFi.SSID(i);
        AP.toCharArray(Buf, 50);
        strcpy(myData.ssid, Buf);
        Serial.print("SSID: ");
        Serial.println(myData.ssid);

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
        EncTy.toCharArray(BufEnc, 16);
        strcpy(myData.encryptionType, BufEnc);
        Serial.print("Encryption: ");
        Serial.println(myData.encryptionType);

        myData.channel = WiFi.channel(i);
        myData.rssi = WiFi.RSSI(i);
        myData.boardID = boardID;  
        Serial.println(myData.boardID);
        save_mac(WiFi.BSSID(i));
        esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
        digitalWrite(2, LOW);
        delay(200);
        digitalWrite(2, HIGH);
      }



      lastTime = millis();
    }
  }

}
