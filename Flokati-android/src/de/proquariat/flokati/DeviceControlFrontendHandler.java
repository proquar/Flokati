package de.proquariat.flokati;

import android.widget.CheckBox;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;
import android.widget.CompoundButton;
import android.widget.TextView;

public class DeviceControlFrontendHandler {
	
	final SeekBar slider;
	final TextView textual;
	final FlokatiDeviceControl dc;
	final CheckBox button;
	
	public DeviceControlFrontendHandler(FlokatiDeviceControl ndc, SeekBar nslider, TextView ntextual) {
		dc=ndc;
		slider=nslider;
		textual=ntextual;
		button=null;
		
		updateFrontend();
		dc.setOnChangeNotifier(this);
		
		slider.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {
			
			public void onStopTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			public void onStartTrackingTouch(SeekBar seekBar) {
				// TODO Auto-generated method stub
				
			}
			
			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
				if (fromUser) {
					if (dc.getViewType()==1)
						dc.setProgress(((float)(progress-32768))/32768f);
					else if (dc.getViewType()==2)
						dc.setProgress(((float)progress)/65536f);
					updateFrontend();
				}
			}
		});
	}
	
	public DeviceControlFrontendHandler(FlokatiDeviceControl ndc, CheckBox toggl) {
		dc=ndc;
		button=toggl;
		slider=null;
		textual=null;
		
		updateFrontend();
		dc.setOnChangeNotifier(this);
		
		button.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			
			public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
				if (dc.getChecked()!=isChecked)
					dc.setChecked(isChecked);
			}
		});
	}
	
	public void unbind() {
		//
	}
	
	public void updateFrontend(){
		if (button!=null) {
			button.setChecked(dc.getChecked());
		}
		if (textual!=null) {
			textual.setText(String.format("%.1f%%",dc.getProgress()*100f));
		}
		if (slider!=null) {
			if (dc.getViewType()==1)
				slider.setProgress((int)((dc.getProgress()+1f)*32768f));
			else if (dc.getViewType()==2)
				slider.setProgress((int)(dc.getProgress()*65536f));
		}
	}
}
