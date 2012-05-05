#ifndef RFCOMMON_H_
#define RFCOMMON_H_

#include <stdio.h>
#include <avr/io.h>
#include "radio.h"
#include "packet.h"

#define FLOKATI_STANDARD 0x34

uint8_t group_addr[5] = { 'F', 'l', 'o', 'k', FLOKATI_STANDARD };

#define DEFAULT_DATA_RATE RADIO_1MBPS

void rf_init(void) {
	Radio_Init();
	Radio_Configure_Rx(RADIO_PIPE_0, group_addr, ENABLE, RADIO_NO_ACK, ABSOLUTE_MAXIMUM_SIZE );
	Radio_Autoack(RADIO_NO_ACK);
	Radio_Configure(DEFAULT_DATA_RATE, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(group_addr);
}

#endif