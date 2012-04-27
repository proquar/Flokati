package de.proquariat.flokati;

import android.os.Message;


public class PacketParser {	
	public void newPacket(byte[] packet) {
		if (packet.length>=9 && (packet[0]==0x40 || packet[0]==0x41)) {
			long id=0;
			for (int i=1; i<9 ; i++) {
				id<<=8;
				id+=(((int)packet[i])&0xff);
			}
			if (packet[0]==0x40) this.handleMessage(id, packet, 9);
			else if (packet[0]==0x41) this.handleNotify(id, packet, 9);
		}
	}
	private void handleNotify(long address, byte[] packet, int offset) {
		FlokatiDevice dev=getDeviceByAddress(address);
		if (dev==null) {
			DeviceRequest rq =new DeviceRequest(address, packet, offset);
			Message rqMsg = new Message();
			rqMsg.arg1 = FlokatiApp.TOSOCKET;
			rqMsg.obj=rq;
			FlokatiApp.infoSocketHandler.sendMessage(rqMsg);
		} else {
			dev.updateFromPacketBytes(packet, offset);
		}
	}
	
	private void handleMessage(long address, byte[] packet, int offset) {
		FlokatiDevice dev = getDeviceByAddress(address);
		if (dev!=null) {
			dev.updateFromPacketBytes(packet, offset);
		}
	}
	
	private FlokatiDevice getDeviceByAddress(long address) {
		for (FlokatiDevice d: FlokatiApp.devices) {
			if (d.id==address) {
				return d;
			}
		}
		return null;
	}
}
