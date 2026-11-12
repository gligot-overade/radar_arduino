# Projet radar : Arduino et simulation Wokwi

Ce dossier contient le programme Arduino du radar ainsi que les fichiers
nécessaires à sa simulation avec Wokwi. Le projet Arduino utilise une carte
Arduino Uno et un écran OLED I2C 128 × 64 basé sur un contrôleur SSD1306.

## Préparer les fichiers Arduino

Les sources utilisées par le projet Arduino se trouvent dans le dépôt
`OXITECH_TC321X`. Depuis sa racine, copiez-les dans l'arborescence de
`radar-sketch` :

```sh
cd /home/gligot/OXITECH_TC321X

mkdir -p /home/gligot/gligot/radar-sketch/radar/src/radar-targets/arduino

cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/affichage.* \
  /home/gligot/gligot/radar-sketch/radar/src/
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/acquisition_signal.* \
  /home/gligot/gligot/radar-sketch/radar/src/
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/main.ino \
  /home/gligot/gligot/radar-sketch/radar/radar.ino
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/config.h \
  /home/gligot/gligot/radar-sketch/radar/config.h
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-targets/arduino/hardware_config.h \
  /home/gligot/gligot/radar-sketch/radar/src/radar-targets/arduino/
```

## Installer les outils et bibliothèques Arduino

Installez Arduino CLI et le cœur Arduino AVR :

```sh
sudo apt install arduino-cli
arduino-cli core update-index
arduino-cli core install arduino:avr
```

Installez les bibliothèques utilisées par l'écran :

```sh
arduino-cli lib install "Adafruit SSD1306" "Adafruit GFX Library"
```

L'adresse I2C de l'écran est `0x3C` et est définie dans
`src/radar-targets/arduino/hardware_config.h`.

## Compiler et téléverser sur la carte

Depuis le dossier `radar` :

```sh
cd /home/gligot/gligot/radar-sketch/radar
arduino-cli compile --fqbn arduino:avr:uno --output-dir build .
```

Pour téléverser le programme, remplacez `/dev/ttyUSB0` par le port série
correspondant à votre carte si nécessaire :

```sh
arduino-cli upload -p /dev/ttyUSB0 --fqbn arduino:avr:uno .
```

Si l'accès au port série est refusé sous Linux, ajoutez votre utilisateur au
groupe `dialout`, puis reconnectez-vous :

```sh
sudo usermod -aG dialout "$USER"
```

## Simuler avec Wokwi

Placez le fichier `diagram.json` du projet Wokwi dans ce dossier. Le projet
utilise la configuration suivante dans `wokwi.toml` :

```toml
[wokwi]
version = 1
firmware = 'build/radar.ino.hex'
elf = 'build/radar.ino.elf'

[[chip]]
name = 'radar'
binary = 'radar.chip.wasm'
```

Compilez le composant radar depuis ce dossier :

```sh
wokwi-cli chip compile radar.chip.c
```

La simulation Wokwi nécessite le fichier `diagram.json`, le composant compilé
`radar.chip.wasm` et le firmware Arduino compilé dans `build/`.

## Réception des trames radar

Le protocole ne fournit ni longueur totale ni marqueur de fin. Les enregistrements
valides font 6 octets ; une trame est considérée terminée après
`RADAR_FRAME_END_TIMEOUT_MS` sans nouvel octet (5 ms par défaut).

La fonction `read_radar(millis())` doit être appelée régulièrement. Elle
surveille les octets reçus dans le buffer `Serial`, attend que leur quantité
reste stable pendant 5 ms, puis lit et valide la trame complète. Le buffer de
trame peut contenir jusqu'à 8 enregistrements, soit 48 octets. Les valeurs
validées sont écrites dans `targets_info`.

## Affichage sur l'écran OLED

La boucle du programme appelle l'acquisition radar et le rafraîchissement de
l'écran à des périodes indépendantes, configurées dans `config.h`. Le transfert
de l'image vers l'écran utilise I2C et reste bloquant pendant son exécution.
