/*
 * Proyectofinal.c
 *
 * Created: 07/02/2022 13:12:00 a. m.
 * Author : @bogdanRivera
 */ 
#define VMAX 65434
#define F_CPU 1000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#define DIR_SLAVE 0x12 
#include "TWI.h"

uint8_t led_On(uint8_t dato);
uint8_t envia_SPI(uint8_t dato);



ISR(USART_RX_vect){
	uint8_t xd;
	uint8_t stat; 
	xd = UDR0; 
	
	switch(xd){
		case '1': //Se desplaza hacia adelante 
			PORTB = (1<<PB0) | PORTB; 
			PORTC = (1<<PB3) | PORTC;
			PORTC = ~(1<<PC2) & PORTC; 
			PORTB = ~(1<<PB7) & PORTB;
			break; 
				
		case '2': //Se desplaza hacia atras
			PORTC = (1<<PC2) | PORTC;
			PORTB = (1<<PB7) | PORTB;
			PORTB = ~(1<<PB0) & PORTB;
			PORTC = ~(1<<PC3) & PORTC;
			break; 
				
		case '3': //Detenido
			PORTB = ~(1<<PB0) & PORTB;
			PORTC = ~(1<<PC2) & PORTC;
			PORTC = ~(1<<PC3) & PORTC;
			PORTB = ~(1<<PB7) & PORTB;
			break;
				 
		case '4': //Se desplaza hacia la derecha
			PORTB = (1<<PB0) | PORTB; 
			PORTC = ~(1<<PC2) & PORTC;
			PORTC = ~(1<<PC3) & PORTC;
			PORTB = ~(1<<PB7) & PORTB;
			break;
		
		case '5': //Se desplaza a la izquierda
			PORTC = (1<<PC3) | PORTC;
			PORTC = ~(1<<PC2) & PORTC;
			PORTB = ~(1<<PB0) & PORTB;
			PORTB = ~(1<<PB7) & PORTB;
			break;
				
		case '6': 
				OCR1A = VMAX*0.25; //Para el 25%
				break; 
				
		case '7':
			OCR1A = VMAX*0.50; //Para el 50%
			break;
			
		case '8':
			OCR1A = VMAX*0.75; //Para el 75%
			break;
			
		case '9':
			OCR1A = VMAX; //Para el 100%
			break;
		case 'A':
		stat = led_On(0x05); //Envía el dato para encender el led en el otro micro
		if(stat==1) asm("NOP");
		else break;
		break; 
		
		case 'B':
		stat = led_On(0x06); //Envía el dato para apagar el led en el otro micro
		if(stat==1) asm("NOP");
		else break;
		break;
		

		
		
	}
	
}

ISR (TIMER0_COMPA_vect){
	ADCSRA |= 1<<ADSC;
}

ISR(ADC_vect){
	uint16_t aux;
	aux = ADCW;
	if (aux>500) PORTB = (1<<PB6) | PORTB; //PB6 Encendido
	else PORTB = ~(1<<PB6) & PORTB; //PB6 Apagado
}

ISR(INT0_vect){
	PORTD = (1<<PD5) | PORTD; //PD5 Encendido
}

ISR(PCINT2_vect){
	if((PIND & 0x10))
	PORTD = ~(1<<PD5) & PORTD; //PD5 Apagado
}

int main(void)
{
	uint8_t i,r; 
	char c;
	char cadena[] = "Cadena de prueba"; 
	
	
	//Configuración de los puertos de entrada y salida
	DDRD = 0xF0; // 
	DDRB = 0xFF; //PB1 como salida PWM en OC1A
	DDRC = 0x0F; //PC0-PC3 como salida (Para control de estados lógicos de los motores) y led
	
	//Configuración de la USART
	UBRR0 = 12; //Ajustado para doble velocidad a 9600 bpm
	UCSR0A = 0X02; //a doble velocidad operacion asincrona
	UCSR0B = 0x90; //Interrupcion RXCIE, habilita RXD, 8 bits
	UCSR0C = 0x06; //Asíncrono, sin bit de paridad, 1 bit de paro, flanco de bajada
	
	//Configuración del timer 0 para mediciones de 0.1 segundos
	TCCR0A = 0x00;
	TCCR0B = 0x05; //Preescaler de 1024
	TIMSK0 = 0x02; //Habilita la interrupción por coincidencia
	OCR0A = 244; //
	
	//Configuración para el ADC
	ADMUX = 0x00; //Selecciona el ADC0 y Vref en AREF
	ADCSRA = 0xCB; //Habilita ADC con interrupción, inicia conversión y divide entre 8
	ADCSRB = 0x00; //El adc tiene carrera libre
	DIDR0 = 0x01; //Anula el buffer digital en ADC1
	
	//Configura INT0
	EICRA = 0X03; //Int0
	EIMSK = 0x01; //Activación de la INT0
	
	//Por interrupción en puertos
	PCMSK2 = 0x10; //PD4
	PCICR = 0x04; //
	
	//Configuración del timer 1
	ICR1 = VMAX;
	TCCR1A = 0x82;
	TCCR1B = 0x19;
	TCCR1C = 0x00;
	OCR1A = VMAX;
	
	//Configura la TWI
	TWI_Config();
	
	//Configuración de la SPI
	PORTB = (1<<PB2) | PORTB; //SS en alto
	SPCR = 0x55; // SPI como maestro en modo 1
	SPSR = 0x01 ; // A 125 kHz
	
	_delay_ms(100); //Espera a que el esclavo esté listo
	i=0;
	
	do{  // Envía caracteres hasta encontrar el caracter nulo
		c = cadena[i]; //
		envia_SPI(c);
		i = i + 1;
	}while(c!=0x00);	//Espera respuesta del esclavo
	_delay_ms(100); //Espera a que el esclavo esté listo
	r = envia_SPI(0x00);	//Solicita respuesta	
	

	
	sei(); //Habilitador global de interrupciones
    while (1) 
    {
    }
}


uint8_t led_On(uint8_t dato){
	uint8_t aux; //Variable auxiliar
	uint8_t dir_aux = DIR_SLAVE;
	dir_aux = dir_aux << 1; //Para realizar la SLA + W
	aux = TWI_Inicio(); //Realiza el inicio de la TWI
	if (aux != 0x01){ //Si hubo error
		TWCR |= (1<<TWINT); //Limpia bandera
		return 0;
	}
	aux = TWI_EscByte(dir_aux); //Direcciona la SLA+W
	if (aux != 0x01){ //Si hubo error
		TWI_Paro();
		return 0;
	}
	TWI_EscByte(dato); //Envía el dato
	TWI_Paro();
	return 1;
}

uint8_t envia_SPI(uint8_t dato){
	uint8_t resp;
	PORTB = (1<<PB3) | PORTB;
	PORTB = (1<<PB4) | PORTB;
	PORTB = (1<<PB5) | PORTB;
	PORTB = ~(1<<PB2) & PORTB;
	SPDR = dato;
	while(!(SPSR&1<<SPIF)) ; // Espera fin de envio
	resp = SPDR; // Lee la respuesta del esclavo
	PORTB = (1<<PB2) | PORTB;
	return resp; 
}
