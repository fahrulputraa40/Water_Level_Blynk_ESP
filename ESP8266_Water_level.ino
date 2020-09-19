

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define PIN_Sensor1         D0
#define PIN_Sensor2         D1
#define PIN_Relay1          D7
#define PIN_Relay2          D8

const char* auth = "Y0RuykHXC3DZfGSLio0-VuYGz35Mt6vq";
const char* ssid = "AndroidAP";
const char* pass = "gampang123";
const char* notifMessage;
const char* notif1 = "Air Keluar";
const char* notif2 = "Air Tidak Keluar";


unsigned long waterTimeout;
uint8_t lastButtonState, buttonState = 0, wifiStatus = 0, initBlynk = 0;

struct{
  unsigned long timeOut;
  uint8_t update;
}notif;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_Sensor1, INPUT_PULLUP);
  pinMode(PIN_Sensor2, INPUT_PULLUP);
  pinMode(PIN_Relay1, OUTPUT);
  pinMode(PIN_Relay2, OUTPUT);
  
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
  }
  notif.update = 0;
  notifMessage = NULL;
  lastButtonState = 0;
}


void loop() {
  // put your main code here, to run repeatedly:
  
  wifiStatus = WiFi.status() == WL_CONNECTED;

  // ================== BLYNK CONTROL =====================
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

  //=================== NOTIF CONTROL===============================
  
  if(wifiStatus && initBlynk > 1 && notif.update == 1){
    if(notifMessage != NULL)
      Blynk.notify(notifMessage);
    notif.update = 2;
    notif.timeOut = millis();
  }
  else if(notif.update == 2 && (millis() - notif.timeOut) > 6000 ){
    notif.update = 0;
  }
 
  Serial.println("ButtonState: "+String(buttonState)+"Wifi Status: "+String(wifiStatus)+" Blynk Status: "+String(initBlynk));

  //================================= BUTTON CONTROL====================
  
  if(buttonState == 1 && lastButtonState == 0){
    lastButtonState = 255;//interlocking
  }
  else if(buttonState == 0){
    lastButtonState = 0;
    relayIs(LOW); //Turn off relay
  }
  
  if(lastButtonState == 1){//Turn on relay
    relayIs(HIGH);
    lastButtonState = 2;
    waterTimeout = millis();
  }
  else if(lastButtonState == 2){//wait water to flow
    if(millis() - waterTimeout > 15000){
      if(notif.update == 0){
        notif.update = 1;
        notifMessage = notif2;
        relayIs(LOW); //Turn off relay
        lastButtonState = 255; //interlock button, the water not flow
      }
    }
    else if(readSensor()){//the water flow
      lastButtonState = 3;
      if(notif.update == 0){
        notif.update = 1;
        notifMessage = notif1;
      }
    }
  }
  else if(lastButtonState == 3){
    if(!readSensor()){
      if(notif.update == 0){
        notif.update = 1;
        notifMessage = notif2;
        relayIs(LOW); //Turn off relay
        lastButtonState = 255; //interlock button, the water not flow
      }
    }
  }
  delay(500);
}

BLYNK_WRITE(V5) // V5 is the number of Virtual Pin  
{
  buttonState = param.asInt();
}


uint8_t readSensor(){
  return digitalRead(PIN_Sensor1) == LOW || digitalRead(PIN_Sensor2) == LOW;
}


void relayIs(bool state){
  digitalWrite(PIN_Relay1, state);
  digitalWrite(PIN_Relay2, state);
}
