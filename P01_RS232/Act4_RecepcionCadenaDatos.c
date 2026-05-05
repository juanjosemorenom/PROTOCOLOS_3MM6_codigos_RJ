#include <msp430.h>
#include <stdint.h>

#define MAX_BUFFER 15  // Constante para tamaño máximo del mensaje
// Variables leídas directamente de memoria, por cambios instantáneos
volatile char rxBuffer[MAX_BUFFER]; // Arreglo para almacenar letras
volatile uint8_t rxIndice = 0; // Contador que indica la posición de la siguiente letra
volatile uint8_t rxListo = 0; // Bandera de estado

// Función para recibir una cadena de caracteres
void sendString(const char *str) {
    while (*str) { // Mientras la letra actual no sea el final invisible del texto
        while (!(UCA0IFG & UCTXIFG)); // Esperar a que el buffer de transmisión (TX) esté libre
        UCA0TXBUF = *str++; // Asigna la letra actual para enviarla, y avanza a la siguiente letra
    }
}

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Detiene el perro guardián
    // --- CONFIGURACIÓN UART (115200, 8 Bits, 1 Stop, Sin Paridad) ---
    P3SEL |= BIT3 + BIT4; // Configura pines 3 y 4 del puerto 3
    UCA0CTL1 |= UCSWRST; // Poner el módulo UART (UCA0) en modo RESET. Se apaga para configurar la comunicación
    UCA0CTL0 = 0;  // Nos aseguramos de borrar cualquier rastro de paridad
    UCA0CTL0 &= ~UCSPB; // Aseguramos que sea 1 bit de Stop
    UCA0CTL1 |= UCSSEL_2; // Seleccionar el reloj SMCLK como fuente de reloj
    UCA0BR0 = 9; // Configuración de baudios (115200)
    UCA0BR1 = 0; // Configuración de baudios
    UCA0MCTL |= UCBRS_1 + UCBRF_0; // Control de modulación para evitar errores
    UCA0CTL1 &= ~UCSWRST; // Se libera el modo reset para habilitar la UART
    UCA0IE |= UCRXIE; // Se habilita la interrupción de recepción (RX)

    // ---CONFIGURACIÓN TIMER_A0---
    TA0CTL = TASSEL_2 + ID_3 + MC_0 + TACLR; // Configurar el reloj del temporizador: TASSEL_2: Usa el reloj interno (SMCLK), MC_0: Mantener el temporizador detenido por ahora
    //  ID_3: Divide la velocidad del reloj entre 8 para que no cuente tan rápido, TACLR: Borra cualquier cuenta vieja a cero
    TA0CCR0 = 13100; // Límite de tiempo. Si el temporizador arranca y llega a contar hasta 13100, significa que pasó mucho tiempo sin recibir datos
    TA0CCTL0 = CCIE; // Enciende la alarma de este temporizador

    __bis_SR_register(GIE); // Habilita interrupciones generales y se pone en modo de bajo consumo

    while(1) // Ciclo infinito
    {
        if (rxListo > 0) { // Si ya se terminó de recibir un mensaje
            __delay_cycles(50000);
            sendString("OK. Fin de lectura por: ");

            if(rxListo == 1) sendString("Salto de linea (\\n)\r\n");
            if(rxListo == 2) sendString("Temporizador (Timeout)\r\n");
            if(rxListo == 3) sendString("Desbordamiento (Overflow)\r\n");

            sendString("Datos recibidos: ");
            sendString((char*)rxBuffer);
            sendString("\r\n\r\n");

            rxIndice = 0; // Reinicializar contador
            rxBuffer[0] = '\0'; // Borra virtualmente el primer carácter, indicando que el arreglo está vacía
            rxListo = 0; // Bandera de que se está listo para recibir un nuevo mensaje
        }
    }
}

// --- INTERRUPCIÓN DE RECEPCIÓN UART ---
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch(__even_in_range(UCA0IV,4)) // Entra aquí en cuanto se recibe un carácter y se revisa el vector de interrupciones
    {
        case 2: // Dato recibido en RX
        {
            P1OUT ^= BIT0;
            char c = UCA0RXBUF;  //Almacenamos el carácter que ha llegado en otra variable

            if (rxListo == 0) { // Se verifica que no se siga procesando el mensaje anterior
                // CONDICIÓN 1: Salto de línea
                if (c == '\n' || c == '\r') { // Si el carácter que llegó es un "ENTER"
                    rxBuffer[rxIndice] = '\0'; // Fin de la cadena de texto
                    rxListo = 1; // Bandera = 1
                    TA0CTL &= ~MC_3; // Se detiene el temporizador
                }
                else { // Si el carácter no es un salto de línea
                    rxBuffer[rxIndice++] = c; // Se aumenta el contador de letras

                    // CONDICIÓN 3: Desbordamiento de buffer
                    if (rxIndice >= MAX_BUFFER - 1) { // Si llega a 15 carácteres
                        rxBuffer[rxIndice] = '\0'; // Fin de cadena de texto
                        rxListo = 3; // Bandera = 3
                        TA0CTL &= ~MC_3; // Se detiene el temporizador
                    }
                    else { // Si aún no se desborda el buffer
                        // CONDICIÓN 2: Reiniciar el timer
                        TA0CTL |= TACLR; // Reiniciar el timer
                        TA0CTL |= MC_1; // Encenderlo nuevamanete para que empiece a contar
                    }
                }
            }
            break;
        }
        default: break;
    }
}

// --- INTERRUPCIÓN DEL TEMPORIZADOR ---
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A_ISR(void) // El timer ejecuta estpa interrupción una vez que llega a 13100
{
    TA0CTL &= ~MC_3; // Apaga el temporizador
    rxBuffer[rxIndice] = '\0'; // Fin de cadena de caracteres
    rxListo = 2; // Bandera = 2
}
