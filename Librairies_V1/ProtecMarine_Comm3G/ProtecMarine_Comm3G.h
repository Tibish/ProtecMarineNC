#ifndef PROTECMARINE_COMM3G_H
#define PROTECMARINE_COMM3G_H

#include <Arduino.h>
#include <ArduinoJson.h> // Gestion des données JSON
#include "esp_mac.h"

class ProtecMarine_Comm3G {
public:
    ProtecMarine_Comm3G(HardwareSerial& serial, const char* mqttUser, const char* mqttPassword, int pressionPin, int potPin);

    String getDefaultMacAddress();
    String sendData(String command, const int timeout, boolean debug = false);
    void initMQTT();
    void connectMQTT();
    void subscribe(String topic);
    void publish(String topic, String payload);
    String getData();
    void handleMessage(String message);
    void checkIncomingMessages();
    void disconnectMQTT();
    void setSendInterval(unsigned long interval);
    unsigned long getSendInterval() const;
    void verifTime();

private:
    HardwareSerial& _serial; // Référence au port série
    const char* _mqttUser;
    const char* _mqttPassword;
    int _pressionPin;
    int _potPin;
    String _receivedMessage;
    bool _isJsonPayload;
    unsigned long sendInterval;
    int id_ordre;
    unsigned long lastSendTime;
};

#endif
