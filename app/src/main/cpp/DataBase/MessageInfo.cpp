//
// Created by luocf on 2019/6/15.
//

#include "MessageInfo.h"
namespace chatrobot {
    MessageInfo::MessageInfo(std::shared_ptr<std::string> friendid, std::shared_ptr<std::string> message,
                           std::time_t send_time):
            mFriendid(friendid), mMsg(message),mSendTimeStamp(send_time){

    }
}