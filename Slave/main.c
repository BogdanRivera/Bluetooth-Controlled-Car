/*
 * bluetooth_slave.c
 *
 * Created: 08/02/2023 08:23:10 a. m.
 * Author : bugy1
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

uint8_t led_On = 0;

ISR (TWI_vect){
	uint8_t dato, edo;
	edo = TWSR & 0xFC;
	switch(edo){
		case 0x60:		//Direccionado con su SLA
		case 0x70: TWCR |= 1 << TWINT;
		break;
		case 0x80:
		case 0x90:	dato = TWDR;
		if(dato == 0x05) led_On=1;
		else led_On = 0;
		TWCR |= 1 <<TWINT;
		break;
		
		default: TWCR |= (1<<TWINT) | (1<<TWSTO);
	}
	
	
}


int main(void)
{
	//SPI
	uint8_t i; 
	char c;
	DDRB = 0b00010000; 
	SPCR = 0x44;
	SPSR = 0x00; 
	i = 0;
	do{  
		while ( ! ( SPSR & 1 << SPIF ) ) ;
		c = SPDR; //
		PORTD = (1<<PD1) | PORTD;
		i = i + 1;
	}while(c!=0x00);	//Espera respuesta del esclavo
	
	SPDR = i;
	while ( ! ( SPSR & 1 << SPIF ) ) ;
	c = SPDR;
	PORTD = (1<<PD1) | PORTD;
	//TWI
	uint8_t dir = 0x12; //Dirección del eslavo
	DDRD = 0xFF; //Salida para el dato
	
	dir <<= 1; //Ubica la dirección y habilita para reconocer el dato
	dir |= 0x01;
	TWAR = dir;
	//Habilita la interfaz, con reconocimiento e interrupción
	TWCR = (1 << TWEA) | (1 << TWEN) | (1 << TWIE) ;
	sei();
	while (1)
	{
		if (led_On == 1) PORTD = (1<<PD0) | PORTD; //Enciende led
		else PORTD = ~(1<<PD0) & PORTD; //PD5 Apagado //Apaga led
	}
}
