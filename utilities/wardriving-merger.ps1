$cheminDossier = '.\'
$ligneSpecifique1 = "WigleWifi-1.4,appRelease=v1.1.9,model=Core2,release=v1.1.9,device=Evil-M5Core2,display=7h30th3r0n3,board=M5Stack Core2,brand=M5Stack`n"
$ligneSpecifique2 = "MAC,SSID,AuthMode,FirstSeen,Channel,RSSI,CurrentLatitude,CurrentLongitude,AltitudeMeters,AccuracyMeters,Type`n"
$cheminFichierFinal = '.\final-wardrive-merged.csv'

$compteurLigne1 = 0
$compteurLigne2 = 0

Get-ChildItem -Path $cheminDossier -Filter *.csv | ForEach-Object {
    $cheminComplet = $_.FullName
    Get-Content $cheminComplet | ForEach-Object {
        $ligne = $_
        if ($ligne -eq $ligneSpecifique1 -and $compteurLigne1 -eq 0) {
            $compteurLigne1++
            Add-Content -Value $ligne -Path $cheminFichierFinal
        } elseif ($ligne -eq $ligneSpecifique2 -and $compteurLigne2 -eq 0) {
            $compteurLigne2++
            Add-Content -Value $ligne -Path $cheminFichierFinal
        } elseif ($ligne -ne $ligneSpecifique1 -and $ligne -ne $ligneSpecifique2) {
            Add-Content -Value $ligne -Path $cheminFichierFinal
        }
    }
}
