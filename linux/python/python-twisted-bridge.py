from twisted.internet.protocol import Protocol
from twisted.internet.serialport import SerialPort
from twisted.internet import reactor
from twisted.internet.protocol import DatagramProtocol
import string

class SLIPProtocol(Protocol):
	
	SLIP_ESC=chr(0xDB)
	SLIP_END=chr(0xC0)
	
	def __init__(self, packetHandler):
		self.packetHandler=packetHandler
		self.msg=""
		self.escaped=False
	
	def dataReceived(self, data):
		for c in data:
			if self.escaped:
				self.msg+=c
				self.escaped=False
			else:
				if c==self.SLIP_END:
					self.packetHandler(self.msg)
					self.msg=""
				elif c==self.SLIP_ESC:
					self.escaped=True
				else:
					self.msg+=c
	
	def sendToPort(self,data):
		data=string.replace(data,self.SLIP_ESC,self.SLIP_ESC+self.SLIP_ESC);
		data=string.replace(data,self.SLIP_END,self.SLIP_ESC+self.SLIP_END);
		
		self.transport.write(data+self.SLIP_END)

class MulticastConnector(DatagramProtocol):
	def startProtocol(self):
		"""
		Called after protocol has started listening.
		"""
		# Set the TTL>1 so multicast will cross router hops:
		self.transport.setTTL(2)
		self.transport.setLoopbackMode(0)
		print self.transport.getLoopbackMode()
		# Join a specific multicast group:
		self.transport.joinGroup("233.133.133.133")
	def __init__(self):
		self.packetHandler=None
	
	def datagramReceived(self, datagram, address):
		#print "<=     Datagram received", address
		self.packetHandler(datagram)
		print "<=   ",
		#print "      ",
		for c in datagram[4:6]:
			print "%02x"%ord(c),
		print
		
	
	def sendToGroup(self,packet):
		self.transport.write(packet, ("233.133.133.133",5331) )
		#print "   =>  Datagram sent"+
		
		print "=> ",
		for c in packet:
			print "%02x"%ord(c),
		print
		#if ord(packet[3])==0x22:
			#print "=>             %02x"%ord(packet[5])

mucastConn=MulticastConnector()
serialConn=SLIPProtocol(mucastConn.sendToGroup)

mucastConn.packetHandler=serialConn.sendToPort

tranport = SerialPort(serialConn,"/dev/ttyUSB0",reactor,baudrate=500000)
reactor.listenMulticast(5331, mucastConn, listenMultiple=True)
reactor.run()