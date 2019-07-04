package com.qcode.chatrobot.ui;

import android.content.Context;
import android.os.Build;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.PopupWindow;

import com.qcode.chatrobot.R;
import com.qcode.chatrobot.manager.MemberInfo;

public class PopUpWindowSelf {
    PopupWindow mWindow;
    private  Context mContext;
    private Button mCloseBtn;
    private ListView mMemberList;
    private LayoutInflater mLayoutInflater;
    public PopUpWindowSelf(Context context, LayoutInflater inflater) {
        mContext = context;
        mLayoutInflater = inflater;
    }
    
    View.OnClickListener closeListener=new View.OnClickListener(){
        @Override
        public void onClick(View v){
            closeWindow();
        }
    };
    
    private void closeWindow(){
        //提交数据时关闭popupwindow。
        if (mWindow != null) {
            mWindow.dismiss();
        }
    }
    
    public void show(View parent, MemberInfo[]memberInfos) {
        if (mWindow == null) {
            LayoutInflater lay = (LayoutInflater) mContext.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View v = lay.inflate(R.layout.popupwindow, null);
            
            //初始化按钮
            mCloseBtn = (Button) v.findViewById(R.id.close);
            mCloseBtn.setOnClickListener(closeListener);
            
            //初始化listview，加载数据。
            mMemberList=(ListView)v.findViewById(R.id.memberList);
            MemberListAdapter adapter=new MemberListAdapter(mLayoutInflater);
            mMemberList.setAdapter(adapter);
            
            adapter.setData(memberInfos);
            mMemberList.setItemsCanFocus(false);
            mMemberList.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
            mMemberList.setOnItemClickListener(listClickListener);
    
            mWindow = new PopupWindow(v, 500,260);
        }
    
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            mWindow.setBackgroundDrawable(mContext.getDrawable(R.drawable.rounded_corners_pop));
        }
        mWindow.setFocusable(true);//如果不设置setFocusable为true，popupwindow里面是获取不到焦点的，那么如果popupwindow里面有输入框等的话就无法输入。
        mWindow.update();
        mWindow.showAtLocation(parent, Gravity.CENTER_VERTICAL, 0, 0);
    }
    
    AdapterView.OnItemClickListener listClickListener = new AdapterView.OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position,
                                long id) {
           
        }
    };
    
}
