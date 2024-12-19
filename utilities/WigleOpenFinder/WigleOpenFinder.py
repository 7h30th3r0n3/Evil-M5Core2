import requests


# token API
api_token = ""

def get_city_coordinates(city_name):
    nominatim_url = "https://nominatim.openstreetmap.org/search"
    params = {
        "q": city_name,
        "format": "json",
        "limit": 1
    }
    response = requests.get(nominatim_url, params=params)
    if response.status_code == 200 and response.json():
        first_result = response.json()[0]
        lat = float(first_result['lat'])
        lon = float(first_result['lon'])
        return lat, lon
    else:
        return None, None

def search_open_networks(lat, lon, api_token):
    url = "https://api.wigle.net/api/v2/network/search"
    headers = {
        "Authorization": "Basic " + api_token
    }
    lat_north = lat + 0.5
    lat_south = lat - 0.5	
    lon_east = lon + 0.6
    lon_west = lon - 0.6
    params = {
        "latrange1": lat_south,
        "latrange2": lat_north,
        "longrange1": lon_west,
        "longrange2": lon_east,
        "encryption": "none"
    }
    response = requests.get(url, headers=headers, params=params)
    if response.status_code == 200:
        data = response.json()
        return data
    else:
        print("API Wigle Error:", response.status_code)
        return None

def extract_and_sort_ssids(network_data):
    ssids = set()
    if network_data and 'results' in network_data:
        for network in network_data['results']:
            # Vérifie si le SSID existe avant de tenter de l'utiliser
            ssid = network.get('ssid')
            if ssid:  # Vérifie que le SSID n'est pas None ou vide
                ssids.add(ssid.strip())  # Utilise strip() ici après avoir vérifié que ssid n'est pas None
    return sorted(ssids)

# Demande le nom de la ville à l'utilisateur
city_name = input("Please select a town: ")

# Obtient les coordonnées de la ville
lat, lon = get_city_coordinates(city_name)

if lat is not None and lon is not None:
    # Recherche les réseaux Wi-Fi ouverts
    network_data = search_open_networks(lat, lon, api_token)
    # Extrait et trie les SSIDs
    unique_ssids = extract_and_sort_ssids(network_data)
    if unique_ssids:
        # Affiche les SSIDs uniques, un par ligne
        print("-------------------------------------------------------------------")
        print(f"Open network for {city_name} : ")
        print("-------------------------------------------------------------------")
        for ssid in unique_ssids:
            print(ssid)
        # Sauvegarde les SSIDs dans un fichier
        filename = f"Open-SSID-{city_name}.txt"
        with open(filename, 'w') as file:
            for ssid in unique_ssids:
                file.write(ssid + '\n')
        print("-------------------------------------------------------------------")
        print(f"SSIDs save in {filename}")
    else:
        print(f"No OPEN networks for {city_name} or error.")
else:
    print(f"Can this coordinates for : {city_name}")
