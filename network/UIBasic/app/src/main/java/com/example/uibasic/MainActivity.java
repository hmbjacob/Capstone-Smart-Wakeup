package com.example.uibasic;

import android.Manifest;
import android.app.ProgressDialog;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcelable;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends Activity implements AdapterView.OnItemClickListener{
    private static final String TAG = "MAIN";

    //items for connection
    private static final UUID MY_UUID = UUID.fromString("8ce255c0-200a-11e0-ac64-0800200c9a66");
    BluetoothAdapter mBluetoothAdapter;
    BluetoothDevice mBTDevice;
    public BluetoothConnectionService mBluetoothConnection;
    public static ProgressDialog mProgressDialog=null;
    private TimePicker timePicker1;
    private TextView time;
    private Calendar calendar;
    private String format = "";
    private Button button;
    //buttons
    Button btnStartConnection;


    //how we display the found devices in the list
    public ArrayList<BluetoothDevice> mBTDevices = new ArrayList<>();
    public DeviceListAdapter mDeviceListAdapter;
    ListView DeviceList;


    //This receiver waits for bluetooth devices to be found when we discover
    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.d(TAG, "onReceive: ACTION FOUND.");

            if (action.equals(BluetoothDevice.ACTION_ACL_CONNECTED)){
                Log.d(TAG, "onReceive: ACTION GOT.");
                mProgressDialog.dismiss();
                pepon();
            }
        }
    };

    public void pepon(){
        Intent intent = new Intent(MainActivity.this, MenuPage.class);
        startActivity(intent);
    }

    public void startConnection(){
        if(mBTDevice !=null){
            mProgressDialog= ProgressDialog.show(MainActivity.this, "Connecting", "Please Wait...", true);
            IntentFilter discoverDevicesIntent = new IntentFilter(BluetoothDevice.ACTION_ACL_CONNECTED);
            registerReceiver(mBroadcastReceiver, discoverDevicesIntent);
            startBTConnection(mBTDevice,MY_UUID);
        } else{
            Toast.makeText(MainActivity.this,"Click a device first",Toast.LENGTH_SHORT).show();
        }

    }
    public void startBTConnection(BluetoothDevice device,UUID uuid) {
        Log.d(TAG, "startBTconnection");
        try {
            mBluetoothConnection.startClient(device, uuid);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }


    //required for any BT comms. called when we discover
    @RequiresApi(api = Build.VERSION_CODES.M)
    private void checkBTPermissions() {
        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.LOLLIPOP){
            int permissionCheck = this.checkSelfPermission("Manifest.permission.ACCESS_FINE_LOCATION");
            permissionCheck += this.checkSelfPermission("Manifest.permission.ACCESS_COARSE_LOCATION");
            if (permissionCheck != 0) {
                this.requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION, Manifest.permission.ACCESS_COARSE_LOCATION}, 1001); //Any number
            }
        }else{
            Log.d(TAG, "checkBTPermissions: No need to check permissions. SDK version < LOLLIPOP.");
        }
    }


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        btnStartConnection=(Button) findViewById((R.id.btnStartConnection));
        mBTDevices = new ArrayList<>();

        DeviceList=(ListView) findViewById(R.id.DeviceList);
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        DeviceList.setOnItemClickListener(MainActivity.this);

        Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();

        if (pairedDevices.size() > 0) {
            // There are paired devices. Get the name and address of each paired device.
            for (BluetoothDevice device : pairedDevices) {
                mBTDevices.add(device);
            }
        }
        mDeviceListAdapter = new DeviceListAdapter(getApplicationContext(), R.layout.device_adapter_view, mBTDevices);
        DeviceList.setAdapter(mDeviceListAdapter);

        btnStartConnection.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startConnection();
            }
        });
        Log.d(TAG, "finished startup");
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy: called.");
        super.onDestroy();
        unregisterReceiver(mBroadcastReceiver);
    }
    //sets the list so clicking an item will select it for connection
    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        //first cancel discovery because its very memory intensive.
        mBluetoothAdapter.cancelDiscovery();

        Log.d(TAG, "onItemClick: You Clicked on a device.");
        String deviceName = mBTDevices.get(i).getName();
        String deviceAddress = mBTDevices.get(i).getAddress();

        Log.d(TAG, "onItemClick: deviceName = " + deviceName);
        Log.d(TAG, "onItemClick: deviceAddress = " + deviceAddress);

        //create the bond.
        //NOTE: Requires API 17+? I think this is JellyBean
        if(Build.VERSION.SDK_INT > Build.VERSION_CODES.JELLY_BEAN_MR2){
            Log.d(TAG, "Trying to pair with " + deviceName);
            mBTDevices.get(i).createBond();

            mBTDevice = mBTDevices.get(i);
            //mBluetoothConnection = new BluetoothConnectionService(MainActivity.this);
            mBluetoothConnection = BluetoothConnectionService.getInstance();

            // need to set Context to use ProgressDialog.
            mBluetoothConnection.setContext(getApplicationContext());
            Log.d(TAG, "Paired with " + deviceName);
        }
    }
}