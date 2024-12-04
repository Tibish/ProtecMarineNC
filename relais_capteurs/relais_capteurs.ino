const int RELAI1 = 42; //pompe a air
const int RELAI2 = 1; // pompe de calle
const int RELAI3 = 21; // electrovanne

const int pressionPin = 34;  // GPIO de l'entrée analogique pour la pression
const int potPin = 35;       // GPIO de l'entrée analogique pour le potentiomètre

int potValue = 0;            // Valeur brute du potentiomètre
float voltage = 0.0;         // Valeur de la tension calculée

// Prototypes des fonctions
float hpaToBar(float pressionHpa);
float barToHpa(float pressionBar);

void setup() {
  Serial.begin(115200);      // Initialisation de la communication série
  delay(1000);               // Délai pour stabiliser la connexion série
  Serial.println("Initialisation du système...");
   // Configuration de la broche du potentiomètre en entrée

  digitalWrite(RELAI1, HIGH);
}

void loop() {
  // Mesure de la pression
  int mesureBrute = analogRead(pressionPin);  // Lecture de la valeur analogique brute
  float tension = mesureBrute * (3.3 / 4095.0);  // Conversion de la valeur brute en tension pour une alimentation de 3,3 V
  float pression = (tension * 2 + 4.42) * (1000.0 / (3.0 - 0.2));  // Conversion de la tension en pression (hPa)

  // Mesure du potentiomètre
  potValue = analogRead(potPin);  // Lecture de la valeur brute du potentiomètre
  voltage = (potValue * 12.0) / 4095;  // Conversion de la valeur brute en tension (0-12V avec pont diviseur)

  // Affichage des résultats
  Serial.println("=================================");
  Serial.print("Tension de pression: ");
  Serial.print(tension);
  Serial.print(" V, Pression: ");
  Serial.print(pression);
  Serial.print(" hPa | Potentiomètre - Valeur brute: ");
  Serial.print(potValue);
  Serial.print(" Tension: ");
  Serial.print(voltage);
  Serial.println(" V");
  Serial.println("=================================");
  // Contrôle des relais
  digitalWrite(RELAI3, HIGH);

  while (Serial.available()) {
    char a = Serial.read(); 
    if (a == '1') { // Si le caractère reçu est '1'
      digitalWrite(RELAI1, HIGH); 
      Serial.println(a);
      Serial.println("Pompe à air activée");
    } else if (a == '0') { // Si le caractère reçu est '0'
      digitalWrite(RELAI1, LOW);
      Serial.println(a);
      Serial.println("Pompe à air désactivée");
    }
    Serial.flush();
  }

  if (digitalRead(RELAI1) == LOW) { 
    digitalWrite(RELAI3, LOW);
    digitalWrite(RELAI2, HIGH); 
    delay(3000);
    digitalWrite(RELAI2, LOW);
    delay(3000);
  }

  delay(1000);  // Pause de 1 seconde entre chaque boucle
}

// Fonction pour convertir hPa -> bar
float hpaToBar(float pressionHpa) {
  return pressionHpa / 1000.0;  // 1 hPa = 0.001 bar
}

// Fonction pour convertir bar -> hPa
float barToHpa(float pressionBar) {
  return pressionBar * 1000.0;  // 1 bar = 1000 hPa
}
