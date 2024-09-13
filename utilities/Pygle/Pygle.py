import pandas as pd
import folium

# Définir les noms des colonnes
colonnes = ['MAC', 'SSID', 'AuthMode', 'FirstSeen', 'Channel', 'RSSI',
            'CurrentLatitude', 'CurrentLongitude', 'AltitudeMeters', 'AccuracyMeters', 'Type']

# Lire le fichier CSV en ignorant les lignes non pertinentes
df = pd.read_csv('data.csv', skiprows=3, names=colonnes)

# Supprimer les lignes avec des valeurs NaN dans RSSI
df = df.dropna(subset=['RSSI'])

# Créer une carte centrée sur le centre des points
avg_lat = df['CurrentLatitude'].mean()
avg_lon = df['CurrentLongitude'].mean()
ma_carte = folium.Map(location=[avg_lat, avg_lon], zoom_start=15)

# Fonction pour déterminer la couleur du marqueur en fonction du RSSI
def get_color(rssi):
    if rssi >= -70:
        return 'green'
    elif rssi >= -85:
        return 'orange'
    else:
        return 'red'

# Ajouter les points sur la carte avec des pop-ups contenant des informations supplémentaires
for index, row in df.iterrows():
    latitude = row['CurrentLatitude']
    longitude = row['CurrentLongitude']
    ssid = row['SSID']
    mac = row['MAC']
    auth_mode = row['AuthMode']
    rssi = row['RSSI']
    first_seen = row['FirstSeen']
    altitude = row['AltitudeMeters']
    accuracy = row['AccuracyMeters']
    wifi_type = row['Type']

    # Contenu du pop-up
    popup_content = f"""
    <b>SSID:</b> {ssid}<br>
    <b>MAC:</b> {mac}<br>
    <b>Auth Mode:</b> {auth_mode}<br>
    <b>RSSI:</b> {rssi} dBm<br>
    <b>First Seen:</b> {first_seen}<br>
    <b>Altitude:</b> {altitude} m<br>
    <b>Accuracy:</b> {accuracy} m<br>
    <b>Type:</b> {wifi_type}
    """

    # Déterminer la couleur du marqueur
    color = get_color(int(rssi))

    # Ajouter le marqueur à la carte
    folium.CircleMarker(
        location=[float(latitude), float(longitude)],
        radius=5,
        popup=folium.Popup(popup_content, max_width=300),
        color=color,
        fill=True,
        fill_color=color
    ).add_to(ma_carte)

# Enregistrer la carte dans un fichier HTML
ma_carte.save('wifi_map.html')
