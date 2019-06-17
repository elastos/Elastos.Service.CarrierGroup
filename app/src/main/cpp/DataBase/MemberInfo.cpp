//
// Created by luocf on 2019/6/15.
//

#include "MemberInfo.h"

namespace chatrobot {
    MemberInfo::MemberInfo(std::shared_ptr<std::string> friendid,
                           std::shared_ptr<std::string> nickname,
                           ElaConnectionStatus status,
                           std::time_t last_online_time,
                           std::time_t last_onffine_time) :
            mFriendid(friendid), mNickName(nickname), mStatus(status),
            mLastOnLineTimeStamp(last_online_time), mLastOffLineTimeStamp(last_onffine_time) {

    }
}