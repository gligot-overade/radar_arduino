




# Protocol Simulation

## Récupérer le projet :

Permet de réaranger le projet pour qu’il soit compréenssible par arduino (et non Telink)
```
mkdir -p /tmp/radar-sketch/radar/src/radar-targets/arduino
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/affichage.* ~/gligot/radar-sketch/radar/src
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/acquisition_signal.* ~/gligot/radar-sketch/radar/src
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/main.ino ~/gligot/radar-sketch/radar/radar.ino
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-core/config.h ~/gligot/radar-sketch/radar/config.h
cp build/tc321x/Firmware_OXI/oxitech_light_targets/radar-targets/arduino/hardware_config.h ~/gligot/radar-sketch/radar/src/radar-targets/arduino/
```

## Mettre les fichiers necessaire à la Simulation
-	Mettre le diagram.json depuis le site internet https://wokwi.com/projects/480295580349611009 dans radar-sketch
-	Ecrire le wokwi.toml

```
[wokwi]
version = 1
firmware = 'build/radar.ino.hex'
elf = 'build/radar.ino.elf' 

[[chip]]
name = 'radar'
binary = 'radar.chip.wasm'
```

## Compilation 
radar : `wokwi-cli chip compile radar.chip.c`

`chmod 777 /dev/ttyUSB0`
Arduino : `arduino-cli compile --fqbn arduino:avr:uno --output-dir build /tmp/radar-sketch/radar`

cp -r radar-sketch/ ~/gligot
