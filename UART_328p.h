#ifndef USART_328P_H
#define USART_328P_H

#include <Arduino.h>
#include <avr/io.h>							/* Include AVR std. library file */
#include <avr/interrupt.h>

#include "UART_328p.h"

#define ASYNCHRONOUS (0<<UMSEL00) // USART Mode Selection

#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED // USART Parity Bit Selection

#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT      // USART Stop Bit Selection

#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT  // USART Data Bit Selection

#define RX_COMPLETE_INTERRUPT         (1<<RXCIE0)
#define DATA_REGISTER_EMPTY_INTERRUPT (1<<UDRIE0)

void USART_Init(unsigned long buadR);				/* USART initialize function */
char USART_RxChar(void);						/* Data receiving function */
void USART_TxChar(char data);					/* Data transmitting function */
void USART_SendString(char* str);				/* Send string of USART data function */

#endif /* USART_328P_H */