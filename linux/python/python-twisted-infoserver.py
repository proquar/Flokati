#!/usr/bin/env python
# -*- coding: utf-8 -*-

from twisted.internet.protocol import Protocol
from twisted.internet.serialport import SerialPort
from twisted.internet import reactor
from twisted.internet.protocol import DatagramProtocol
import string, copy
import json

FlokatiDB=[
	{
		'family': "%x"%0x0000000100010000,
		'mask': 48,
		'ttl': 600,
		'ttl-orig': 600,
		'manufacturer': 'Proquariat',
		'model': 'Vertiko',
		'name': 'White Spotlight',
		'names': {
			'de': 'Weißer Scheinwerfer',
		},
		'sinks': [
			{
				'id': 1,
				'type': 'unsigned short',
				'name': 'Brightness',
				'names': {
					'de': 'Helligkeit',
				}
			},
			{
				'id': 2,
				'type': 'boolean',
				'name': 'Strobo?',
			},
			{
				'id': 3,
				'type': 'unsigned short',
				'name': 'Pulsewidth',
				'names': {
					'de': 'Pulsweite',
				}
			},
			{
				'id': 4,
				'type': 'unsigned short',
				'name': 'Pulse spacing',
				'names': {
					'de': 'Pulsabstand',
				}
			},
		],
	},
	{
		'family': "%x"%0x0000000100080000,
		'mask': 48,
		'ttl': 600,
		'ttl-orig': 600,
		'manufacturer': 'Proquariat',
		'model': 'Ottoman',
		'name': 'Mirror ball',
		'names': {
			'de': 'Spiegelkugel',
		},
		'sinks': [
			{
				'id': 1,
				'type': 'short',
				'name': 'Speed',
				'names': {
					'de': 'Geschwindigkeit',
				}
			},
			{
				'id': 2,
				'type': 'boolean',
				'name': 'Boost',
			},
		],
	},
]
FlokatiDB.append(copy.copy(FlokatiDB[0]))
FlokatiDB[len(FlokatiDB)-1]['family']="%x"%0x0000000100020000
FlokatiDB[len(FlokatiDB)-1]['name']='Green Spotlight'
FlokatiDB[len(FlokatiDB)-1]['names']={ 'de': 'Grüner Scheinwerfer', }

class MulticastConnector(DatagramProtocol):
	def startProtocol(self):
		"""
		Called after protocol has started listening.
		"""
		# Set the TTL>1 so multicast will cross router hops:
		self.transport.setTTL(1)
		self.transport.setLoopbackMode(0)
		print self.transport.getLoopbackMode()
		# Join a specific multicast group:
		self.transport.joinGroup("233.133.133.133")
		
	def __init__(self):
		pass
	
	def datagramReceived(self, datagram, address):
		try:
			request=json.loads(datagram)
		except:
			return
		print "Received %i bytes, that are legitimate JSON:"%len(datagram),
		print request
		
		if 'request' in request:
			theid=int(request['request'],16)
			if theid>=0x0000000100010001 and theid<0x000000010001ffff:
				self.sendToGroup(json.dumps(FlokatiDB[0]))
			elif theid>=0x0000000100080001 and theid<0x000000010008ffff:
				self.sendToGroup(json.dumps(FlokatiDB[1]))
			elif theid>=0x0000000100020001 and theid<0x000000010002ffff:
				self.sendToGroup(json.dumps(FlokatiDB[2]))
		
	
	def sendToGroup(self,packet):
		self.transport.write(packet, ("233.133.133.133",5332) )
		#print "   =>  Datagram sent"+
		print "sent %i bytes"%len(packet)

mucastConn=MulticastConnector()

reactor.listenMulticast(5332, mucastConn, listenMultiple=True)
reactor.run()