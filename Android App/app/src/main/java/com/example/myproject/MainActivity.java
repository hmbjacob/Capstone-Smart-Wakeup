package com.example.myproject;

import androidx.appcompat.app.AppCompatActivity;
import androidx.fragment.app.DialogFragment;

import android.app.TimePickerDialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;

import java.util.Calendar;
import java.util.Date;

import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.TimePicker;

import static java.lang.reflect.Array.getInt;

public class MainActivity extends Activity {
    private TimePicker timePicker1;
    private TextView time;
    private Calendar calendar;
    private String format = "";
    private Button button;

    //Object to access key/val stores
    SharedPreferences pref;
    //keys for hour/minute values
    private String HourTimeKey = "com.example.myproject.hour";
    private String MinTimeKey = "com.example.myproject.minute";
    int wakeupHour;
    int wakeupMin;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        time = (TextView) findViewById(R.id.textView1);
        button = (Button) findViewById(R.id.Graph_Button);
        calendar = Calendar.getInstance();
        //create object and get previously set values
        pref = this.getSharedPreferences("com.example.myproject", Context.MODE_PRIVATE);
        wakeupHour = pref.getInt(HourTimeKey, calendar.get(Calendar.HOUR_OF_DAY));
        wakeupMin = pref.getInt(MinTimeKey, calendar.get(Calendar.MINUTE));

        showTime(wakeupHour, wakeupMin);

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v){
                openGraphPage();
            }
        });


    }
    /* Display timepicker dialog on button press */
    public void setTime(View view) {
        TimePickerDialog timePicker = new TimePickerDialog(this, new TimePickerDialog.OnTimeSetListener() {

            /* Says what happens once "ok" is pressed on the dialog */
            @Override
            public void onTimeSet(TimePicker timePicker, int h, int m) {
                showTime(h, m);
                wakeupHour=h;
                wakeupMin=m;
                //Sets both key/val stores
                pref.edit().putInt(HourTimeKey, h).apply();
                pref.edit().putInt(MinTimeKey, m).apply();
            }
        },wakeupHour,wakeupMin,false);
        timePicker.setTitle("Select Time");
        timePicker.show();
    }

    public void showTime(int hour, int min) {
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

        time.setText(new StringBuilder().append(hour).append(" : ").append(min)
                .append(" ").append(format));
    }

    public void openGraphPage(){
        Intent intent = new Intent(this, GraphPage.class);
        startActivity(intent);
    }


    }

