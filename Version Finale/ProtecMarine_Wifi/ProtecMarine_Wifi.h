#ifndef PROTECMARINE_WIFI_H
#define PROTECMARINE_WIFI_H

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "esp_mac.h"
#include <LittleFS.h>

class ProtecMarine_Wifi {
public:
    ProtecMarine_Wifi(const char* ssid, const char* password, int pressionPin, int potPin);

    String getDefaultMacAddress();
    void setupWifi();
    void disconnectWifi();

    void handleRequests();
    void handleRoot();
    void handleImage();

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
};

#endif
