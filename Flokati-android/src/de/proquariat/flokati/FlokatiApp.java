package de.proquariat.flokati;

import java.util.Vector;

import android.app.Application;
import android.os.Handler;
import android.os.Message;

public class FlokatiApp extends Application {
	
	static FlokatiActivity activity=null;
	static RemoteConnectionService connection=null;
	static Vector<FlokatiDevice> devices = new Vector<FlokatiDevice>();
	static PacketParser parser=new PacketParser();

	static final int FROMSOCKET=1;
	static final int TOSOCKET=2;
	//
    static Handler controlSocketHandler = new Handler() {
    	public void handleMessage(Message msg) {
    		if (msg.arg1==FROMSOCKET) {
    			if (parser!=null) parser.newPacket((byte[])msg.obj);
    		}
    		else if (msg.arg1==TOSOCKET) {
    			if (connection!=null && connection.controlThread!=null)
    				connection.controlThread.sendPacket((byte[])msg.obj);
    		}
    	}
	};
	
	static Handler infoSocketHandler = new Handler() {
    	public void handleMessage(Message msg) {
    		if (msg.arg1==FROMSOCKET) {
    			FlokatiDevice newDev = (FlokatiDevice)msg.obj;
    			for (FlokatiDevice d: FlokatiApp.devices) {
    				if (d.id==newDev.id) return;
    			}
    			FlokatiApp.devices.add((FlokatiDevice)msg.obj);
    			if (FlokatiApp.activity!=null)
    				FlokatiApp.activity.flokatiDeviceList.notifyDataSetChanged();
    		}
    		else if (msg.arg1==TOSOCKET) {
    			if (connection!=null && connection.infoThread!=null)
    				connection.infoThread.sendPacket((DeviceRequest)msg.obj);
    		}
    	}
	};
	
	@Override
	public void onCreate() {
		// TODO Auto-generated method stub
		super.onCreate();
	}
	static public String getLanguage() {
		return activity.getResources().getConfiguration().locale.getLanguage();
	}
}
