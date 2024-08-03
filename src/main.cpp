/* 
  Alarma al whatsapp:
   Cuando se enciende, se conecta a internet (ssid y pass guardados en FS)
   y envía un mensaje al telefono (telefono y key guardados en FS).
   Se conecta a la salida de la sirena (en paralelo con la sirena interior.)
   La sirena pasa de 0 a 13.8 V cuando suena. En ese momento se enciende el ESP8266 y envía el mensaje. 
   Luego se inicia en modo AP (access point) y permite cambiar los parametros.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <UrlEncode.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"

// Program variables
//const char* ssid = "Cabin 4";
//const char* password = "tiasilvia";
String ssid;
String pass;
String phone;
String apiKey;

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

// LittleFS paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* phonePath = "/phone.txt";
const char* apiKeyPath = "/apikey.txt";

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "phone";
const char* PARAM_INPUT_4 = "apikey";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


// --- Functions ---

// LittleFS - Initialize 
void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// LittleFS - Read File 
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  } 
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// LittleFS - Write file 
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// WIFI-STA Inicialize, (station mode)
bool initWiFiSTA() {
  // first inicialize, without ssid save in LittleFS
  if(ssid==""){
    Serial.println("Undefined SSID or IP address. Failed to connect in Station Mode.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  // Try to connect for interval time (10s)
  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    delay(100);
    Serial.print(".");
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect in Station Mode.");
      return false;
    }
  }

  Serial.println("Conected in Station Mode. Local IP:");
  Serial.println(WiFi.localIP());
  return true;
}

// Start http client and send message to phone
void sendMessage(String message){

  // Data to send with HTTP POST
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phone + "&apikey=" + apiKey + "&text=" + urlEncode(message);
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

// WIFI-AP Inicialize, (access point mode). Init Web Server.
bool initWiFiAP(){
    Serial.println("Setting AP (Access Point)");
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", LittleFS, "/");
    
    // Server declaration. It updates the variables into LittleFS. Then restart.
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST phone value
          if (p->name() == PARAM_INPUT_3) {
            phone = p->value().c_str();
            Serial.print("Phone Number set to: ");
            Serial.println(phone);
            // Write file to save value
            writeFile(LittleFS, phonePath, phone.c_str());
          }
          // HTTP POST apiKey value
          if (p->name() == PARAM_INPUT_4) {
            apiKey = p->value().c_str();
            Serial.print("API key set to: ");
            Serial.println(apiKey);
            // Write file to save value
            writeFile(LittleFS, apiKeyPath, apiKey.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart ");
      delay(3000);
      ESP.restart();
    });

    server.begin();    
  return true;
}

void setup() {
  Serial.begin(115200);

  // Init LittleFS and read variables 
  initLittleFS();
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  phone = readFile(LittleFS, phonePath);
  apiKey = readFile (LittleFS, apiKeyPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(phone);
  Serial.println(apiKey);

  // First, sets an station mode and Send Message to WhatsAPP
  if(initWiFiSTA()){  
    sendMessage("ALARMA DISPARADA!!");
    delay(1000);    
  };

  // Second, sets an open Access Point to change parameters
    initWiFiAP();
}


void loop() {
  // Nothing to do. The server started in the initWiFiAP function and wait for update variables
}