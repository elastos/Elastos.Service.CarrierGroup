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
import java.util.Random;

public class GroupManager {
    public static final String TAG = "GroupManager";
    private Context mContext;
    private List<GroupInfo> mGroupList = new ArrayList<GroupInfo>();
    
    private int mServiceTotal = CommonVar.CONST_SERVICE_LIMIT_NUM;
    private int mCurrentId = 0;
    private SharedPreferences mPerf;
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
                        int service_id = groupinfo.getInt("id");
                        
                        GroupInfo groupInfo = new GroupInfo(GroupManager.this);
                        String class_name = groupinfo.optString("class_name", "");
                        String nickname = groupinfo.optString("nickname", "");
                        String address = groupinfo.optString("address", "");
                        int membercount = groupinfo.optInt("membercount", 0);
                        groupInfo.mDataDir =  groupinfo.optString("data_dir", "");
                        groupInfo.mClassName = class_name;
                        groupInfo.mId = service_id;
                        groupInfo.mNickName = nickname;
                        groupInfo.mAddress = address;
                        groupInfo.mMemberCount = membercount;
                        ServiceConnection service_connection = getServiceConnection();
                        groupInfo.mServiceConnection = service_connection;
                        mGroupList.add(groupInfo);
    
                        Intent service_intent = bindService(service_id, service_connection);
                        groupInfo.mServiceIntent = service_intent;
                    }
                    
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
            if (mGroupList.size() > 1) {
                //排序
                Collections.sort(mGroupList);
                mCurrentId = mGroupList.get(0).mId;
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
                        group_info.put("data_dir", mGroupList.get(i).mDataDir);
                        group_info.put("nickname", mGroupList.get(i).mNickName);
                        group_info.put("address", mGroupList.get(i).mAddress);
                        group_info.put("id", mGroupList.get(i).mId);
                        group_info.put("class_name", mGroupList.get(i).mClassName);
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
        //找出有效的service id
        synchronized (mGroupList) {
            int current_total = mGroupList.size();
            if (current_total + 1 > mServiceTotal) {
                Toast.makeText(mContext, "超出群组上限：" + mServiceTotal, Toast.LENGTH_SHORT).show();
                return ;
            }
            
            int service_id = 0;
            boolean found = false;
            if (current_total > 0) {
                for (int i = 0; i < mServiceTotal; i++) {
                    found = false;
                    for (int j = 0; j < current_total; j++) {
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
            
            ServiceConnection service_connection = getServiceConnection();
            GroupInfo group_info = new GroupInfo(this);
            group_info.mId = service_id;
            group_info.mServiceConnection = service_connection;
            
            Intent service_intent = bindService(service_id, service_connection);
            group_info.mServiceIntent = service_intent;
            mGroupList.add(group_info);
        }
    }
    
    public Intent bindService(int service_num, ServiceConnection service_connection) {
        //绑定Service
        Intent service_intent = new Intent(CommonVar.CONST_CARRIER_SERVICE_ACTION + service_num);
        service_intent.setPackage(CommonVar.CONST_CARRIER_SERVICE_PACKAGE_NMAE);
        
        boolean ret = mContext.bindService(service_intent, service_connection, mContext.BIND_AUTO_CREATE);
        Log.d(TAG, "bindService service_num:"+service_num+",ret:"+ret);
        
        return service_intent;
    }
    
    public void registerGroupListener(GroupListener listener) {
        mGroupListener = listener;
    }
    
    private void stopService(int service_num) {
        Intent service_intent = new Intent(CommonVar.CONST_CARRIER_SERVICE_ACTION + service_num);
        service_intent.setPackage(CommonVar.CONST_CARRIER_SERVICE_PACKAGE_NMAE);
        boolean ret = mContext.stopService(service_intent);
        Log.d(TAG, "stopService service_num:"+service_num+",ret:"+ret);
    }
    public void removeGroup(int id) {
        synchronized (mGroupList) {
            for (int i = 0; i < mGroupList.size(); i++) {
                GroupInfo group_info = mGroupList.get(i);
                if (group_info.mId == id) {
                    //停止服务
                    if (group_info.mServiceConnection !=  null) {
                        mContext.unbindService(group_info.mServiceConnection);
                        group_info.mServiceConnection = null;
                    }
                    if (group_info.mServiceIntent != null) {
                        mContext.stopService(group_info.mServiceIntent);
                        group_info.mServiceIntent = null;
                    }
                    
                    mGroupList.remove(i);
                    if (mCurrentId == id && mGroupList.size() > 0) {
                        mCurrentId = mGroupList.get(0).mId;
                    } else {
                        mCurrentId = -1;
                    }
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
        unbindAllService();
    }
    
    private void unbindAllService() {
        synchronized (mGroupList) {
            for (int i = 0; i < mGroupList.size(); i++) {
                GroupInfo group_info = mGroupList.get(i);
                if (group_info.mServiceConnection != null) {
                    mContext.unbindService(group_info.mServiceConnection);
                }
            }
        }
    }
    private ServiceConnection getServiceConnection() {
        ServiceConnection service_connection = new ServiceConnection() {
            @Override
            public void onServiceConnected(final ComponentName name, IBinder service) {
                Log.d(TAG, "CarrierService onServiceConnected name:" + name.getClassName());
                final Messenger messenger = new Messenger(service);
                final Message msg = Message.obtain(null, 1, 0, 0);
                synchronized (mGroupList) {
                    String class_name = name.getClassName();
                    int id = Integer.parseInt(class_name.substring(CommonVar.CONST_CARRIER_SERVICE_BASENAME.length()));
                    
                    GroupInfo groupInfo = getGroupInfo(id);
                    if (groupInfo == null) {
                        Log.d(TAG, "CarrierService onServiceConnected error groupInfo == null:" + name.getClassName());
                        return ;
                    }
                    groupInfo.mMessanger = messenger;
                    groupInfo.mClassName = class_name;
                    msg.replyTo = groupInfo.mClientMessanger;
                    if (groupInfo.mDataDir == null || groupInfo.mDataDir.isEmpty()) {
                        groupInfo.mDataDir = getLocalCacheDir(class_name);
                    }
                    Bundle bundle = new Bundle();
                    bundle.putString("data_path", groupInfo.mDataDir);
                    msg.setData(bundle);
                    try {
                        //发送消息
                        messenger.send(msg);
                    } catch (Exception e) {
                        Log.i(TAG, "客户端向service发送消息失败: " + e.getMessage());
                    }
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
        
        return service_connection;
    }
    
    public static String getRandomString(int length){
        String str="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        Random random=new Random();
        StringBuffer sb=new StringBuffer();
        for(int i=0;i<length;i++){
            int number=random.nextInt(62);
            sb.append(str.charAt(number));
        }
        return sb.toString();
    }
    
    public String getLocalCacheDir(String class_name) {
        return mContext.getCacheDir() + "/" + class_name + "/"+getRandomString(5)+"/";
    }
}
