package com.qcode.chatrobot.manager;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Messenger;
import android.util.Log;
import android.widget.Toast;

import com.qcode.chatrobot.common.CommonVar;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class GroupManager {
    public static final String TAG = "GroupManager";
    private Context mContext;
    private List<GroupInfo> mGroupList = new ArrayList<GroupInfo>();
    private int mServiceTotal = CommonVar.CONST_SERVICE_LIMIT_NUM;
    private int mServiceNum = 0;
    private int mCurrentId = 0;
    private SharedPreferences mPerf;
    private Handler mMainThreadHandler = new Handler(Looper.getMainLooper());
    
    public interface GroupListener {
        void onGroupInfoUpdate();
    }
    
    private GroupListener mGroupListener;
    public GroupManager(Context context) {
        mContext = context;
    }
    public void recoveryGroup() {
        synchronized (mGroupList) {
            SharedPreferences sharedPreferences = getPerf();
            String grouplist = sharedPreferences.getString("grouplist", null);
            if (grouplist != null) {
                try {
                    JSONArray grouplist_json = new JSONArray(grouplist);
                    for (int i = 0; i < grouplist_json.length(); i++) {
                        JSONObject groupinfo = grouplist_json.getJSONObject(i);
                        final int id = groupinfo.getInt("id");
                        if (i == 0) {
                            mCurrentId = id;
                        }
                        bindService(id);
                       
                    }
                    mServiceNum = grouplist_json.length();
                    
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        }
    }
    
    public void storeGroupInfo() {
        synchronized (mGroupList) {
            SharedPreferences sharedPreferences = getPerf();
            JSONArray group_list = new JSONArray();
            if (mGroupList != null) {
                for (int i = 0; i < mGroupList.size(); i++) {
                    JSONObject group_info = new JSONObject();
                    try {
                        group_info.put("membercount", mGroupList.get(i).mMemberCount);
                        group_info.put("nickname", mGroupList.get(i).mNickName);
                        group_info.put("address", mGroupList.get(i).mAddress);
                        group_info.put("id", mGroupList.get(i).mId);
                        group_list.put(group_info);
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }
            }
            
            SharedPreferences.Editor editor = sharedPreferences.edit();
            editor.putString("grouplist", group_list.toString());
            editor.commit();
        }
    }
    
    public void createGroup() {
        int service_id = mServiceNum++;
        if (mServiceNum > mServiceTotal) {
            mServiceNum = mServiceTotal;
            Toast.makeText(mContext, "超出群组上限：" + mServiceTotal, Toast.LENGTH_SHORT).show();
            return;
        }
        
        //找出有效的service id
        synchronized (mGroupList) {
            if (mGroupList != null && mGroupList.size() > 0) {
                for (int i = 0; i < mServiceNum; i++) {
                    boolean found = false;
                    for (int j = 0; j < mGroupList.size(); j++) {
                        GroupInfo group_info = mGroupList.get(j);
                        if (group_info.mId == i) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        service_id = i;
                        break;
                    }
                }
            }
            bindService(service_id);
        }
    }
    public void stopService(int service_num) {
        Intent service_intent = new Intent(CommonVar.CONST_CARRIER_SERVICE_ACTION + service_num);
        service_intent.setPackage(CommonVar.CONST_CARRIER_SERVICE_PACKAGE_NMAE);
        boolean ret = mContext.stopService(service_intent);
        
    }
    public void bindService(int service_num) {
        //绑定Service
        Intent service_intent = new Intent(CommonVar.CONST_CARRIER_SERVICE_ACTION + service_num);
        service_intent.setPackage(CommonVar.CONST_CARRIER_SERVICE_PACKAGE_NMAE);
        boolean ret = mContext.bindService(service_intent, mServiceConnection, mContext.BIND_AUTO_CREATE);
    }
    
    public void registerGroupListener(GroupListener listener) {
        mGroupListener = listener;
    }
    
    public void removeGroup(int id) {
        synchronized (mGroupList) {
            for (int i = 0; i < mGroupList.size(); i++) {
                GroupInfo group_info = mGroupList.get(i);
                if (group_info.mId == id) {
                    mGroupList.remove(i);
                    //停止服务
                    stopService(id);
                    break;
                }
            }
           
        }
    }
    
    public void onGroupInfoUpdate() {
        if (mGroupListener != null) {
            mGroupListener.onGroupInfoUpdate();
        }
        
        //保存goup list信息
        storeGroupInfo();
    }
    
    private SharedPreferences getPerf() {
        if (mPerf == null) {
            mPerf = mContext.getSharedPreferences("GroupChatInfo", Context.MODE_PRIVATE);
        }
        return mPerf;
    }
    
    public void switchGroup(int id) {
        mCurrentId = id;
        if (mGroupListener != null) {
            mGroupListener.onGroupInfoUpdate();
        }
    }
    
    public int getCurrentId() {
        return mCurrentId;
    }
    
    public GroupInfo getGroupInfo(int id) {
        synchronized (mGroupList) {
            for (int i = 0; i < mGroupList.size(); i++) {
                GroupInfo group_info = mGroupList.get(i);
                if (group_info.mId == id) {
                    return group_info;
                }
            }
            
            return null;
        }
    }
    
    public String getAddress(int id) {
        synchronized (mGroupList) {
            for (int i = 0; i < mGroupList.size(); i++) {
                GroupInfo group_info = mGroupList.get(i);
                if (group_info.mId == id) {
                    return group_info.mAddress;
                }
            }
            
            return null;
        }
    }
    
    public List<GroupInfo> getGroupList() {
        synchronized (mGroupList) {
            return mGroupList;
        }
    }
    
    public void destroy() {
        if (mServiceConnection != null) {
            mContext.unbindService(mServiceConnection);
        }
    }
    
    ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(final ComponentName name, IBinder service) {
            Log.d(TAG, "CarrierService onServiceConnected name:" + name.getClassName());
            final Messenger messenger = new Messenger(service);
            final Message msg = Message.obtain(null, 1, 0, 0);
            synchronized (mGroupList) {
                String class_name = name.getClassName();
                int id = Integer.parseInt(class_name.substring(CommonVar.CONST_CARRIER_SERVICE_BASENAME.length()));
                if (getGroupInfo(id) != null) {
                    return;
                }
                final GroupInfo groupInfo = new GroupInfo(GroupManager.this);
                groupInfo.mClassName = class_name;
                groupInfo.mMessanger = messenger;
                groupInfo.mId = id;
                
                msg.replyTo = groupInfo.mClientMessanger;
                Bundle bundle = new Bundle();
                bundle.putString("data_path", getLocalCacheDir(class_name));
                msg.setData(bundle);
                try {
                    //发送消息
                    messenger.send(msg);
                } catch (Exception e) {
                    Log.i(TAG, "客户端向service发送消息失败: " + e.getMessage());
                }
                
                mGroupList.add(groupInfo);
                //排序
                Collections.sort(mGroupList);
                if (mGroupListener != null) {
                    mGroupListener.onGroupInfoUpdate();
                }
            }
        }
        
        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.v(TAG, "CarrierService onServiceDisconnected");
            String class_name = name.getClassName();
            final int index = Integer.parseInt(class_name.substring(CommonVar.CONST_CARRIER_SERVICE_BASENAME.length()));
            /*synchronized (mGroupList) {
                if (mGroupList.size() > index) {
                    mGroupList.remove(index);
                }
            }*/
        }
    };
    
    public String getLocalCacheDir(String class_name) {
        return mContext.getCacheDir() + "/" + class_name + "/";
    }
}
