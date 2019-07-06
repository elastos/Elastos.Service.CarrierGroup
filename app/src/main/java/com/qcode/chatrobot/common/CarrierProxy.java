package com.qcode.chatrobot.common;

import com.qcode.chatrobot.manager.MemberInfo;

public class CarrierProxy {
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int startChatRobot(String data_dir);
    
    public native int runChatRobot();
    
    public native String getAddress();
    
    public native String getUserId();
    
    public native int getMemberNum();
    
    public native String getNickName();
}
