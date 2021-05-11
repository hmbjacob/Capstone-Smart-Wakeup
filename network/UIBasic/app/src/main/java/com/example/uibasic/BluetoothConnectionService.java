package com.example.uibasic;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
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
    Context mContext;
    //States. 0=idle,1=connected
    int connected=0;

    private static class InstanceHolder {
        private static final BluetoothConnectionService INSTANCE = new BluetoothConnectionService();
    }

    public static BluetoothConnectionService getInstance() {
        return InstanceHolder.INSTANCE;
    }

    private BluetoothConnectionService() {
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        start();
    }
    public void setContext(Context context) {
        mContext = context;
    }

    //public BluetoothConnectionService(Context context){
    //    mContext = context;
     //   mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    //    start();
    //}
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
            connected=1;
            mmSocket = tmp;
            //mBluetoothAdapter.cancelDiscovery();
            try {
                mmSocket.connect();
                Log.d(TAG,"connection worked");
                connected=1;
            } catch (IOException e) {
                try {
                    mmSocket.close();
                } catch (IOException ioException) {
                    ioException.printStackTrace();
                }
                e.printStackTrace();
                connected=0;
            }
            //all worked
            connected(mmSocket,mmDevice);
        }

    }
    public synchronized void start(){
        Log.d(TAG,"start");
        if (mConnectThread != null){
            mConnectThread = null;
        }
    }

    public void startClient(BluetoothDevice device,UUID uuid) throws InterruptedException {
        Log.d(TAG,"started client");
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
            Intent intent = new Intent();
            intent.setAction("com.example.uibasic.DONECONNECT");
            intent.putExtra("data", "Nothing to see here, move along.");
            mContext.sendBroadcast(intent);
            byte[] buffer = new byte[1024];
            //keep listening until exception
            while(true){
                try {
                    //get msg from pi
                    mmInStream.read(buffer);
                    Log.d(TAG,"inputstream received msg");
                    int opcode = (int)buffer[0];
                    switch(opcode){
                        case 50: //received OK to start receiving file contents
                            fileGet(1);
                            fileGet(2);
                            Intent intent2 = new Intent();
                            intent2.setAction("com.example.uibasic.DONEFILE");
                            intent2.putExtra("data", "Nothing to see here, move along.");
                            mContext.sendBroadcast(intent2);
                            Log.d(TAG, "finished getting file");
                        case 51:
                            break;
                        case 49:
                            Log.d(TAG, "got settings opcode back");
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    break;
                }
            }
        }

        public void fileGet(int which){
            try {
                byte[] buffer = new byte[1024];
                File outputDir = mContext.getCacheDir();
                String ending = (which == 1) ? "state_output.txt" : "accel_output.txt";
                String dataFile = outputDir + "/" + File.separator + ending;
                Log.d(TAG, "before");
                File output;

                output = new File(dataFile);
                OutputStream fo;
                try {
                    fo = new FileOutputStream(output, false);
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                    return;
                }
                Log.d(TAG, "file created: " + output);
                Log.d(TAG, "tell pi you want the file");
                byte[] sendOP = "4".getBytes(); //opcode for telling pi you received data
                mmOutStream.write(sendOP);
                int broken = 0;
                int r = 0;
                /*
                This Loop continuously receives either opcodes or chunks of data.
                To control data flow, waits for opcode from sender
                 */
                while (broken == 0 && (r = mmInStream.read(buffer)) > 0) {
                    Log.d(TAG, "received data, code=" + (int) buffer[0]);
                    Log.d(TAG, "len= " + r);
                    if (r == 1) { //received opcode
                        if ((int) buffer[0] == 4) {
                            Log.d(TAG, "breaking out");
                            broken = 1;
                            break;
                        } else {
                            fo.write(buffer, 0, 1);
                            Log.d(TAG, "wrote to file");
                            mmOutStream.write(sendOP);
                        }
                    } else { //received file contents
                        fo.write(buffer, 0, r);
                        Log.d(TAG, "wrote to file");
                        mmOutStream.write(sendOP);
                    }

                }
                fo.close();
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        //call from outside thread to send data to server
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

    public void end(){
        Log.d(TAG,"ending");
        mConnectedThread.cancel();
    }

    //write method that accesses connection service
    public void write(byte[] out){
        ConnectedThread r;
        Log.d(TAG, "write: Write Called.");
        mConnectedThread.write(out);

    }
}
