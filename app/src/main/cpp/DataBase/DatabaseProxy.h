//
// Created by luocf on 2019/6/12.
//

#ifndef CHATROBOT_DB_PROXY_H
#define CHATROBOT_DB_PROXY_H

#include <string>
#include <map>
#include <vector>
#include "MessageInfo.h"
#include "MemberInfo.h"

namespace chatrobot {
    class DatabaseProxy {
    public:
        DatabaseProxy();

        virtual ~DatabaseProxy();

        void updateMemberInfo(std::shared_ptr<std::string> friendid,
                              std::shared_ptr<std::string> nickname, ElaConnectionStatus status,
                              std::time_t time_stamp);

        std::shared_ptr<MemberInfo> getMemberInfo(std::shared_ptr<std::string> friendid);
        std::shared_ptr<MemberInfo> getMemberInfo(int index);
        void
        addMessgae(std::shared_ptr<std::string> friend_id, std::shared_ptr<std::string> message,
                   std::time_t send_time);

        std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> getMessages();

        std::map<std::string, std::shared_ptr<MemberInfo>> getFriendList();
        bool removeMember(std::string friendid);
    private:
        std::map<std::string, std::shared_ptr<MemberInfo>> mMemberList;
        std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> mMessageList;
    };

}
#endif //CHATROBOT_DB_PROXY_H
