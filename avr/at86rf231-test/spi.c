#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "spi.h"
#include "common.h"

uint8_t readSPI_register (uint8_t addr) {
	return accessSPI_register(0, addr, 0);
}

uint8_t writeSPI_register(uint8_t addr, uint8_t content) {
	return accessSPI_register(1, addr, content);
}

uint8_t accessSPI_register(uint8_t write, uint8_t addr, uint8_t content) {
	SEL_LOW();
	
	if (write)
		SPDR = (0xC0 | addr); // register write access (datasheet 6.2)
	else
		SPDR = (0x80 | addr); // register read access (datasheet 6.2)
	
	while(!(SPSR&(1<<SPIF)));
	
	SPDR=content;
	
	while(!(SPSR&(1<<SPIF)));
	
	uint8_t data=SPDR;
	SEL_HIGH();
	
	return data;
}


uint8_t readFrame(uint8_t *frame) {
	SEL_LOW();
	
	SPDR = 0x20; // frame buffer read access (datasheet 6.2)
	while(!(SPSR&(1<<SPIF)));
	
	SPDR = 0;
	while(!(SPSR&(1<<SPIF)));
	uint8_t length=SPDR;
	
	uint8_t i=0;
	for(i=0; i<length; i++) {
		SPDR = 0;
		while(!(SPSR&(1<<SPIF)));
		
		if (frame!=0)
			frame[i]=SPDR;
	}
	
	SPDR=0;
	while(!(SPSR&(1<<SPIF)));
	
	if (frame!=0)
		frame[i+1]=SPDR;
	
	SEL_HIGH();
	
	return length+1;
}

void writeFrame(uint8_t *frame, uint8_t length) {
	SEL_LOW();
	
	SPDR = 0x60; // frame buffer write access (datasheet 6.2)
	while(!(SPSR&(1<<SPIF)));
	
	SPDR = length;
	while(!(SPSR&(1<<SPIF)));
	
	uint8_t i=0;
	for(i=0; i<length; i++) {
		SPDR = frame[i];
		while(!(SPSR&(1<<SPIF)));
	}
	
	SEL_HIGH();
}