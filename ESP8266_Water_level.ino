

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>


const char* auth = "Y0RuykHXC3DZfGSLio0-VuYGz35Mt6vq";
const char* ssid = "AndroidAP";
const char* pass = "gampang123";

uint8_t wifiStatus = 0, initBlynk = 0;

struct{
  unsigned long timeOut;
  uint8_t update;
}notif;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
  }
  notif.update = 0;

}


void loop() {
  // put your main code here, to run repeatedly:
  
  wifiStatus = WiFi.status() == WL_CONNECTED;

  if(wifiStatus){
      if(initBlynk == 0){//init
        Blynk.config(auth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
        initBlynk = 1;
      }
      else if(initBlynk == 1){//connecting
        initBlynk =  Blynk.connect() == true ? 2 : 1;
      }
      else {//connected
        Blynk.run();      
      }
  }

  if(wifiStatus && initBlynk > 1 && && notif.update == 1){
    Blynk.notify("Tanki Penuh!");
    notif.update = 2;
    notif.timeOut = millis();
  }
  else if(notif.update == 2 && (millis() - notif.timeOut) > 6000 ){
    notif.update = 0;
  }
 
  Serial.println("Wifi Status: "+String(wifiStatus)+" Blynk Status: "+String(initBlynk));
  
  delay(500);
}
