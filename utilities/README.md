# deauth_prerequis :

Scripts and instructions from nemo project that transform esp32 firmware to be able to send deauth frames

nemo project https://github.com/n0xa/m5stick-nemo/tree/main/deauth_prerequisites


## Procedure :

Choose your plateform, run the script, your esp32 firmware is now patched.


# Wardriving :

These scripts allow you to merge the different wardriving files into one to facilitate uploading to wigle.

## Procedure :

Place the script with the language you want to use in a folder, 
then place all the wardriving files you want to merge, 
run the script and file final-wardrive-merged.csv is created.

# pcap2hccapx.py :

A script that transform pcap to hccapx to be able to use it with hashcat

## prerequis:
installed :
- tshark
- hcxpcapngtool

## Procedure :

Place the script in a folder, 
then place all the pcap files you want to transform, 
run the script and folder hccapx is created.
