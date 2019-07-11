package com.qcode.chatrobot.service;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;

import com.qcode.chatrobot.common.CarrierProxy;
import com.qcode.chatrobot.common.CommonVar;
import com.qcode.chatrobot.common.FileUtils;
import com.qcode.chatrobot.manager.MemberInfo;

import java.util.Timer;
import java.util.TimerTask;

public class CarrierServiceBase extends Service {
    public static final String TAG = "CarrierServiceBase";
    private HandlerThread mWorkhandlerThread = null;
    private Handler mWorkHandler = null;
    private Context mContext = null;
    private Timer mTimer = null;
    private TimerTask mTimerTask = null;
    private CarrierProxy mCarrierProxy;
    private int mMemberNum = -1;
    private String mGroupNickName = null;
    private int mGroupStatus = -1;
    
    private Handler mMainThreadHandler = new Handler(Looper.getMainLooper());
    
    private int mServiceId;
    
    static {
        System.loadLibrary("chatrobot");
    }
    
    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate()'ed");
        super.onCreate();
        // Used to load the 'native-lib' library on application startup.
        mCarrierProxy = new CarrierProxy();
        this.Init();
    }
    
    
    private void Init() {
        Log.d(TAG, "Init()'ed");
        mContext = getApplicationContext();
        
        //启动事务线程
        mWorkhandlerThread = new HandlerThread("CarrierServiceThread");
        mWorkhandlerThread.start();
        Looper looper = mWorkhandlerThread.getLooper();
        mWorkHandler = new Handler(looper);
        
    }
    
    private void startWatchDog() {
        if (mTimer == null) {
            mTimer = new Timer();
        }
        if (mTimerTask == null) {
            mTimerTask = new TimerTask() {
                @Override
                public void run() {
                    Log.d("CarrierService", "WatchDog");
                }
            };
        } else {
            mTimer.cancel();
        }
        
        if (mTimer != null && mTimerTask != null) {
            mTimer.schedule(mTimerTask, 0, 2000);
        }
    }
    
    private void stopWatchDog() {
        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
        
        if (mTimerTask != null) {
            mTimerTask.cancel();
            mTimerTask = null;
        }
    }
    
    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Bundle bundle = msg.getData();
            Log.d(TAG, "IncomingHandler msg.what:" + msg.what);
            
            switch (msg.what) {
                case CommonVar.Connected: {
                    String data_path = bundle.getString("data_path");
                    FileUtils.mkdir(data_path, 777);
                    clientMessenger = msg.replyTo;
                    Log.d(TAG, "IncomingHandler msg:" + msg);
                    //启动service
                    mCarrierProxy.startChatRobot(data_path);
                    //启动聊天机器人
                    mCarrierProxy.runChatRobot();
                    sendCarrierAddress();
                    updateGroupInfo();
                    break;
                }
                case CommonVar.Command_UpdateAddress: {
                    sendCarrierAddress();
                    break;
                }
                case CommonVar.Command_UpdateMemberCount: {
                    sendCarrierMemberCount();
                    break;
                }
                case CommonVar.Command_UpdateNickName: {
                    sendCarrierNickName();
                    break;
                }
            }
        }
    }
    
    private void sendCarrierAddress() {
        Log.d(TAG, "sendCarrierAddress");
        if (clientMessenger != null) {
            try {
                String address = mCarrierProxy.getAddress();
                Message reply_msg = new Message();
                Bundle reply_bundle = new Bundle();
                reply_msg.what = CommonVar.Command_UpdateAddress;
                reply_bundle.putString("address", address);
                reply_msg.setData(reply_bundle);
                clientMessenger.send(reply_msg);
                Log.d(TAG, "sendCarrierAddress address:" + address);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }
    
    /**
     * 创建Messenger并传入Handler实例对象
     */
    final Messenger mMessenger = new Messenger(new IncomingHandler());
    private Messenger clientMessenger = null;
    
    /**
     * 当绑定Service时,该方法被调用,将通过mMessenger返回一个实现
     * IBinder接口的实例对象
     */
    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind()'ed");
        return mMessenger.getBinder();
    }
    
    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy()'ed");
        mWorkhandlerThread.quit();
        stopWatchDog();
        System.exit(0);
        super.onDestroy();
    }
    
    private void sendCarrierMemberCount() {
        Log.d(TAG, "sendCarrierMemberCount");
        if (clientMessenger != null) {
            synchronized (mCarrierProxy) {
                if (mMemberNum != -1) {
                    try {
                        Message reply_msg = new Message();
                        Bundle reply_bundle = new Bundle();
                        reply_msg.what = CommonVar.Command_UpdateMemberCount;
                        reply_bundle.putInt("memberCount", mMemberNum);
                        reply_msg.setData(reply_bundle);
                        clientMessenger.send(reply_msg);
                        Log.d(TAG, "sendCarrierMemberCount memberCount:" + mMemberNum);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                
            }
        }
    }
    
    private void sendCarrierNickName() {
        Log.d(TAG, "sendCarrierNickName");
        if (clientMessenger != null) {
            synchronized (mCarrierProxy) {
                if (mGroupNickName != null) {
                    try {
                        
                        Message reply_msg = new Message();
                        Bundle reply_bundle = new Bundle();
                        reply_msg.what = CommonVar.Command_UpdateNickName;
                        reply_bundle.putString("nickName", mGroupNickName);
                        reply_msg.setData(reply_bundle);
                        clientMessenger.send(reply_msg);
                        Log.i(TAG, "sendCarrierNickName nickName:" + mGroupNickName);
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }
    private void sendCarrierGroupStatus() {
        Log.d(TAG, "sendCarrierGroupStatus");
        if (clientMessenger != null) {
            synchronized (mCarrierProxy) {
                if (mGroupStatus != -1) {
                    try {
                        Message reply_msg = new Message();
                        Bundle reply_bundle = new Bundle();
                        reply_msg.what = CommonVar.Command_UpdateStatus;
                        reply_bundle.putInt("status", mGroupStatus);
                        reply_msg.setData(reply_bundle);
                        clientMessenger.send(reply_msg);
                        Log.d(TAG, "sendCarrierGroupStatus mGroupStatus:" + mGroupStatus);
                        
                    } catch (RemoteException e) {
                        e.printStackTrace();
                    }
                }
                
            }
        }
    }
    public void updateGroupInfo() {
        synchronized (mCarrierProxy) {
            sendCarrierMemberCount();
            sendCarrierNickName();
            sendCarrierGroupStatus();
            mMainThreadHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    mMemberNum = mCarrierProxy.getMemberNum();
                    mGroupNickName = mCarrierProxy.getNickName();
                    mGroupStatus = mCarrierProxy.getGroupStatus();
                    updateGroupInfo();
                }
            }, 3000);
        }
    }
    
}