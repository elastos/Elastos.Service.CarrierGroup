//
// Created by luocf on 2019/6/15.
//

#ifndef CHATROBOT_GITHUB_GROUPINFO_H
#define CHATROBOT_GITHUB_GROUPINFO_H

#include <memory>
#include <mutex>
#include <ctime>
#include <memory> // std::unique_ptr
class GroupInfo {
public:
    GroupInfo(std::string friedn_id, std::string nickname,
              std::string data_dir, int membercount, int service_id);
    ~GroupInfo();
    std::string getAddress();
    std::string getNickName();
    std::string getDataDir();
    int getMemberCount();
    int getServiceId();
    int getPid();
    void setAddress(std::string address);
    void setNickName(std::string nickname);
    void setDataDir(std::string data_dir);
    void setMemberCount(int member_count);
    void setServiceId(int service_id);
    void setPid(int pid);
    void Lock();
    void UnLock();

private:
    std::string mFriendid;
    std::string mNickName;
    std::string mDataDir;
    int mMemberCount;
    int mServiceId;
    std::recursive_mutex mMutex;
    int mPid;
};


#endif //CHATROBOT_GITHUB_GROUPINFO_H
