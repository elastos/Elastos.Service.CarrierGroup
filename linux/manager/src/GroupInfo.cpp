//
// Created by luocf on 2019/6/15.
//
#include <string>
#include <sstream>
#include <iostream>
#include "GroupInfo.h"
#include "../../common/Log.hpp"
GroupInfo::GroupInfo(std::string friedn_id, std::string nickname,
        std::string data_dir, int membercount, int service_id):
        mFriendid(friedn_id),
        mNickName(nickname),
        mDataDir(data_dir),
        mMemberCount(membercount),
        mServiceId(service_id) {
}
GroupInfo::~GroupInfo() {
    
}
std::string GroupInfo::getAddress() {
    return mFriendid;
};

std::string GroupInfo::getNickName() {
    return mNickName;
}

std::string GroupInfo::getDataDir() {
    return mDataDir;
}

int GroupInfo::getMemberCount() {
    return mMemberCount;
}

int GroupInfo::getServiceId() {
    return mServiceId;
}

void GroupInfo::setAddress(std::string address) {
    mFriendid = address;
}

void GroupInfo::setNickName(std::string nickname) {
    mNickName = nickname;
}

void GroupInfo::setDataDir(std::string data_dir) {
    mDataDir = data_dir;
}

void GroupInfo::setMemberCount(int member_count) {
    mMemberCount = member_count;
}

void GroupInfo::setServiceId(int service_id) {
    mServiceId = service_id;
}

void GroupInfo::Lock() {
    Log::D(Log::TAG, "GroupInfo lock %s", mFriendid.c_str());
    mMutex.lock();
}

void GroupInfo::UnLock() {
    Log::D(Log::TAG, "GroupInfo unLock %s", mFriendid.c_str());
    mMutex.unlock();
}
