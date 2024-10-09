#include <esp_now.h>
#include <WiFi.h>

// Keep as all zeroes so we do a broadcast
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

//say how many macs we should keep in the buffer to compare for uniqueness
#define mac_history_len 512

struct mac_addr {
  unsigned char bytes[6];
};
struct mac_addr mac_history[mac_history_len];
unsigned int mac_history_cursor = 0;

// Structure example to send data
// Must match the receiver structure
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
// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 200;  // send readings timer

// Callback when data is sent
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void save_mac(unsigned char* mac) {
  //Save a MAC address into the recently seen array.
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
  //Return true if this MAC address is in the recently seen array.

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
  //Print a mac_addr struct nicely.
  for (int x = 0; x < 6; x++) {
    Serial.print(mac.bytes[x], HEX);
    Serial.print(":");
  }
}

boolean mac_cmp(struct mac_addr addr1, struct mac_addr addr2) {
  //Return true if 2 mac_addr structs are equal.
  for (int y = 0; y < 6; y++) {
    if (addr1.bytes[y] != addr2.bytes[y]) {
      return false;
    }
  }
  return true;
}

String security_int_to_string(int security_type) {
  //Provide a security type int from WiFi.encryptionType(i) to convert it to a String which Wigle CSV expects.
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

    //Requires at least v2.0.0 of https://github.com/espressif/arduino-esp32/
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
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(2, OUTPUT);  //setup built in led
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  //esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
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
  //Serial.println("Starting");
  char Buf[50];
  char bufBSSID[64];
  char BufEnc[50];
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send

    //myData.b = random(1,20);
    //myData.c = 1.2;
    int n = WiFi.scanNetworks(false, true, false, 500, boardID);
    if (n == 0) {
      Serial.println("No networks found");
    } else {
      for (int8_t i = 0; i < n; i++) {
        //delay(10);
        if (seen_mac(WiFi.BSSID(i))) {
          Serial.println("We've already seen it");
          //BSSIDchar = WiFi.BSSID(i);
          //BSSIDchar.toCharArray(bufBSSID, 64);
          //strcpy(myData.bssid, Buf);
          //Serial.println(myData.bssid);
          Serial.println(myData.boardID);
          continue;
        }
        Serial.println("We havent seen it");
        String MacString = WiFi.BSSIDstr(i).c_str();
        //myData.bssid = MacString;
        MacString.toCharArray(bufBSSID, 64);
        strcpy(myData.bssid, bufBSSID);
        Serial.println(myData.bssid);
        //myData.bssid = WiFi.BSSID(i);
        //Serial.print("MyData.bssid: ");

        //Serial.println(myData.bssid);
        String AP = WiFi.SSID(i);
        AP.toCharArray(Buf, 50);
        strcpy(myData.ssid, Buf);
        Serial.print("SSID: ");
        Serial.println(myData.ssid);
        //String ENC = security_int_to_string(WiFi.encryptionType(i));

        //ENC.toCharArray(BufEnc, 32);
        //strcpy(myData.encryptionType, BufEnc);
        //myData.encryptionType = authtype;
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
        myData.boardID = boardID;  //YOU NEED TO CHANGE THE BOARDID TO BE UNIQUE FOR EVERY SUB BVEFORE YOU FLASH IT. DONT DO IT HERE THOUGH
        Serial.println(myData.boardID);
        save_mac(WiFi.BSSID(i));
        esp_now_send(broadcastAddress, (uint8_t*)&myData, sizeof(myData));
        //digitalWrite(2, LOW);
        delay(200);
        //digitalWrite(2, HIGH);
      }



      lastTime = millis();
    }
  }

}
