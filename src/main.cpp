/* 
  Alarma al whatsapp:
   Cuando se enciende, se conecta a internet (ssid y pass guardados en FS)
   y envía un mensaje al telefono (telefono y key guardados en FS).
   Luego se inicia en modo AP (access point) y permite cambiar los parametros.
   Luego de 10 minutos se reinicia el módulo.
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <UrlEncode.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"

// Program variables
String ssid;
String pass;
String phone;
String apikey;
int resetflag = 0; // if this changes to 1, then the module restart in the next loop

// Timer variables
unsigned long previousMillis = 0;
const long interval = 20000;  // interval to wait for Wi-Fi connection (milliseconds)

// LittleFS paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* phonePath = "/phone.txt";
const char* apikeyPath = "/apikey.txt";

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

  // Try to connect for interval time (30s)
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
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phone + "&apikey=" + apikey + "&text=" + urlEncode(message);
  WiFiClient client;    
  HTTPClient http;
  http.begin(client, url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.println("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

// Replaces HTML placeholders with values
String processor(const String& var){
  //Serial.println(var);
  if(var == "SSID_VALUE"){
    return ssid;
  }
  else if(var == "PASS_VALUE"){
    return pass;
  }
  else if(var == "PHONE_VALUE"){
    return phone;
  }
  else if(var == "APIKEY_VALUE"){
    return apikey;
  }
  return String();
}

// WIFI-AP Inicialize, (access point mode). Init Web Server.
bool initWiFiAP(){
    Serial.println("Setting AP (Access Point)");
    WiFi.softAP("ESP-WIFI-SETUP", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Server - Root
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", String(), false, processor);
    });
    
    // Server - Reset button
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/plain", "Done. ESP will restart ");
      Serial.println("Reset response sended");
      resetflag = 1;
    });
    
    server.serveStatic("/", LittleFS, "/");
    
    // Server - Submit button. It updates the variables into LittleFS. Then restart.
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
            apikey = p->value().c_str();
            Serial.print("API key set to: ");
            Serial.println(apikey);
            // Write file to save value
            writeFile(LittleFS, apikeyPath, apikey.c_str());
          }
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart ");
      Serial.println("Post response sended");
      resetflag = 1;
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
  apikey = readFile (LittleFS, apikeyPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(phone);
  Serial.println(apikey);

  // First, sets an station mode and Send Message to WhatsAPP
  if(initWiFiSTA()){  
    sendMessage("ALARMA DISPARADA!!");

  // If fails to connect, wait 1 second and try again
  }else{
    delay(1000);
    if(initWiFiSTA()){  
    sendMessage("ALARMA DISPARADA!!");
    }
  };

  // Second, sets an open Access Point to change parameters
    initWiFiAP();

  // Set timer to zero
  previousMillis = millis();
}


void loop() {
  // Nothing to do. The server started in the initWiFiAP function and wait for update variables

  // Reset handle, with delays out of server callback functions 
  if(resetflag==1){
    Serial.println("Restarting ESP");
    delay(2000);
    ESP.restart();  
  }      
  // Reset module after 10 minutes
  if(millis()-previousMillis>600000){
    Serial.println("10 minutes have passed");
    resetflag=1;
  }
}
