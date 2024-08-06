# ESP8266-alarm

Funciones del programa
+ Inicia en modo Station. Se conecta con SSID y PASS guardados en memoria.
+ Manda mensaje de Alarma al Número de telefono y API key guardados en memoria.
+ Cambia a modo Access Point. Permite conectarse a la IP y cambiar los parámetros.

Hardware utilizado:
+ ESP8266-01
+ módulo de reducción de voltaje de 12V a 3.3V
+ bornera para conectar la alimentación
+ zocalo de 8 puntos para ESP8266-01

Cuidado con el diseño de la placa, contemplar que no es version 01s. La placa debe tener 3.3V en el pin CH_PD

TODO: 
+ Mostrar los parámetros guardados en la memoria. -> Ver con función Procesor en el html y el backend.
+ Agregar botón en el html para reiniciar el módulo sin cambiar de parámetros.
