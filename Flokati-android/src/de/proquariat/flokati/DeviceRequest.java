package de.proquariat.flokati;

public class DeviceRequest {
	public long id;
	public byte[] packet;
	public int offset;
	
	public DeviceRequest(long id, byte[] packet, int offset) {
		this.id=id;
		this.packet=packet;
		this.offset=offset;
	}
}
