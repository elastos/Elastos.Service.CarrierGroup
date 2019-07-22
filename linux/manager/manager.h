//
// Created by luocf on 2019/7/19.
//

#ifndef TESTMANAGER_MANAGER_H
#define TESTMANAGER_MANAGER_H

#include <string>
#include <iostream>
#include <thread>
#include <future>
#include <queue>

typedef std::lock_guard<std::mutex> MUTEX_LOCKER;
class manager {
public:
    manager();
    ~manager();
    void start(std::string ip, int port, std::string data_root_dir);
    void stop();
    int createGroup();
    void runCommunicationThread();
    void runWorkThread();

private:
    void sendMsgToWorkThread(std::string msg);
    int _createGroup();
    void _removeGroup(std::string address);
    void _updateGroupNickName(int id, std::string nick_name);
    void _updateGroupAddress(int id, std::string address);
    void _updateGroupMemberCount(int id, int member_count);
    void _bindService(int service_id, const std::string data_dir);
    std::thread mCommunicationThread;
    std::thread mWorkThread;
    std::string getLocalCacheDir(std::string class_name);
    std::string mRootDir;
    std::string mIp;
    int mPort;
    bool mIsReady;
    std::queue<std::shared_ptr<std::string>> mTmpQueue;
    std::queue<std::shared_ptr<std::string>> mQueue;
    std::mutex mQueue_lock;
    std::condition_variable mQueue_cond;
    std::condition_variable mWrite_cond;
};


#endif //TESTMANAGER_MANAGER_H
