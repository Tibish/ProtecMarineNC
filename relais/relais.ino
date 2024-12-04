#define RELAI1 42
#define RELAI2 1 
#define RELAI3 21 

void setup() {
  Serial.begin(9600);
  pinMode(RELAI1, OUTPUT); 
  pinMode(RELAI2, OUTPUT);
  pinMode(RELAI3, OUTPUT);

digitalWrite(RELAI1, HIGH);
}

void loop() {

  digitalWrite(RELAI3, HIGH);

  while (Serial.available()) 
  {
    char a = Serial.read(); 
    if (a == '1') // Si le caractère reçu est '1'
    { 
      digitalWrite(RELAI1, HIGH); 
      Serial.println(a);
      Serial.println("Pompe à air activée");
    }
    else if (a == '0') // Si le caractère reçu est '0'
    {
      digitalWrite(RELAI1, LOW);
      Serial.println(a);
      Serial.println("Pompe à air désactivée");
    }
    Serial.flush();
  }
  if (digitalRead(RELAI1) == LOW)
  { 
    digitalWrite(RELAI3, LOW);

    digitalWrite(RELAI2, HIGH); 
    delay(3000);
    digitalWrite(RELAI2, LOW);
    delay(3000);
  }

}
