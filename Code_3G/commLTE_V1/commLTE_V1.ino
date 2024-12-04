#include <Arduino.h>
#include "esp_mac.h"
#include <ArduinoJson.h>

HardwareSerial mySerial2(2);

#define DEBUG true


#define IO_RXD2 17
#define IO_TXD2 18



// change this to match your SD shield or module;
// ESP32-S3 4G LTE: pin 10
const int PIN_SD_SELECT = 10;

char user[] = "user2";
char mdp[] = "1234."; 


#define IO_GSM_PWRKEY 4
#define IO_GSM_RST    5

unsigned long currentTime;

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


String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  mySerial2.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (mySerial2.available())
    {
      char c = mySerial2.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
    delay(1000);
  }
  return response;
}


//fonction des commandes MQTT

void Sub(String topic){
  
  const char* topicChar = topic.c_str();  // Convertir String en char*
  
  // Créer un tableau suffisamment grand pour contenir user + "/" + topicChar
  char fullTopic[50];  // Ajustez la taille selon vos besoins
  strcpy(fullTopic, user);      // Copier le contenu de user dans fullTopic
  strcat(fullTopic, "/");       // Ajouter "/" à la fin de fullTopic
  strcat(fullTopic, topicChar); // Ajouter topicChar à la fin de fullTopic

  int len = strlen(fullTopic);  // Calculer la longueur de la chaîne complète

  char atCommand[50];  // Taille suffisante pour contenir la commande finale
  sprintf(atCommand, "AT+CMQTTSUB=0,%d,0", len);  // Créer la commande

  sendData(atCommand, 1000, DEBUG);    // Envoyer la commande
  sendData(fullTopic, 1000, DEBUG);    // Envoyer le topic complet
  
  }


void Pub(String topic, String payload){

  const char* topicChar = topic.c_str();
  
  char fullTopic[50];  // Ajustez la taille selon vos besoins
  strcpy(fullTopic, user);      // Copier le contenu de user dans fullTopic
  strcat(fullTopic, "/");       // Ajouter "/" à la fin de fullTopic
  strcat(fullTopic, topicChar); // Ajouter topicChar à la fin de fullTopic

  int len = strlen(fullTopic);  // Calculer la longueur de la chaîne complète

  char atCommandTopic[30];
  sprintf(atCommandTopic, "AT+CMQTTTOPIC=0,%d", len);
  
  sendData(atCommandTopic,1000,DEBUG);
  sendData(fullTopic,1000,DEBUG);


  
  const char* payloadChar = payload.c_str();
  int lenpayload = strlen(payloadChar);

  char atCommandPayload[30];
  sprintf(atCommandPayload, "AT+CMQTTPAYLOAD=0,%d", lenpayload);
  
  sendData(atCommandPayload,1000,DEBUG);
  sendData(payload,1000,DEBUG);

  

  sendData("AT+CMQTTPUB=0,0,60",1000,DEBUG);
  
  }


  JsonDocument doc;
  String Data = "";
  String id = getDefaultMacAddress();
  unsigned long lastSendTime = 0;  // Variable pour stocker le temps du dernier envoi
  const unsigned long sendInterval = 30000;  // Intervalle de 10 secondes

  
String RandomData() {
  // put your main code here, to run repeatedly:
  float tension = random(220, 230) + random(0, 100) / 100.0;  // Tension aléatoire entre 220 et 230V
  int niveauEau = random(0, 100);  // Niveau d'eau aléatoire entre 0 et 100%
  float pression = random(1, 10) + random(0, 100) / 100.0;  // Pression aléatoire entre 1 et 10 bars

  doc["tension"] = tension;
  doc["nivEau"] = niveauEau;
  doc["pression"] = pression;
  doc["id"] = id;
  doc["etat"] = "a sec";

  serializeJson(doc,Data);

  return Data;
}
  
  
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print(F("Hello! ESP32-S3 AT command V1.0 Test"));
  mySerial2.begin(115200,SERIAL_8N1, IO_RXD2, IO_TXD2);
  pinMode(IO_GSM_RST, OUTPUT);
  digitalWrite(IO_GSM_RST, LOW);
  
  pinMode(IO_GSM_PWRKEY, OUTPUT);
  digitalWrite(IO_GSM_PWRKEY, HIGH);
  delay(3000);
  digitalWrite(IO_GSM_PWRKEY, LOW);
  
  Serial.println("ESP32-S3 4G LTE CAT1 Test Start!");
  delay(2000);
  Serial.println("Wait a few minutes for 4G star"); 


  delay(1000);
  sendData("AT", 1000, DEBUG);
  delay(1000);
  sendData("AT+SIMCOMATI", 1000, DEBUG);
  delay(1000);
  sendData("AT+GMR", 1000, DEBUG); 
  sendData("AT+CICCID", 1000, DEBUG);
  sendData("AT+CNUM", 1000, DEBUG);
  sendData("AT+COPS?", 1000, DEBUG);    
  sendData("AT+CPSI?", 1000, DEBUG);
  sendData("AT+CNMP?", 1000, DEBUG);

   //MQTT connexion INIT
  sendData("AT+CMQTTSTART", 1000, DEBUG);
  
  sendData("AT+CMQTTACCQ=0,\"PrtMarineNC\",1", 1000, DEBUG);
  
  sendData("AT+CMQTTWILLTOPIC=0,4", 1000, DEBUG);
  sendData("tata", 1000, DEBUG);
  
  sendData("AT+CMQTTWILLMSG=0,4,0", 1000, DEBUG);
  sendData("tata", 1000, DEBUG);


  char atCommandConnect[120];  // Taille suffisante pour contenir la commande
  sprintf(atCommandConnect, "AT+CMQTTCONNECT=0,\"tcp://mqttsprotectmarine.btssn.nc:8883\",100,1,\"%s\",\"%s\"", user, mdp);
  
  sendData(atCommandConnect,1000,DEBUG);

  Sub("ordres");
 
  currentTime = millis();

}

void loop() {
 
  while (Serial.available() > 0) {
    mySerial2.write(Serial.read());
    yield();
  }
  while (mySerial2.available() > 0) {
    Serial.write(mySerial2.read());
    yield();
  }

  unsigned long currentMillis = millis();
  
  if (currentMillis - lastSendTime >= sendInterval) {
    Pub("data",RandomData());
    lastSendTime = currentMillis;
  }
}
