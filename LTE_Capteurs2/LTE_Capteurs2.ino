#include <ProtecMarine_Comm3G.h>

#define IO_RXD2 17
#define IO_TXD2 18
#define DEBUG true

HardwareSerial mySerial2(2);
ProtecMarine_Comm3G comm3G(mySerial2, "user3", "User123456789.");

void setup() {
    Serial.begin(115200);
    mySerial2.begin(115200, SERIAL_8N1, IO_RXD2, IO_TXD2);

    comm3G.connectMQTT();
    comm3G.subscribe("action");
}

void loop() {
    comm3G.checkIncomingMessages();

    static unsigned long lastSendTime = 0;
    const unsigned long sendInterval = 120000;

    if (millis() - lastSendTime >= sendInterval) {
        comm3G.publish("data", comm3G.getData(34, 35));
        lastSendTime = millis();
    }
}
