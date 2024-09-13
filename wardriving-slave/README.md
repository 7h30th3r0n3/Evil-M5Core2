# ESP32 Wardriving Slave

This code is designed to run on any ESP32 and use it as a slave from wardriving in combination with wardriving master mode on cardputer. 
Each ESP32 collects SSIDs of nearby access points (APs) on a specific channel or can hop between configured channels. 
You can add multiple ESP32 devices to improve the accuracy and strength of the scan. 
Devices with external antennas can enhance the performance for wardriving.
You can monitor all 14 Wi-Fi channel on 2.4GHz without hoping with 14 esp32.
Cardputer use the GPS to link each reiceived information to a CSV compatible with Wigle.

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
2. Each device scans and captures SSID information on designated channels.
3. The data can be aggregated and monitored in master mode, reducing missed APs and improving overall signal strength.

## Hardware Requirements:
- ESP32 devices (e.g., AtomS3, AtomS3 Lite, ESP32-C3, WEMOS D1 Mini)
- External antenna (optional for better performance)
