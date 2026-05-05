#include <SoftwareSerial.h>

SoftwareSerial miBluetooth(10, 11);  // Pines 10 (RX) y 11 (TX)
const int ledPin = 13; // LED interno

void setup() {
  miBluetooth.begin(9600); 
  pinMode(ledPin, OUTPUT);
  miBluetooth.println("Sistema Listo. Envia '1' para encender, '0' para apagar.");
}

void loop() {
  if (miBluetooth.available()) { // Se verifica si el celular ha enviado algún dato por Bluetooth
    char comando = miBluetooth.read(); // Se lee el carácter recibido
    if (comando == '1') { 
      digitalWrite(ledPin, HIGH); // Se ENCIENDE el LED
      miBluetooth.println("Estado: LED ENCENDIDO");
    } 
    else if (comando == '0') {
      digitalWrite(ledPin, LOW); // Se APAGA el LED
      miBluetooth.println("Estado: LED APAGADO");
    }
  }
}
