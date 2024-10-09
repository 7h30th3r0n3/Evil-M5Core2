# ESP32 Wardriving/Sniff Slave

This code is designed to run on any ESP32 and use it as a slave for wardriving and EAPOL sniffing on static channel in combination with wardriving master mode on cardputer. 
- Wardriving : Each ESP32 collects SSIDs of nearby access points (APs) on a specific channel or can hop between configured channels. 
- Sniffer : Each ESP32 collects EPAOL and beacon probes around on a specific channel.

You can add multiple ESP32 devices to improve the accuracy and strength of the scan/sniff. 
Devices with external antennas can enhance the performance for wardriving.
You can monitor all 14 Wi-Fi channel on 2.4GHz without hoping with 14 esp32.
Cardputer use the GPS to link each reiceived information to a CSV compatible with Wigle in wardriving.
Or create a pcap file with all EAPOL captured inside.

### `slave_deauth_hopping.ino`
This script handles a slave device that performs deauthentication (deauth) attacks on different WiFi channels dynamically, constantly changing the channel (hopping).

### `slave_deauth_hopping_atoms3.ino`
A variant of the `slave_deauth_hopping.ino` script, optimized for the AtomS3 microcontroller. This script use the screen of the AtomS3.

### `slave_deauth_static.ino`
This script performs deauthentication attacks on a static WiFi channel. The slave stays on a single channel to execute its attacks.

### `slave_gps_channel_static.ino`
This script enables the slave to collect GPS data while remaining on a static WiFi channel. It can be used for scenarios where geolocating access points on a specific channel is required.

### `slave_gps_hopping.ino`
This script combines WiFi channel hopping with GPS data collection. The device dynamically switches channels while sending detected APs.

### `slave_gps_hopping_atoms3.ino`
An AtomS3-adapted version of the `slave_gps_hopping.ino` script. Same but with avatar face.

### `slave_sniffer_channel_static.ino`
EAPOL sniffer that remains on a static WiFi channel. It captures EAPOL packets on this channel and send it to cardputer for centralization of captured data.



## Tested on:
- **AtomS3**: [Buy here](https://s.click.aliexpress.com/e/_DnDXSKJ)
- **AtomS3 Lite**: [Buy here](https://s.click.aliexpress.com/e/_Dm0e95D)
- **ESP32-C3** (with external antenna): [Buy here](https://s.click.aliexpress.com/e/_DD1yibp)
- **WEMOS D1 Mini**: [Buy here](https://s.click.aliexpress.com/e/_DEWPrnz)

## Features:
- **Multi-Device Support**: Add any number of ESP32 devices to increase AP detection.
- **Channel Hopping**: Configure the ESP32 to scan on a specific channel or hop between selected channels.
- **Better Signal Strength**: ESP32 devices with external antennas improve the signal capture, making it ideal for long-range wardriving.
- **Sending to master**: Use in combination with the cardputer in wardriving master mode to collect and aggregate data from multiple ESP32 slaves.

## How it Works:
1. Deploy one or more ESP32 devices in slave mode.
2. Each device scans and captures SSID information or EPAOL on designated channels.
3. The data can be aggregated and monitored in master mode, reducing missed APs and improving overall signal strength.

## Hardware Requirements:
- Cardputer in lastest version.
- ESP32 devices (e.g., AtomS3, AtomS3 Lite, ESP32-C3, WEMOS D1 Mini)
- External antenna (optional for better performance)

# ESP32 Deauth Slave

This code can be used with Handshake master and Passif Handshake/Deauth Sniffing,
It check each AP on desired channel or with hopping depending on your choose an send a deauth packet to it. 
