#ifndef COMMON_H
#define COMMON_H

#define SEL_HIGH()		(PORTB |= (1<<2))
#define SEL_LOW()		(PORTB &= ~(1<<2))

#define SLPTR_HIGH()	(PORTB |= 1)
#define SLPTR_LOW()	(PORTB &= ~1)

#define RES_HIGH()		(PORTD |= (1<<4))
#define RES_LOW()		(PORTD &= ~(1<<4))

#define DIG2_PIN()		(PIND&(1<<7))
#define IRQ_PIN()		(PIND&(1<<2))

// status-led
#define STATUS_INIT() 	DDRC |= (1<<3);
#define STATUS_ON() 	PORTC |= (1<<3);
#define STATUS_OFF() 	PORTC &= ~(1<<3);

#endif