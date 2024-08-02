#include <Arduino.h>

/* 
  Alarma al whatsapp:
   Cuando se enciende, se conecta a internet (red wifi Cabaña 4) y envía un mensaje al telefono.
   Se conecta a la salida de la sirena (en paralelo con la sirena interior.)
   La sirena pasa de 0 a 13.8 V cuando suena. En ese momento se enciende el ESP8266 y envía el mensaje. Luego se duerme hasta que se apaga la sirena (que se apaga).

  Besed on: 
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-send-messages-whatsapp/
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <UrlEncode.h>

const char* ssid = "Cabin 4";
const char* password = "tiasilvia";

// +international_country_code + phone number
// Portugal +351, example: +351912345678
String phoneNumber = "5493425057171";
String apiKey = "2385958";

void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  WiFiClient client;    
  HTTPClient http;
  http.begin(client, url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Send Message to WhatsAPP
  sendMessage("ALARMA DISPARADA!!");
  Serial.println("Mensaje Enviado");
  Serial.println("Durmiendo...");
  ESP.deepSleep(0); // para que consuma menos, reset por hardware
}

void loop() {
  
}