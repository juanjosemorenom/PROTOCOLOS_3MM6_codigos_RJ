int main(void)
{
  WDTCTL = WDTPW + WDTHOLD; // Detiene el perro guardián
 
  P3SEL |= BIT3+BIT4; // Configura pines 3 y 4 del puerto 3
  UCA0CTL1 |= UCSWRST; // Poner el módulo UART (UCA0) en modo RESET. Se apaga para configurar la comunicación
  UCA0CTL1 |= UCSSEL_2; // Seleccionar el reloj SMCLK para la UART
  UCA0BR0 = 9; // Configuración de baudios
  UCA0BR1 = 0; // Configuración de baudios
  UCA0MCTL |= UCBRS_1 + UCBRF_0; // Control de modulación para evitar errores
  UCA0CTL1 &= ~UCSWRST; // Se saca a la UART del modo RESET
  UCA0IE |= UCRXIE; // Se habilita la interrupción de recepción (RX)
 
  __bis_SR_register(LPM0_bits + GIE); // Habilita interrupciones generales y se pone en modo de bajo consumo
  __no_operation(); // Nada, señal para poner un breakpoint
}
 
// Echo back RXed character, confirm TX buffer is ready first
// Rutina para cuando el módulo UART genere una interrupción
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
  case 0:break;
  case 2: // Si se recibió un dato
    while (!(UCA0IFG&UCTXIFG)); // Revisa si el buffer de envío está vacío, hasta que se desocupe
    UCA0TXBUF = UCA0RXBUF; // Pasa el dato del buffer de recepción al buffer de transmisión
    break;
  case 4:break;
  default: break;
  }
}
