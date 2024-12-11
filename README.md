# ESP8266-alarm

[![en](https://img.shields.io/badge/lang-en-red.svg)](https://github.com/esteban-at/ESP8266-alarm/blob/master/README.md)
[![es](https://img.shields.io/badge/lang-es-blue.svg)](https://github.com/esteban-at/ESP8266-alarm/blob/master/README.es.md)

Program functions:
+ Start in Station Mode. It connects to internet with the SSID and PASS stored in internal memory.
+ Send Alarm message to the Phone number, using the API key. Both stored in memory.
+ Switche to Access Point mode. To change the parameters, connect to the ESP-WIFI-SETUP network and go to 192.168.4.1
+ After 10 minutes the module is restarted.

Hardware:
+ ESP8266-01 
+ Voltage reduction module: 12V to 3.3V  (based on AMS1117)
+ Terminal block for power supply connection
+ 8-point socket for ESP8266-01

Comments: 
+ If not version 01s, the board must have 3.3V on the CH_PD pin.
+ See how to get API key in https://www.callmebot.com/blog/free-api-whatsapp-messages/

Connection and operation:

Connects to the alarm output going to the siren, in parallel with the indoor siren.

When the alarm is triggered, the output to the siren goes from 0 to 12V. This voltage is used to power the ESP8266 and send the message.