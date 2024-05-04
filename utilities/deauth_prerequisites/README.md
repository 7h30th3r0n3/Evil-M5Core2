

All of these scripts and instructions are from nemo project https://github.com/n0xa/m5stick-nemo/tree/main/deauth_prerequisites

# DEAUTHENTIFICATION
In order to make the Deauthentication broadcast work properly, you must follow some steps before compiling


## Why these prerequisites
Every time you build your project, using ESP boards, compiler will look uppon your code to see if you are trying to use their boards to something not allowed..

As sending deauthentication frames (and some others wifi packets) is not something desireable, they have a funcion called: ieee80211_raw_frame_sanity_check, wich should be "silenced" in order to use this attack.

To do that, we must change some flags on the board "platform.txt" file and declare this function in your code, returning 0, you will see this funcion in the deauth.h file


## What you need to do..
* Win+R, and run: %localappdata%\Arduino15\packages\m5stack\hardware\esp32
* open the folder with the version of your board, it will be something like: 2.1.0, or 2.0.9, or whatever
* in this floder, open the file "platform.txt", you will need to edit this badboy.
* Ctrl+F and search for: build.extra_flags.esp32
  - You will find a line that looks like that:  'build.extra_flags.esp32=-DARDUINO_USB_CDC_ON_BOOT=0'
  - Add an "-w " after the =, or at the end of the line: 'build.extra_flags.esp32=-w -DARDUINO_USB_CDC_ON_BOOT=0'
* you should make these changes to the lines:
  - build.extra_flags.esp32
  - build.extra_flags.esp32s2
  - build.extra_flags.esp32s3
  - build.extra_flags.esp32c3
* Ctrl+F again and search for: compiler.c.elf.libs.esp32
  - You will find a line that looks like that: "compiler.c.elf.libs.esp32s3=-lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls....... " this is a huuuuuuge line...
  - Add an "-zmuldefs " after the =, shoul look like this: "compiler.c.elf.libs.esp32s3=-zmuldefs -lesp_ringbuf -lefuse -lesp_ipc -ldriver -lesp_pm -lmbedtls .............."
* you should make these changes to the lines:
  - compiler.c.elf.libs.esp32
  - compiler.c.elf.libs.esp32s2
  - compiler.c.elf.libs.esp32s3
  - compiler.c.elf.libs.esp32c3
* Save the file
* Restart your arduino IDE
* Note: everytime you update your M5 boards to a new version, you will need to do this again..


## What did I n0xa do to help..
n0xa created a batch file to meka all these changes automatically, you just need to download the "install-prerequisites.bat" file and run.

Mac users: You can use install-prerequisites_MAC.sh (thanks to @danny8972)

Linux users: You can use install-prerequisites_Linux.sh (thanks to @danny8972, Just changed folder address might work just fine)

## Thanks
* Thanks to @danny8972 (https://github.com/danny8972) for dev the MAC batch file.

