#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "../nrf24l01/packet.h"
#include "../nrf24l01/radio.h"
#include "../nrf24l01/common.h"

// Strobe light controller: a device for controlling external strobes or flashes on pin B1
// It can also be controlled by a quadrature encoder on pins C1 and C2
//
// The strobe controller has 1 sink:
//    1: Strobe spacing (16 bit unsigned)


#define OWN_ADDRESS	"\0\0\0\x01\x00\xf1\x01\x01"

#define REVERSE 1


// status-led
#define STATUS_INIT()	DDRC |= (1<<3)
#define STATUS_ON() 	DDRC |= (1<<3)
#define STATUS_OFF() 	DDRC &= ~(1<<3)


radiopacket_t packet, notify;
volatile uint8_t newPacket=0;
volatile uint16_t nrf_reset_cnt=0;
volatile uint8_t notify_cnt=0;

volatile uint16_t strobeLen=0x2fff;

volatile int8_t encPos;
volatile uint8_t encoder_collect_cnt=0;



void prepareNotify(void) {
	notify.type=NOTIFY;
	memcpy(notify.address, OWN_ADDRESS, 8);
	
	notify.data.notify.content[ 0]=0x01;
// 	notify.data.notify.content[ 1]=strobeLen>>8;
// 	notify.data.notify.content[ 2]=strobeLen;
	notify.data.notify.content[ 3]=0x02;
}
void sendNotify(void) {
// 	notify.data.notify.content[ 0]=0x01;
	notify.data.notify.content[ 1]=strobeLen>>8;
	notify.data.notify.content[ 2]=strobeLen;
	
	Radio_Transmit(&notify, ABSOLUTE_MAXIMUM_SIZE, RADIO_WAIT_FOR_TX);
}

#define PULSELEN 0x01

#define PRESCALER_MASK 0b111
#define PRESCALER_NORMAL 0b101

int main(void)
{
 	STATUS_INIT();
	
// 	STATUS_OFF();
	
	// Initialize Timer1 for PWM on OC1A (inverting) with maximum frequency and 10bit -> 7.8kHz PWM
	OCR1A=PULSELEN;
	ICR1=strobeLen+PULSELEN+0xff;
	//ICR1 = (strobeLen>>3 + 0x2ff);
	
	TCCR1A = (1<<COM1A1)|(1<<COM1A0)|(1<<WGM11);	 // inverted, ICR1 max
	TCCR1B = (1<<WGM13)|(1<<WGM12)|PRESCALER_NORMAL;
	
	DDRB |= (1<<1);
	PORTB |= (1<<1);
	
	// Timer 2 for blinking status LED (clk/1024)
	TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
	TIMSK2 |= (1<<TOIE2);
	
	//Pin change interrupt on pins C1, C2 (pcint9+10)
	PCMSK1 = (1<<PCINT9)|(1<<PCINT10);
	PCICR |= (1<<PCIE1);
	PORTC |= 0b110;
	
	prepareNotify();
	 
	rf_init();
	sei();
	
	uint8_t strobeSet=0;

	for(;;) {
		if ( newPacket ) {
			newPacket--;
			if ( Radio_Receive(&packet) >= RADIO_RX_MORE_PACKETS ) {
				
				// a real data package received
				if (packet.type==MESSAGE && (memcmp(packet.address, OWN_ADDRESS, 8)==0)) {
					STATUS_ON();
					
					cli();
					uint8_t content_ptr=0;
					while (content_ptr<(MESSAGE_CONTENT_MAX_SIZE-2)) {
						uint8_t sink=packet.data.message.content[content_ptr];
						if (sink==1) {
							strobeLen = packet.data.message.content[content_ptr+1];
							strobeLen <<= 8;
							strobeLen |= packet.data.message.content[content_ptr+2];
							content_ptr+=3;
							strobeSet=1;
						} else {
							break;
						}
					}
					sei();
				}
			}
			
		}
		
		if (encoder_collect_cnt>2) {
			if ((encPos>>2) && encoder_collect_cnt>2) {
				cli();
				int32_t newStrobeLen;
				if ((encPos>>2)>=0)
					newStrobeLen = 4<<(encPos>>2);
				else
					newStrobeLen = -(4<<(-(encPos>>2)));
				encPos -= (encPos>>2);
				sei();
				
				newStrobeLen += strobeLen;
				if (newStrobeLen>0xffff) strobeLen=0xffff;
				else if (newStrobeLen<0) strobeLen=0;
				else strobeLen=newStrobeLen;
				strobeSet = 1;
			}
			encoder_collect_cnt=0;
		}
		
		
#define OFFSET 0x1ff
		if (strobeSet) {
			strobeSet=0;
			if (strobeLen >= (0xffff-OFFSET-PULSELEN)) {
				TCCR1B &= ~PRESCALER_MASK; //timer off
			} else {
				ICR1 = (strobeLen + PULSELEN + OFFSET);
				if (TCNT1>=ICR1) TCNT1=0;
				TCCR1B |= PRESCALER_NORMAL;
			}
		}
		
		if (nrf_reset_cnt>180) {
			// sometimes it seems that the nrf-chip crashes, so we re-initialise it after ~6s without receiving anything
			rf_init();
			nrf_reset_cnt=0;
		}
		
		if (notify_cnt>28) {
			sendNotify();
			notify_cnt=0;
		}
	}
	return 0;
}

ISR( TIMER2_OVF_vect ) {
	// overflows every 32ms
	nrf_reset_cnt++;
	notify_cnt++;
	encoder_collect_cnt++;
	STATUS_OFF();
}


ISR (PCINT1_vect) {
	uint8_t raw_encPos = (PINC>>1)&3;
	uint8_t new_encPos=0;
	if (raw_encPos==3) new_encPos=2;
	else if (raw_encPos==2) new_encPos=3;
	else new_encPos=raw_encPos;
	
#ifdef REVERSE
	new_encPos = (~new_encPos)&0x3;
#endif
	if (((encPos+1)&3) == new_encPos) encPos++;
	else if (((encPos-1)&3) == new_encPos) encPos--;
}

void radio_rxhandler(uint8_t pipe_number)
{
	nrf_reset_cnt=0;
	newPacket++;
}
