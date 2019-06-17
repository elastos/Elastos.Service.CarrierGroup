//
// Created by luocf on 2019/6/12.
//

#include "DatabaseProxy.h"
#include <map>

namespace chatrobot {
    DatabaseProxy::DatabaseProxy() {
        mMemberList = std::make_shared<std::map<std::string, std::shared_ptr<MemberInfo>>>();
        mMessageList = std::make_shared<std::vector<std::shared_ptr<MessageInfo>>>();
    }

    DatabaseProxy::~DatabaseProxy() {
        mMemberList.reset();
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
            auto iter = mMemberList->find(*friendid.get());
            if (iter != mMemberList->end()) {
                auto member_info = iter->second;  //second返回iter的value
                if (member_info.get() != nullptr) {
                    last_onffine_time = member_info->mLastOffLineTimeStamp;
                }
            }

        } else {
            last_onffine_time = time_stamp;
            auto iter = mMemberList->find(*friendid.get());
            if (iter != mMemberList->end()) {
                auto member_info = iter->second;  //second返回iter的value
                if (member_info.get() != nullptr) {
                    last_online_time = member_info->mLastOnLineTimeStamp;
                }
            }
        }
        mMemberList->insert(std::pair<std::string, std::shared_ptr<MemberInfo>>(*friendid.get(),
                                                                                std::make_shared<MemberInfo>(
                                                                                        friendid,
                                                                                        nickname,
                                                                                        status,
                                                                                        last_online_time,
                                                                                        last_onffine_time)));
    }

    std::shared_ptr<MemberInfo>
    DatabaseProxy::getMemberInfo(std::shared_ptr<std::string> friendid) {

        auto iter = mMemberList->find(*friendid.get());
        if (iter != mMemberList->end()) {
            auto member_info = iter->second;  //second返回iter的value
            return member_info;
        }
        return std::shared_ptr<MemberInfo>(nullptr);
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

    std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> DatabaseProxy::getFriendList() {
        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> friendlist = std::make_shared<std::vector<std::shared_ptr<MemberInfo>>>();

        for (auto item = mMemberList->begin(); item != mMemberList->end(); item++) {
            auto value = item->second;
            friendlist->push_back(value);
        }

        return friendlist;
    }
}