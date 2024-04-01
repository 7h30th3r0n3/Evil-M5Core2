import os

chemin_dossier = './'

ligne_specifique_1 = "WigleWifi-1.4,appRelease=v1.1.9,model=Core2,release=v1.1.9,device=Evil-M5Core2,display=7h30th3r0n3,board=M5Stack Core2,brand=M5Stack\n"
ligne_specifique_2 = "MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type\n"

chemin_fichier_final = './final-wardrive-merged.csv'

compteur_ligne_1 = 0
compteur_ligne_2 = 0


with open(chemin_fichier_final, 'w') as fichier_final:
    for fichier in os.listdir(chemin_dossier):
        if fichier.endswith('.csv'):
            chemin_complet = os.path.join(chemin_dossier, fichier)
            with open(chemin_complet, 'r') as fichier_source:
                for ligne in fichier_source:
                    if ligne == ligne_specifique_1 and compteur_ligne_1 == 0:
                        compteur_ligne_1 += 1
                        fichier_final.write(ligne)
                    elif ligne == ligne_specifique_2 and compteur_ligne_2 == 0:
                        compteur_ligne_2 += 1
                        fichier_final.write(ligne)
                    elif ligne != ligne_specifique_1 and ligne != ligne_specifique_2:
                        fichier_final.write(ligne)
