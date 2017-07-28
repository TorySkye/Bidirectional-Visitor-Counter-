/*
 * Bidirectional Visitor Counter.c
 *
 * Created: 28-Jul-17 1:57:53 PM
 * Author : Viktorija Alilovic
 */ 

/* Includes io.h header file where all the Input/Output Registers and its Bits are defined for all AVR microcontrollers */
#include <avr/io.h>
/* Defines a macro for the delay.h header file. F_CPU is the microcontroller frequency value for the delay.h header file */
#define	F_CPU	8000000
/* Includes delay.h header file which defines two functions, _delay_ms (millisecond delay) and _delay_us (microsecond delay) */
#include <util/delay.h>
#include <stdlib.h>
/* Includes adc.h header file which defines different functions for Analog to Digital Converter */
#include "adc.h"
#include "lcd.h"

/* Variable declarations */
uint16_t ch2_ref;
uint16_t ch3_ref;
uint16_t ch2_read;
uint16_t ch3_read;
char ch2_string[20];
char ch3_string[20];
uint8_t data;
uint16_t ADC_ch2_value;
uint16_t ADC_ch3_value;
char num_in[20];
char num_out[20];
uint8_t counter = 0;

/* Function declarations */
void ADC_ch2();
void ADC_ch3();

void USART_init (uint16_t baud)
{
	uint16_t baudPrescaler;
	baudPrescaler = (F_CPU/(16UL*baud))-1;
	UBRRH = (uint8_t)(baudPrescaler>>8);
	UBRRL = (uint8_t)baudPrescaler;
	UCSRA &= ~(1<<U2X);
	UCSRB = (1<<RXEN)|(1<<TXEN);
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);
}
void USART_send (uint8_t data)
{
	while (!(UCSRA &(1<<UDRE)));
	UDR=data;
}

uint8_t USART_receive (void)
{
	while (!(UCSRA&(1<<RXC)));
	return UDR;
}

void USART_message (char *data)
{
	while ( *data != '\0')
	USART_send(*data++);
}

int main(void)
{
	DDRD = 0b00110000;						           /* Configure PortD as output */
	PORTD = 0x00;							               /* Set LOW value to PORT D */
	DDRA = 0x00;							               /* Configure PortA as input */
	
	USART_init (9600);
	adc_init();								               /* ADC initialization */
	lcd_init(LCD_DISP_ON);
	
	lcd_gotoxy(0,0);						            /* Set cursor to 1. row (row0) and 1. character */
	lcd_puts("Dobrodosli");
	_delay_ms(1000);
	
	ch2_read = read_adc_channel(2);			    /* Converting analog output of Digital light sensor */
	sprintf(ch2_string, "Referentna vrijednost 1. fotootpornika: %d  \r\n", ch2_read);
	USART_message(ch2_string);
	ch2_ref = ch2_read;

	ch3_read = read_adc_channel(3);			    /* Converting analog output of Digital light sensor */
	sprintf(ch3_string, "Referentna vrijednost 2. fotootpornika: %d  \r\n", ch3_read);
	USART_message(ch3_string);
	ch3_ref = ch3_read;
	
	USART_send(10);
	USART_send(13);

	/* Start of infinite loop */
	while (1)
	{
		ADC_ch2();
		ADC_ch3();
	
		if (ADC_ch2_value < (ch2_ref - 100))
		{
			if (counter > 0)
			{
				counter--;
			}
			else if (counter == 0)
			{
				counter = 0;
			}
			PORTD |= (1<<PD4);
			lcd_gotoxy(0,0);				            /* Set cursor to 1. row (row0) and 1. character */
			lcd_puts("Smjer: <--");
			sprintf(num_in, "Broj osoba: %d ", counter);
			lcd_gotoxy(0,1);				            /* Set cursor to 2. row (row1) and 1. character */
			lcd_puts(num_in);
			_delay_ms(1000);
		}
		
		else if (ADC_ch3_value < (ch3_ref - 100))
		{
			counter++;		
			PORTD |= (1<<PD5);
			lcd_gotoxy(0,0); 
			lcd_puts("Smjer: -->");
			sprintf(num_out, "Broj osoba: %d", counter);
			lcd_gotoxy(0,1); 
			lcd_puts(num_out);
			_delay_ms(1000);
		}
		else
		{
			PORTD = 0x00;
		}
		ch2_last = ADC_ch2_value;
		ch3_last = ADC_ch3_value;
	}
	return 0;
}

void ADC_ch2()
{
	ADC_ch2_value = read_adc_channel(2);	/* Converting analog output of Digital light sensor */	
	sprintf(ch2_string, "Vrijednost 1. fotootpornika: %d  \r\n", ADC_ch2_value);
	USART_message(ch2_string);
}

void ADC_ch3()
{
	ADC_ch3_value=read_adc_channel(3);		/*Converting analog output of Digital light sensor*/
	sprintf(ch3_string, "Vrijednost 2. fotootpornika: %d  \r\n", ADC_ch3_value);
	USART_message(ch3_string);
	USART_send(10);
	USART_send(13);
}