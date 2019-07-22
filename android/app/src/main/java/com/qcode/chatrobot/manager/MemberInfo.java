package com.qcode.chatrobot.manager;

import android.os.Parcel;
import android.os.Parcelable;

public class MemberInfo implements Parcelable {
    public String NickName;
    public String FriendId;
    public String Status;
    public MemberInfo() {
    
    }
    
    protected MemberInfo(Parcel in) {
        NickName = in.readString();
        FriendId = in.readString();
        Status = in.readString();
    }
    
    public static final Creator<MemberInfo> CREATOR = new Creator<MemberInfo>() {
        @Override
        public MemberInfo createFromParcel(Parcel in) {
            return new MemberInfo(in);
        }
        
        @Override
        public MemberInfo[] newArray(int size) {
            return new MemberInfo[size];
        }
    };
    
    @Override
    public int describeContents() {
        return 0;
    }
    
    @Override
    public void writeToParcel(Parcel parcel, int i) {
    
        parcel.writeString(NickName);
        parcel.writeString(FriendId);
        parcel.writeString(Status);
    }
}
