HardwareSerial mySerial2(2);

#define DEBUG true


#define IO_RXD2 17
#define IO_TXD2 18


// change this to match your SD shield or module;
// ESP32-S3 4G LTE: pin 10
const int PIN_SD_SELECT = 10;

#define IO_GSM_PWRKEY 4
#define IO_GSM_RST    5

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


void setup() {
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
}
