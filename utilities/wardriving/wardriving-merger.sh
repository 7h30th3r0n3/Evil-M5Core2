#!/bin/bash

chemin_dossier='./'
ligne_specifique_1="WigleWifi-1.4,appRelease=v1.1.9,model=Core2,release=v1.1.9,device=Evil-M5Core2,display=7h30th3r0n3,board=M5Stack Core2,brand=M5Stack"
ligne_specifique_2="MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type"
chemin_fichier_final='./final-wardrive-merged.csv'

compteur_ligne_1=0
compteur_ligne_2=0

> "$chemin_fichier_final"

for fichier in "$chemin_dossier"*.csv; do
  while IFS= read -r ligne; do
    if [ "$ligne" = "$ligne_specifique_1" ]; then
      if [ $compteur_ligne_1 -eq 0 ]; then
        echo "$ligne" >> "$chemin_fichier_final"
        compteur_ligne_1=$((compteur_ligne_1+1))
      fi
    elif [ "$ligne" = "$ligne_specifique_2" ]; then
      if [ $compteur_ligne_2 -eq 0 ]; then
        echo "$ligne" >> "$chemin_fichier_final"
        compteur_ligne_2=$((compteur_ligne_2+1))
      fi
    else
      echo "$ligne" >> "$chemin_fichier_final"
    fi
  done < "$fichier"
done
