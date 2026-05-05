\#include <SoftwareSerial.h>

SoftwareSerial mspSerial(10, 11); // Pines para UART (RX y TX)

void setup() {
  Serial.begin(115200);
  mspSerial.begin(115200); // Inicialización de puerto virtual para comunicar con el MSP430
  Serial.println("Arduino listo. Escribe algo para mandarlo al MSP430...");
}

void loop() {
  if (mspSerial.available()) { // Impresión de lectura de mensaje del MSP430
    char c = mspSerial.read();
    Serial.print(c);
  }
  if (Serial.available()) { // Lectura desde el monitor serial, y se envía al MSP430
    char c = Serial.read();
    mspSerial.print(c);
  }
}
