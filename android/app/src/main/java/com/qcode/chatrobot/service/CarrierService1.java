package com.qcode.chatrobot.service;
import android.util.Log;
public class CarrierService1 extends CarrierServiceBase {
    public static final String TAG = "CarrierService";
    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate()'ed");
        super.onCreate();
    }
    
    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()'ed");
        super.onDestroy();
    }
}