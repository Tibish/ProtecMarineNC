#include "ProtecMarine_Comm3G.h"

ProtecMarine_Comm3G::ProtecMarine_Comm3G(HardwareSerial& serial, const char* mqttUser, const char* mqttPassword)
    : _serial(serial), _mqttUser(mqttUser), _mqttPassword(mqttPassword), _receivedMessage(""), _isJsonPayload(false) {}

String ProtecMarine_Comm3G::getDefaultMacAddress() {
    String mac = "";
    unsigned char mac_base[6] = {0};
    if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
        char buffer[18];
        sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
        mac = buffer;
    }
    return mac;
}

String ProtecMarine_Comm3G::sendData(String command, const int timeout, boolean debug) {
    String response = "";
    _serial.println(command);
    long int time = millis();
    while ((time + timeout) > millis()) {
        while (_serial.available()) {
            char c = _serial.read();
            response += c;
        }
    }
    if (debug) {
        Serial.print(response);
        delay(500);
    }
    return response;
}

void ProtecMarine_Comm3G::connectMQTT() {
    char atCommandConnect[120];
    sprintf(atCommandConnect, "AT+CMQTTCONNECT=0,\"tcp://mqttsprotectmarine.btssn.nc:8883\",100,1,\"%s\",\"%s\"", _mqttUser, _mqttPassword);
    sendData(atCommandConnect, 1000, true);
}

void ProtecMarine_Comm3G::subscribe(String topic) {
    char fullTopic[50];
    strcpy(fullTopic, _mqttUser);
    strcat(fullTopic, "/");
    strcat(fullTopic, topic.c_str());
    int len = strlen(fullTopic);
    char atCommand[50];
    sprintf(atCommand, "AT+CMQTTSUB=0,%d,0", len);
    sendData(atCommand, 1000, true);
    sendData(fullTopic, 1000, true);
}

void ProtecMarine_Comm3G::publish(String topic, String payload) {
    char fullTopic[50];
    strcpy(fullTopic, _mqttUser);
    strcat(fullTopic, "/");
    strcat(fullTopic, topic.c_str());
    int len = strlen(fullTopic);
    char atCommandTopic[30];
    sprintf(atCommandTopic, "AT+CMQTTTOPIC=0,%d", len);
    sendData(atCommandTopic, 1000, false);
    sendData(fullTopic, 1000, false);

    const char* payloadChar = payload.c_str();
    int lenpayload = strlen(payloadChar);
    char atCommandPayload[30];
    sprintf(atCommandPayload, "AT+CMQTTPAYLOAD=0,%d", lenpayload);
    sendData(atCommandPayload, 1000, false);
    sendData(payload, 1000, false);
    sendData("AT+CMQTTDISC=0,0,60", 1000, false);
}

String ProtecMarine_Comm3G::getData(int pressionPin, int potPin) {
    StaticJsonDocument<200> doc;
    String Data;
    String id = getDefaultMacAddress();

    int mesureBrute = analogRead(pressionPin);
    float tensionP = mesureBrute * (3.3 / 4095.0);
    float pression = (tensionP * 2 + 4.42) * (1000.0 / (4.7 - 0.2));

    int potValue = analogRead(potPin);
    float tension = (potValue * 12.0) / 4095;

    doc["id"] = id;
    doc["tension"] = tension;
    doc["pression"] = pression;
    doc["etat"] = "E";

    serializeJson(doc, Data);
    return Data;
}

void ProtecMarine_Comm3G::handleMessage(String message) {
    message = "{" + message;
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, message);

    if (error) {
        Serial.print(F("Erreur d'analyse du JSON: "));
        Serial.println(error.c_str());
        return;
    }

    if (doc.containsKey("action")) {
        String actions = doc["action"];
        if (actions == "G") {
            Serial.println("Action: Gonfler");
        } else if (actions == "D") {
            Serial.println("Action: Dégonfler");
        } else if (actions == "A") {
            Serial.println("Action: Arrêt d'urgence");
        } else {
            Serial.println("Action inconnue reçue.");
        }
    } else {
        Serial.println("Clé 'action' manquante dans le JSON.");
    }
}

void ProtecMarine_Comm3G::checkIncomingMessages() {
    while (_serial.available() > 0) {
        char c = _serial.read();
        if (c == '\n') {
            if (_isJsonPayload) {
                handleMessage(_receivedMessage);
                _isJsonPayload = false;
            }
            _receivedMessage = "";
        } else {
            _receivedMessage += c;
            if (_receivedMessage.startsWith("{")) {
                _isJsonPayload = true;
                _receivedMessage = "";
            } else if (_receivedMessage.startsWith("+CMQTTCONNLOST")) {
                sendData("AT+CMQTTDISC=0,120", 1000, true);
                connectMQTT();
                _receivedMessage = "";
            }
        }
    }
}
