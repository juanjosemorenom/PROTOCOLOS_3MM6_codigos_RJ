void setup() {
  Serial.begin(115200, SERIAL_8N2); // 8 bits, Sin paridad, 2 bits de Stop
}

void loop() {
  Serial.write('e');  // Envío de la letra 'e'
  delay(10);
}