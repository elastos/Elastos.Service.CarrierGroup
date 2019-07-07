//
// Created by luocf on 2019/6/12.
//

#ifndef CHATROBOT_DB_PROXY_H
#define CHATROBOT_DB_PROXY_H

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <sqlite3.h>
#include "MessageInfo.h"
#include "MemberInfo.h"

namespace chatrobot {
    typedef std::lock_guard<std::mutex> MUTEX_LOCKER;
    typedef bool (*MessageListCallback)(void *context,
                                        std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> message_list);

    class DatabaseProxy : std::enable_shared_from_this<DatabaseProxy> {
    public:

        DatabaseProxy();

        virtual ~DatabaseProxy();

        static constexpr const char *TAG = "DatabaseProxy";

        void updateMemberInfo(std::shared_ptr<MemberInfo> memberinfo);

        std::shared_ptr<MemberInfo> getMemberInfo(std::shared_ptr<std::string> friendid);
        std::shared_ptr<MemberInfo> getMemberInfo(int index);
        std::shared_ptr<MemberInfo> getBlockMemberInfo(std::shared_ptr<std::string> friendid);
        std::shared_ptr<MemberInfo> getBlockMemberInfo(int index);
        std::shared_ptr<std::string> getGroupNickName();
        void updateGroupNickName(std::shared_ptr<std::string> nick_name);
        void
        addMessgae(std::shared_ptr<std::string> friend_id, std::string message,
                   std::time_t send_time);

        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> getFriendList();

        bool removeMember(std::string friendid);
        bool deleteRemovedListMember(std::string friendid);
        bool inRemovedList(std::string friendid);
        bool startDb(const char *data_dir);
        bool closeDb();
        std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>>
        getMessages(std::shared_ptr<std::string> friend_id, std::time_t send_time, int max_limit);

        void addBlockMember(std::shared_ptr<MemberInfo> memberinfo);
        bool removeBlockMember(std::string friendid);
    private:
        std::mutex _SyncedGroupInfo;
        std::mutex _SyncedMemberList;
        std::mutex _SyncedBlockMemberList;
        std::mutex _SyncedMessageList;
        std::shared_ptr<std::string> mNickName;
        sqlite3 *mDb;
        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> mBlockMemberList;
        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> mMemberList;
        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> mRemovedMemberList;
        std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> mMessageList;
        void syncBlockList();
        void syncMemberList();
        void syncGroupInfo();
        static int callback(void *context, int argc, char **argv, char **azColName);

    };

}
#endif //CHATROBOT_DB_PROXY_H
