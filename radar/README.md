

Compilation avec Wokwi : 
-	Mettre le diagram.json depuis le site internet dans radar-sketch
-	Mettre le wokwi.toml
[wokwi]
version = 1
firmware = 'build/radar.ino.hex'
elf = 'build/radar.ino.elf' 

[[chip]]
name = 'radar'
binary = 'radar.chip.wasm'
-	Compile avec : 
arduino-cli compile --fqbn arduino:avr:uno --output-dir build /tmp/radar-sketch/radar

Compilation radar :
wokwi-cli chip compile radar.chip.c


cp -r radar-sketch/ ~/gligot
