#include "ProtecMarine_Wifi.h"

ProtecMarine_Wifi::ProtecMarine_Wifi(const char* ssid, const char* password, int pressionPin, int potPin)
    : _ssid(ssid), _password(password), _pressionPin(pressionPin), _potPin(potPin), _server(80), _etat("arrêté"), _potValue(0), _tension(0.0) {}


String  ProtecMarine_Wifi::getDefaultMacAddress() {
    String mac = "";
    unsigned char mac_base[6] = {0};
    if (esp_efuse_mac_get_default(mac_base) == ESP_OK) {
        char buffer[18];
        sprintf(buffer, "%02X-%02X-%02X-%02X-%02X-%02X", mac_base[0], mac_base[1], mac_base[2], mac_base[3], mac_base[4], mac_base[5]);
        mac = buffer;
    }
    return mac;
}

void ProtecMarine_Wifi::setupWifi() {
    if (!LittleFS.begin()) {
        Serial.println("Erreur de montage LittleFS !");
        return;
    }
    Serial.println("LittleFS monté avec succès.");

    WiFi.softAP(_ssid, _password);

    Serial.println("Point d'accès créé avec succès");
    Serial.print("SSID: ");
    Serial.println(_ssid);
    Serial.print("Adresse IP: ");
    Serial.println(WiFi.softAPIP());

    // Configurer les routes
    _server.on("/", [this]() { handleRoot(); });
    _server.on("/getdata", [this]() { handleGetData(); });
    _server.on("/image", HTTP_GET, [this]() { handleImage(); });
    _server.on("/gonflage", [this]() { handleGonflage(); });
    _server.on("/degonflage", [this]() { handleDegonflage(); });
    _server.on("/arret", [this]() { handleArret(); });

    _server.begin();
    Serial.println("Serveur web démarré");
}

void ProtecMarine_Wifi::disconnectWifi() {
    WiFi.softAPdisconnect(true);
}

void ProtecMarine_Wifi::handleRequests() {
    _server.handleClient();
}

void ProtecMarine_Wifi::handleImage() {
    File file = LittleFS.open("/logo-ProtecMarineNC3.png", "r");
    if (!file) {
        _server.send(404, "text/plain", "Image non trouvée");
        return;
    }
    _server.streamFile(file, "image/jpeg");
    file.close();
}

void ProtecMarine_Wifi::handleRoot() {
    String htmlPage = R"rawliteral(
    <!DOCTYPE html>
    <html lang="fr">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>ProtecMarineNC</title>
      <style>
        h1 { text-align: center; font-family: Arial, Helvetica, sans-serif; }
        img { display: block; margin: 0 auto; max-width: 100%; height: auto; }
        #dataDisplay { text-align: center; }
        button { font-size: 18px; padding: 10px 20px; display: block; margin: 10px auto; }
        #etat { text-align: center; background-color: #007bff; color: white; padding: 10px 20px; margin: 10px auto; }
        #arretButton { background-color: red; color: white; }
        #tension, #pression { background-color: #f0f0f0; padding: 10px; margin-top: 20px; text-align: center; border-radius: 5px; }
      </style>
    </head>
    <body>
      <img src="/logo-ProtecMarineNC3.png" alt="Logo">
      <h1>Protection de coque</h1>
      <div id="dataDisplay">
        <p>Tension : </p>
        <p><span id="tension"></span> V</p>
        <p>Pression :</p>
        <p><span id="pression"></span> bars</p>
        <p>ID : <span id="id"></span></p>
        <p><span id="etat"></span></p>
        <p>Alerte : <span id="alerte"></span></p>
      </div>

      <button onclick="sendAction('/gonflage')">Gonflage</button>
      <button onclick="sendAction('/degonflage')">Dégonflage</button>
      <button id="arretButton" onclick="sendAction('/arret')">Arrêt</button>

      <script>
        function getData() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
            if (this.readyState == 4 && this.status == 200) {
              var data = JSON.parse(this.responseText);
              document.getElementById('tension').innerHTML = data.tension;
              document.getElementById('alerte').innerHTML = data.alerte;
              document.getElementById('pression').innerHTML = data.pression;
              document.getElementById('id').innerHTML = data.id;
              document.getElementById('etat').innerHTML = data.etat;
            }
          };
          xhttp.open('GET', '/getdata', true);
          xhttp.send();
        }

        function sendAction(actionUrl) {
          var xhttp = new XMLHttpRequest();
          xhttp.open('GET', actionUrl, true);
          xhttp.send();
        }

        setInterval(getData, 1000);
      </script>
    </body>
    </html>
    )rawliteral";

    _server.send(200, "text/html", htmlPage);
}


void ProtecMarine_Wifi::handleGetData() {
    String data = GetData();
    _server.send(200, "application/json", data);
}

void ProtecMarine_Wifi::handleGonflage() {
    _etat = "gonflage en cours";
    Serial.println("Gonflage activé !");
    _server.send(200, "text/plain", "Gonflage activé");
}

void ProtecMarine_Wifi::handleDegonflage() {
    _etat = "dégonflage en cours";
    Serial.println("Dégonflage activé !");
    _server.send(200, "text/plain", "Dégonflage activé");
}

void ProtecMarine_Wifi::handleArret() {
    _etat = "arrêté";
    Serial.println("Arrêt activé !");
    _server.send(200, "text/plain", "Arrêt activé");
}

String ProtecMarine_Wifi::GetData() {
    StaticJsonDocument<200> doc;
    String Data;
    String id = getDefaultMacAddress();
    int alerte = random(0, 4);

    // Mesure de pression
    int mesureBrute = analogRead(_pressionPin);
    float tensionP = mesureBrute * (3.3 / 4095.0);
    float pression = (tensionP * 2 + 4.42) * (1000.0 / (4.7 - 0.2));

    // Mesure du potentiomètre
    _potValue = analogRead(_potPin);
    _tension = (_potValue * 12.0) / 4095;

    doc["id"] = id;
    doc["alerte"] = alerte;
    doc["tension"] = _tension;
    doc["pression"] = pression;
    doc["etat"] = _etat;

    serializeJson(doc, Data);
    return Data;
}
