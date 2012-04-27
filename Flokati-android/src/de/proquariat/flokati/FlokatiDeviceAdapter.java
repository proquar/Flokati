package de.proquariat.flokati;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import de.proquariat.flokati.R;

public class FlokatiDeviceAdapter extends BaseAdapter {
	private Context context;
	
	public FlokatiDeviceAdapter(Context context) {
		this.context=context;
	}
	
	public void unbindDevices() {
		for (FlokatiDevice d : FlokatiApp.devices) {
			for (FlokatiDeviceControl c : d.Controls) {
				if (c!=null) c.setOnChangeNotifier(null);
			}
		}
	}
	
	public int getCount() {
		int c=0;
		
		for (FlokatiDevice d : FlokatiApp.devices) {
			c++; //Header
			c+=d.getSize();
		}
		
		return c;
	}

	public Object getItem(int arg0) {
		if (arg0<this.getCount()) {
			int c=0;
			
			for (FlokatiDevice d : FlokatiApp.devices) {
				if (c==arg0) return d;
				c++; //skip Header
				for (FlokatiDeviceControl dc : d.Controls) {
					if (c==arg0) return dc;
					c++;
				}
			}
		}
		
		return null;
	}

	public long getItemId(int arg0) {
		Object thing = this.getItem(arg0);
		if (thing instanceof FlokatiDevice) {
			FlokatiDevice d = (FlokatiDevice)thing;
			return (d.id<<8);
		}
		else if (thing instanceof FlokatiDeviceControl){
			FlokatiDeviceControl dc = (FlokatiDeviceControl)thing;
			return dc.parent.id+dc.sink;
		}
		
		return 0;
	}

	public int getItemViewType(int arg0) {
		Object thing = this.getItem(arg0);
		if (thing instanceof FlokatiDevice) {
			return 0;
		}
		else if (thing instanceof FlokatiDeviceControl){
			FlokatiDeviceControl dc = (FlokatiDeviceControl)thing;
			return dc.getViewType();
		}
		
		return -1;
	}

	public View getView(int arg0, View arg1, ViewGroup arg2) {
		LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
		Object thing = this.getItem(arg0);
		if (thing instanceof FlokatiDevice) {
			FlokatiDevice d = (FlokatiDevice)thing;
			LinearLayout ll = (LinearLayout)inflater.inflate(R.layout.device_header, null);
			TextView tv = (TextView)ll.getChildAt(0);
			tv.setText(d.name);
			tv.destroyDrawingCache();
			return ll;
		}
		else if (thing instanceof FlokatiDeviceControl){
			FlokatiDeviceControl dc = (FlokatiDeviceControl)thing;
			LinearLayout ll;
			TextView sinkName;

			if (dc.getViewType()==1 || dc.getViewType()==2) {
				if (dc.getViewType()==1)
					ll = (LinearLayout)inflater.inflate(R.layout.list_item_nslider, null);
				else
					ll = (LinearLayout)inflater.inflate(R.layout.list_item_slider, null);
				sinkName = (TextView)((LinearLayout)ll.getChildAt(0)).getChildAt(0);
				TextView target=(TextView)((LinearLayout)ll.getChildAt(0)).getChildAt(1);
				SeekBar slider=(SeekBar)ll.getChildAt(1);
				if (dc.getViewType()==1)
					slider.setProgressDrawable(this.context.getResources().getDrawable(R.drawable.transparent_progress));
				new DeviceControlFrontendHandler(dc,slider,target);
			}
			else if (dc.getViewType()==3) {
				ll = (LinearLayout)inflater.inflate(R.layout.list_item_toggle, null);
				sinkName = (TextView)ll.getChildAt(0);
				CheckBox t = (CheckBox) ll.getChildAt(1);
				new DeviceControlFrontendHandler(dc, t);
			}
			else {
				ll = (LinearLayout)inflater.inflate(R.layout.list_item, null);
				sinkName = (TextView)ll.getChildAt(0);
			}
			sinkName.setText(dc.name);
			return ll;
		} else {
			return (TextView)inflater.inflate(R.layout.list_item_undiscovered, null);
		}
	}

	public int getViewTypeCount() {
		return 4;
	}

	public boolean hasStableIds() {
		return true;
	}

	public boolean isEmpty() {
		if (FlokatiApp.devices.size()>0) return false;
		else return true;
	}

	public boolean areAllItemsEnabled() {
		return false;
	}

	public boolean isEnabled(int arg0) {
		Object thing = this.getItem(arg0);
		if (thing instanceof FlokatiDevice) {
			return false;
		}
		return true;
	}
}
