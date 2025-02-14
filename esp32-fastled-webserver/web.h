/*
   ESP32 FastLED WebServer: https://github.com/jasoncoon/esp32-fastled-webserver
   Copyright (C) 2017 Jason Coon

   Built upon the amazing FastLED work of Daniel Garcia and Mark Kriegsman:
   https://github.com/FastLED/FastLED

   ESP32 support provided by the hard work of Sam Guyer:
   https://github.com/samguyer/FastLED

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
  Modified for https://github.com/me-no-dev/ESPAsyncWebServer
  https://github.com/philbowles/ESPAsyncWebServer/issues/
  https://github.com/CDFER/Captive-Portal-ESP32/blob/main/src/main.cpp
*/
#include "esp_wpa2.h"
void setupWeb() {
  webServer.on("/all", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 0);
    String json = getFieldsJson(fields, fieldCount);
     request->send(200, "text/json", json);
    digitalWrite(led, 1);
  });

  webServer.on("/fieldValue", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 0);
    String name =  request->getParam("name")->value();
    String value = getFieldValue(name, fields, fieldCount);
     request->send(200, "text/json", value);
    digitalWrite(led, 1);
  });

  webServer.on("/fieldValue", HTTP_POST, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 0);
    String name =  request->getParam("name")->value();
    String value =  request->getParam("value")->value();
    String newValue;
    // Particularly like this since we cahnged some 
    if(request->hasParam("r") && request->hasParam("g") && request->hasParam("b")){
      String r = request->getParam("r")->value();
      String g = request->getParam("g")->value();
      String b = request->getParam("b")->value();
      String comb = r + "," + g + "," + b; //reuased named somewhere else asmoge
      newValue = setFieldValue(name, value, fields, fieldCount, comb);
    }
    else{
      newValue = setFieldValue(name, value, fields, fieldCount);
    }
    
    request->send(200, "text/json", newValue);
    digitalWrite(led, 1);
  });
   webServer.on("/connect2", HTTP_POST, [](AsyncWebServerRequest *request) {
    String ssid = "Test";
    if(request->hasParam("ssid")){
        ssid =  request->getParam("ssid")->value();
      }
    WiFi.disconnect(true);  
    esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)EAP_ID, strlen(EAP_ID));
    esp_wifi_sta_wpa2_ent_set_username((uint8_t *)EAP_USERNAME, strlen(EAP_USERNAME));
    esp_wifi_sta_wpa2_ent_set_password((uint8_t *)EAP_PASSWORD, strlen(EAP_PASSWORD));
    esp_wifi_sta_wpa2_ent_enable();
    WiFi.begin(ssid);
    String json = "[";
    json += "{\"connected\":\"";
    json += (WiFi.isConnected()) ? "true" : "false";
    json += "}]";
    request->send(200, "text/json", json);
    });
  
  webServer.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 0);
    String json = "[";
    if(WiFi.status() != WL_CONNECTED ){
      String ssid =  wifi_ssid;
      String pass =  wifi_password;
      /* If we got an arg, replace our values.*/
      if(request->hasParam("ssid")){
        ssid =  request->getParam("ssid")->value();
      }
      if(request->hasParam("pass")){
        pass =  request->getParam("pass")->value();
      }
      if (String(WiFi.SSID()) != String(ssid)) { // if we aren'tcurrently connected
        WiFi.begin(wifi_ssid, wifi_password);
        t = timeout;
        while(WiFi.status() != WL_CONNECTED){
          Serial.print(".");
          delay(1000);
          t -= 1;
          if (t < 0) {
            Serial.println("Took too long");
            break;
          }
        }
        if(WiFi.status() == WL_CONNECTED){
          Serial.printf("IP address at %s\n", WiFi.localIP().toString());
        }
      }
      else{
        // already conencted to said network
        json += "{\"connected\":\"";
        json += (WiFi.isConnected()) ? "true" : "false";
        json += "{\"local ip\":\"";
        json += WiFi.localIP().toString();
        json += "}]";
        request->send(409, "text/json", json);
      }
    }
    // check if we timed out or rnot
    if(WiFi.status() != WL_CONNECTED){
      json += "{\"connected\":\"";
      json += (WiFi.isConnected()) ? "true" : "false";
      json += "}]";
       request->send(200, "text/json", json);
    }else{
      json += "{\"connected\":\"";
      json += (WiFi.isConnected()) ? "true" : "false";
      json += "{\"local ip\":\"";
      json += WiFi.localIP().toString();
      json += "}]";
       request->send(409, "text/json", json);
    }
    digitalWrite(led, 1);
  });
  webServer.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 0);
    String json = "[";
    json += "{\"connected\":\"";
    json += (WiFi.isConnected()) ? "true" : "false";
    json += "{\"ap ip\":\"";
    json += WiFi.softAPIP().toString() + "},";
    json += "{\"local ip\":\"";
    json += WiFi.localIP().toString();
    json += "}]";
     request->send(200, "text/json", json);
    digitalWrite(led, 1);
  });
  webServer.on("/disconnect", HTTP_POST, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 0);
    String json = "[";
    if(WiFi.status() == WL_CONNECTED){
      WiFi.disconnect();
       request->send(200, "text/plain", "success");
    }else{
       request->send(409, "text/plain", "not even connected");
    }
    digitalWrite(led, 1);
  });
  webServer.on("/bypass_pass", HTTP_POST, [](AsyncWebServerRequest *request) {
    digitalWrite(led, 0);
    String json = "[";
    if(request->hasParam("new") && request->hasParam("old")){
      if(request->getParam("old")->value().compareTo(bypass_pass) == 0){
         bypass_pass = request->getParam("new")->value();
         request->send(200, "text/plain", "Sucessful request to change bypass_pass");
      }else{
        request->send(409, "text/plain", "Incorrect old value");
      }
      
    }else{
       request->send(409, "text/plain", "Invalid Request to change, missing new or old.");
    }
    digitalWrite(led, 1);
  });
  
  webServer.serveStatic("", SPIFFS, "/home.htm", "max-age=86400");
  webServer.serveStatic("/", SPIFFS, "/home.htm", "max-age=86400");
  webServer.serveStatic("/home", SPIFFS, "/home.htm", "max-age=86400");
  webServer.serveStatic("/admin", SPIFFS, "/index.htm", "max-age=86400").setAuthentication(auth1,auth2);// Changed authentication to not let other contorl your led's
  webServer.serveStatic("/index.htm", SPIFFS, "/index.htm", "max-age=86400");
  webServer.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico", "max-age=86400");
  webServer.serveStatic("/css/styles.css", SPIFFS, "/css/styles.css", "max-age=86400");
  webServer.serveStatic("/js/app.js", SPIFFS, "/js/app.js", "max-age=86400");
   
  webServer.serveStatic("/css/ugly.css", SPIFFS, "/css/ugly.css", "max-age=86400");
  webServer.serveStatic("/images/atom196.png", SPIFFS, "/images/atom196.png", "max-age=86400");
  webServer.serveStatic("/images/me.jpg", SPIFFS, "/images/me.jpg", "max-age=86400");
  webServer.serveStatic("/images/thneed1.jpg", SPIFFS, "/images/thneed1.jpg", "max-age=86400");
  webServer.serveStatic("/images/thneed2.jpg", SPIFFS, "/images/thneed2.jpg", "max-age=86400");
  webServer.serveStatic("/images/ugly.jpg", SPIFFS, "/images/ugly.jpg", "max-age=86400");

  

  webServer.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// windows 11 captive portal workaround
	webServer.on("/wpad.dat", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(404); });								// Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)
  webServer.on("/generate_204",HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/home"); });		   // android captive portal redirect
	webServer.on("/redirect", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/home"); });			   // microsoft redirect
	webServer.on("hotspot-detect.html", HTTP_GET,  [](AsyncWebServerRequest *request) { request->redirect("/home"); });  // apple call home
  webServer.on("netcts.cdn-apple.com", HTTP_GET,  [](AsyncWebServerRequest *request) { request->send(200); });  // apple call home
  webServer.on("/hotspot-detect.html", HTTP_GET,  [](AsyncWebServerRequest *request) { request->redirect("/home"); });  // apple call home
  webServer.on("/netcts.cdn-apple.com", HTTP_GET,  [](AsyncWebServerRequest *request) { request->send(200); });  // apple call home
	webServer.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/home"); });	   // firefox captive portal call home
	webServer.on("/success.txt", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200); });					   // firefox captive portal call home
	webServer.on("/ncsi.txt", HTTP_GET, [](AsyncWebServerRequest *request) { request->redirect("/home"); });			   // windows call home
  webServer.onNotFound([](AsyncWebServerRequest *request) {
		request->redirect("/home");
		Serial.print("onnotfound ");
		Serial.print(request->host());	// This gives some insight into whatever was being requested on the serial monitor
		Serial.print(" ");
		Serial.print(request->url());
		Serial.print(" sent redirect to /home \n");
	});
  webServer.begin();
  Serial.println ( "HTTP server started" );
}

void handleWeb() {
  static bool webServerStarted = false;
  // check for connection
  if ( WiFi.status() == WL_CONNECTED ) {
    if (!webServerStarted) {
      // turn off the board's LED when connected to wifi
      t = timeout;
      while(t > 0){
        digitalWrite(led, 1);
        Serial.println();
        Serial.println("WiFi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        webServerStarted = true;
        t--;
      }
      setupWeb();
      //
    }
  } else {
    // blink the board's LED while connecting to wifi
    static uint8_t ledState = 0;
    EVERY_N_MILLIS(125) {
      ledState = ledState == 0 ? 1 : 0;
      digitalWrite(led, ledState);
      Serial.print (".");
    }
  }
}

