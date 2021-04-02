

#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define PIN_Sensor1         D1
#define PIN_Sensor2         D2
#define PIN_Relay1          D7
#define PIN_Relay2          D8

const char* auth = "TpgjnqeeO2y30G3b_49YYH4T_a7QbhyQ";//"Y0RuykHXC3DZfGSLio0-VuYGz35Mt6vq";
const char* ssid = "Rajawali lojajar";
const char* pass = "lojajar142";
const char* notifMessage;
const char* notif1 = "Air Keluar";
const char* notif2 = "Air Tidak Keluar";
const char* notif3 = "Air dimatikan";


unsigned long waterTimeout;
uint8_t lastButtonState, buttonState = 0, wifiStatus = 0, initBlynk = 0;
uint8_t ledState = 0;

struct {
  unsigned long timeOut;
  uint8_t update;
} notif;

unsigned long timer_waterOn;

void setup() {
  // put your setup code here, to run once:
  pinMode(PIN_Sensor1, INPUT_PULLUP);
  pinMode(PIN_Sensor2, INPUT_PULLUP);
  pinMode(PIN_Relay1, OUTPUT);
  pinMode(PIN_Relay2, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
 WiFi.disconnect();
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
//  if (WiFi.status() != WL_CONNECTED) 
//{
 IPAddress ip(192, 168, 1, 8);
  IPAddress gw(192, 168, 1, 1);
  IPAddress sn(255, 255, 255, 0);
  WiFi.config(ip, gw, sn, gw);
    WiFi.begin(ssid, pass);
//  }
  notif.update = 0;
  notifMessage = NULL;
  lastButtonState = 0;
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(PIN_Relay1, LOW);
}

unsigned long reconnected_timer = 0;
void loop() {
  // put your main code here, to run repeatedly:

  wifiStatus = WiFi.status() == WL_CONNECTED;

  // ================== BLYNK CONTROL =====================
  if (wifiStatus) {
    if (initBlynk == 0) { //init
      Blynk.config(auth, BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT);
      initBlynk = 1;
      relayIs(LOW);
    Serial.println("Blynk init");
    }
    else if (initBlynk == 1) { //connecting
      digitalWrite(LED_BUILTIN, ledState == HIGH);
      relayIs(LOW); //Turn off relay
      ledState = !ledState;
      initBlynk =  Blynk.connected() == true ? 2 : 1;
      if(initBlynk == 1)
        initBlynk =  Blynk.connect() == true ? 2 : 1;
        
    Serial.println("Blynk Reconnect");
    }
    else {//connected
      digitalWrite(LED_BUILTIN, LOW);
      initBlynk =  Blynk.connected() == true ? 2 : 0;
      Blynk.run();
      
    Serial.println("Blynk connected");
    }
    reconnected_timer = millis();
    Serial.println("Wifi Connect");
  }
  else {
    static unsigned int dis = 0;
    if (millis() - reconnected_timer > 5000) {
      WiFi.reconnect();
      reconnected_timer = millis();
      dis++;
      if(dis >= 3){
        WiFi.disconnect();
        delay(100);
        dis = 0;
       WiFi.begin(ssid, pass);
      }
      
    Serial.println("Wifi Reconnected");
    }
    Serial.println("Wifi not Connected");
    initBlynk = 0;
    digitalWrite(LED_BUILTIN, HIGH);
    relayIs(LOW); //Turn off relay
    lastButtonState = 255; //interlock button, the water not flow
  }

  //=================== NOTIF CONTROL===============================

  if (wifiStatus && initBlynk > 1 && notif.update == 1) {
    if (notifMessage != NULL)
      Blynk.notify(notifMessage);
    notif.update = 2;
    notif.timeOut = millis();
  }
  else if (notif.update == 2 && (millis() - notif.timeOut) > 7000 ) {
    notif.update = 0;
  }

  Serial.println("ButtonState: " + String(buttonState) + ", Wifi Status: " + String(wifiStatus) + ", Blynk Status: " + String(initBlynk) + ", Sensor: " + String(readSensor()));

  //================================= BUTTON CONTROL====================

  if (buttonState == 1 && lastButtonState == 0) {
    lastButtonState = 1;//interlocking
  }
  else if (buttonState == 0) {
    lastButtonState = 0;
    relayIs(LOW); //Turn off relay
    notif.update = 0;
    Serial.println("turn off relay");
  }

  if (lastButtonState == 1) { //Turn on relay
    relayIs(HIGH);
    lastButtonState = 2;
    waterTimeout = millis();
    Serial.println("turn on relay");
  }
  else if (lastButtonState == 2) { //wait water to flow
    Serial.println("Wait water to flow");
    if (millis() - waterTimeout > 120000) {
      if (notif.update == 0) {
        notif.update = 1;
        notifMessage = notif2;
        relayIs(LOW); //Turn off relay
        lastButtonState = 255; //interlock button, the water not flow
      }
    }
    else if (readSensor()) { //the water flow
      Serial.println("water flow");
      lastButtonState = 3;
      if (notif.update == 0) {
        notif.update = 1;
        notifMessage = notif1;
      }
      timer_waterOn = millis();
    }
  }
  else if (lastButtonState == 3) {
    Serial.println("OKE");
    if (!readSensor()) {
      if (notif.update == 0) {
        notif.update = 1;
        notifMessage = notif2;
      }
      relayIs(LOW); //Turn off relay
      lastButtonState = 255; //interlock button, the water not flow
    }
    else if(millis() - timer_waterOn > (30*60000)){
      relayIs(LOW); //Turn off relay
      lastButtonState = 255; //interlock button, the water not flow
      if (notif.update == 0) {
        notif.update = 1;
        notifMessage = notif3;
      }
    }
  }
  else{
    relayIs(LOW); //Turn off relay
  }
  delay(500);
}

BLYNK_WRITE(V5) // V5 is the number of Virtual Pin
{
  buttonState = param.asInt();
}


uint8_t readSensor() {
  return !digitalRead(PIN_Sensor2);// || digitalRead(PIN_Sensor2) == HIGH;
}


void relayIs(bool state) {
  digitalWrite(PIN_Relay1, !state);
  digitalWrite(PIN_Relay2, !state);
}
