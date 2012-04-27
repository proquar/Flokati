#include "packet.h"
#include "radio.h"
#include <avr/io.h>
#include <util/delay.h>

radiopacket_t packet;
uint8_t common_addr[5] = { 0xE4, 0xE4, 0xE4, 0xE4, 0xE4 };

int main()
{
	Radio_Init();
	Radio_Configure_Rx(RADIO_PIPE_0, common_addr, ENABLE);
	Radio_Configure(RADIO_2MBPS, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(common_addr);
	sei();
	
	DDRC |= (1<<3);
	_delay_ms(300);
	
	for(;;) {
		Radio_Transmit(&packet, RADIO_WAIT_FOR_TX);
		DDRC ^= (1<<3);
		_delay_ms(300);
	}
	return 0;
}


void radio_rxhandler(uint8_t pipe_number)
{
	
	//DDRC ^= (1<<3);
	// This function is called when the radio receives a packet.
	// It is called in the radio's ISR, so it must be kept short.
	// The function may be left empty if the application doesn't need to respond immediately to the interrupt.
}