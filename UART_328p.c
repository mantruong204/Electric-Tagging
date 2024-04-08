#include "UART_328p.h"

void USART_Init(unsigned long buadR)				/* USART initialize function */
{
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);				
	/* Enable USART transmitter and receiver with RX Interupt*/
	uint8_t br_prescale = (((F_CPU / (buadR * 16UL))) - 1);	/* Define prescale value */
	UBRR0L = br_prescale;							/* Load UBRRL with lower 8 bit of prescale value */
	UBRR0H = (br_prescale >> 8);					/* Load UBRRH with upper 8 bit of prescale value */
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
}

char USART_RxChar()									/* Data receiving function */
{
	while (!(UCSR0A & (1 << RXC0)));					/* Wait until new data receive */
	return(UDR0);									/* Get and return received data */
}

void USART_TxChar(char data)						/* Data transmitting function */
{
	while (!(UCSR0A & (1<<UDRE0)));					/* Wait until data transmit and buffer get empty */
	UDR0 = data;										/* Write data to be transmitting in UDR */
}

void USART_SendString(char *str)					/* Send string of USART data function */
{
	int i=0;
	while (str[i]!=0)
	{
		USART_TxChar(str[i]);						/* Send each char of string till the NULL */
		i++;
	}
}