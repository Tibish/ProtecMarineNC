#include <ProtecMarine_Wifi.h>

ProtecMarine_Wifi wifi("ProtecMarineNC", "12345678", 34, 35);

void setup() {
    Serial.begin(115200);
    wifi.setupWifi();
}

void loop() {
    wifi.handleRequests();
}
