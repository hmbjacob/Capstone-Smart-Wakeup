package com.example.uibasic;

import android.Manifest;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import java.util.ArrayList;
import java.util.Set;
import java.util.UUID;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemClickListener{
    private static final String TAG = "MAIN";

    //items for connection
    private static final UUID MY_UUID = UUID.fromString("8ce255c0-200a-11e0-ac64-0800200c9a66");
    BluetoothAdapter mBluetoothAdapter;
    BluetoothDevice mBTDevice;
    BluetoothConnectionService mBluetoothConnection;
    ProgressDialog mProgressDialog=null;

    //buttons
    Button btnStartConnection;
    Button btnChangeSettings;
    Button btnRequestFile;

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

            if (action.equals(BluetoothDevice.ACTION_FOUND)){
                BluetoothDevice device = intent.getParcelableExtra (BluetoothDevice.EXTRA_DEVICE);
                mBTDevices.add(device);
                Log.d(TAG, "onReceive: " + device.getName() + ": " + device.getAddress());
                mDeviceListAdapter = new DeviceListAdapter(context, R.layout.device_adapter_view, mBTDevices);
                DeviceList.setAdapter(mDeviceListAdapter);
            }
        }
    };

    public void startConnection(){
        if(mBTDevice !=null){
            startBTConnection(mBTDevice,MY_UUID);
        } else{
            Toast.makeText(MainActivity.this,
                    "Click a device first",
                    Toast.LENGTH_SHORT).show();
        }

    }
    public void startBTConnection(BluetoothDevice device,UUID uuid) {
        Log.d(TAG, "startBTconnection");
        mBluetoothConnection.startClient(device, uuid);
        Toast.makeText(MainActivity.this,
                R.string.success,
                Toast.LENGTH_SHORT).show();
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
        btnChangeSettings=(Button) findViewById((R.id.btnChangeSettings));
        btnRequestFile=(Button) findViewById((R.id.btnRequestFile));
        mBTDevices = new ArrayList<>();

        DeviceList=(ListView) findViewById((R.id.DeviceList));
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

        btnChangeSettings.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage("1",0);
            }
        });
        btnRequestFile.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendMessage("2",1);
            }
        });
        Log.d(TAG, "finished startup");
    }

    private void sendMessage(String message,int cas) {
        // Check that we're actually connected before trying anything
        if (mBluetoothConnection.connected==0) {
            Toast.makeText(MainActivity.this, R.string.connected_bool, Toast.LENGTH_SHORT).show();
            return;
        }
        mProgressDialog=null;
        // Check that there's actually something to send
        if (message.length() > 0) {
            // Get the message bytes and tell the BluetoothChatService to write
            byte[] send = message.getBytes();
            if (cas==1){
                mProgressDialog= ProgressDialog.show(MainActivity.this, "Transferring file", "Please Wait...", true);
            }
            mBluetoothConnection.write(send,1,mProgressDialog);
        }
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
            mBluetoothConnection = new BluetoothConnectionService(MainActivity.this);
            Log.d(TAG, "Paired with " + deviceName);
        }
    }
}