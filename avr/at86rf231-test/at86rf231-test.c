#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#include "spi.h"
#include "common.h"
#include "at86rf230_registermap.h"


void uartInit(uint8_t baudrate) {
	//enable transmitter+receiver, 8 databits
	UCSR0B = (1<<TXEN0)|(1<<RXEN0);
	
	//asynchronous, no parity, 1 stopbit, 8 databits
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

	UBRR0H = 0;
	UBRR0L = baudrate;
}

void uartSend(const char *data) {
	uint16_t i=0;
	while (data[i]!=0) {
		UDR0=data[i];
		while(!(UCSR0A&(1<<UDRE0)));
		i++;
	}
}

int main(void) {
	STATUS_INIT();
	SEL_HIGH();
	uartInit(51);
	
	DDRB = (1<<0)|(1<<2)|(1<<3)|(1<<5); //SLP_TR, SEL, MOSI, SCK
	DDRD |= (1<<4); //reset high
	
	//spi init
	SPCR = (1<<SPE)|(1<<MSTR);
	SPSR = (1<<SPI2X);
	
	// at86rf231 init
	RES_LOW();
	SLPTR_LOW();
	_delay_ms(6);
	RES_HIGH();
	
	uint8_t allPassed=1;
	
	
	uartSend("Self-test starting.\r\n");
	
	
	uartSend("Setting new state TRX_OFF...    ");
	writeSPI_register(RG_TRX_STATE, CMD_TRX_OFF);
	
	while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != TRX_OFF) ;
	uartSend("OK\r\n");
	// SPI communication works, SEL, SCLK, MOSI and MISO pins tested.
	
	
	uartSend("Testing SLP_TR pin...           ");
	writeSPI_register(RG_TRX_STATE, CMD_RX_ON);
	while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != RX_ON) ;
	SLPTR_HIGH(); // state should transition to RX_ON_NOCLK
	while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != RX_ON_NOCLK) ;
	SLPTR_LOW();
	while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != RX_ON) ;
	uartSend("OK\r\n");
	
	
	// we were in RX_ON state, Reset should bring us back into TRX_OFF
	uartSend("Testing RESET pin...            ");
	RES_LOW();
	_delay_ms(6);
	RES_HIGH();
	while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != TRX_OFF) ;
	uartSend("OK\r\n");
	
	
	uartSend("Enabling relevant interrupts... ");
	writeSPI_register(RG_IRQ_MASK, (1<<2)|(1<<3)); //RX_START,TRX_END
	
	while ( (readSPI_register(RG_IRQ_MASK))!= ((1<<2)|(1<<3))) ;
	uartSend("OK\r\n");
	
	
	uartSend("Enabling DIG2 pin...            ");
	writeSPI_register(RG_TRX_CTRL_1, 0x60);
	
	while ( (readSPI_register(RG_TRX_CTRL_1))!= 0x60) ;
	uartSend("OK\r\n");
	
	
	// as described in datasheet 10.2
	uartSend("Sending dummy frame...          ");
	writeSPI_register(RG_TRX_STATE, CMD_PLL_ON); //sending mode
	while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != PLL_ON) ;
	writeFrame((uint8_t *)"FOOBAR", 6);
	SLPTR_HIGH(); //start sending, same as setting TRX_START mode
	_delay_ms(1);
	SLPTR_LOW();
	while(IRQ_PIN()==0); //wait for TRX_END on interrupt pin
	if (readSPI_register(RG_IRQ_STATUS)&(1<<3)) { // TRX_END should now be set
		while (IRQ_PIN()!=0) ; //IRQ pin should clear after reading IRQ_STATUS
		writeSPI_register(RG_TRX_STATE, CMD_RX_ON);
		uartSend("OK\r\n");
		uartSend("IRQ pin working...              OK\r\n");
	} else {
		writeSPI_register(RG_TRX_STATE, CMD_RX_ON);
		uartSend("ERROR\r\n");
		allPassed=0;
	}
	
	while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != RX_ON) ;
	
	uartSend("Listening for frame...          ");
	while (DIG2_PIN()==0); // test DIG2 pin, should be issued the same time as RX_START
	uint8_t interrupts = readSPI_register(RG_IRQ_STATUS);
	if (interrupts&(1<<2)) { // RX_START
		if (!(interrupts&(1<<3))) { // wait for TRX_END
			while (IRQ_PIN()==0) ;
			while (!(readSPI_register(RG_IRQ_STATUS)&(1<<3))) ;
		}
		uartSend("OK\r\n");
		uartSend("DIG2 pin working...             OK\r\n");
	} else {
		uartSend("ERROR\r\n");
		allPassed=0;
	}
	
	uartSend("\r\n##################################\r\n");
	if (allPassed) {
		STATUS_ON();
		uartSend("########## PASSED ################\r\n");
	}
	else
		uartSend("######## NOT PASSED ##############\r\n");
	uartSend("##################################\r\n");
	
	_delay_ms(1000);
	uartSend("\r\n\r\n");
	
	uint8_t frame[129];
	while(1) {
		uint8_t interrupts = readSPI_register(RG_IRQ_STATUS);
		if (interrupts&(1<<3)) {
			STATUS_OFF();
			uint8_t len = readFrame(frame);
			uartSend("LQI: ");
			char value[4];
			value[3]=0;
			uartSend(utoa(frame[len],value,10));
			uartSend("\r\n");
			
			_delay_ms(10);
			
			//reply
			while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != RX_ON) ;
			writeSPI_register(RG_TRX_STATE, CMD_PLL_ON);
			while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != PLL_ON) ;
			writeFrame((uint8_t *)"FOOBAR", 6);
			writeSPI_register(RG_TRX_STATE, CMD_TX_START);
			while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != PLL_ON) ;
			
			writeSPI_register(RG_TRX_STATE, CMD_RX_ON);
			while ( (readSPI_register(RG_TRX_STATUS)&0x1f) != RX_ON) ;
			
			
			_delay_ms(50);
			STATUS_ON();
		}
	}
	
	return 0;
}