#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

const char* ssid = "ProtecMarineNC";
const char* password = "12345678";

WebServer server(80);

String etat = "arrêté";  // État initial

const int pressionPin = 34;  // GPIO de l'entrée analogique pour la pression
const int potPin = 35;       // GPIO de l'entrée analogique pour le potentiomètre

int potValue = 0;            // Valeur brute du potentiomètre
float tension = 0.0;         // Valeur de la tension calculée

String RandomData() {
  StaticJsonDocument<200> doc;
  String Data;
  float tension = random(220, 230) + random(0, 100) / 100.0;
  int alerte = random(0, 4);
  float pression = random(1, 10) + random(0, 100) / 100.0;
  String id = WiFi.softAPmacAddress();

  doc["tension"] = tension;
  doc["alerte"] = alerte;
  doc["pression"] = pression;
  doc["id"] = id;
  doc["etat"] = etat;  // Inclure l’état actuel dans la réponse

  serializeJson(doc, Data);
  return Data;
}

String GetData(){
  StaticJsonDocument<200> doc;
  String Data;
  String id = WiFi.softAPmacAddress();
  int alerte = random(0, 4);

  //mesure de pression
  int mesureBrute = analogRead(pressionPin);  // Lecture de la valeur analogique brute
  float tensionP = mesureBrute * (3.3 / 4095.0);  // Conversion de la valeur brute en tension pour une alimentation de 3,3 V
  float pression = (tensionP * 2 + 4.42) * (1000.0 / (4.7 - 0.2));  // Conversion de la tension en pression (hPa)

  // Mesure du potentiomètre
  potValue = analogRead(potPin);  // Lecture de la valeur brute du potentiomètre
  tension = (potValue * 12.0) / 4095;  // Conversion de la valeur brute en tension (0-12V avec pont diviseur)

  
  doc["id"] = id;
  doc["alerte"] = alerte;
  doc["tension"] = tension;
  doc["pression"] = pression;
   doc["etat"] = etat;

  serializeJson(doc, Data);
  return Data;
}

void handleRoot() {
  String htmlPage = "<!DOCTYPE html><html lang=\"fr\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>ProtecMarineNC</title><style>";
  htmlPage += "h1{text-align: center;font-family: Arial, Helvetica, sans-serif;}p{text-align: center;font-size: x-large;font-family: Arial, Helvetica, sans-serif;}button{font-size: 18px;padding: 10px 20px;display: block;margin: 10px auto;}#etat{text-align: center;background-color: #007bff;color: white;padding: 10px 20px;margin : 10px auto;}#arretButton{background-color: red;color: white;}#tension, #pression{background-color: #f0f0f0;padding: 10px;margin-top: 20px;text-align: center;border-radius: 5px;}</style></head><body>";
  htmlPage += "<img src=\"logo-ProtecMarineNC3.png\" alt=\"Image Description\" style=\"display: block; margin: 0 auto;\">";
  htmlPage += "<h1>Protection de coque</h1><div id=\"dataDisplay\"><p>Tension : </p><p><span id=\"tension\"></span> V</p><p>Pression :</p><p> <span id=\"pression\"></span> bars</p><p>ID : <span id=\"id\"></span></p><p><span id=\"etat\"></span></p><p>Alerte : <span id=\"alerte\"></span></p></div>";
  htmlPage += "<button onclick=\"sendAction('/gonflage')\">Gonflage</button><button onclick=\"sendAction('/degonflage')\">Dégonflage</button><button id=\"arretButton\" onclick=\"sendAction('/arret')\">Arrêt</button>";
  htmlPage += "<script>function getData() {var xhttp = new XMLHttpRequest();xhttp.onreadystatechange = function() {if (this.readyState == 4 && this.status == 200) {";
  htmlPage += "var data = JSON.parse(this.responseText);document.getElementById('tension').innerHTML = data.tension;document.getElementById('alerte').innerHTML = data.alerte;";
  htmlPage += "document.getElementById('pression').innerHTML = data.pression;document.getElementById('id').innerHTML = data.id;document.getElementById('etat').innerHTML = data.etat;}};";
  htmlPage += "xhttp.open('GET', '/getdata', true);xhttp.send();}function sendAction(actionUrl) {var xhttp = new XMLHttpRequest();xhttp.open('GET', actionUrl, true);xhttp.send();}";
  htmlPage += "setInterval(getData, 1000);</script></body></html>";

  server.send(200, "text/html", htmlPage);
}

void handleGetData() {
  String data = GetData();
  server.send(200, "application/json", data);
}

void handleGonflage() {
  etat = "gonflage en cours";
  Serial.println("Gonflage activé !");
  server.send(200, "text/plain", "Gonflage activé");
}

void handleDegonflage() {
  etat = "dégonflage en cours";
  Serial.println("Dégonflage activé !");
  server.send(200, "text/plain", "Dégonflage activé");
}

void handleArret() {
  etat = "arrêté";
  Serial.println("Arrêt activé !");
  server.send(200, "text/plain", "Arrêt activé");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.softAP(ssid, password);

  Serial.println("Point d'accès créé avec succès");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Adresse IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/getdata", handleGetData);
  server.on("/gonflage", handleGonflage);
  server.on("/degonflage", handleDegonflage);
  server.on("/arret", handleArret);

  server.begin();
  Serial.println("Serveur web démarré");
}

void loop() {
  server.handleClient();
}
