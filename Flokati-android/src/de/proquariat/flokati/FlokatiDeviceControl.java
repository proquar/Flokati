package de.proquariat.flokati;

import android.os.Message;

public class FlokatiDeviceControl {
	public static final char INTEGER = 'i';
	public static final char UNSIGNED_INTEGER = 'I';
	public static final char SHORT_INTEGER = 'h';
	public static final char UNSIGNED_SHORT_INTEGER = 'H';
	public static final char BOOLEAN = '?';
	
	public static final long UNSIGNED_INT_MAX = 4294967295l;
	public static final int UNSIGNED_SHORT_MAX = 65535;
	public static final int INT_MAX = 2147483647;
	public static final int INT_MIN = -2147483648;
	public static final int SHORT_MAX = 32767;
	public static final int SHORT_MIN = -32768;
	
	public static final float GAMMA=1.9f;
	
	public String name;
	private char typeTag;
	private float number_argument;
	private boolean boolean_argument;
	public int sink;
	public FlokatiDevice parent;
	public DeviceControlFrontendHandler frontend;
	public boolean isValid=false;
	
	public byte[] getSinkAndValueBytes() {
		byte toreturn[];
		
		if (this.typeTag==FlokatiDeviceControl.BOOLEAN) {
			toreturn=new byte[2];
			if (this.boolean_argument) toreturn[1]=1;
			else toreturn[1]=0;
		}
		else if (this.typeTag==FlokatiDeviceControl.UNSIGNED_SHORT_INTEGER) {
			int corrected = (int)Math.round( 
					(float)UNSIGNED_SHORT_MAX * Math.pow( this.number_argument, GAMMA) );
			
			toreturn=new byte[3];
			toreturn[1]=(byte) (corrected>>8);
			toreturn[2]=(byte) (corrected);
		}
		else if (this.typeTag==FlokatiDeviceControl.SHORT_INTEGER) {
			int corrected;
			if (this.number_argument<0)
				corrected = (int)Math.round( 
						(float)SHORT_MIN * Math.pow( -this.number_argument, GAMMA) );
			else
				corrected = (int)Math.round( 
						(float)SHORT_MAX * Math.pow( this.number_argument, GAMMA) );
			
			toreturn=new byte[3];
			toreturn[1]=(byte) (corrected>>8);
			toreturn[2]=(byte) (corrected);
		}
		else if (this.typeTag==FlokatiDeviceControl.UNSIGNED_INTEGER) {
			long corrected = Math.round( 
					(double)UNSIGNED_INT_MAX * Math.pow( this.number_argument, GAMMA) );
			
			toreturn=new byte[5];
			toreturn[1]=(byte) (corrected>>24);
			toreturn[2]=(byte) (corrected>>16);
			toreturn[3]=(byte) (corrected>>8);
			toreturn[4]=(byte) (corrected);
		}
		else if (this.typeTag==FlokatiDeviceControl.INTEGER) {
			long corrected;
			if (this.number_argument<0)
				corrected = Math.round( 
						(double)INT_MIN * Math.pow( -this.number_argument, GAMMA) );
			else
				corrected = Math.round( 
						(double)INT_MAX * Math.pow( this.number_argument, GAMMA) );
			
//			Log.v("getbytes","will send "+corrected);
			toreturn=new byte[5];
			toreturn[1]=(byte) (corrected>>24);
			toreturn[2]=(byte) (corrected>>16);
			toreturn[3]=(byte) (corrected>>8);
			toreturn[4]=(byte) (corrected);
			
		} else {
			toreturn=new byte[1];
		}
		toreturn[0]=(byte) this.sink;
		
		return toreturn;
	}
	
	public int updateFromPacket(byte[]packet, int pos) {
//		Log.v(this.name, "update from packet");
		//pos should point to the first char of the value
		if (this.typeTag==FlokatiDeviceControl.BOOLEAN) {
			if (pos>=32) return 32;
			if (packet[pos]>0) this.boolean_argument=true;
			else this.boolean_argument=false;
			if (this.frontend!=null) this.frontend.updateFrontend();
			return pos+1;
		}
		else if (this.typeTag==FlokatiDeviceControl.UNSIGNED_SHORT_INTEGER) {
			if (pos+1>=32) return 32;
			int raw = ((packet[pos]<<8)&0xff00);
			raw += (packet[pos+1]&0xff);
			
			this.number_argument =(float) Math.pow(((float)raw)/((float)UNSIGNED_SHORT_MAX), 1f/GAMMA);
			
			if (this.frontend!=null) this.frontend.updateFrontend();
			return pos+2;
		}
		else if (this.typeTag==FlokatiDeviceControl.SHORT_INTEGER) {
			if (pos+1>=32) return 32;
			int raw = (packet[pos])<<8;
			raw |= (packet[pos+1]&0xff);
			
			if (raw<0)
				this.number_argument = (float) -Math.pow(((float)raw)/((float)SHORT_MIN), 1f/GAMMA);
			else
				this.number_argument = (float) Math.pow(((float)raw)/((float)SHORT_MAX), 1f/GAMMA);

			if (this.frontend!=null) this.frontend.updateFrontend();
			return pos+2;
		}
		else if (this.typeTag==FlokatiDeviceControl.UNSIGNED_INTEGER) {
			if (pos+3>=32) return 32;
			long raw = ((((long)packet[pos])&0xff)<<24);
			raw += ((packet[pos+1]&0xff)<<16);
			raw += ((packet[pos+2]&0xff)<<8);
			raw += (packet[pos+3]&0xff);
			
			this.number_argument = (float) Math.pow(((double)raw)/((double)UNSIGNED_INT_MAX), 1f/GAMMA);
//			Log.v("getbytes","raw: "+raw+" float: "+this.number_argument);
			if (this.frontend!=null) this.frontend.updateFrontend();
			return pos+4;
		}
		else if (this.typeTag==FlokatiDeviceControl.INTEGER) {
			if (pos+3>=32) return 32;
			int raw = (packet[pos])<<24;
			raw |= ((packet[pos+1]&0xff)<<16);
			raw |= ((packet[pos+2]&0xff)<<8);
			raw |= (packet[pos+3]&0xff);
			
			if (raw<0)
				this.number_argument = (float) -Math.pow(((float)raw)/((float)INT_MIN), 1f/GAMMA);
			else
				this.number_argument = (float) Math.pow(((float)raw)/((float)INT_MAX), 1f/GAMMA);
			
//			Log.v("getbytes",String.format("int: %x %x %x %x", packet[pos], packet[pos+1], packet[pos+2], packet[pos+3]));
//			Log.v("getbytes","int raw: "+raw+" float: "+this.number_argument);
			
			if (this.frontend!=null) this.frontend.updateFrontend();
			return pos+4;
		}
		return 32; // unknown type, so skip the rest of the packet
	}
	static public short getNeededSize(char type) {
		if (type==FlokatiDeviceControl.BOOLEAN) return 1;
		else if (type==FlokatiDeviceControl.UNSIGNED_SHORT_INTEGER) return 2;
		else if (type==FlokatiDeviceControl.SHORT_INTEGER) return 2;
		else if (type==FlokatiDeviceControl.UNSIGNED_INTEGER) return 4;
		else if (type==FlokatiDeviceControl.INTEGER) return 4;
		else return -1;
	}
	
	public FlokatiDeviceControl(FlokatiDevice parent, int sink, String name, String typeStr) {
		// packet_pos should point to the first char of the value
		this.parent=parent;
		
		if (sink<=0 || sink>=255) return;
		this.sink=sink;
		
		if (typeStr.equals("int") || typeStr.equals("integer"))
			this.typeTag=FlokatiDeviceControl.INTEGER;
		else if (typeStr.equals("unsigned int") || typeStr.equals("unsigned integer"))
			this.typeTag=FlokatiDeviceControl.UNSIGNED_INTEGER;
		else if (typeStr.equals("short") || typeStr.equals("short int") || 
				typeStr.equals("short integer"))
			this.typeTag=FlokatiDeviceControl.SHORT_INTEGER;
		else if (typeStr.equals("unsigned short") || typeStr.equals("unsigned short int") || 
				typeStr.equals("unsigned short integer"))
			this.typeTag=FlokatiDeviceControl.UNSIGNED_SHORT_INTEGER;
		else if (typeStr.equals("bool") || typeStr.equals("boolean"))
			this.typeTag=FlokatiDeviceControl.BOOLEAN;
		else return;
		
		if (name.equals("")) return;
		this.name=name;
		this.isValid=true;
	}
	
	public float getProgress() {
		if (this.typeTag!=BOOLEAN)
			return this.number_argument;
		else {
			if (this.boolean_argument) return 1;
			else return 0;
		}
	}
	public void setProgress(float progress) {
		this.number_argument=progress;
		
		Message msg=new Message();
		msg.obj=this.parent.getMessageBytes(this.sink);
		msg.arg1=FlokatiApp.TOSOCKET;
		FlokatiApp.controlSocketHandler.sendMessage(msg);
	}
	
	public boolean getChecked() {
		if (this.typeTag==FlokatiDeviceControl.BOOLEAN)
			return boolean_argument;
		return false;
	}
	public void setChecked(boolean checked) {
		this.boolean_argument=checked;
		Message msg=new Message();
		msg.obj=this.parent.getMessageBytes(this.sink);
		msg.arg1=FlokatiApp.TOSOCKET;
		FlokatiApp.controlSocketHandler.sendMessage(msg);
	}
	
	public int getViewType(){
		if (this.typeTag==FlokatiDeviceControl.SHORT_INTEGER || this.typeTag==FlokatiDeviceControl.INTEGER)
			return 1; //SeekBar with center
		if (this.typeTag==FlokatiDeviceControl.UNSIGNED_SHORT_INTEGER || this.typeTag==FlokatiDeviceControl.UNSIGNED_INTEGER)
			return 2; //normal Seekbar
		if (this.typeTag==FlokatiDeviceControl.BOOLEAN)
			return 3; //Checkbox
		return -1;
	}
	
	public void setOnChangeNotifier(DeviceControlFrontendHandler notifier) {
		if (notifier==null && this.frontend!=null) {
			this.frontend.unbind();
		}
		this.frontend=notifier;
	}
}
