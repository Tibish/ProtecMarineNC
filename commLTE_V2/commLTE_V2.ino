// ajout des bibliothèques necéssaires
#include <Arduino.h>
#include "esp_mac.h"
#include <ArduinoJson.h> //bibliothèque permettant la gestion des données JSON

HardwareSerial mySerial2(2); // initialisation de la communication avec l'esp

//initialisation des reglages de communiquation
#define DEBUG true
#define IO_RXD2 17
#define IO_TXD2 18

//utilisateur de connection au serveur MQTT
char user[] = "user2";
char mdp[] = "1234."; 

#define IO_GSM_PWRKEY 4
#define IO_GSM_RST    5

//variables pour l'envoie regulier de donner
unsigned long lastSendTime = 0;  // Variable pour stocker le temps du dernier envoi
const unsigned long sendInterval = 120000;  // Intervalle d'envoie


//fonction de recuperation de l'adresse MAC
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


//fonction permettant d'envoyer les commandes AT a l'esp
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


//fonction pour la connection au serveur MQTT
void conn(){
  
  char atCommandConnect[120];
  sprintf(atCommandConnect, "AT+CMQTTCONNECT=0,\"tcp://mqttsprotectmarine.btssn.nc:8883\",100,1,\"%s\",\"%s\"", user, mdp);
  sendData(atCommandConnect, 1000, DEBUG);
  
}


//fonction pour d'abonnement a un topic du serveur MQTT
void Sub(String topic) {
  const char* topicChar = topic.c_str();
  char fullTopic[50];
  //creation du topic sous format utilisateur/topic
  strcpy(fullTopic, user);
  strcat(fullTopic, "/");
  strcat(fullTopic, topicChar);
  //creation de commande AT correspondante
  int len = strlen(fullTopic);
  char atCommand[50];
  sprintf(atCommand, "AT+CMQTTSUB=0,%d,0", len);
  //envoie de la commande AT
  sendData(atCommand, 1000, DEBUG);
  sendData(fullTopic, 1000, DEBUG);
}

//fonction de publication d'un message sur le serveur MQTT
void Pub(String topic, String payload) {
  Serial.println("debut du pub");
  const char* topicChar = topic.c_str();
  char fullTopic[50];
  //creation du topic sur le quel va etre envoyer la donner
  strcpy(fullTopic, user);
  strcat(fullTopic, "/");
  strcat(fullTopic, topicChar);
  //creation de la commande AT pour l'initialisation du topic
  int len = strlen(fullTopic);
  char atCommandTopic[30];
  sprintf(atCommandTopic, "AT+CMQTTTOPIC=0,%d", len);
  //envoie de la commande AT
  sendData(atCommandTopic, 1000, false);
  sendData(fullTopic, 1000, false);
  //creation de la commande AT pour l'envoie du message
  const char* payloadChar = payload.c_str();
  int lenpayload = strlen(payloadChar);
  char atCommandPayload[30];
  sprintf(atCommandPayload, "AT+CMQTTPAYLOAD=0,%d", lenpayload);
  //envoie de la commande AT
  sendData(atCommandPayload, 1000, false);
  //envoie du message
  sendData(payload, 1000, false);
  //envoie de la commande AT permettant la publication
  sendData("AT+CMQTTPUB=0,0,60", 1000, false);
  Serial.println("fin du pub");
}

//creationde donner aleatoire
// TEST // TEST // TEST //
String RandomData() {
  StaticJsonDocument<200> doc;
  String Data;
  float tension = random(220, 230) + random(0, 100) / 100.0;
  float pression = random(1, 10) + random(0, 100) / 100.0;
  String id = getDefaultMacAddress();

  doc["id"] = id;
  doc["tension"] = tension;
  doc["pression"] = pression;
  doc["alerte"] = 1;
  doc["etat"] = "E";

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
  if (doc.containsKey("action")) {
    String actions = doc["action"];  // Récupère la clé "actions"
    
    if (actions == "G") {
      // Si l'action est "gonfler"
      Serial.println("Actions: gonfler");
    }
    else if (actions == "D") {
      // Si l'action est "degonfler"
      Serial.println("Actions: degonfler");
    }
    else if (actions == "A") {
      // Si l'action est "arrete d'urgence"
      Serial.println("Actions: Arret d'urgence");
    }
    else {
      Serial.println("Actions inconnue reçue.");
    }
  } 
  else {
    Serial.println("Clé 'action' manquante dans le message JSON.");
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

  Sub("action");
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

  // Envoi toutes les minutes
  if (currentMillis - lastSendTime >= sendInterval) {
    Pub("data", RandomData());
    lastSendTime = currentMillis;
  }
}
