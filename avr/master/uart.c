#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "uart.h"
#include "../nrf24l01/packet.h"

extern volatile rxBuffer_t rxbuffer;
extern volatile txBuffer_t txbuffer;

uint8_t uartRxAvailable() {
	if (rxbuffer.readpos!=rxbuffer.writepos) return 1;
	else return 0;
}
packetContainer_t *uartRxGet() {
	if (rxbuffer.readpos!=rxbuffer.writepos)
		return (packetContainer_t*)&(rxbuffer.buffer[rxbuffer.readpos]);
	else return 0;
}

void uartRxConfirm() {
	rxbuffer.readpos++;
	rxbuffer.readpos&=RX_BUFFER_MASK;
}


packetContainer_t *uartTxGet() {
	return (packetContainer_t*)&(txbuffer.buffer[txbuffer.writepos]);
}

void uartTxConfirm() {
	if (((txbuffer.writepos+1)&TX_BUFFER_MASK)!=txbuffer.readpos) {
		txbuffer.writepos++;
		txbuffer.writepos&=TX_BUFFER_MASK;
	}
	ENABLE_TX_INT();
}

void uartInit(uint8_t baudrate) {
	//enable transmitter+receiver, 8 databits
	UCSR0B = (1<<TXEN0)|(1<<RXEN0);
	
	//asynchronous, no parity, 1 stopbit, 8 databits
	UCSR0C = (1<<UCSZ01)|(1<<UCSZ00);

	UBRR0H = 0;
	UBRR0L = baudrate;
	
	txbuffer.writepos=0;
	txbuffer.readpos=0;
	txbuffer.escape=0;
	txbuffer.inpacket_pos=0;
	
	
	rxbuffer.writepos=0;
	rxbuffer.readpos=0;
	rxbuffer.escape=0;
	rxbuffer.inpacket_pos=0;
	
	ENABLE_RX_INT();
}

#define SLIP_END 0xC0
#define SLIP_ESC 0xDB

ISR( USART_RX_vect ) {
	char read=UDR0;
	if (rxbuffer.escape) {
		if (rxbuffer.inpacket_pos<(ABSOLUTE_MAXIMUM_SIZE+1))
			((uint8_t*)&(rxbuffer.buffer[rxbuffer.writepos]))[rxbuffer.inpacket_pos++]=read;
		rxbuffer.escape=0;
	}
	else {
		if (read==SLIP_ESC) {
			rxbuffer.escape=1;
		}
		else if (read==SLIP_END) {
			rxbuffer.inpacket_pos=0;
			rxbuffer.escape=0;
			if (((rxbuffer.writepos+1)&RX_BUFFER_MASK) != rxbuffer.readpos) {
				rxbuffer.writepos++;
				rxbuffer.writepos&=RX_BUFFER_MASK;
			}
		}
		else {
			if (rxbuffer.inpacket_pos<(ABSOLUTE_MAXIMUM_SIZE+1))
				((uint8_t*)&(rxbuffer.buffer[rxbuffer.writepos]))[rxbuffer.inpacket_pos++]=read;
		}
	}
}

ISR( USART_UDRE_vect ) {
	if ( txbuffer.readpos != txbuffer.writepos ) {
		if (txbuffer.inpacket_pos<(ABSOLUTE_MAXIMUM_SIZE+1)) {
			if (txbuffer.escape==0 &&
				( ((uint8_t*)(txbuffer.buffer+txbuffer.readpos))[txbuffer.inpacket_pos]==SLIP_END ||
				((uint8_t*)(txbuffer.buffer+txbuffer.readpos))[txbuffer.inpacket_pos]==SLIP_ESC ) ) {
				
				UDR0=SLIP_ESC;
				txbuffer.escape=1;
			}
			else {
				UDR0=((uint8_t*)(txbuffer.buffer+txbuffer.readpos))[txbuffer.inpacket_pos++];
				txbuffer.escape=0;
			}
		}
		else {
			UDR0=SLIP_END;
			txbuffer.readpos++;
			txbuffer.readpos &= TX_BUFFER_MASK;
			txbuffer.inpacket_pos=0;
			txbuffer.escape=0;
		}
	} else {
		DISABLE_TX_INT();
	}
}

