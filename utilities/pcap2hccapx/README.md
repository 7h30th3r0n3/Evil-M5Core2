
# pcap2hccapx :

A script that extract and transform pcap to hccapx to be able to use it with hashcat.
It's also really usefull to extract each PMKID or 4-wayhandshakes in a single file.

## prerequis:
installed :
- tshark
- hcxpcapngtool

## Procedure :

Place the script in a folder, 
then place all the pcap files you want to transform, 
run the script and folders with all data is created.

note:
You can merge all the hccapx file in one file for parrallele cracking with :

```cat *.hccapx > all.hccapx```
