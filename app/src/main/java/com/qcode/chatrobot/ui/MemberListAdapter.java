package com.qcode.chatrobot.ui;

import android.app.Activity;
import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.qcode.chatrobot.R;

import java.util.ArrayList;
import java.util.List;

public class MemberListAdapter extends BaseAdapter {
    private Context mContext;
    private TextView mNickNameView;
    private TextView mStatusView;
    LayoutInflater mLayoutInflater;
    private List<MemberInfo> mUserList = new ArrayList<MemberInfo>();
    public MemberListAdapter(LayoutInflater inflater) {
        mLayoutInflater = inflater;
    }
    public void setData(List<MemberInfo> userList) {
        mUserList = userList;
        //refresh
        this.notifyDataSetChanged();
    }
    @Override
    public int getCount() {
        return mUserList.size();
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
            Log.i("info","有缓存，不需要重新生成"+position);
        }
        mNickNameView = (TextView) view.findViewById(R.id.memberName);//找到Textviewname
        mNickNameView.setText(mUserList.get(position).getNickName());//设置参数
    
        mStatusView = (TextView) view.findViewById(R.id.memberStatus);//找到Textviewage
        mStatusView.setText(mUserList.get(position).getStatus());//设置参数
        return view;
    }
   }
