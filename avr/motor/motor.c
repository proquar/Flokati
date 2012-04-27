#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "../nrf24l01/packet.h"
#include "../nrf24l01/radio.h"
#include "../nrf24l01/common.h"

// motors have 2 sinks:
//   1: speed (16bit signed)
//   2: boost (boolean)

#define OWN_ADDRESS		"\0\0\0\x01\x00\x08\x00\x01"

// status-led
#define RXMON_INIT()	DDRC |= (1<<3)
#define RXMON_ON() 		DDRC |= (1<<3)
#define RXMON_OFF() 	DDRC &= ~(1<<3)

// Motor:	Dir1: D6, OC0A (dir1)
//			Dir2: D5, OC0B
//			EN:   C0
#define MOTOR_INIT()		PORTC&=~1; DDRC|=1; PORTD&=~((1<<5)|(1<<6)); DDRD|=((1<<5)|(1<<6))
#define MOTOR_ENABLE() 		PORTC|=1
#define MOTOR_DISABLE()		PORTC&=~1
#define MOTOR_FW_ON()		PORTD|=(1<<6)
#define MOTOR_FW_OFF()		PORTD&=~(1<<6)
#define MOTOR_RW_ON()		PORTD|=(1<<5)
#define MOTOR_RW_OFF()		PORTD&=~(1<<5)

radiopacket_t packet, notify;
volatile uint8_t newPacket=0;
volatile uint16_t nrf_reset_cnt=0;
volatile uint8_t notify_cnt=0;


volatile uint8_t direction=0;
volatile uint16_t speed=140;	// 140 = 1rpm, 230 = 2rpm, 300 = 3rpm, 370 = 4rpm,   max 500 @4.8V
volatile int16_t receivedSpeed=0;

volatile uint16_t pwmCnt=0;

volatile uint16_t boostCnt=0;
volatile int8_t boost=0;



void prepareNotify(void) {
	notify.type=NOTIFY;
	memcpy(notify.address, OWN_ADDRESS, 8);
	
	notify.data.notify.content[ 0]=0x01;
	notify.data.notify.content[ 3]=0x02;
	notify.data.notify.content[ 5]=0x03;
}
void sendNotify(void) {
	notify.data.notify.content[ 1]=receivedSpeed>>8;
	notify.data.notify.content[ 2]=receivedSpeed;
	notify.data.notify.content[ 4]=boost;
	
	Radio_Transmit(&notify, ABSOLUTE_MAXIMUM_SIZE, RADIO_WAIT_FOR_TX);
}


int main(void)
{
	RXMON_INIT();
	
	// STRATEGY:
	//     pwm is too fast for our motor, so we will just give it a short push twice a second
	//     so, we use the overflow interrupt to do our own very slow pwm
	MOTOR_INIT();
	TCCR0A = 0; 			// normal mode, no pwm
	TCCR0B = (1<<CS01);		// clk/8, enable counter
	TIMSK0 |= (1<<TOIE0);	//overflow interrupt on
	
	// Timer 2 for blinking status LED (clk/1024)
	TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
	TIMSK2 |= (1<<TOIE2);
	
	// TImer 1 (clk/1024) for boost timing
	TCCR1B=(1<<CS12)|(1<<CS10);
	
	receivedSpeed=speed<<6;
	if (direction) speed *= -1;
	prepareNotify();
	
	rf_init();
	sei();

	for(;;) {		
		if ( newPacket ) {
			newPacket--;
			if ( Radio_Receive(&packet) >= RADIO_RX_MORE_PACKETS ) {
				RXMON_ON();
				
				// a real data package received
				if (packet.type==MESSAGE && (memcmp(packet.address, OWN_ADDRESS, 8)==0)) {
					
					uint8_t content_ptr=0;
					while (content_ptr<MESSAGE_CONTENT_MAX_SIZE) {
						uint8_t sink=packet.data.message.content[content_ptr];
						if (sink==1) {
							receivedSpeed = packet.data.message.content[content_ptr+1];
							receivedSpeed <<= 8;
							receivedSpeed |= packet.data.message.content[content_ptr+2];
							content_ptr+=3;
						} else if (sink==2) {
							if (packet.data.message.content[content_ptr+1] && boost<=0)
								boost = 1;
							//TODO: immediately reply with notify
							content_ptr+=2;
						} else {
							break;
						}
					}
					
					// ...
					if (receivedSpeed>=0) {
						speed=receivedSpeed>>6; //only use 9 bit (max. duty cycle 1/4)
						direction=0;
					} else {
						speed=(receivedSpeed*-1)>>6;
						direction=1;
					}
				}
			}
			
		}
		
		if (boost==-1) {
			// TODO send notify when boost is done
			boost=0;
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

ISR( TIMER0_OVF_vect ) {
	if (boost>0) {
		if (boost==1) {
			MOTOR_ENABLE();
			if (direction) { MOTOR_RW_OFF(); MOTOR_FW_ON(); }
			else { MOTOR_FW_OFF(); MOTOR_RW_ON(); }
			boostCnt=TCNT1;
			boost=2;
		}
		else if (boost==2 && (TCNT1-boostCnt)>7000) { // ~900ms
			MOTOR_DISABLE();
			MOTOR_FW_OFF();
			MOTOR_RW_OFF();
			boost=-1;
			pwmCnt=0x03ff;
		}
	}
	else {
		if (pwmCnt==0) {
			if (speed>0) {
				MOTOR_ENABLE();
				if (direction) { MOTOR_RW_OFF(); MOTOR_FW_ON(); }
				else { MOTOR_FW_OFF(); MOTOR_RW_ON(); }
			}
		}
		if (pwmCnt == speed) {
			MOTOR_DISABLE();
			MOTOR_FW_OFF();
			MOTOR_RW_OFF();
		}
		
		pwmCnt++;
		pwmCnt&=0x07ff; //2047 max
	}
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
