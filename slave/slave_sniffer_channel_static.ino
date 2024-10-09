#include <esp_wifi.h>
#include <esp_now.h>
#include <WiFi.h>

#define MAX_FRAME_SIZE 2346  // Maximum size of a Wi-Fi frame
#define ESPNOW_MAX_DATA_LEN 250  // Maximum data size for ESP-NOW
const uint16_t MAX_FRAGMENT_SIZE = ESPNOW_MAX_DATA_LEN - sizeof(uint16_t) - sizeof(uint8_t) - sizeof(bool) - sizeof(uint8_t);

// *** ADAPT THIS VALUE FOR EACH ESP32 ***
#define CHANNEL 8  // Replace with the channel number for this ESP32 (e.g., 1, 2, 3, ..., 14)
#define BOARD_ID CHANNEL 

// Broadcast address for ESP-NOW
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct {
  uint16_t frame_len;
  uint8_t fragment_number;
  bool last_fragment;
  uint8_t boardID;  // Unique ID for each ESP32
  uint8_t frame[MAX_FRAGMENT_SIZE];
} wifi_frame_fragment_t;

wifi_frame_fragment_t wifiFrameFragment;

// Variables to track EAPOL and Beacon frame capture
bool firstEAPOLCaptured = false;
uint8_t apBSSID[6]; // BSSID of the associated AP
bool beaconCaptured = false;
bool allFramesCollected = false;

const int MAX_EAPOL_FRAMES = 4;
uint8_t eapolFrames[MAX_EAPOL_FRAMES][MAX_FRAME_SIZE];
int eapolFrameLengths[MAX_EAPOL_FRAMES];
int eapolFramesCaptured = 0;

uint8_t beaconFrame[MAX_FRAME_SIZE];
int beaconFrameLength = 0;

// Queue for fragments to send
typedef struct {
  uint8_t data[ESPNOW_MAX_DATA_LEN];
  size_t len;
} fragment_queue_item_t;

#define FRAGMENT_QUEUE_SIZE 20  // Maximum size of the queue
fragment_queue_item_t fragmentQueue[FRAGMENT_QUEUE_SIZE];
int fragmentQueueStart = 0;
int fragmentQueueEnd = 0;
bool isSending = false;

// New variable to wait for send completion
bool waitingForSendCompletion = false;

void enqueueFragment(const uint8_t* data, size_t len) {
  int nextEnd = (fragmentQueueEnd + 1) % FRAGMENT_QUEUE_SIZE;
  if (nextEnd == fragmentQueueStart) {
    Serial.println("Fragment queue is full!");
    return;  // The queue is full
  }
  memcpy(fragmentQueue[fragmentQueueEnd].data, data, len);
  fragmentQueue[fragmentQueueEnd].len = len;
  fragmentQueueEnd = nextEnd;
}

void sendNextFragment() {
  if (fragmentQueueStart == fragmentQueueEnd) {
    isSending = false;  // No more fragments to send
    return;
  }
  isSending = true;
  esp_err_t result = esp_now_send(broadcastAddress, fragmentQueue[fragmentQueueStart].data, fragmentQueue[fragmentQueueStart].len);
  if (result != ESP_OK) {
    Serial.printf("Error sending fragment via ESP-NOW: %s (%d)\n", esp_err_to_name(result), result);
    isSending = false;  // Stop sending to avoid overload
  }
}

void resetCaptureState() {
  // Reset state variables to allow for new capture
  firstEAPOLCaptured = false;
  beaconCaptured = false;
  allFramesCollected = false;
  eapolFramesCaptured = 0;
  memset(apBSSID, 0, sizeof(apBSSID));
  // Optionally: Clear previously captured frames
  memset(eapolFrames, 0, sizeof(eapolFrames));
  memset(eapolFrameLengths, 0, sizeof(eapolFrameLengths));
  memset(beaconFrame, 0, sizeof(beaconFrame));
  beaconFrameLength = 0;
}

void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Fragment sent successfully");
    digitalWrite(8, LOW);
    delay(50);  // Attendre 100 ms pour que la LED reste allumée brièvement
    digitalWrite(8, HIGH);  // Éteindre la LED
  } else {
    Serial.printf("Failed to send fragment: %d\n", status);
  }
  // Move to the next fragment
  fragmentQueueStart = (fragmentQueueStart + 1) % FRAGMENT_QUEUE_SIZE;

  if (fragmentQueueStart == fragmentQueueEnd) {
    // No more fragments to send
    isSending = false;

    if (waitingForSendCompletion) {
      // All fragments have been sent, can return to the initial channel
      waitingForSendCompletion = false;
      // Return to the initial channel
      esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
      delay(10); // Wait for the channel change to take effect
      Serial.printf("Returning to channel %d to continue capture.\n", CHANNEL);

      // Reset state to allow for new capture
      resetCaptureState();
    }
    return;
  }

  sendNextFragment();
}

// Function to compare two MAC addresses
boolean mac_cmp(const uint8_t* addr1, const uint8_t* addr2) {
  return memcmp(addr1, addr2, 6) == 0;
}

void checkAndAddPeer() {
  // Always remove the existing peer to update the channel
  if (esp_now_is_peer_exist(broadcastAddress)) {
    esp_err_t delStatus = esp_now_del_peer(broadcastAddress);
    if (delStatus != ESP_OK) {
      Serial.printf("Error removing peer: %s (%d)\n", esp_err_to_name(delStatus), delStatus);
    } else {
      Serial.println("Existing peer removed successfully.");
    }
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  // For broadcast, channel must be 0
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  esp_err_t peer_add_result = esp_now_add_peer(&peerInfo);
  if (peer_add_result != ESP_OK) {
    Serial.printf("Error adding ESP-NOW peer: %s (%d)\n", esp_err_to_name(peer_add_result), peer_add_result);
  } else {
    Serial.println("ESP-NOW peer added successfully.");
  }
}

// Function to send frame fragments via ESP-NOW
void queueFrameFragments(const uint8_t* data, uint16_t len) {
  uint16_t bytes_sent = 0;
  uint8_t fragment_number = 0;

  while (bytes_sent < len) {
    uint16_t bytes_to_send = min(MAX_FRAGMENT_SIZE, (uint16_t)(len - bytes_sent));
    wifiFrameFragment.frame_len = bytes_to_send;
    wifiFrameFragment.fragment_number = fragment_number++;
    wifiFrameFragment.last_fragment = (bytes_sent + bytes_to_send >= len);
    wifiFrameFragment.boardID = BOARD_ID;
    memcpy(wifiFrameFragment.frame, data + bytes_sent, bytes_to_send);

    // Add the fragment to the queue
    enqueueFragment((uint8_t*)&wifiFrameFragment, sizeof(wifi_frame_fragment_t));

    bytes_sent += bytes_to_send;
  }

  // Start sending if not already in progress
  if (!isSending) {
    sendNextFragment();
  }
}

// Function to check if a frame is an EAPOL frame
bool isEAPOL(const wifi_promiscuous_pkt_t* packet) {
  const uint8_t *payload = packet->payload;
  int len = packet->rx_ctrl.sig_len;

  if (len < 38) {  // Minimum size of an EAPOL frame
    return false;
  }

  // Extract the Frame Control field
  uint16_t frameControl = ((uint16_t)payload[1] << 8) | payload[0];
  uint8_t frameType = (frameControl & 0x000C) >> 2;
  uint8_t frameSubType = (frameControl & 0x00F0) >> 4;

  // Check if it's a Data frame (frameType == 2)
  if (frameType != 2) {
    return false;
  }

  // Check for the presence of a QoS field
  bool hasQoS = (frameSubType & 0x08) ? true : false;
  int headerLen = 24;  // Standard Data frame header size
  if (hasQoS) {
    headerLen += 2;  // Add length of QoS field if present
  }

  // Check LLC/SNAP headers for EAPOL: AA-AA-03 and 88-8E
  if (payload[headerLen] == 0xAA && payload[headerLen + 1] == 0xAA &&
      payload[headerLen + 2] == 0x03 && payload[headerLen + 6] == 0x88 &&
      payload[headerLen + 7] == 0x8E) {
    return true;
  }

  return false;
}

// Function to check if a frame is a Beacon frame
bool isBeacon(const wifi_promiscuous_pkt_t* packet) {
  const uint8_t *payload = packet->payload;
  uint16_t len = packet->rx_ctrl.sig_len;

  if (len < 24) {  // Minimum size of a Beacon frame
    return false;
  }

  // Extract the Frame Control field
  uint16_t frameControl = ((uint16_t)payload[1] << 8) | payload[0];
  uint8_t frameType = (frameControl & 0x000C) >> 2;
  uint8_t frameSubType = (frameControl & 0x00F0) >> 4;

  // Check if it's a management frame (frameType == 0) and Beacon subtype (frameSubType == 8)
  if (frameType == 0 && frameSubType == 8) {
    return true;
  }

  return false;
}

// Function to extract the BSSID from a frame
void extractBSSID(const uint8_t* payload, uint16_t frameControl, uint8_t* bssid) {
  uint8_t frameType = (frameControl & 0x000C) >> 2;
  uint8_t toDS = (frameControl & 0x0100) >> 8;
  uint8_t fromDS = (frameControl & 0x0200) >> 9;

  if (frameType == 0) {  // Management frames
    // BSSID is in Address 3 (bytes 16-21)
    memcpy(bssid, &payload[16], 6);
  } else if (frameType == 2) {  // Data frames
    if (toDS == 0 && fromDS == 0) {
      // Address 3 is the BSSID
      memcpy(bssid, &payload[16], 6);
    } else if (toDS == 1 && fromDS == 0) {
      // Address 1 is the BSSID
      memcpy(bssid, &payload[4], 6);
    } else if (toDS == 0 && fromDS == 1) {
      // Address 2 is the BSSID
      memcpy(bssid, &payload[10], 6);
    } else if (toDS == 1 && fromDS == 1) {
      // WDS mode, BSSID not easily determinable
      memset(bssid, 0, 6);
    }
  } else {
    // Other frames, BSSID not applicable
    memset(bssid, 0, 6);
  }
}

// Callback to handle sniffed packets
void wifi_sniffer_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (allFramesCollected) {
    // Do nothing if all frames have been collected
    return;
  }

  wifi_promiscuous_pkt_t *snifferPacket = (wifi_promiscuous_pkt_t*)buf;
  const uint8_t *payload = snifferPacket->payload;
  uint16_t len = snifferPacket->rx_ctrl.sig_len;
  uint16_t frameControl = ((uint16_t)payload[1] << 8) | payload[0];

  uint8_t bssid[6];
  extractBSSID(payload, frameControl, bssid);

  if (isEAPOL(snifferPacket)) {
    if (!firstEAPOLCaptured) {
      // First EAPOL frame captured
      memcpy(apBSSID, bssid, 6);
      firstEAPOLCaptured = true;

      // Store the EAPOL frame
      memcpy(eapolFrames[eapolFramesCaptured], payload, len);
      eapolFrameLengths[eapolFramesCaptured] = len;
      eapolFramesCaptured++;

      Serial.println("First EAPOL captured!");
      Serial.print("Captured BSSID: ");
      for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", apBSSID[i]);
        if (i < 5) Serial.print(":");
      }
      Serial.println();
    } else {
      // Check if the BSSID matches
      if (mac_cmp(apBSSID, bssid)) {
        // Store the EAPOL frame
        if (eapolFramesCaptured < MAX_EAPOL_FRAMES) {
          memcpy(eapolFrames[eapolFramesCaptured], payload, len);
          eapolFrameLengths[eapolFramesCaptured] = len;
          eapolFramesCaptured++;

          Serial.printf("EAPOL #%d captured!\n", eapolFramesCaptured);
        }
      }
    }
  } else if (isBeacon(snifferPacket)) {
    if (firstEAPOLCaptured && mac_cmp(apBSSID, bssid) && !beaconCaptured) {
      // Adjust the length of the Beacon frame
      len -= 4;  // Reduce the length of the signal by 4 bytes

      // Store the Beacon frame with the adjusted length
      memcpy(beaconFrame, payload, len);
      beaconFrameLength = len;
      beaconCaptured = true;

      Serial.println("Beacon frame captured!");
    }
  }

  // Check if all frames have been collected
  if (firstEAPOLCaptured && eapolFramesCaptured >= MAX_EAPOL_FRAMES && beaconCaptured && !allFramesCollected) {
    allFramesCollected = true;
    Serial.println("4 EAPOL frames and one Beacon frame captured from the same AP.");
    // Proceed to send the frames via ESP-NOW on channel 1
    sendCollectedFrames();
  }
}

void sendCollectedFrames() {
  // Switch to channel 1
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  delay(10); // Wait for the channel change to take effect
  Serial.println("Switching to channel 1 for sending via ESP-NOW.");

  // Reset ESP-NOW peer if necessary
  checkAndAddPeer();

  // Indicate that we are waiting for send completion
  waitingForSendCompletion = true;

  // Send the Beacon frame
  if (beaconFrameLength > 0) {
    queueFrameFragments(beaconFrame, beaconFrameLength);
    Serial.println("Beacon frame queued for sending via ESP-NOW.");
  }

  // Send the EAPOL frames
  for (int i = 0; i < eapolFramesCaptured; i++) {
    queueFrameFragments(eapolFrames[i], eapolFrameLengths[i]);
    Serial.printf("EAPOL frame #%d queued for sending via ESP-NOW.\n", i + 1);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(8, OUTPUT);  // Définir la broche en sortie
  WiFi.mode(WIFI_STA);  // Station mode for the ESP32

  // Initialize ESP-NOW
  esp_err_t esp_now_init_result = esp_now_init();
  if (esp_now_init_result != ESP_OK) {
    Serial.printf("ESP-NOW initialization error: %s (%d)\n", esp_err_to_name(esp_now_init_result), esp_now_init_result);
    return;
  } else {
    Serial.println("ESP-NOW initialized successfully");
  }

  // Register the send callback
  esp_now_register_send_cb(onDataSent);
  
  checkAndAddPeer();  // Add the ESP-NOW peer

  // Enable promiscuous mode to capture Wi-Fi frames
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_packet_handler);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);  // Configure the Wi-Fi channel

  Serial.printf("Listening on Wi-Fi channel %d, ready to capture and send EAPOL and Beacon frames...\n", CHANNEL);
}

void loop() {
  // The main loop is empty as everything is handled via callbacks
}
