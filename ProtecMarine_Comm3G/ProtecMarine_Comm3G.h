#ifndef PROTECMARINE_COMM3G_H
#define PROTECMARINE_COMM3G_H

#include <Arduino.h>
#include <ArduinoJson.h> // Gestion des données JSON
#include "esp_mac.h"

class ProtecMarine_Comm3G {
public:
    ProtecMarine_Comm3G(HardwareSerial& serial, const char* mqttUser, const char* mqttPassword);

    String getDefaultMacAddress();
    String sendData(String command, const int timeout, boolean debug = false);
    void connectMQTT();
    void subscribe(String topic);
    void publish(String topic, String payload);
    String getData(int pressionPin, int potPin);
    void handleMessage(String message);
    void checkIncomingMessages();

private:
    HardwareSerial& _serial; // Référence au port série
    const char* _mqttUser;
    const char* _mqttPassword;
    String _receivedMessage;
    bool _isJsonPayload;
};

#endif
