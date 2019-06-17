//
// Created by luocf on 2019/6/15.
//

#ifndef CHATROBOT_GITHUB_MEMBERINFO_H
#define CHATROBOT_GITHUB_MEMBERINFO_H


#include <memory>
#include <ela_carrier.h>
#include <ctime>

namespace chatrobot {
    class MemberInfo {
    public:
        MemberInfo(std::shared_ptr<std::string>, std::shared_ptr<std::string>, ElaConnectionStatus, std::time_t, std::time_t);
        std::shared_ptr<std::string> mFriendid;
        std::shared_ptr<std::string> mNickName;
        ElaConnectionStatus mStatus;
        std::time_t mLastOnLineTimeStamp;
        std::time_t mLastOffLineTimeStamp;
    };
}


#endif //CHATROBOT_GITHUB_MEMBERINFO_H
