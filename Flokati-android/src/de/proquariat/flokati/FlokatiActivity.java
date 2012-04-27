package de.proquariat.flokati;

import android.app.ListActivity;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import de.proquariat.flokati.R;

public class FlokatiActivity extends ListActivity {
	
	FlokatiDeviceAdapter flokatiDeviceList;
	
	@Override
	public void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);
		
		this.flokatiDeviceList = new FlokatiDeviceAdapter(this);
		
		setListAdapter(flokatiDeviceList);
		FlokatiApp.activity=this;
		
		if (FlokatiApp.connection==null) {
			NotificationManager mNM = (NotificationManager)this.getSystemService(Context.NOTIFICATION_SERVICE);
			mNM.cancel(RemoteConnectionService.NOTIFICATION);
		}
		
	}
	public void onResume() {
		super.onResume();
	}
	public void onPause() {
		super.onPause();
	}

	public void onStop() {
		super.onStop();
	}
	
	public void onDestroy() {
		super.onDestroy();
		this.flokatiDeviceList.unbindDevices();
		this.flokatiDeviceList=null;
		FlokatiApp.activity=null;
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	    MenuInflater inflater = getMenuInflater();
	    inflater.inflate(R.menu.main, menu);
	    this.setActiveConnections(menu);
	    return true;
	}
	
	public boolean onPrepareOptionsMenu (Menu menu) {
		super.onPrepareOptionsMenu(menu);
		this.setActiveConnections(menu);
		return true;
	}
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    // Handle item selection
	    switch (item.getItemId()) {
	        case R.id.clear_list:
	        	FlokatiApp.devices.clear();
	        	flokatiDeviceList.notifyDataSetInvalidated();
	            return true;
	        case R.id.wlan_enable:
	        	if (FlokatiApp.connection==null || !FlokatiApp.connection.getWLANState()) {
	        		FlokatiApp.devices.clear();
	        		this.flokatiDeviceList.notifyDataSetInvalidated();
	        		startService(new Intent(this, RemoteConnectionService.class));
	        	} else {
	        		FlokatiApp.connection.stopConnection();
	        	}
	            return true;
	        default:
	            return super.onOptionsItemSelected(item);
	    }
	}
	
	public void setActiveConnections(Menu menu) {
		if (menu!=null) {
			MenuItem j = (MenuItem)menu.findItem(R.id.wlan_enable);
			if (FlokatiApp.connection!=null && FlokatiApp.connection.getWLANState())
				j.setTitle(R.string.wifi_disconnect);
			else
				j.setTitle(R.string.wifi_connect);
			j.setTitleCondensed(j.getTitle());
		}
	}
}