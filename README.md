# ESP8266-alarm

Funciones del programa:
+ Inicia en modo Station. Se conecta con SSID y PASS guardados en memoria.
+ Manda mensaje de Alarma al Número de telefono y API key guardados en memoria.
+ Cambia a modo Access Point. Para cambiar los parámetros conectarse a la red ESP-WIFI-SETUP e ir a 192.168.4.1
+ Luego de 10 minutos se reinicia el módulo.

Hardware utilizado:
+ ESP8266-01 
+ Módulo de reducción de voltaje de 12V a 3.3V (basado en AMS1117)
+ Bornera para conectar la alimentación
+ Zocalo de 8 puntos para ESP8266-01

Comentarios: 

No es version 01s. La placa debe tener 3.3V en el pin CH_PD.
Ver como obtener API key en https://www.callmebot.com/blog/free-api-whatsapp-messages/

