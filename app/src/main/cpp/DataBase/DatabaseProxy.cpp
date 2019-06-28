//
// Created by luocf on 2019/6/12.
//

#include "DatabaseProxy.h"
#include <map>
#include <iostream>
#include <Tools/Log.hpp>

namespace chatrobot {
    DatabaseProxy::DatabaseProxy() {
        mMessageList = std::make_shared<std::vector<std::shared_ptr<MessageInfo>>>();
        //anypeer Id format
        std::string reg_str("(\\w{8})-(\\w{4})-(\\w{4})-(\\w{4})-(\\w{13})");
        mMsgReg = std::make_shared<std::regex>(reg_str,std::regex::icase)
    }

    DatabaseProxy::~DatabaseProxy() {
        mMemberList.clear();
        mMessageList.reset();
    }

    void DatabaseProxy::updateMemberInfo(std::shared_ptr<MemberInfo> target_memberinfo) {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        std::shared_ptr<MemberInfo> member_info = mMemberList[*target_memberinfo->mFriendid.get()];
        std::shared_ptr<std::string> tmp_nickname = target_memberinfo->mNickName;
        std::string t_strSql = "";
        char *errMsg = NULL;
        if (member_info.get() != nullptr) {
            member_info->Lock();
            if (!tmp_nickname->empty()) {
                member_info->mNickName = tmp_nickname;
            } else {
                tmp_nickname = member_info->mNickName;
            }
            member_info->mStatus = target_memberinfo->mStatus;
            if(target_memberinfo->mMsgTimeStamp > member_info->mMsgTimeStamp) {
                member_info->mMsgTimeStamp = target_memberinfo->mMsgTimeStamp;
            }

            t_strSql = "update member_table set NickName='"+*tmp_nickname.get()+"'";
            t_strSql += ",Status="+std::to_string(member_info->mStatus);
            t_strSql += ",MsgTimeStamp="+std::to_string(member_info->mMsgTimeStamp);
            t_strSql += " where Friendid='"+*member_info->mFriendid.get()+"';";
            member_info->UnLock();

        } else {
            member_info = target_memberinfo;
            mMemberList[*member_info->mFriendid.get()] = member_info;
            t_strSql = "insert into member_table values(NULL,'"+*member_info->mFriendid.get()+"'";
            t_strSql += ",'"+*member_info->mNickName.get()+"'";
            t_strSql += ","+std::to_string(member_info->mStatus);
            t_strSql += ",0);";
        }
        //消息直接入库
        int rv = sqlite3_exec(mDb, t_strSql.c_str(), callback, this, &errMsg);
        if (rv != SQLITE_OK) {
            Log::D(DatabaseProxy::TAG, "SQLite update updateMemberInfo error: %s\n",
                   errMsg);
        }
    }

    std::shared_ptr<MemberInfo>
    DatabaseProxy::getMemberInfo(std::shared_ptr<std::string> friendid) {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        return mMemberList[*friendid.get()];
    }

    std::shared_ptr<MemberInfo> DatabaseProxy::getMemberInfo(int index) {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
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
        MUTEX_LOCKER locker_sync_data(_SyncedMessageList);
        char *errMsg = NULL;
        std::string msg = *message.get();

        if(msg.size() > 37 && std::regex_match(msg.substr(0, 37).c_str(),*mMsgReg.get()) == true)  {
            msg = msg.substr(37);
        }

        std::string t_strSql;
        t_strSql = "insert into message_table values(NULL,'"+*friend_id.get()+"'";
        t_strSql += ",'"+msg+"'";
        t_strSql += ","+std::to_string(send_time)+"";
        t_strSql += ");";

                //消息直接入库
        int rv = sqlite3_exec(mDb, t_strSql.c_str(), callback, this, &errMsg);
        if (rv != SQLITE_OK) {
            Log::D(DatabaseProxy::TAG, "SQLite addMessgae error: %s\n",
                   errMsg);
        }
    }
    std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>>
    DatabaseProxy::getMessages(std::shared_ptr<std::string> friend_id, std::time_t send_time, int max_limit) {
        MUTEX_LOCKER locker_sync_data(_SyncedMessageList);
        //查询一条记录
        char **azResult;   //二维数组存放结果
        char *errMsg = NULL;
        int nrow;          /* Number of result rows written here */
        int ncolumn;
        std::string t_strSql;
        t_strSql = "select * from message_table where FriendId!='"+*friend_id.get()+"'";
        t_strSql += " and SendTimeStamp>"+std::to_string(send_time);
        t_strSql += " order by SendTimeStamp asc limit 0,"+std::to_string(max_limit)+";";
        /*step 2: sql语句对象。*/
        sqlite3_stmt *pStmt;
        int rc = sqlite3_prepare_v2(
                mDb, //数据库连接对象
                t_strSql.c_str(), //指向原始sql语句字符串
                strlen(t_strSql.c_str()), //
                &pStmt,
                NULL
        );
        if (rc != SQLITE_OK) {
            Log::D(Log::TAG, "sqlite3_prepare_v2 error:");
            return std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>>(nullptr);
        }

        rc = sqlite3_get_table(mDb, t_strSql.c_str(), &azResult, &nrow, &ncolumn, &errMsg);
        if (rc == SQLITE_OK) {
            Log::D(Log::TAG, "getMessages, successful sql:%s", t_strSql.c_str());
        } else {
            Log::D(Log::TAG, "getMessages, Can't get table: %s", sqlite3_errmsg(mDb));
            return std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>>(nullptr);
        }

        std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> messages_list = std::make_shared<std::vector<std::shared_ptr<MessageInfo>>>();
        if (nrow != 0 && ncolumn != 0) {     //有查询结果,不包含表头所占行数
            for (int i = 1; i <= nrow; i++) {        // 第0行为数据表头
                std::shared_ptr<MessageInfo> message_info = std::make_shared<MessageInfo>();
                message_info->mFriendid = std::make_shared<std::string>(azResult[4*i + 1]);
                message_info->mMsg = std::make_shared<std::string>(azResult[4*i + 2]);
                message_info->mSendTimeStamp = atol(azResult[4*i + 3]);
                messages_list->push_back(message_info);
            }
        }

        sqlite3_free_table(azResult);
        sqlite3_finalize(pStmt);     //销毁一个SQL语句对象
        return messages_list;
    }

    std::map<std::string, std::shared_ptr<MemberInfo>> DatabaseProxy::getFriendList() {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        return mMemberList;
    }

    bool DatabaseProxy::removeMember(std::string friendid) {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        std::map<std::string, std::shared_ptr<MemberInfo>>::iterator key = mMemberList.find(
                friendid);
        if (key != mMemberList.end()) {
            mMemberList.erase(key);
            char *errMsg = NULL;
            std::string t_strSql;

            t_strSql = "delete from member_table where FriendId='"+friendid+"';";
            //消息直接入库
            int rv = sqlite3_exec(mDb, t_strSql.c_str(), callback, this, &errMsg);
            if (rv != SQLITE_OK) {
                Log::D(DatabaseProxy::TAG, "SQLite removeMember error: %s\n",
                       errMsg);
            }
            return true;
        } else {
            Log::D(Log::TAG, "removeMember not exist, friendid:%s", friendid.c_str());
            return false;
        }
    }

    int DatabaseProxy::callback(void *context, int argc, char **argv, char **azColName) {
        auto database_proxy = reinterpret_cast<DatabaseProxy *>(context);
        int i;
        for (i = 0; i < argc; ++i) {
            Log::D(DatabaseProxy::TAG, "database %s = %s\n", azColName[i],
                   argv[i] ? argv[i] : "NULL");
        }
        return 0;
    }

    bool DatabaseProxy::closeDb() {
        if (mDb != nullptr) {
            sqlite3_close(mDb);
        }
        return true;
    }

    void DatabaseProxy::syncMemberList() {
        MUTEX_LOCKER locker_sync_data(_SyncedMemberList);
        //查询一条记录
        char **azResult;   //二维数组存放结果
        char *errMsg = NULL;
        int nrow;          /* Number of result rows written here */
        int ncolumn;
        std::string t_strSql;
        t_strSql = "select * from member_table order by id asc";
        /*step 2: sql语句对象。*/
        sqlite3_stmt *pStmt;
        int rc = sqlite3_prepare_v2(
                mDb, //数据库连接对象
                t_strSql.c_str(), //指向原始sql语句字符串
                strlen(t_strSql.c_str()), //
                &pStmt,
                NULL
        );
        if (rc != SQLITE_OK) {
            Log::D(Log::TAG, "sqlite3_prepare_v2 error:%s", sqlite3_errmsg(mDb));
            return;
        }

        rc = sqlite3_get_table(mDb, t_strSql.c_str(), &azResult, &nrow, &ncolumn, &errMsg);
        if (rc == SQLITE_OK) {
            Log::D(Log::TAG, "getMessages, successful sql:%s", t_strSql.c_str());
        } else {
            Log::D(Log::TAG, "getMessages, Can't get table: %s", sqlite3_errmsg(mDb));
            return;
        }

        if (nrow != 0 && ncolumn != 0) {     //有查询结果,不包含表头所占行数
            for (int i = 1; i <= nrow; i++) {        // 第0行为数据表头
                mMemberList[azResult[5*i + 1]] = std::make_shared<MemberInfo>(
                        std::make_shared<std::string>(azResult[5*i + 1]),
                        std::make_shared<std::string>(azResult[5*i + 2]),
                        atoi(azResult[5*i + 3]),
                        atol(azResult[5*i + 4]));
            }
        }

        sqlite3_free_table(azResult);
        sqlite3_finalize(pStmt);     //销毁一个SQL语句对象
    }

    bool DatabaseProxy::startDb(const char *data_dir) {
        std::string strConn = std::string(data_dir) + "/chatrobot.db";
        char *errMsg;
        //打开一个数据库，如果改数据库不存在，则创建一个名字为databaseName的数据库文件
        int rv;
        rv = sqlite3_open(strConn.c_str(), &mDb);
        if (rv) {
            Log::D(DatabaseProxy::TAG, "Cannot open database: %s\n", sqlite3_errmsg(mDb));
            sqlite3_close(mDb);
            return 1;
        }
        char create_table[256] = "CREATE TABLE IF NOT EXISTS member_table(id INTEGER PRIMARY KEY AUTOINCREMENT,Friendid TEXT NOT NULL, NickName TEXT, Status INTEGER, MsgTimeStamp INTEGER)";
        rv = sqlite3_exec(mDb, create_table, callback, this, &errMsg);
        if (rv != SQLITE_OK) {
            Log::D(DatabaseProxy::TAG, "SQLite create_table statement execution error: %s\n",
                   errMsg);
            return 1;
        }
        char create_message_table[256] = "CREATE TABLE IF NOT EXISTS message_table (id INTEGER PRIMARY KEY AUTOINCREMENT,Friendid TEXT NOT NULL, Msg TEXT, SendTimeStamp INTEGER)";
        rv = sqlite3_exec(mDb, create_message_table, callback, this, &errMsg);
        if (rv != SQLITE_OK) {
            Log::D(DatabaseProxy::TAG,
                   "SQLite create_message_table statement execution error: %s\n", errMsg);
            return 1;
        }
        char create_blackmember_table[256] = "CREATE TABLE IF NOT EXISTS black_member_table (id INTEGER PRIMARY KEY AUTOINCREMENT,Friendid TEXT NOT NULL)";
        rv = sqlite3_exec(mDb, create_blackmember_table, callback, this, &errMsg);
        if (rv != SQLITE_OK) {
            Log::D(DatabaseProxy::TAG,
                   "SQLite create_blackmember_table statement execution error: %s\n", errMsg);
            return 1;
        }
        //同步Member信息
        syncMemberList();
        return 0;
    }
}