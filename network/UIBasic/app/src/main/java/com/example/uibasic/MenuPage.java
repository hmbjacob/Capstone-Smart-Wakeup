package com.example.uibasic;

import android.app.Activity;
import android.app.ProgressDialog;
import android.app.TimePickerDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;



import java.io.IOException;
import java.util.Calendar;

import static android.content.ContentValues.TAG;

public class MenuPage extends Activity {
    BluetoothConnectionService mBluetoothConnection;
    ProgressDialog mProgressDialog=null;

    private TimePicker timePicker1;
    private Calendar calendar;
    //Object to access key/val stores
    SharedPreferences pref;
    //keys for hour/minute values

    private String HourTimeKey = "com.example.uibasic.hour";
    private String MinTimeKey = "com.example.uibasic.minute";
    int wakeupHour;
    int wakeupMin;
    private String UseAlarmKey = "com.example.uibasic.useAlarm";
    private String ManualLightKey = "com.example.uibasic.manualLight";
    int manualLight;
    int useAlarm;
    private String predictKey = "com.example.uibasic.IntpredictTime";
    private String wakeupKey = "com.example.uibasic.IntwakeupTime";

    TextView time;
    Switch alarmSwitch;
    Switch lightSwitch;

    String wakeupInterval;
    String predictionInterval;
    EditText wakeupEdit;
    EditText predictionEdit;

    //waits for disconnect
    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.d(TAG, "onReceive: ACTION GOT to end process.");
            finish();
        }
    };

    //waits for full file transfer
    private BroadcastReceiver mBroadcastReceiver2 = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.d(TAG, "onReceive: ACTION FOUND.");
            mProgressDialog.dismiss();
            pepon();
        }
    };

    public void pepon(){
        Intent intent = new Intent(MenuPage.this, GraphPage.class);
        startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_menu_page);

        IntentFilter discoverDevicesIntent = new IntentFilter(BluetoothDevice.ACTION_ACL_DISCONNECTED);
        registerReceiver(mBroadcastReceiver, discoverDevicesIntent);

        mBluetoothConnection = BluetoothConnectionService.getInstance();
        calendar = Calendar.getInstance();
        pref = this.getSharedPreferences("com.example.uibasic", Context.MODE_PRIVATE);
        manualLight = pref.getInt(ManualLightKey, 0);
        useAlarm = pref.getInt(UseAlarmKey, 1);

        wakeupHour = pref.getInt(HourTimeKey, calendar.get(Calendar.HOUR_OF_DAY));
        wakeupMin = pref.getInt(MinTimeKey, calendar.get(Calendar.MINUTE));



        time = (TextView) findViewById(R.id.textView2);
        alarmSwitch=(Switch) findViewById(R.id.switch1);
        lightSwitch=(Switch) findViewById(R.id.switch2);
        wakeupEdit=(EditText) findViewById((R.id.editTextWak));
        predictionEdit=(EditText) findViewById((R.id.editTextPred));


        wakeupInterval= pref.getString(wakeupKey, "24");
        predictionInterval= pref.getString(predictKey, "30");

        wakeupEdit.setText(wakeupInterval);
        predictionEdit.setText(predictionInterval);

        showTime(wakeupHour, wakeupMin);
        alarmSwitch.setChecked(useAlarm == 1);
        lightSwitch.setChecked(manualLight == 1);

        Log.d(TAG, "finished startup");
    }
    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy: called.");
        super.onDestroy();
        mBluetoothConnection.end();
        try {
            unregisterReceiver(mBroadcastReceiver);
        }catch(IllegalArgumentException e) {
            //e.printStackTrace();
        }
        try {
            unregisterReceiver(mBroadcastReceiver2);
        }catch(IllegalArgumentException e) {
            //e.printStackTrace();
        }
    }
    @Override
    public void onBackPressed() {
        //mBluetoothConnection.end();
        finish();
    }


    /* Display timepicker dialog on button press */
    public void SetTime(View view) {
        TimePickerDialog timePicker = new TimePickerDialog(this, new TimePickerDialog.OnTimeSetListener() {

            /* Says what happens once "ok" is pressed on the dialog */
            @Override
            public void onTimeSet(TimePicker timePicker, int h, int m) {
                //showTime(h, m);
                wakeupHour=h;
                wakeupMin=m;
                //Sets both key/val stores
                pref.edit().putInt(HourTimeKey, h).apply();
                pref.edit().putInt(MinTimeKey, m).apply();
                showTime(wakeupHour, wakeupMin);
            }
        },wakeupHour,wakeupMin,false);
        timePicker.setTitle("Select Time");
        timePicker.show();
    }

    public void showTime(int hour, int min) {
        String format;
        if (hour == 0) {
            hour += 12;
            format = "AM";
        } else if (hour == 12) {
            format = "PM";
        } else if (hour > 12) {
            hour -= 12;
            format = "PM";
        } else {
            format = "AM";
        }

        if(wakeupMin <10){
            time.setText(String.valueOf(wakeupHour)+":0"+String.valueOf(wakeupMin));
        } else{
            time.setText(String.valueOf(wakeupHour)+":"+String.valueOf(wakeupMin));
        }
    }

    private void sendMessage(String message,int cas) {
        mProgressDialog=null;
        // Check that there's actually something to send
        if (message.length() > 0) {
            // Get the message bytes and tell the BluetoothChatService to write
            byte[] send = message.getBytes();
            if (cas==1){
                mProgressDialog= ProgressDialog.show(MenuPage.this, "Retrieving sleep data", "This may take a while...", true);
            }
            mBluetoothConnection.write(send);

        }
    }

    public void UpdateSettings(View view) {
        String setting;
        wakeupInterval=wakeupEdit.getText().toString();
        predictionInterval=predictionEdit.getText().toString();
        pref.edit().putString(predictKey, predictionInterval).apply();
        pref.edit().putString(wakeupKey, wakeupInterval).apply();
        if(wakeupMin <10){
            setting = "1."+String.valueOf(wakeupHour)+":0"+String.valueOf(wakeupMin)+";"+String.valueOf(useAlarm)+";"+String.valueOf(manualLight)+";"+wakeupInterval+";"+predictionInterval;
        } else{
            setting = "1."+String.valueOf(wakeupHour)+":"+String.valueOf(wakeupMin)+";"+String.valueOf(useAlarm)+";"+String.valueOf(manualLight)+";"+wakeupInterval+";"+predictionInterval;
        }

        sendMessage(setting,0);
        Toast.makeText(MenuPage.this,"Settings updated!",Toast.LENGTH_SHORT).show();
    }

    public void OpenGraph(View view) {
        IntentFilter discoverDevicesIntent = new IntentFilter("com.example.uibasic.DONEFILE");
        registerReceiver(mBroadcastReceiver2, discoverDevicesIntent);
        sendMessage("2",1);
    }

    public void checkAlarm(View view) {
        useAlarm = 1-useAlarm;
    }
    public void checkLight(View view) {
        manualLight = 1-manualLight;
    }
}
