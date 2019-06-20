//
// Created by luocf on 2019/6/15.
//

#include "MemberInfo.h"

namespace chatrobot {
    MemberInfo::MemberInfo(std::shared_ptr<std::string> friendid,
                           std::shared_ptr<std::string> nickname,
                           ElaConnectionStatus status,
                           std::time_t msg_timestamp):
            mFriendid(friendid), mNickName(nickname), mStatus(status),
            mMsgTimeStamp(msg_timestamp) {

    }
}