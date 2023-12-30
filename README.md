<div align="center">

# Evil-M5Core2

<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/startup.jpg" />

<b>Evil-M5Core2</b> is an innovative tool developed for ethical testing and exploration of WiFi networks. It harnesses the power of the M5Core2 device to scan, monitor, and interact with WiFi networks in a controlled environment. This project is designed for educational purposes, aiding in understanding network security and vulnerabilities.

> <i>Disclaimer</i>: The creator of Evil-M5Core2 is not responsible for any misuse of this tool. It is intended solely for ethical and educational purposes. Users are reminded to comply with all applicable laws and regulations in their jurisdiction. All files provided with Evil-M5Core2 are designed to be used in a controlled environment and must be used in compliance with all applicable laws and regulations. Misuse or illegal use of this tool is strictly prohibited and not supported by the creator.

### Screenshots and Media

#### Booting Screen
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2-/blob/main/Github-Img/1703771744391.jpg" width="300" />

With a funny little reference at each boot.

#### Menu Screen
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2-/blob/main/Github-Img/1703771744399.jpg" width="300" />

</div>

## Features

- **WiFi Network Scanning**: Identify and display nearby WiFi networks.
- **Network Cloning**: Check information and replicate networks for in-depth analysis.
- **Captive Portal Management**: Create and operate a captive portal to prompt users with a page upon connection.
- **Credential Handling**: Capture and manage network credentials.
- **Remote Web Server**: Monitor the device remotely via a simple web interface that can provide credentials and upload portal that store file on SD card.

## Hardware Requirements

- M5Core2 device
- SD card

## Installation

1. Connect your M5Core2 to your computer.
2. Open the Arduino IDE and load the provided code.
3. Ensure all required libraries (listed at the beginning of the script) are installed.
4. Upload the script to your M5Core2 device.
5. Place SD file content on the SD card.
6. Restart the device.

## Usage

Follow these steps to efficiently utilize each feature of Evil-M5Core2.

### Menu

#### Scan WiFi

- **Scan Near WiFi**: A fast scan is already made when starting up.

#### Select Network

- **Menu**: Select a network from a list, use left and right keys to navigate and select a network.

#### Clone & Details

- **List Details**: Informations about the selected network. You can clone the SSID in this menu.

#### Start Captive Portal

- **Operate Captive Portal**: With `normal.html` page, a mock WiFi passord page designed to mimic a legitimate error on box.

#### Special Pages

- **/credentials**: Lists captured credentials.
- **/uploadhtmlfile**: Provides an upload form to store files on the SD card (for new portal pages and file exfiltration).
- **/check-sd-file**: Provides an index of to check, download and delete files on the SD card.
  
When Captive Portal is ON you can connect to it to acces to 3 fonctionnality protected by password :
/credentials
This page can list the captured credentials. 

/uploadhtmlfile
This page provide a upload form that store files in SD card in sites folder to be able to send new portal page and exfiltrate file trough wifi.

/check-sd-file
This page provide an index of to check, download and delete files on the SD card.

To prevent unauthorised access of these page they are simply protected by a password that you need to change in the code. 
To acces to these page use : 

http://192.168.4.1/uploadhtmlfile?pass=7h30th3r0n3

Any other tried page should redirect to the choosen portal. 

#### Stop Captive Portal

- **Deactivate**: Stops the captive portal and DNS.

#### Change Portal

- **Menu**: Choose the portal provided to connecting users. Lists only HTML files.

#### Check Credentials

- **Menu**: To check captured credentials.

#### Delete Credentials

- **Option**: Delete all captured credentials.

### Monitor Status

The Monitor Status feature consists of three static menus that can be navigated using the left and right buttons. Each menu provides specific information about the current status of the system:

#### Menu 1: System Overview
- **Number of Connected Clients**: Displays how many clients are currently connected.
- **Credentials Count**: Shows the number of passwords stored in `credentials.txt`.
- **Current Cloned Portal**: Indicates which portal is currently being cloned.
- **Portal Status**: Displays whether the portal is ON or OFF.
- **Provided Portal Page**: Details about the portal page currently in use.

#### Menu 2: Client Information
- **MAC Addresses**: Lists the MAC addresses of all connected clients.

#### Menu 3: Device Status
- **CPU Usage**: (To be implemented)
- **Available RAM**: Displays the remaining RAM in the device.
- **Battery Level**: Shows the current battery level.
- **Temperature**: Reports the device's internal temperature.

## Flipper Zero Friend ? 
### **Yes, it is !!!**

Evil-M5Core2 sends messages via serial for debugging purposes, you can use the serial app on Flipper to see these messages.


Plug your flipper with :

- On flipper : 

PIN 13/TX

PIN 14/RX 


- On M5Core2 :

PIN G3/RXD0

PIN G1/TXD0

<div align="center">
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/flipper-friends.jpg" width="500" />
</div>

# License

MIT License

Copyright (c) 2023 7h30th3r0n3

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

