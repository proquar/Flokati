#ifndef UART_H
#define UART_H
#include "../nrf24l01/packet.h"

#define ENABLE_RX_INT()		UCSR0B |= (1<<RXCIE0)
#define DISABLE_RX_INT()	UCSR0B &= ~(1<<RXCIE0)
#define ENABLE_TX_INT()		UCSR0B |= (1<<UDRIE0)
#define DISABLE_TX_INT()	UCSR0B &= ~(1<<UDRIE0)

// from host (only 2 packets cached, because serial connection is slower than rf)
#define RX_BUFFER_SIZE 2
#define RX_BUFFER_MASK 0x1

typedef struct _pCont {
	uint8_t type;
	radiopacket_t packet;
} packetContainer_t;

typedef struct _pbufr {
	uint8_t writepos;
	uint8_t readpos;
	uint8_t inpacket_pos;
	uint8_t escape;
	packetContainer_t buffer[RX_BUFFER_SIZE];
} rxBuffer_t;

// to host 
#define TX_BUFFER_SIZE 4
#define TX_BUFFER_MASK 0x3

typedef struct _pbuft {
	uint8_t writepos;
	uint8_t readpos;
	uint8_t inpacket_pos;
	uint8_t escape;
	packetContainer_t buffer[TX_BUFFER_SIZE];
} txBuffer_t;

volatile rxBuffer_t rxbuffer;
volatile txBuffer_t txbuffer;

// !=0 when there's a packet available in the buffer
uint8_t uartRxAvailable(void);

// returns the next available packet or 0
packetContainer_t *uartRxGet(void);

// confirm that you are done processing that packet
void uartRxConfirm(void);


// returns number of next available spot in buffer
packetContainer_t *uartTxGet(void);

// confirm that you are done writing to that spot
void uartTxConfirm(void);

// baudrate actually is the UBRR-clockrate divisor
void uartInit(uint8_t baudrate);

#endif
