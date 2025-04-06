# R909-VFO-ESP
Install the ESP32-C3 mini on the R909-VFO PCB
R909受信機の制御部であるArduinoやSW類、OLED表示を搭載したR909-VFO基板をESP32-C3 Super mini dev kit対応にしました。回路図、部品表、基板ガーバーファイル、組立時チェック用スケッチ、簡単なVFOスケッチなどをまとめてアップロードしました。

ESP32-C3 Super mini dev kitにはオリジナルの14P、コピー品の16P各種信号配置基板など色々と種類があります。ここで採用したのはK2と言うらしい、GPIO0-10,20,21と電源端子が配置された基板です。

基板パターンはKiCADで設計しましたので、ESP32-C3 Super mini dev kitの回路シンボルとフットプリントを作りました。（R1とR2の実装位置が間違っています。シルクR1に330RをシルクR2に2ｋを実装してください。）

この基板はアルミ引き抜き材のケースに収容できるように設計しています。ケースのフロントパネルとバックパネルもプリント基板で用意しています。

https://github.com/Nobcha/R909-VFO-ESP/blob/main/

R909-VFO-ESP回路図                   R909-VFO_ESP32_SCM.pdf

R909-VFO-ESP基板部品表　　    5531_esp_25_bom.pdf

R909-VFO-ESP基板ガーバーファイル　　5531_esp_25.kicad_pcb.zip

組立時チェック用スケッチ            R909_VFO_esp_ol_SWRE_i2c_TEST.ino

簡単なVFOスケッチ（JCR VFO移植)   JCR_R909-VFO-ESP_kpa.ino

R909-VFO-ESPスケッチ（10ｋHz-220MHｚ)　R909_VFO_ESP_UNIV0406.ino

ESP32-C3 Super mini dev kitの回路シンボル      ESP32-C3.kicad_sym

ESP32-C3 Super mini dev kitのフットプリント  ECP32C3-PinHeader-2x8.kicad_mod

ケースのフロントパネルとバックパネルプリント基板　https://github.com/Nobcha/R909-VFO/blob/main/front-back-p.kicad_pcb.zip

R909シリーズではVFO、GPS較正VFO、DSP（Si4732使用）エアバンド受信機1などをリリースしています。



I tranported the 10kHz-220MhzVFO from UNO/ATmega320P to ESP-C3 super mini dev kit. The PCB is a slim and holding the OLED display, the rotary encoder, switches, and LEDs. There are some RCs almostly SMD devices. I uploarded the Gerver ZIP, the schematic, and BOM as below.
I will also provide the sketches as the VFOs and the diagnostic for check.
You can install PCB assy into the AL case with the front and back panel PCB.

I'm reporting the relating articles on my BLOGs. Please visit.
 https://nobcha23.hatenablog.com/
 
R909-VFO-ESP Schematics  R909-VFO_ESP32_SCM.pdf
R909-VFO-ESP BOM    5531_esp_25_bom.pdf
R909-VFO-ESP Gerber files    5531_esp_25.kicad_pcb.zip
The diagnostic sketch after ASSY  R909_VFO_esp_ol_SWRE_i2c_TEST.ino
Ported JCR VFO    JCR_R909-VFO-ESP_kpa.ino
R909-VFO-ESP(10ｋHz-220MHｚ 50ch memories) R909_VFO_ESP_UNIV0406.ino
KiCAD symbol for ESP32-C3 Super mini dev kit  ESP32-C3.kicad_sym
KiCAD foot print for ESP32-C3 Super mini dev kit  ECP32C3-PinHeader-2x8.kicad_mod
Gerber files for the front and back panels　https://github.com/Nobcha/R909-VFO/blob/main/front-back-p.kicad_pcb.zip

