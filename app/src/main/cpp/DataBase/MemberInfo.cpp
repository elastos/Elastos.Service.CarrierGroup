//
// Created by luocf on 2019/6/15.
//
#include <string>
#include <sstream>
#include <iostream>
#include <Tools/Log.hpp>
#include "MemberInfo.h"

namespace chatrobot {
    int MemberInfo::sMemberCount = 0;

    MemberInfo::MemberInfo(std::shared_ptr<std::string> friendid,
                           std::shared_ptr<std::string> nickname,
                           ElaConnectionStatus status,
                           std::time_t msg_timestamp) :
            mFriendid(friendid), mNickName(nickname),
            mStatus(status), mMsgTimeStamp(msg_timestamp) {
        mIndex = ++sMemberCount;
        if (nickname->empty() == true) {
            if (mIndex == 1) {
                mNickName = std::make_shared<std::string>("群主");
            } else {
                mNickName = std::make_shared<std::string>(
                        std::string("匿名") + std::to_string(mIndex));
            }
        }
    }

    void MemberInfo::Lock() {
        Log::D(Log::TAG, "MemberInfo lock %s", mFriendid.get()->c_str());
        mMutex.lock();
    }

    void MemberInfo::UnLock() {
        Log::D(Log::TAG, "MemberInfo unLock %s", mFriendid.get()->c_str());
        mMutex.unlock();
    }
}