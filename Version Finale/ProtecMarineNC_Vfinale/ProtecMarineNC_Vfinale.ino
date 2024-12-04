#include <ProtecMarine_Comm3G.h>
#include <ProtecMarine_Wifi.h>

ProtecMarine_Wifi wifi("ProtecMarineNC", "12345678", 34, 35);

#define IO_RXD2 17
#define IO_TXD2 18
#define DEBUG true

HardwareSerial mySerial2(2);
ProtecMarine_Comm3G comm3G(mySerial2, "7C-DF-A1-ED-5B-74", "[MHHf0572NFX4#E", 34, 35, 42, 1, 21);

// Prototypes des fonctions
float hpaToBar(float pressionHpa);

int currentMode = 1;
const int swPin = 5;
int sw = 0;

static unsigned long lastSendTime = 0;

void setup() {
    Serial.begin(115200);
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
}

void loop() {
  sw = analogRead(swPin);
  if(sw > 2050){
    if(currentMode == 0){
      wifi.disconnectWifi();
      Serial.println("init 3G");
      comm3G.initMQTT();
      comm3G.connectMQTT();
      comm3G.subscribe("action");
      delay(200);
      currentMode = 1;
    }
    else{}
    comm3G.verifTime();
    comm3G.checkIncomingMessages();
  }
  else { 
    if(currentMode == 1){
      comm3G.disconnectMQTT();
      Serial.println("init wifi"); 
      wifi.setupWifi();
      delay(200);
      currentMode = 0;
    }
    wifi.handleRequests();
  }
}
