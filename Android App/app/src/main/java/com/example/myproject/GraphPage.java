package com.example.myproject;


import androidx.appcompat.app.AppCompatActivity;
import android.content.Intent;

import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;


import com.github.mikephil.charting.components.LimitLine;
import com.github.mikephil.charting.components.XAxis;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.github.mikephil.charting.interfaces.datasets.ILineDataSet;
import com.github.mikephil.charting.listener.OnChartGestureListener;
import com.github.mikephil.charting.listener.OnChartValueSelectedListener;

import java.util.ArrayList;

public class GraphPage extends AppCompatActivity {

    private static final String TAG = "GraphPage";

    private com.github.mikephil.charting.charts.LineChart mChart;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_graph_page);

        mChart = (com.github.mikephil.charting.charts.LineChart) findViewById(R.id.sleeplinechart);

        mChart.setDragEnabled(true);
        mChart.setScaleEnabled(false);


        ArrayList<Entry> yValues = new java.util.ArrayList<>();

        yValues.add(new com.github.mikephil.charting.data.Entry(0, 0f));
        yValues.add(new com.github.mikephil.charting.data.Entry(1, 0f));
        yValues.add(new com.github.mikephil.charting.data.Entry(2, -1f));
        yValues.add(new com.github.mikephil.charting.data.Entry(3, -1f));
        yValues.add(new com.github.mikephil.charting.data.Entry(4, 0f));
        yValues.add(new com.github.mikephil.charting.data.Entry(5, 0f));
        yValues.add(new com.github.mikephil.charting.data.Entry(6, 1f));
        yValues.add(new com.github.mikephil.charting.data.Entry(7, 1f));

        LineDataSet set1 = new LineDataSet(yValues, "SleepLine");
        set1.setLineWidth(5f);

        XAxis xAxis = mChart.getXAxis();
        xAxis.setDrawGridLines(false);

        YAxis yAxis = mChart.getAxisLeft();
        yAxis.setDrawLabels(false); // no axis labels
        yAxis.setDrawAxisLine(true);
        yAxis.setDrawGridLines(false);
        yAxis.setDrawZeroLine(true); // draw a zero line
        mChart.getAxisRight().setEnabled(false); // no right axis

        LimitLine wakeLine = new LimitLine(1f, "Wake");
        wakeLine.setLineColor(Color.RED);
        wakeLine.setLineWidth(4f);
        wakeLine.setTextColor(Color.BLACK);
        wakeLine.setTextSize(12f);
        yAxis.addLimitLine(wakeLine);


        LimitLine DeepLine = new LimitLine(-1f, "DeepSleep");
        DeepLine.setLineColor(Color.RED);
        DeepLine.setLineWidth(4f);
        DeepLine.setTextColor(Color.BLACK);
        DeepLine.setTextSize(12f);
        yAxis.addLimitLine(DeepLine);




        LineDataSet Sleepset = new LineDataSet(yValues, "Data Set 1");

        Sleepset.setFillAlpha(110);

        java.util.ArrayList<ILineDataSet> dataSets = new java.util.ArrayList<>();
        dataSets.add(set1);

        LineData data = new LineData(dataSets);

        mChart.setData(data);




    }


}