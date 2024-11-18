#ifndef PROTECMARINE_WIFI_H
#define PROTECMARINE_WIFI_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

class ProtecMarine_Wifi {
public:
    ProtecMarine_Wifi(const char* ssid, const char* password, int pressionPin, int potPin);
    void setupWifi();
    void handleRequests();
    void handleRoot();
    void handleGetData();
    void handleGonflage();
    void handleDegonflage();
    void handleArret();

private:
    const char* _ssid;
    const char* _password;
    int _pressionPin;
    int _potPin;

    WebServer _server;
    String _etat; // État actuel
    int _potValue;
    float _tension;

    String GetData();
    String RandomData();
};

#endif
