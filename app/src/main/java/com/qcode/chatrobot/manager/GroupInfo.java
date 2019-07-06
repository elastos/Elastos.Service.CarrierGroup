package com.qcode.chatrobot.manager;

import android.content.ComponentName;
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
    public Messenger mClientMessanger;
    public Messenger mMessanger;
    public int mId;
    public int mMemberCount;
    private GroupManager mGroupManager;
    
    public GroupInfo(GroupManager manager) {
        mGroupManager = manager;
        mClientMessanger = new Messenger(new ClientHandler());
    }
    
    @Override
    public int compareTo(@NonNull GroupInfo groupInfo) {
        return mId - groupInfo.mId;
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
                            mAddress = address;
                            mGroupManager.onGroupInfoUpdate();
                        }
                    }
                    break;
                }
                case CommonVar.Command_UpdateMemberCount: {
                    Bundle data = msg.getData();
                    if (data != null) {
                        synchronized (GroupInfo.this) {
                            mMemberCount = data.getInt("memberCount");
                            mGroupManager.onGroupInfoUpdate();
                        }
                    }
                    break;
                }
                case CommonVar.Command_UpdateNickName: {
                    Bundle data = msg.getData();
                    if (data != null) {
                        final String nickname = data.getString("nickName");
                        synchronized (GroupInfo.this) {
                            mNickName = nickname;
                            mGroupManager.onGroupInfoUpdate();
                        }
                    }
                    break;
                }
                case CommonVar.Command_DeleteGroup:{
                    synchronized (GroupInfo.this) {
                        mGroupManager.removeGroup(mId);
                        mGroupManager.onGroupInfoUpdate();
                    }
                    break;
                }
            }
            
        }
    }
}

