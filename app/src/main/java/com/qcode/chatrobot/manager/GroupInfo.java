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
    public String mUserId;
    public String mClassName;
    public Messenger mClientMessanger;
    public Messenger mMessanger;
    public int mId;
    public MemberInfo[] mMemberList = new MemberInfo[0];
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
            Log.i(TAG, "ClientHandler -> handleMessage");
            switch (msg.what) {
                case CommonVar.Command_GetAddress: {
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
                case CommonVar.Command_GetMemberList: {
                    Bundle data = msg.getData();
                    if (data != null) {
                       MemberInfo[] memberlists = (MemberInfo[]) data.getParcelableArray("memberlist");
                        synchronized (GroupInfo.this) {
                            mMemberList = memberlists;
                            mGroupManager.onMemberListUpdate();
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

