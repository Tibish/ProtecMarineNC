const int pressionPin = 34;  // GPIO de l'entrée analogique pour la pression
const int potPin = 35;       // GPIO de l'entrée analogique pour le potentiomètre

int potValue = 0;            // Valeur brute du potentiomètre
float voltage = 0.0;         // Valeur de la tension calculée

void setup() {
  Serial.begin(115200);      // Initialisation de la communication série
  delay(1000);               // Délai pour stabiliser la connexion série
  pinMode(potPin, INPUT);    // Configuration de la broche du potentiomètre en entrée
}

void loop() {
  // Mesure de la pression
  int mesureBrute = analogRead(pressionPin);  // Lecture de la valeur analogique brute
  float tension = mesureBrute * (3.3 / 1023.0);  // Conversion de la valeur brute en tension pour une alimentation de 3,3 V
  float pression = (tension * 2 + 4.42) * (1000.0 / (4.7 - 0.2));  // Conversion de la tension en pression (hPa)

  // Mesure du potentiomètre
  potValue = analogRead(potPin);  // Lecture de la valeur brute du potentiomètre
  voltage = (potValue * 12.0) / 4095;  // Conversion de la valeur brute en tension (0-12V avec pont diviseur)

  // Affichage des résultats
  Serial.print("Tension de pression: ");
  Serial.print(tension);
  Serial.print(" V, Pression: ");
  Serial.print(pression);
  Serial.print(" hPa | Potentiomètre - Valeur brute: ");
  Serial.print(potValue);
  Serial.print(" Tension: ");
  Serial.print(voltage);
  Serial.println(" V");

  delay(1000);  // Pause de 1 seconde entre chaque mesure
}
