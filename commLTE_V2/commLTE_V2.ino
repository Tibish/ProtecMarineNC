#include <Arduino.h>
#include "esp_mac.h"
#include <ArduinoJson.h>

HardwareSerial mySerial2(2);

#define DEBUG true
#define IO_RXD2 17
#define IO_TXD2 18

char user[] = "user2";
char mdp[] = "1234."; 

#define IO_GSM_PWRKEY 4
#define IO_GSM_RST    5

unsigned long lastSendTime = 0;  // Variable pour stocker le temps du dernier envoi
const unsigned long sendInterval = 60000;  // Intervalle d'envoie

String getDefaultMacAddress() {
  String mac = "";
  unsigned char mac_base[6] = {0};
  if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
    char buffer[18];  // 6*2 characters for hex + 5 characters for colons + 1 character for null terminator
    sprintf(buffer, "%02X:%02X:%02X:%02X:%02X:%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
    mac = buffer;
  }
  return mac;
}

String sendData(String command, const int timeout, boolean debug) {
  String response = "";
  mySerial2.println(command);
  long int time = millis();
  while ((time + timeout) > millis()) {
    while (mySerial2.available()) {
      char c = mySerial2.read();
      response += c;
    }
  }
  if (debug) {
    Serial.print(response);
    delay(500);
  }
  return response;
}

void conn(){
  
  char atCommandConnect[120];
  sprintf(atCommandConnect, "AT+CMQTTCONNECT=0,\"tcp://mqttsprotectmarine.btssn.nc:8883\",100,1,\"%s\",\"%s\"", user, mdp);
  sendData(atCommandConnect, 1000, DEBUG);
  
}

void Sub(String topic) {
  const char* topicChar = topic.c_str();
  char fullTopic[50];
  strcpy(fullTopic, user);
  strcat(fullTopic, "/");
  strcat(fullTopic, topicChar);

  int len = strlen(fullTopic);
  char atCommand[50];
  sprintf(atCommand, "AT+CMQTTSUB=0,%d,0", len);

  sendData(atCommand, 1000, DEBUG);
  sendData(fullTopic, 1000, DEBUG);
}

void Pub(String topic, String payload) {
  const char* topicChar = topic.c_str();
  char fullTopic[50];
  strcpy(fullTopic, user);
  strcat(fullTopic, "/");
  strcat(fullTopic, topicChar);

  int len = strlen(fullTopic);
  char atCommandTopic[30];
  sprintf(atCommandTopic, "AT+CMQTTTOPIC=0,%d", len);

  sendData(atCommandTopic, 1000, DEBUG);
  sendData(fullTopic, 1000, DEBUG);

  const char* payloadChar = payload.c_str();
  int lenpayload = strlen(payloadChar);

  char atCommandPayload[30];
  sprintf(atCommandPayload, "AT+CMQTTPAYLOAD=0,%d", lenpayload);

  sendData(atCommandPayload, 1000, DEBUG);
  sendData(payload, 1000, DEBUG);

  sendData("AT+CMQTTPUB=0,0,60", 1000, DEBUG);
}

String RandomData() {
  StaticJsonDocument<200> doc;
  String Data;
  float tension = random(220, 230) + random(0, 100) / 100.0;
  int niveauEau = random(0, 100);
  float pression = random(1, 10) + random(0, 100) / 100.0;
  String id = getDefaultMacAddress();

  doc["tension"] = tension;
  doc["nivEau"] = niveauEau;
  doc["pression"] = pression;
  doc["id"] = id;
  doc["etat"] = "a sec";

  serializeJson(doc, Data);
  return Data;
}

String receivedMessage = "";  // Variable pour stocker le message reçu
bool isJsonPayload = false;   // Indicateur pour savoir si nous sommes dans la partie JSON

// Fonction pour traiter le message JSON reçu
void handleMessage(String message) {

  message = "{" + message;
  
  // Utilise la bibliothèque ArduinoJson pour analyser le JSON
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);  // Analyse du JSON

  if (error) {
    Serial.print(F("Erreur d'analyse du JSON: "));
    Serial.println(error.c_str());
    return;  // Quitte la fonction si l'analyse échoue
  }

  // Si l'analyse réussit, nous pouvons accéder aux données JSON
  if (doc.containsKey("actions")) {
    String actions = doc["actions"];  // Récupère la clé "actions"
    
    if (actions == "gonfler") {
      // Si l'action est "gonfler"
      Serial.println("Actions: gonfler reçu ! Exécution de la fonction correspondante.");
      // Appelle ta fonction ici
    } 
    else {
      Serial.println("Actions inconnue reçue.");
    }
  } 
  else {
    Serial.println("Clé 'actions' manquante dans le message JSON.");
  }
}

// Fonction pour lire les messages entrants
void checkIncomingMessages() {
  while (mySerial2.available() > 0) {
    char c = mySerial2.read();  // Lire chaque caractère disponible

    if (c == '\n') {
      if (isJsonPayload) {
        handleMessage(receivedMessage);  // Traiter le message JSON reçu
        isJsonPayload = false;  // Réinitialiser pour le prochain message
      }
      receivedMessage = "";  // Réinitialiser pour le prochain message
    } 
    else {
      receivedMessage += c;  // Ajouter le caractère au message reçu
      
      // Détecte le début du JSON après la ligne +CMQTTRXPAYLOAD
      if (receivedMessage.startsWith("{")) {
        isJsonPayload = true;  // Indique qu'on s'attend à recevoir le JSON
        receivedMessage = "";  // Réinitialiser pour commencer à lire le JSON
      }
      //Détecte une erreur de connexion
      else if (receivedMessage.startsWith("+CMQTTCONNLOST: 0,3")) {
        //redemarrage de la connection MQTT
        sendData("AT+CMQTTDISC=0,120", 1000, DEBUG);
        conn();
        receivedMessage = "";
      }
    }
  }
}



void setup() {
  Serial.begin(115200);
  Serial.print(F("Hello! ESP32-S3 AT command V1.0 Test"));
  mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);
  pinMode(IO_GSM_RST, OUTPUT);
  digitalWrite(IO_GSM_RST, LOW);

  pinMode(IO_GSM_PWRKEY, OUTPUT);
  digitalWrite(IO_GSM_PWRKEY, HIGH);
  delay(3000);
  digitalWrite(IO_GSM_PWRKEY, LOW);

  Serial.println("ESP32-S3 4G LTE CAT1 Test Start!");
  delay(2000);
  Serial.println("Wait a few minutes for 4G star");

  sendData("AT", 1000, DEBUG);
  sendData("AT+SIMCOMATI", 1000, DEBUG);
  sendData("AT+GMR", 1000, DEBUG); 
  sendData("AT+CICCID", 1000, DEBUG);
  sendData("AT+CNUM", 1000, DEBUG);
  sendData("AT+COPS?", 1000, DEBUG);    
  sendData("AT+CPSI?", 1000, DEBUG);
  sendData("AT+CNMP?", 1000, DEBUG);

  sendData("AT+CMQTTSTART", 1000, DEBUG);
  sendData("AT+CMQTTACCQ=0,\"PrtMarineNC\",1", 1000, DEBUG);
  sendData("AT+CMQTTWILLTOPIC=0,4", 1000, DEBUG);
  sendData("tata", 1000, DEBUG);
  sendData("AT+CMQTTWILLMSG=0,4,0", 1000, DEBUG);
  sendData("tata", 1000, DEBUG);

  conn();

  Sub("actions");
}

void loop() {
  unsigned long currentMillis = millis();

  // Gestion de la communication série pour les commandes
  while (Serial.available() > 0) {
    mySerial2.write(Serial.read());
    yield();
  }

  // Vérifier s'il y a des messages entrants
  checkIncomingMessages();

  // Envoi toutes les 10 secondes
  if (currentMillis - lastSendTime >= sendInterval) {
    Pub("data", RandomData());
    lastSendTime = currentMillis;
  }
}
