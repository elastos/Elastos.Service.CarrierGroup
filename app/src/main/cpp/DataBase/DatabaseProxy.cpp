//
// Created by luocf on 2019/6/12.
//

#include "DatabaseProxy.h"
#include <map>
#include <iostream>
#include <Tools/Log.hpp>

namespace chatrobot {
    typedef std::lock_guard<std::mutex> MUTEX_LOCKER;
    DatabaseProxy::DatabaseProxy() {
        mMessageList = std::make_shared<std::vector<std::shared_ptr<MessageInfo>>>();
    }

    DatabaseProxy::~DatabaseProxy() {
        mMemberList.clear();
        mMessageList.reset();
    }

    void DatabaseProxy::updateMemberInfo(std::shared_ptr<std::string> friendid,
                                         std::shared_ptr<std::string> nickname,
                                         ElaConnectionStatus status,
                                         std::time_t time_stamp) {
        std::shared_ptr<MemberInfo> member_info = mMemberList[*friendid.get()];
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        if (member_info.get() != nullptr) {
            member_info->Lock();
            member_info->mNickName = nickname;
            member_info->mFriendid = friendid;
            member_info->mStatus = status;
            member_info->UnLock();
        } else {
            mMemberList[*friendid.get()] = std::make_shared<MemberInfo>(
                    friendid,
                    nickname,
                    status,
                    0);
        }
    }

    std::shared_ptr<MemberInfo>
    DatabaseProxy::getMemberInfo(std::shared_ptr<std::string> friendid) {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        return mMemberList[*friendid.get()];
    }

    std::shared_ptr<MemberInfo> DatabaseProxy::getMemberInfo(int index) {
        for (auto item = mMemberList.begin(); item != mMemberList.end(); item++) {
            auto value = item->second;
            if (value->mIndex == index) {
                return value;
            }
        }
        return std::shared_ptr<MemberInfo>(nullptr);
    }

    void DatabaseProxy::addMessgae(std::shared_ptr<std::string> friend_id,
                                   std::shared_ptr<std::string> message, std::time_t send_time) {

        mMessageList->push_back(std::make_shared<MessageInfo>(friend_id, message, send_time));
    }

    std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>>
    DatabaseProxy::getMessages() {
        return mMessageList;
    }

    std::map<std::string, std::shared_ptr<MemberInfo>> DatabaseProxy::getFriendList() {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        return mMemberList;
    }

    bool DatabaseProxy::removeMember(std::string friendid) {
        std::map<std::string, std::shared_ptr<MemberInfo>>::iterator key = mMemberList.find(
                friendid);
        if (key != mMemberList.end()) {
            mMemberList.erase(key);
            return true;
        } else {
            Log::D(Log::TAG, "removeMember not exist, friendid:%s", friendid.c_str());
            return false;
        }
    }
}