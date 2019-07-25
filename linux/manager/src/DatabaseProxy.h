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
#include <stdlib.h>
#include <iostream>
#include <memory> // std::unique_ptr
#include <ctime>
#include <regex>
#include "GroupInfo.h"

typedef std::lock_guard<std::mutex> MUTEX_LOCKER;
class GroupInfo;

class DatabaseProxy : std::enable_shared_from_this<DatabaseProxy> {
public:

    DatabaseProxy();

    virtual ~DatabaseProxy();

    static constexpr const char *TAG = "DatabaseProxy";

    void updateGroupMemberCount(std::string friendid,
                                int member_count);

    void updateGroupNickName(std::string friendid, std::string nickname);
    void updateGroupAddress(int service, std::string friend_id);

    void addGroup(std::string friendid,
                  std::string nickname,
                  std::string data_dir,
                  int member_count,
                  int service_id);

    std::shared_ptr<GroupInfo> getGroupInfo(std::string friendid);

    std::shared_ptr<GroupInfo> getGroupInfo(int service_id);

    std::shared_ptr<std::vector<std::shared_ptr<GroupInfo>>> getGroupList();

    bool removeGroup(std::string friendid);

    bool startDb(const char *data_dir);

    bool closeDb();

private:
    std::mutex _SyncedGroupList;
    sqlite3 *mDb;
    std::shared_ptr<std::vector<std::shared_ptr<GroupInfo>>> mGroupList;

    void syncGroupList();

    static int callback(void *context, int argc, char **argv, char **azColName);

};

#endif //CHATROBOT_DB_PROXY_H
