//
// Created by luocf on 2019/6/12.
//

#include "DatabaseProxy.h"
#include <map>

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

        std::time_t last_online_time = 0;
        std::time_t last_onffine_time = 0;
        if (status == ElaConnectionStatus_Connected) {
            last_online_time = time_stamp;
            std::shared_ptr<MemberInfo> member_info  = mMemberList[*friendid.get()];
            if (member_info.get() != nullptr) {
                last_onffine_time = member_info->mLastOffLineTimeStamp;
            }

        } else {
            last_onffine_time = time_stamp;
            std::shared_ptr<MemberInfo> member_info  = mMemberList[*friendid.get()];
            if (member_info.get() != nullptr) {
                last_online_time = member_info->mLastOnLineTimeStamp;
            }
        }
        mMemberList[*friendid.get()] = std::make_shared<MemberInfo>(
                friendid,
                nickname,
                status,
                last_online_time,
                last_onffine_time);
    }

    std::shared_ptr<MemberInfo>
    DatabaseProxy::getMemberInfo(std::shared_ptr<std::string> friendid) {
        return  mMemberList[*friendid.get()];
    }

    void DatabaseProxy::addMessgae(std::shared_ptr<std::string> friend_id,
                                   std::shared_ptr<std::string> message, std::time_t send_time) {
        mMessageList->push_back(std::make_shared<MessageInfo>(friend_id, message, send_time));
    }

    std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>>
    DatabaseProxy::getMessages(std::time_t offline_time_stamp) {
        std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> messages = std::make_shared<std::vector<std::shared_ptr<MessageInfo>>>();
        for (int i = 0; i < mMessageList->size(); i++) {
            std::shared_ptr<MessageInfo> message = mMessageList->at(i);
            if (message.get() != nullptr &&
                message->mSendTimeStamp > offline_time_stamp) {//>offline time 的所有消息
                messages->push_back(message);
            }
        }
        return messages;
    }

    std::map<std::string, std::shared_ptr<MemberInfo>> DatabaseProxy::getFriendList() {
        return mMemberList;
    }
}