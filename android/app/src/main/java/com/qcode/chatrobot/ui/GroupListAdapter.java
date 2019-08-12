package com.qcode.chatrobot.ui;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.qcode.chatrobot.R;
import com.qcode.chatrobot.manager.GroupInfo;
import com.qcode.chatrobot.manager.MemberInfo;

import java.util.List;

public class GroupListAdapter extends BaseAdapter {
    private Context mContext;
    private TextView mNickView;
    private TextView mNumView;
    LayoutInflater mLayoutInflater;
    private List<GroupInfo> mGroupList;
    public GroupListAdapter(LayoutInflater inflater) {
        mLayoutInflater = inflater;
    }
    public void setData(List<GroupInfo>groupList) {
        mGroupList = groupList;
        //refresh
        this.notifyDataSetChanged();
        
    }
    
    @Override
    public int getCount() {
        return mGroupList == null ? 0: mGroupList.size();
    }
    
    @Override
    public Object getItem(int i) {
        return null;
    }
    
    @Override
    public long getItemId(int i) {
        return 0;
    }
    
    @Override
    public View getView(int position, View contentView, ViewGroup viewGroup) {
        LayoutInflater inflater = mLayoutInflater;
        View view;
    
        if (contentView==null) {
            //因为getView()返回的对象，adapter会自动赋给ListView
            view = inflater.inflate(R.layout.item, null);
        }else{
            view=contentView;
        }
    
        mNickView = (TextView) view.findViewById(R.id.txtNickName);//找到Textviewname
        String nickname = "";
        GroupInfo groupInfo = mGroupList.get(position);
        synchronized (groupInfo) {
            if (groupInfo.mNickName != null && !groupInfo.mNickName.isEmpty()) {
                nickname = groupInfo.mNickName;
            } else if (groupInfo.mAddress != null) {
                nickname = groupInfo.mAddress.substring(0, 10);
            }
            mNickView.setText(nickname);//设置参数
            mNumView = (TextView) view.findViewById(R.id.txtNum);//找到Textviewname
            mNumView.setText(" Members： "+mGroupList.get(position).mMemberCount);//设置参数
        }
        return view;
    }
   }
