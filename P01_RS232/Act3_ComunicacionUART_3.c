#include <msp430.h>

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Detiene el perro guardián
  
  P3SEL |= BIT3 + BIT4; // Configura pines 3 y 4 del puerto 3
  UCA0CTL1 |= UCSWRST;  /// Poner el módulo UART (UCA0) en modo RESET. Se apaga para configurar la comunicación
  UCA0CTL1 |= UCSSEL_2; // Seleccionar el reloj SMCLK como fuente de reloj

  // -- CONFIGURACIÓN DE TRAMA CON PARIDAD--
  // Por defecto, el MSP430 ya está en 8 bits de datos (UC7BIT=0) y sin paridad (UCPEN=0).
  // Por defecto, el MSP430 usa un bit de Stop. Se agrega la configuración de 2 Bits de Stop:
  // UCPEN: Habilita paridad
  // UCPAR: Selecciona paridad par (1)
  // UCSPB: Lo dejamos en 0 para tener 1 solo Bit de Paro
  UCA0CTL0 |= UCPEN + UCPAR; 
  UCA0CTL0 &= ~UCSPB; // Aseguramos que sea 1 bit de Stop
                   

  UCA0BR0 = 9; // Configuración de baudios (115200)
  UCA0BR1 = 0; // Configuración de baudios 
  UCA0MCTL |= UCBRS_1 + UCBRF_0; // Control de modulación para evitar errores
  UCA0CTL1 &= ~UCSWRST; // Se libera el modo reset para habilitar la UART
  UCA0IE |= UCRXIE; // Se habilita la interrupción de recepción (RX)

  __bis_SR_register(LPM0_bits + GIE); // Habilita interrupciones generales y se pone en modo de bajo consumo
  __no_operation(); // Nada, señal para poner un breakpoint
}

// Rutina de Servicio de Interrupción (ISR)
// Hacer un "Echo" del carácter recibido
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void) // Rutina de Servicio de Interrupción: Cuando se interrumpe, salta a esta instrucción
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,4)) // Se revisa el vector de interrupciones
  {
  case 0:break; // Vector 0: Sin interrupción
  case 2: // Vector 2: RXIFG (Dato recibido)
    while (!(UCA0IFG & UCTXIFG)); // Esperar a que el buffer de transmisión (TX) esté libre
    UCA0TXBUF = UCA0RXBUF;// Rebota el mismo dato que se acaba de recibir
    break;
  case 4:break; // Vector 4: TXIFG (Transmisión completada) 
  default: break;
  }
}
