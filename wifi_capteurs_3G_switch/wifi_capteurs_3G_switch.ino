#include <ProtecMarine_Comm3G.h>
#include <ProtecMarine_Wifi.h>

ProtecMarine_Wifi wifi("ProtecMarineNC", "12345678", 34, 35);

#define IO_RXD2 17
#define IO_TXD2 18
#define DEBUG true

HardwareSerial mySerial2(2);
ProtecMarine_Comm3G comm3G(mySerial2, "user3", "User123456789.");

int currentMode = 1;
const int swPin = 5;
int sw = 0;

static unsigned long lastSendTime = 0;
const unsigned long sendInterval = 120000;

void setup() {
    Serial.begin(115200);
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
}

void loop() {
  sw = analogRead(swPin);
  if(sw > 2050){
    if(currentMode == 0){
      Serial.println("init 3G");
      comm3G.initMQTT();
      comm3G.connectMQTT();
      comm3G.subscribe("action");
      delay(200);
      currentMode = 1;
    }
    comm3G.checkIncomingMessages();
    if (millis() - lastSendTime >= sendInterval) {
      comm3G.publish("data", comm3G.getData(34, 35));
      lastSendTime = millis();
    }
  }
  else { 
    if(currentMode == 1){
      Serial.println("init wifi"); 
      wifi.setupWifi();
      delay(200);
      currentMode = 0;
    }
    wifi.handleRequests();
  }
}