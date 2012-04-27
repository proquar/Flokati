package de.proquariat.flokati;


public class FlokatiDevice {
	public String name;
	public FlokatiDeviceControl[] Controls;
	public long id;
	private boolean complete=false;
	
	int getSize() {
		return Controls.length;
	}
	
	public FlokatiDevice (int size, long id, String name) {
		this.Controls=new FlokatiDeviceControl[size];
		this.name=name;
		this.id=id;
	}
	
	public void setControl(int pos, FlokatiDeviceControl c) {
		if (pos>0 && pos<=this.Controls.length && c.isValid) {
			this.Controls[pos-1]=c;
			c.parent=this;
			c.sink=pos;
			this.complete=true;
			for (FlokatiDeviceControl cont: Controls) {
				if (cont==null) this.complete=false;
			}
		}
	}
	public FlokatiDeviceControl getControl(int i) {
		if (i>0 && i<=this.Controls.length) {
			return this.Controls[i-1];
		}
		return null;
	}
	
	public boolean isComplete() {
		return this.complete;
	}
	
	public byte[] getMessageBytes(int ...sinks) {
		byte[] message=new byte[32];
		message[0]=0x40;
		for (int i=1; i<9; i++) {
			message[i]=(byte)(this.id>>(64-i*8));
		}
		
		int i=9;
		for (int s: sinks) {
			if (s>0 && s<=this.Controls.length) {
				byte[] sinkBytes=this.Controls[s-1].getSinkAndValueBytes();
				if ((i+sinkBytes.length)<32) {
					System.arraycopy(sinkBytes, 0, message, i, sinkBytes.length);
					i+=sinkBytes.length;
				}
			}
		}
		return message;
	}
	
	public int updateFromPacketBytes(byte[] packet, int offset) {
		while (offset<packet.length) {
			if (packet[offset]>0 && packet[offset]<=this.getSize()) {
				FlokatiDeviceControl dc = this.getControl(packet[offset]);
				if (dc!=null) {
					offset=dc.updateFromPacket(packet, offset+1);
				} else {
					break; // not yet discovered, so we don't know the type
				}
			} else break; //invalid packet
		}
		return offset;
	}
}
