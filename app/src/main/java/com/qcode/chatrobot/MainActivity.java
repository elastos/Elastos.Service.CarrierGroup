package com.qcode.chatrobot;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("chatrobot");
    }
    private Handler mMainThreadHandler = new Handler(Looper.getMainLooper());
    private Context mContext;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mContext = this.getApplicationContext();
        // Example of a call to a native method
        final TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());
        
        new Thread(new Runnable() {
            @Override
            public void run() {
                startChatRobot(getLocalCacheDir());
                mMainThreadHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        tv.setText(getAddress());
                    }
                });
                
            }
        }).start();
        
    }
    
    public String getLocalCacheDir() {
        return mContext.getCacheDir() + "/";
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
    public native int startChatRobot(String data_dir);
    public native String getAddress();
}
