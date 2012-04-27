#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "../nrf24l01/packet.h"
#include "../nrf24l01/radio.h"
#include "../nrf24l01/common.h"

// spotlights have 4 sinks:
//   1: brightness (16bit unsigned)
//   2: strobelight (boolean)
//   3: strobe length (16bit unsigned)
//   4: strobe spacing (16bit unsigned)

//#define ID_WHITE 1
#define ID_GREEN 1

#ifdef ID_WHITE
	#define OWN_ADDRESS		"\0\0\0\x01\x00\x01\x00\x01"
#endif
#ifdef ID_WHITE2
	#define OWN_ADDRESS		"\0\0\0\x01\x00\x01\x00\x02"
#endif
#ifdef ID_GREEN
	#define OWN_ADDRESS		"\0\0\0\x01\x00\x02\x00\x01"
#endif


// status-led
#define STATUS_INIT()	DDRC |= (1<<2)
#define STATUS_ON() 	DDRC |= (1<<2)
#define STATUS_OFF() 	DDRC &= ~(1<<2)

// status-led
#define RXMON_INIT()	DDRC |= (1<<3)
#define RXMON_ON() 		DDRC |= (1<<3)
#define RXMON_OFF() 	DDRC &= ~(1<<3)


radiopacket_t packet, notify;
volatile uint8_t newPacket=0;
volatile uint16_t nrf_reset_cnt=0;
volatile uint8_t notify_cnt=0;

volatile uint16_t tBrightness=0xffff;
volatile uint8_t tStrobe=0;
volatile uint16_t tStrobeSpac=0x0900;
volatile uint16_t tStrobeLen=0x0900;



void prepareNotify(void) {
	notify.type=NOTIFY;
	memcpy(notify.address, OWN_ADDRESS, 8);
	
	notify.data.notify.content[ 0]=0x01;
// 	notify.data.notify.content[ 1]=tBrightness>>8;
// 	notify.data.notify.content[ 2]=tBrightness;
	notify.data.notify.content[ 3]=0x02;
// 	notify.data.notify.content[ 4]=tStrobe;
	notify.data.notify.content[ 5]=0x03;
// 	notify.data.notify.content[ 6]=tStrobeLen>>8;
// 	notify.data.notify.content[ 7]=tStrobeLen;
	notify.data.notify.content[ 8]=0x04;
// 	notify.data.notify.content[ 9]=tStrobeSpac>>8;
// // 	notify.data.notify.content[10]=tStrobeSpac;
}
void sendNotify(void) {
// 	notify.data.notify.content[ 0]=0x01;
	notify.data.notify.content[ 1]=tBrightness>>8;
	notify.data.notify.content[ 2]=tBrightness;
// 	notify.data.notify.content[ 3]=0x02;
	notify.data.notify.content[ 4]=tStrobe;
// 	notify.data.notify.content[ 5]=0x03;
	notify.data.notify.content[ 6]=tStrobeLen>>8;
	notify.data.notify.content[ 7]=tStrobeLen;
// 	notify.data.notify.content[ 8]=0x04;
	notify.data.notify.content[ 9]=tStrobeSpac>>8;
	notify.data.notify.content[10]=tStrobeSpac;
	
	Radio_Transmit(&notify, ABSOLUTE_MAXIMUM_SIZE, RADIO_WAIT_FOR_TX);
}


int main(void)
{
// 	STATUS_INIT();
	RXMON_INIT();
	
// 	STATUS_OFF();
	
	// Initialize Timer1 for PWM on OC1A (inverting) with maximum frequency and 10bit -> 7.8kHz PWM
	TCCR1A = (1<<COM1A1)|(1<<COM1A0)|(1<<WGM11)|(1<<WGM10);
	TCCR1B = (1<<WGM12)|(1<<CS10);
	OCR1A=tBrightness>>6;
	DDRB |= (1<<1);
	PORTB |= (1<<1);
	
	// Timer 2 for blinking status LED (clk/1024)
	TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
	TIMSK2 |= (1<<TOIE2);
	
	prepareNotify();
	
	rf_init();
	sei();

	for(;;) {
// 		sendNotify();
// 		DDRC ^= (1<<2);
// 		_delay_ms(700);
// 		Radio_Receive(&packet);
// 		Radio_Receive(&packet);
		
		
		if ( newPacket ) {
			newPacket--;
			if ( Radio_Receive(&packet) >= RADIO_RX_MORE_PACKETS ) {
				
				// a real data package received
				if (packet.type==MESSAGE && (memcmp(packet.address, OWN_ADDRESS, 8)==0)) {
					RXMON_ON();
					
					cli();
					uint8_t content_ptr=0;
					while (content_ptr<MESSAGE_CONTENT_MAX_SIZE) {
						uint8_t sink=packet.data.message.content[content_ptr];
						if (sink==1) {
							tBrightness = packet.data.message.content[content_ptr+1];
							tBrightness <<= 8;
							tBrightness |= packet.data.message.content[content_ptr+2];
							content_ptr+=3;
						} else if (sink==2) {
							tStrobe = packet.data.message.content[content_ptr+1];
							content_ptr+=2;
						} else if (sink==3) {
							tStrobeLen = packet.data.message.content[content_ptr+1];
							tStrobeLen <<= 8;
							tStrobeLen |= packet.data.message.content[content_ptr+2];
							content_ptr+=3;
						} else if (sink==4) {
							tStrobeSpac = packet.data.message.content[content_ptr+1];
							tStrobeSpac <<= 8;
							tStrobeSpac |= packet.data.message.content[content_ptr+2];
							content_ptr+=3;
						} else {
							break;
						}
					}
					sei();
					
					if (tStrobe) {
						OCR1A = tStrobeLen>>1;
						ICR1 = (tStrobeLen>>1) + (tStrobeSpac>>1);				// divide by 2, so we have a maximum of 2^16
						
						TCCR1A = (1<<COM1A1)|(1<<COM1A0)|(1<<WGM11);		// prescaler 256, ICR1 max
						TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS12);
						
						if (TCNT1>ICR1) TCNT1=0;
					}
					else {
						OCR1A = tBrightness>>6; //dimming with 10 bit, so discard lower 6 bit
								
						TCCR1A = (1<<COM1A1)|(1<<COM1A0)|(1<<WGM11)|(1<<WGM10);		// max frequency, 10 bit in counter 1
						TCCR1B = (1<<WGM12)|(1<<CS10);
							
						if (TCNT1>1024) TCNT1=0;
						
					}
				}
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
	nrf_reset_cnt++;
	notify_cnt++;
	RXMON_OFF();
}

void radio_rxhandler(uint8_t pipe_number)
{
	nrf_reset_cnt=0;
	newPacket++;
}
