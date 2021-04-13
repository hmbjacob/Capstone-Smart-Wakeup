package com.example.uibasic;

import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.Charset;
import java.util.UUID;

public class BluetoothConnectionService{
    private static final String TAG ="BTCommserv";
    private static final String appName = "MyApp";
    private static final UUID MY_UUID = UUID.fromString("8ce255c0-200a-11e0-ac64-0800200c9a66");
    private final BluetoothAdapter mBluetoothAdapter;
    private ConnectThread mConnectThread;
    private ConnectedThread mConnectedThread;
    private BluetoothDevice mmDevice;
    private UUID deviceUUID;
    ProgressDialog mProgressDialog;
    Context mContext;

    public BluetoothConnectionService(Context context){
        mContext = context;
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    //runs while attempting to make outgoing connection
    private class ConnectThread extends Thread{
        private BluetoothSocket mmSocket;
        public ConnectThread(BluetoothDevice device, UUID uuid){
            Log.d(TAG,"Started connect thread with uuid="+uuid);
            mmDevice = device;
            deviceUUID=uuid;
        }
        public void run(){
            BluetoothSocket tmp=null;
            Log.i(TAG,"run,uuid="+deviceUUID);
            try {
                tmp=mmDevice.createRfcommSocketToServiceRecord(deviceUUID);
            } catch (IOException e) {
                e.printStackTrace();
            }
            mmSocket = tmp;
            mBluetoothAdapter.cancelDiscovery();
            try {
                mmSocket.connect();
                Log.d(TAG,"connection worked");
            } catch (IOException e) {
                try {
                    mmSocket.close();
                } catch (IOException ioException) {
                    ioException.printStackTrace();
                }
                e.printStackTrace();
            }

            //all worked
            connected(mmSocket,mmDevice);
        }
        public void cancel() {
            try {
                Log.d(TAG, "cancel: Closing Client Socket.");
                mmSocket.close();
            } catch (IOException e) {
                Log.e(TAG, "cancel: close() of mmSocket in Connectthread failed. " + e.getMessage());
            }
        }

    }
    public synchronized void start(){
        Log.d(TAG,"start");
        if (mConnectThread != null){
            mConnectThread = null;
        }
    }

    public void startClient(BluetoothDevice device,UUID uuid){
        Log.d(TAG,"started client");
        mProgressDialog = ProgressDialog.show(mContext,"Connecting Bluetooth"
                ,"Please Wait...",true);
        mConnectThread = new ConnectThread(device,uuid);
        mConnectThread.start();
    }

    private class ConnectedThread extends Thread{
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        //constructor method. grab input and output streams from connected socket
        public ConnectedThread(BluetoothSocket socket){
            Log.d(TAG,"starting connected thread");
            mmSocket = socket;
            InputStream tmpIn=null;
            OutputStream tmpOut=null;
            mProgressDialog.dismiss();
            try {
                tmpIn=mmSocket.getInputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
            try {
                tmpOut=mmSocket.getOutputStream();
            } catch (IOException e) {
                e.printStackTrace();
            }
            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }
        public void run(){
            byte[] buffer = new byte[1024];
            int bytes;
            //keep listening until exception
            while(true){
                try {
                    bytes = mmInStream.read(buffer);
                    String incomingMessage = new String(buffer,0,bytes);
                    Log.d(TAG,"inputstream received msg");
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }
        }
        //call from main activity to send data to server
        public void write(byte[] bytes){
            String text = new String(bytes,Charset.defaultCharset());
            try {
                mmOutStream.write(bytes);
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        public void cancel(){
            try{
                mmSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
    //establishes and starts new connected thread
    private void connected(BluetoothSocket mmSocket,BluetoothDevice mmDevice){
        Log.d(TAG,"connected. starting");
        mConnectedThread = new ConnectedThread(mmSocket);
        mConnectedThread.start();
    }

    //write method that accesses connection service
    public void write(byte[] out){
        ConnectedThread r;
        Log.d(TAG, "write: Write Called.");
        mConnectedThread.write(out);

    }
}
