package com.qcode.chatrobot;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.os.Handler;
import android.os.Looper;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.qcode.chatrobot.ui.MemberInfo;
import com.qcode.chatrobot.ui.MemberListAdapter;
import com.qcode.chatrobot.ui.QRCode;

import org.w3c.dom.Text;

import java.lang.reflect.Member;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends Activity {
    
    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("chatrobot");
    }
    private ListView mMemberListView;
    private Handler mMainThreadHandler = new Handler(Looper.getMainLooper());
    private Context mContext;
    
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mContext = this.getApplicationContext();
        TextView text_address = findViewById(R.id.text_address);
        TextView text_userid = findViewById(R.id.text_userid);
        startChatRobot(getLocalCacheDir());
        String address = getAddress();
        String user_id = getUserId();
        ImageView view = findViewById(R.id.qr_image);
        MyQRCode qrcode = new MyQRCode(-1, 1);
        view.setImageBitmap(qrcode.getBitmap(address, 512, 512));
        text_address.setText(address);
        text_userid.setText(user_id);
        new Thread(new Runnable() {
            @Override
            public void run() {
                runChatRobot();
            }
        }).start();
    
        mMemberListView = (ListView) findViewById(R.id.memberList);
        //获取当前ListView点击的行数，并且得到该数据
        mMemberListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            
            }
        });
        MemberInfo[] member_info = getMemberList();
        
        MemberListAdapter adapter = new MemberListAdapter(this.getLayoutInflater());
        adapter.setData(member_info);
        mMemberListView.setAdapter(adapter);
    }
    
    class MyQRCode extends QRCode {
        public MyQRCode(int typeNumber, int errorCorrectLevel) {
            super(typeNumber, errorCorrectLevel);
        }
        
        public Bitmap getBitmap(String src, int width, int height) {
            addData(src);
            make();
            //1.创建Bitmap
            Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
            Canvas canvas = new Canvas();
            canvas.setBitmap(bitmap);
            // compute tileW/tileH based on options.width/options.height
            float tileW = (float) width / (getModuleCount() + 4);
            float tileH = (float) height / (getModuleCount() + 4);
            Paint paint = new Paint();
            paint.setStyle(Paint.Style.FILL);
            
            // draw in the canvas
            for (int row = 0; row < getModuleCount() + 4; row++) {
                for (int col = 0; col < getModuleCount() + 4; col++) {
                    int color = 0xffffffff;
                    
                    if (row < 2 || row > (getModuleCount() + 1) || col < 2 || col > (getModuleCount() + 1)) {
                    
                    } else {
                        color = isDark(row - 2, col - 2) ? 0xff000000 : 0xffffffff;
                    }
                    paint.setColor(color);
                    float w = (float) ((Math.ceil((col + 1) * tileW) - Math.floor(col * tileW)));
                    float h = (float) ((Math.ceil((row + 1) * tileW) - Math.floor(row * tileW)));
                    float x = (float) Math.round(col * tileW);
                    float y = (float) Math.round(row * tileH);
                    canvas.drawRect(x, y, x + w, y + h, paint);
                }
            }
            return bitmap;
        }
    }
    
    public String getLocalCacheDir() {
        return mContext.getCacheDir() + "/";
    }
    
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int startChatRobot(String data_dir);
    
    public native int runChatRobot();
    
    public native String getAddress();
    
    public native String getUserId();
    
    public native MemberInfo[] getMemberList();
}
