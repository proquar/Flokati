#ifndef RFCOMMON_H_
#define RFCOMMON_H_

#include <stdio.h>
#include <avr/io.h>
#include "radio.h"
#include "packet.h"

uint8_t group_addr[5] = { 0xa7, 0x6c, 0x92, 0x0d, 0x30 };

#define DEFAULT_DATA_RATE RADIO_1MBPS

void rf_init(void) {
	Radio_Init();
	Radio_Configure_Rx(RADIO_PIPE_0, group_addr, ENABLE, RADIO_NO_ACK, ABSOLUTE_MAXIMUM_SIZE );
	Radio_Autoack(RADIO_NO_ACK);
	Radio_Configure(DEFAULT_DATA_RATE, RADIO_HIGHEST_POWER);
	Radio_Set_Tx_Addr(group_addr);
}

#endif