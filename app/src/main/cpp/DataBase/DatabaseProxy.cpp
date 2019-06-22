//
// Created by luocf on 2019/6/12.
//

#include "DatabaseProxy.h"
#include <map>
#include <Tools/Log.hpp>

namespace chatrobot {
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

        std::time_t msg_timestamp = 0;
        std::shared_ptr<MemberInfo> member_info = mMemberList[*friendid.get()];
        if (member_info.get() != nullptr) {
            msg_timestamp = member_info->mMsgTimeStamp;
        }

        mMemberList[*friendid.get()] = std::make_shared<MemberInfo>(
                friendid,
                nickname,
                status,
                msg_timestamp);
    }

    std::shared_ptr<MemberInfo>
    DatabaseProxy::getMemberInfo(std::shared_ptr<std::string> friendid) {
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