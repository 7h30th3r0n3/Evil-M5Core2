<div align="center">

# Evil-M5Core2 v1.1.3 - Serial Command

<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/SD-Card-File/img/startup.jpg" width="300" />

<b>Evil-M5Core2</b> is an innovative tool developed for ethical testing and exploration of WiFi networks. It harnesses the power of the M5Core2 device to scan, monitor, and interact with WiFi networks in a controlled environment. This project is designed for educational purposes, aiding in understanding network security and vulnerabilities.

> <i>Disclaimer</i>: The creator of Evil-M5Core2 is not responsible for any misuse of this tool. It is intended solely for ethical and educational purposes. Users are reminded to comply with all applicable laws and regulations in their jurisdiction. All files provided with Evil-M5Core2 are designed to be used in a controlled environment and must be used in compliance with all applicable laws and regulations. Misuse or illegal use of this tool is strictly prohibited and not supported by the creator.

</div>

<div align="center">

#### Booting Screen
<div align="center">
  
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/startup.jpg" width="300" />
  
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/startup2.jpg" width="300" />
  
With more than 100 references at each boot.


## !! New Parasite Added !! 

<div align="center">
Your Evil-Core 2 or Flipper Zero feels lonely?
  
**Add a small parasite to it !!!**
</div>

<div align="center">
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/evil-atom.jpg" width="200" />
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/evil-atom2.jpg" width="200" />
</div>

<div align="center">
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/atom-flipper.jpg" width="200" />
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/atom-flipper-probes.jpg" width="200" />
  
He can also be used standalone but he needs a host for energy like a phone or a powersupply to survive or he die. 
</div>
</div>
</div>

Parasite Functionnality : 

- **Cute** (yes it's a useful feature to survive).
- **Accelerometer interaction** (don't shake it or it get mad).
- **AutoKarmaAttack** when face is pressed ( when a karma attack is successfull your little parasiste tell the name of the SSID in a textbubble until the next karma successfull or death).
- **Whitelist** (hardcoded, need to be change by compiling the code again)

(For the moment no portal is sent it just tests if a device connect).

Hardware Requirement :
- M5AtomS3
  
Software Requirement :
This little parasite use m5stack-avatar to render face, donwload avatar librairie before compile.
- https://github.com/meganetaaan/m5stack-avatar

It pretty small so you can also check serial on USB to get information.

## Features Evil-M5core2

- **WiFi Network Scanning**: Identify and display nearby WiFi networks.
- **Network Cloning**: Check information and replicate networks for in-depth analysis.
- **Captive Portal Management**: Create and operate a captive portal to prompt users with a page upon connection.
- **Credential Handling**: Capture and manage portal credentials.
- **Remote Web Server**: Monitor the device remotely via a simple web interface that can provide credentials and upload portal that store file on SD card.
- **Sniffing probes**: Sniff and store on SD near probes.
- **Karma Attack**: Try a simple Karma Attack on a captured probe.
- **Automated Karma Attack**: Try Karma Attack on near probe automatically.

## Hardware Requirements

- M5Stack Core2 (this project is coded with M5Unified, it should work on other M5Stack).
- SD card (fat32 max 32Go, consider 8Go is already more than enough).

Tested working others device :
- M5stack fire
- M5stack core1
- M5stack AWS

It should also work on another esp32 board with SD card but only via serial.
  
## Installation

1. Connect your M5Core2 to your computer.
2. Open the Arduino IDE and load the provided code.
3. Ensure M5unified and adafruit_neopixel libraries are installed.
4. Ensure esp32 and M5stack board are installed (Error occur with esp32 3.0.0-alpha3, please use esp32 v2.0.14 and below).
5. Place SD file content needed on the SD card. ( Needed to get IMG startup and sites folder).
6. Ensure that the baudrates is at 115200.
7. Upload the script to your M5Core2 device.
8. Restart the device if needed.

It's your first time with arduino IDE or something not working correctly? You should check out video section ! 

<div align="center">
  
### Screenshots and Media
  
#### Booting Screen
<div align="center">
  
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/startup.jpg" width="300" />
  
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2/blob/main/Github-Img/startup2.jpg" width="300" />
  
With more than 100 references at each boot.
</div>

#### Menu Screen
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2-/blob/main/Github-Img/menu-1.jpg" width="300" />
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2-/blob/main/Github-Img/menu-2.jpg" width="300" />

#### Probes Sniffing
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2-/blob/main/Github-Img/probes.jpg" width="300" />

#### Low Battery at boot ( when under 15%)
<img src="https://github.com/7h30th3r0n3/Evil-M5Core2-/blob/main/Github-Img/low-battery.jpg" width="300" />


### Video 
Samxplogs tutorial video thx to him :

<a href="https://www.youtube.com/watch?v=ueIAf9Q3EeM">
    <img alt="Samxplogs turorial" src="https://img.youtube.com/vi/ueIAf9Q3EeM/0.jpg" width="33%" height="33%"/>
</a>

You want a demo ? :

<a href="https://www.youtube.com/watch?v=qr01vU4UIJc">
    <img alt="Samxplogs Features" src="https://img.youtube.com/vi/qr01vU4UIJc/0.jpg" width="33%" height="33%"/>
</a>

More demo ? Thx to TalkingSasquatch for making a video about the project : 

<a href="https://www.youtube.com/watch?v=jcVm4cysmnE">
    <img alt="Talking Sasquatch" src="https://img.youtube.com/vi/jcVm4cysmnE/0.jpg" width="33%" height="33%"/>
</a>


</div>
</div>

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
- **/evil-m5core2-menu**: Menu for pages bellow.
- **/credentials**: Lists captured credentials.
- **/uploadhtmlfile**: Provides an upload form to store files on the SD card (for new portal pages and file exfiltration).
- **/check-sd-file**: Provides an index of to check, download and delete files on the SD card.
- **/Change-Portal-Password**: Provides a page to change the password of the deployed access point.


When Captive Portal is ON you can connect to it to acces to 3 fonctionnality protected by password :

- /evil-m5core2-menu
This page is just a menu to provide easy access to others page with authentification form.

- /credentials
This page can list the captured credentials. 

- /uploadhtmlfile
This page provide a upload form that store files in SD card in any folder of the SD to be able to send new portal page, exfiltrate file trough wifi or change the startup image.
please considere to upload file under 1Mo to ensure no lag during the transfert process.

- /check-sd-file
This page provide an index of to check, download and delete files on the SD card.


- /Change-Portal-Password
Provides a page to change the password of the deployed Access Point. Required if attempting a Karma attack on a network with a known password.

  
To prevent unauthorised access of these page they are really simply protected by a password that you need to change in the code. 
To acces to these page use the password form in menu: 

http://192.168.4.1/evil-m5core2-menu

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
- **Current Selected Portal**: Indicates which portal is currently being cloned.
- **Portal Status**: Displays whether the portal is ON or OFF.
- **Provided Portal Page**: Details about the portal page currently in use.

#### Menu 2: Client Information
- **MAC Addresses**: Lists the MAC addresses of all connected clients.

#### Menu 3: Device Status
- **Stack left**: Displays the remaining Stack in the device.
- **Available RAM**: Displays the remaining RAM in the device.
- **Battery Level**: Shows the current battery level.
- **Temperature**: Reports the device's internal temperature.

### Probe Attack 

Send fake random probes near you on all channel. Perfect for counter the Probe Sniffing attack. Press left or right to reduce or increase time delay. (200 ms to 1000ms)

### Probe Sniffing

Probe Sniffing start a probe scan that capture the SSID receive, you can store and reuse then. Restricted to 150 probes max.

### Karma Attack

Same as Probe Sniffing but provide a menu after stopping scan to choose a unique SSID, when SSID is chosen, a portal with the same SSID is deploy, if the original AP is an Open Network and the machine is vulnerable it should connect automaticaly to the network and dependind of the machine can pop up automatically the portal, if a client is present when scan end or stopped, the portal stay open, if not the portal is shutdown. 
(Can be used with password if set on web interface).

### NEW ! from v1.1.2 : Karma Auto 

Same as Karma Attack but try the first probe seen, if no client connects after 15 seconds the Evil-m5core2 returns to sniffing mode to try another captured probe and continues in a cycle until stopped by the user.
Can also be used with a password if set on the web interface, if you have a password and you don't know on which AP it work you could try it with different probe request to test if karma work and get the SSID.
This feature is inspired by the pwnagotchi project but with probe request and karma attack, you can use both to ensure a full attack of the near devices around you.

You can add SSID on KarmaAutoWhitelist line like this : KarmaAutoWhitelist=notmybox,thisonetoo 

Probe should be ignored and serial message send to notify that this network is whitelisted, it also work on probe sniffing and karma attack.

### Select Probe

Menu to select a previous captured probe SSID and deploy it. List is restricted to 150 probes.

### Delete Probe

Menu to delete a previous captured probe SSID and deploy it. List is restricted to 150 probes.

### Delete All Probes

Delete ALL previous captured probes. Basically reset probes.txt on SD.

### Brightness

Change the Brightness of the screen. 


### Change startup image 
Upload a startup.jpg 320x240 image to replace original startup.jpg and make your Evil-M5Core2 more special.

## Flipper Zero Friend ? 
### **Yes, it is !!!**

Evil-M5Core2 sends messages via serial for debugging purposes and message when you navigate on the Core2, you can use the serial app on Flipper to see these messages.
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

### NEW ! from v1.1.3 : Serial Command 

You can control almost all functionnaly with serial command:

Available Commands:
- scan_wifi - Scan for WiFi networks
- select_network <index> - Select a network by index for operations
- detail_ssid <index> - Show details of a specific WiFi network
- clone_ssid - Clone the selected network SSID for captive portal
- start_portal - Start the captive portal
- stop_portal - Stop the captive portal
- list_portal - List available portal files
- change_portal <index> - Change the current portal to another
- check_credentials - Check for saved credentials
- probe_attack - Start a probe request attack
- stop_probe_attack - Stop the probe request attack
- probe_sniffing - Start sniffing for probe requests
- stop_probe_sniffing - Stop sniffing for probe requests
- list_probes - List captured probe requests
- select_probes <index> - Select captured probe request by index
- karma_auto - Start an automatic karma attack that stop automaticaly when successfull


## Discord 

https://discord.gg/G9qXGVHN 

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

