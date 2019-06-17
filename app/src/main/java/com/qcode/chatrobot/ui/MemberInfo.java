package com.qcode.chatrobot.ui;

public class MemberInfo {
    private String mNickName;
    private String mStatus;
    public MemberInfo(String nick_name, String status) {
        mNickName = nick_name;
        mStatus = status;
    }
    
    public String getStatus() {
        return mStatus;
    }
    
    public void setStatus(String mStatus) {
        this.mStatus = mStatus;
    }
    
    public String getNickName() {
        return mNickName;
    }
    
    public void setNickName(String mNickName) {
        this.mNickName = mNickName;
    }
}
