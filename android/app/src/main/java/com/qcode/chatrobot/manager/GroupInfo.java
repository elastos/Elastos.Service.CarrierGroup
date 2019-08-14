package com.qcode.chatrobot.manager;

import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.support.annotation.NonNull;
import android.util.Log;

import com.qcode.chatrobot.common.CommonVar;

import java.util.ArrayList;
import java.util.List;
public class GroupInfo implements Comparable<GroupInfo>{
    private static final String TAG = "GroupInfo";
    public String mAddress;
    public String mNickName;
    public String mUserId;
    public String mClassName;
    public String mDataDir;
    public Messenger mClientMessanger;
    public Messenger mMessanger;
    public int mGroupIndex;
    public int mId;
    public int mMemberCount;
    public ServiceConnection mServiceConnection;
    public Intent mServiceIntent;
    private GroupManager mGroupManager;
    
    public GroupInfo(GroupManager manager) {
        mGroupManager = manager;
        mClientMessanger = new Messenger(new ClientHandler());
    }
    
    @Override
    public int compareTo(@NonNull GroupInfo groupInfo) {
        return mGroupIndex - groupInfo.mGroupIndex;
    }
    
    private class ClientHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "ClientHandler -> handleMessage, msg:"+msg.toString());
            switch (msg.what) {
                case CommonVar.Command_UpdateAddress: {
                     Bundle data = msg.getData();
                    if (data != null) {
                        final String address = data.getString("address");
                        synchronized (GroupInfo.this) {
                            if (mAddress == null || !mAddress.equals(address)) {
                                mAddress = address;
                                mGroupManager.onGroupListUpdate();
                            }
                        }
                    }
                    break;
                }
                case CommonVar.Command_UpdateMemberCount: {
                    Bundle data = msg.getData();
                    if (data != null) {
                        synchronized (GroupInfo.this) {
                            int member_count = data.getInt("memberCount");
                            if (member_count != mMemberCount) {
                                mMemberCount = member_count;
                                mGroupManager.onGroupListUpdate();
                            }
                           
                        }
                    }
                    break;
                }
                case CommonVar.Command_UpdateNickName: {
                    Bundle data = msg.getData();
                    if (data != null) {
                        final String nickname = data.getString("nickName");
                        synchronized (GroupInfo.this) {
                            if (mNickName == null || !mNickName.equals(nickname)) {
                                mNickName = nickname;
                                mGroupManager.onGroupListUpdate();
                            }
                        }
                    }
                    break;
                }
                
                case CommonVar.Command_UpdateStatus:{
                    synchronized (GroupInfo.this) {
                        Bundle data = msg.getData();
                        if (data != null) {
                            final int status = data.getInt("status");
                            if (status == -1) {
                                mGroupManager.removeGroup(mId);
                                mGroupManager.onGroupListUpdate();
                            }
                        }
                    }
                    break;
                }
               /* case CommonVar.Command_WatchDog:{
                    synchronized (GroupInfo.this) {
                        //TODO
                    }
                    break;
                }*/
            }
        }
    }
}

