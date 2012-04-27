/*
 * packet.h
 *
 *  Created on: 26-Apr-2009
 *      Author: Neil MacMillan
 */

#ifndef PACKET_H_
#define PACKET_H_


#include <avr/io.h>


/*****					Add labels for the packet types to the enumeration					*****/

typedef enum _pt
{
	MESSAGE=0x40,
	NOTIFY=0x41,
} PACKET_TYPE;

/*****							Construct payload format structures							*****/

#define ABSOLUTE_MAXIMUM_SIZE 32
#define MESSAGE_CONTENT_MAX_SIZE 23

typedef struct _msg
{
	uint8_t content[MESSAGE_CONTENT_MAX_SIZE];
} payload_message;

#define payload_notify payload_message //notify looks the same as message...

/*****							Add format structures to the union							*****/

/// The application-dependent packet format.  Add structures to the union that correspond to the packet types defined
/// in the PACKET_TYPE enumeration.  The format structures may not be more than 29 bytes long.  The _filler array must
/// be included to ensure that the union is exactly 23 bytes long.
typedef union _pf
{
	uint8_t _filler[23];	// maximum size 32
	payload_message message;
	payload_notify notify;
} payloadformat_t;


typedef struct _rp
{
	uint8_t type;
	uint8_t address[8];
	payloadformat_t data;
} radiopacket_t;

#endif /* PACKET_H_ */
