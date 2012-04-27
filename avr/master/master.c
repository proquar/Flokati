#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "../nrf24l01/packet.h"
#include "../nrf24l01/radio.h"
#include "../nrf24l01/common.h"

#include "uart.h"

volatile uint16_t blinker_cnt=0;
volatile uint8_t packet_available=0;

// status-led
#define STATUS_ON() 	DDRC |= (1<<3)
#define STATUS_OFF() 	DDRC &= ~(1<<3)


int main(void)
{
	// Timer 1 for blinking led, as general low speed counter
	TCCR1B=(1<<CS12)|(1<<CS10);	// clk/1024
	
	uartInit(0); //500kb/s @ 8MHz
	
	rf_init();
	
	STATUS_ON();
	
	sei();
		
	radiopacket_t pack;
	for(;;) {
		if (uartRxAvailable()) {
			Radio_Transmit(uartRxGet(), ABSOLUTE_MAXIMUM_SIZE, RADIO_WAIT_FOR_TX);
			uartRxConfirm();
		}
		if (packet_available) {
			packet_available--;
			if (Radio_Receive(uartTxGet())>=RADIO_RX_MORE_PACKETS) {
				STATUS_ON();
				blinker_cnt=TCNT1;
				uartTxConfirm();
			}
		}
		
		// turn off the status-led, after some ms after sending
		if ((TCNT1-blinker_cnt)>170) STATUS_OFF();
		
	}
	return 0;
}


void radio_rxhandler(uint8_t pipe_number)
{
	packet_available++;
}
