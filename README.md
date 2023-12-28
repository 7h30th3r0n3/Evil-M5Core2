# Evil-M5Core2

**Evil-M5Core2** is an innovative tool developed for ethical testing and exploration of WiFi networks. It harnesses the power of the M5Core2 device to scan, monitor, and interact with WiFi networks in a controlled environment. This project is designed for educational purposes, aiding in understanding network security and vulnerabilities.

> **Disclaimer**: The creator of Evil-M5Core2 is not responsible for any misuse of this tool. It is intended solely for ethical and educational purposes. Users are reminded to comply with all applicable laws and regulations in their jurisdiction.

### Screenshots and Media

(Insert screenshots or media demonstrating your tool in action here.)


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

- **List Details**: About the selected network. You can clone the SSID in this menu.

#### Start Captive Portal

- **Operate Captive Portal**: With `normal.html`, a mock WiFi password page designed to mimic a legitimate login.

#### Special Pages

- **/credentials**: Lists captured credentials (protected by a password).
- **/uploadhtmlfile**: Provides an upload form to store files on the SD card (for new portal pages and file exfiltration).

When Captive Portal is ON you can connect to it to acces to 2 fonctionnality protected by password :
/credentials
This page can list the captured credentials. 

/uploadhtmlfile
This page provide a upload form that store files in SD card in sites folder to be able to send new portal page and exfiltrate file trough wifi.

To prevent unauthorised checking of these page they are simply protected by a password that is modifyable in the code. 
To acces to these page use : 

http://192.168.4.1/uploadhtmlfile?pass=7h30th3r0n3

Any other tried page should redirect to the portal. 

#### Stop Captive Portal

- **Deactivate**: Stops the captive portal and DNS.

#### Change Portal

- **Menu**: Choose the portal provided to connecting users. Lists only HTML files.

#### Check Credentials

- **Menu**: To check captured credentials.

#### Delete Credentials

- **Option**: Delete all captured credentials.

#### Monitor Status

Contain 3 STATIC menu that can be checked with left right button :
- menu 1:
Clients number connected
Number of password in credentials.txt 
curently cloned portal
Is the portal is ON or OFF 
portal page provided 

- menu 2:
Mac address of connected client 

- menu 3 :
CPU // todo
RAM left
Batterie 
Temperature

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

