//
// Created by luocf on 2019/6/13.
//
#include <cstring>
#include <future>
#include <stack>
#include <iostream>
#include <ctime>

using namespace std;

#include <ela_carrier.h>
#include <ela_session.h>
#include <Tools/Log.hpp>
#include "ThirdParty/json.hpp"
#include "CarrierRobot.h"
#include "ErrCode.h"

namespace chatrobot {
    using Json = nlohmann::json;
    CarrierRobot *CarrierRobot::instance = new CarrierRobot();

    CarrierRobot *CarrierRobot::getInstance() {
        return instance;
    }

    CarrierRobot::~CarrierRobot() {
        mCarrierConfig.reset();
    }

    CarrierRobot::CarrierRobot() {
        mCarrierConfig = std::make_shared<CarrierConfig>();
        mDatabaseProxy = std::make_shared<DatabaseProxy>();
    }

    int CarrierRobot::GetCarrierUsrIdByAddress(const std::string &address, std::string &usrId) {
        char buf[ELA_MAX_ID_LEN + 1] = {0};
        auto ret = ela_get_id_by_address(address.c_str(), buf, sizeof(buf));
        if (ret == nullptr) {
            int err = ela_get_error();
            char strerr_buf[512] = {0};
            ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
            Log::E(Log::TAG, "Failed to add friend! ret=%s(0x%x)", strerr_buf, ret);
            return ErrCode::FailedCarrier;
        }

        usrId = ret;
        return 0;
    }

    void CarrierRobot::runCarrier() {
        int ret = ela_run(mCarrier.get(), 500);
        if (ret < 0) {
            ela_kill(mCarrier.get());
            Log::E(Log::TAG, "Failed to run carrier!");
            return;
        }
    }

    void CarrierRobot::OnCarrierConnection(ElaCarrier *carrier,
                                           ElaConnectionStatus status, void *context) {
        auto channel = reinterpret_cast<CarrierRobot *>(context);
        Log::I(Log::TAG, "OnCarrierConnection status: %d", status);
        std::string carrierAddr, carrierUsrId;
        channel->getAddress(carrierAddr);
        GetCarrierUsrIdByAddress(carrierAddr, carrierUsrId);
    }

    void CarrierRobot::OnCarrierFriendRequest(ElaCarrier *carrier, const char *friendid,
                                              const ElaUserInfo *info,
                                              const char *hello, void *context) {
        Log::I(Log::TAG, "OnCarrierFriendRequest from: %s", friendid);
        ela_accept_friend(carrier, friendid);
    }

    std::time_t CarrierRobot::getTimeStamp() {
        return time(0);
    }

    void CarrierRobot::OnCarrierFriendConnection(ElaCarrier *carrier, const char *friendid,
                                                 ElaConnectionStatus status, void *context) {
        Log::I(Log::TAG, "OnCarrierFriendConnection from: %s %d", friendid, status);
        auto carrier_robot = reinterpret_cast<CarrierRobot *>(context);
        carrier_robot->updateMemberInfo(std::make_shared<std::string>(friendid), status,
                                        carrier_robot->getTimeStamp());
    }

    void CarrierRobot::updateMemberInfo(std::shared_ptr<std::string> friendid,
                                        ElaConnectionStatus status,
                                        std::time_t time_stamp) {
        if (mCreaterFriendId.get() == nullptr) {
            mCreaterFriendId = friendid;
        }
        ElaFriendInfo info;
        int ret = ela_get_friend_info(mCarrier.get(), friendid.get()->c_str(), &info);
        std::shared_ptr<std::string> nickanme;
        if (ret == 0) {
            nickanme = std::make_shared<std::string>(info.user_info.name);
        } else {
            nickanme = std::make_shared<std::string>("");
        }

        mDatabaseProxy->updateMemberInfo(friendid, nickanme, status, time_stamp);
        //当前状态为上线时，获取该成员offline以后的所以消息，并发送给该人,
        if (status == ElaConnectionStatus_Connected) {
            relayMessages();
        }
    }

    bool CarrierRobot::relayMessages() {
        std::map<std::string, std::shared_ptr<MemberInfo>> memberlist = mDatabaseProxy->getFriendList();
        std::map<std::string, std::shared_ptr<chatrobot::MemberInfo>>::iterator iter;
        for (iter = memberlist.begin(); iter != memberlist.end(); iter++) {
            std::shared_ptr<chatrobot::MemberInfo> memberInfo = iter->second;
            memberInfo->Lock();
            if (memberInfo->mStatus != ElaConnectionStatus_Connected) {
                memberInfo->UnLock();
                continue;
            }
            std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> message_list = mDatabaseProxy->getMessages();
            for (int i = 0; i < message_list->size(); i++) {
                std::shared_ptr<MessageInfo> message = message_list->at(i);
                if (message.get() != nullptr
                    && (*memberInfo->mFriendid.get()).compare((*message->mFriendid.get())) != 0
                    && memberInfo->mMsgTimeStamp < message->mSendTimeStamp) {
                    if (100 + i < message_list->size()) {
                        i += message_list->size() - 100;
                        Log::I(Log::TAG,
                               "relayMessages last 100 messages, i:%d, message_list->size():%d", i,
                               message_list->size());
                        continue;
                    }

                    char msg[1024];
                    sprintf(msg, "%s: \n%s \n[%s]", memberInfo->mNickName.get()->c_str(),
                            message->mMsg.get()->c_str(),
                            this->convertDatetimeToString(message->mSendTimeStamp).c_str());
                    int msg_ret = ela_send_friend_message(mCarrier.get(),
                                                          memberInfo->mFriendid.get()->c_str(),
                                                          msg, strlen(msg));
                    if (msg_ret != 0) {
                        break;
                    }

                    memberInfo->mMsgTimeStamp = message->mSendTimeStamp;
                }

            }
            memberInfo->UnLock();
        }
        return false;
    }

    bool CarrierRobot::handleSpecialMessage(std::shared_ptr<std::string> friend_id,
                                            const std::string &message) {
        bool ret = false;
        //Test
        if ((*mCreaterFriendId.get()).compare((*friend_id.get())) == 0) {
            //群主时，解析特殊指令,若有特殊指令，执行相应的任务，如踢人、退群等
            if (message.find("/list") == 0) {
                Json data = Json::object();
                Json data_json = Json::array();
                std::map<std::string, std::shared_ptr<MemberInfo>> mMemberList = mDatabaseProxy->getFriendList();
                std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> friendlist = std::make_shared<std::vector<std::shared_ptr<MemberInfo>>>();
                std::string ret_msg_str = "";
                for (auto item = mMemberList.begin(); item != mMemberList.end(); item++) {
                    std::shared_ptr<MemberInfo> memberInfo = item->second;
                    memberInfo->Lock();
                    if ((*mCreaterFriendId.get()).compare((*memberInfo->mFriendid.get())) == 0) {
                        memberInfo->UnLock();
                        continue;
                    }
                    ret_msg_str += std::string(
                            std::to_string(memberInfo->mIndex) + ": " + (*memberInfo->mNickName.get()) + " " +
                            std::to_string(memberInfo->mStatus) + "\n");
                    memberInfo->UnLock();
                }
                int ela_ret = ela_send_friend_message(mCarrier.get(), friend_id->c_str(),
                                                      ret_msg_str.c_str(),
                                                      strlen(ret_msg_str.c_str()));
                if (ela_ret != 0) {
                    Log::I(Log::TAG,
                           "handleSpecialMessage .c_str(): %s errno:(0x%x)",
                           ret_msg_str.c_str(), ela_get_error());
                }
                ret = true;
            } else if (message.find("/del") == 0) {
                std::string del_userindex = message.substr(4, message.length() - 4);
                int user_num = std::atoi(del_userindex.c_str());
                std::shared_ptr<MemberInfo> memberInfo = mDatabaseProxy->getMemberInfo(user_num);
                char msg_str[256];
                if (memberInfo.get() != nullptr) {
                    memberInfo->Lock();
                    mDatabaseProxy->removeMember(*memberInfo->mFriendid.get());
                    int ela_ret = ela_remove_friend(mCarrier.get(),
                                                    memberInfo->mFriendid.get()->c_str());
                    sprintf(msg_str, "%s has been kicked out!",
                            memberInfo->mNickName.get()->c_str());
                    if (ela_ret != 0) {
                        Log::I(Log::TAG,
                               "handleSpecialMessage can't delete this user: %s errno:(0x%x)",
                               memberInfo->mFriendid.get()->c_str(), ela_get_error());
                    }
                    memberInfo->UnLock();
                } else {
                    sprintf(msg_str, "num %s member not exist!", del_userindex.c_str());
                }

                ela_send_friend_message(mCarrier.get(), friend_id->c_str(),
                                        msg_str, strlen(msg_str));
                ret = true;
            } else if (message.find("/help") == 0) {
                const char *msg_str = "/list list the members \n/del delete the member, format:del member number";
                ela_send_friend_message(mCarrier.get(), friend_id->c_str(),
                                        msg_str, strlen(msg_str));
                ret = true;
            }
        }

        return ret;
    }

    void CarrierRobot::addMessgae(std::shared_ptr<std::string> friend_id,
                                  std::shared_ptr<std::string> message, std::time_t send_time) {

        bool spec_command = handleSpecialMessage(friend_id, *message.get());
        if (spec_command == false) {
            //save message
            mDatabaseProxy->addMessgae(friend_id, message, send_time);
            //将该消息转发给其他人
            relayMessages();
        }
    }

    std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> CarrierRobot::getFriendList() {
        std::map<std::string, std::shared_ptr<MemberInfo>> mMemberList = mDatabaseProxy->getFriendList();
        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> friendlist = std::make_shared<std::vector<std::shared_ptr<MemberInfo>>>();
        for (auto item = mMemberList.begin(); item != mMemberList.end(); item++) {
            auto memberInfo = item->second;
            if (memberInfo.get() != nullptr) {
                memberInfo->Lock();
                friendlist->push_back(memberInfo);
                memberInfo->UnLock();
            }
        }

        return friendlist;
    }

    void CarrierRobot::OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                              const void *msg, size_t len, void *context) {
        Log::I(Log::TAG, "OnCarrierFriendMessage from: %s len=%d", from, len);
        auto carrier_robot = reinterpret_cast<CarrierRobot *>(context);
        const char *data = (const char *) (msg);
        carrier_robot->addMessgae(std::make_shared<std::string>(from),
                                  std::make_shared<std::string>(data),
                                  carrier_robot->getTimeStamp());
    }

    int CarrierRobot::start(const char *data_dir) {
        ElaOptions carrierOpts;
        ElaCallbacks carrierCallbacks;

        memset(&carrierOpts, 0, sizeof(carrierOpts));
        memset(&carrierCallbacks, 0, sizeof(carrierCallbacks));
        carrierCallbacks.connection_status = OnCarrierConnection;
        carrierCallbacks.friend_request = OnCarrierFriendRequest;
        carrierCallbacks.friend_connection = OnCarrierFriendConnection;
        carrierCallbacks.friend_message = OnCarrierFriendMessage;

        carrierOpts.udp_enabled = mCarrierConfig->mEnableUdp;
        carrierOpts.persistent_location = data_dir;

        // set BootstrapNode
        size_t carrierNodeSize = mCarrierConfig->mBootstrapNodes.size();
        BootstrapNode carrierNodeArray[carrierNodeSize];

        memset(carrierNodeArray, 0, sizeof(carrierNodeArray));
        for (int idx = 0; idx < carrierNodeSize; idx++) {
            const auto &node = mCarrierConfig->mBootstrapNodes[idx];
            carrierNodeArray[idx].ipv4 = strdup(node->mIpv4->c_str());
            carrierNodeArray[idx].ipv6 = NULL;
            carrierNodeArray[idx].port = strdup(node->mPort->c_str());
            carrierNodeArray[idx].public_key = strdup(node->mPublicKey->c_str());
        }
        carrierOpts.bootstraps_size = carrierNodeSize;
        carrierOpts.bootstraps = carrierNodeArray;

        // set HiveBootstrapNode
        /* size_t hiveNodeSize = mCarrierConfig->mHiveNodes.size();
         HiveBootstrapNode hiveNodeArray[hiveNodeSize];
         memset(hiveNodeArray, 0, sizeof(hiveNodeArray));
         for(int idx = 0; idx < hiveNodeSize; idx++) {
             const auto& node = mCarrierConfig->mHiveNodes[idx];
             hiveNodeArray[idx].ipv4 = node.mIpv4.c_str();
             hiveNodeArray[idx].port = node.mPort.c_str();
         }*/
        //carrierOpts.hive_bootstraps_size = hiveNodeSize;
        //carrierOpts.hive_bootstraps = hiveNodeArray;

        ela_log_init(static_cast<ElaLogLevel>(mCarrierConfig->mLogLevel), nullptr, nullptr);

        auto creater = [&]() -> ElaCarrier * {
            auto ptr = ela_new(&carrierOpts, &carrierCallbacks, this);
            return ptr;
        };

        auto deleter = [=](ElaCarrier *ptr) -> void {
            if (ptr != nullptr) {
                ela_session_cleanup(ptr);
                ela_kill(ptr);
            }
        };

        mCarrier = std::unique_ptr<ElaCarrier, std::function<void(ElaCarrier *)>>(creater(),
                                                                                  deleter);
        if (mCarrier == nullptr) {
            Log::E(TAG, "Failed to new carrier!");
            int err = ela_get_error();
            char strerr_buf[512] = {0};
            ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
            Log::E(Log::TAG, "CarrierRobot::start failed! ret=%s(0x%x)", strerr_buf, err);
            return ErrCode::FailedCarrier;
        }
        //runCarrier();
        //std::async(std::bind(&CarrierRobot::runCarrier, this));
        return 0;
    }

    int CarrierRobot::getUserId(std::string &userid) {
        char addr[ELA_MAX_ID_LEN + 1] = {0};
        auto ret = ela_get_userid(mCarrier.get(), addr, sizeof(addr));
        if (ret == nullptr) {
            int err = ela_get_error();
            char strerr_buf[512] = {0};
            ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
            return ErrCode::FailedCarrier;
        }
        userid = addr;
        return 0;
    }

    int CarrierRobot::getAddress(std::string &address) {
        char addr[ELA_MAX_ADDRESS_LEN + 1] = {0};
        auto ret = ela_get_address(mCarrier.get(), addr, sizeof(addr));
        if (ret == nullptr) {
            int err = ela_get_error();
            char strerr_buf[512] = {0};
            ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));

            return ErrCode::FailedCarrier;
        }

        address = addr;
        return 0;
    }

    std::string CarrierRobot::convertDatetimeToString(std::time_t time) {
        tm *tm_ = localtime(&time);                // 将time_t格式转换为tm结构体
        int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
        year = tm_->tm_year +
               1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900。
        month = tm_->tm_mon +
                1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1。
        day = tm_->tm_mday;                        // 临时变量，日。
        hour = tm_->tm_hour;                       // 临时变量，时。
        minute = tm_->tm_min;                      // 临时变量，分。
        second = tm_->tm_sec;                      // 临时变量，秒。
        char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// 定义时间的各个char*变量。
        sprintf(yearStr, "%d", year);              // 年。
        sprintf(monthStr, "%d", month);            // 月。
        sprintf(dayStr, "%d", day);                // 日。
        sprintf(hourStr, "%d", hour);              // 时。
        sprintf(minuteStr, "%d", minute);          // 分。
        if (minuteStr[1] == '\0')                  // 如果分为一位，如5，则需要转换字符串为两位，如05。
        {
            minuteStr[2] = '\0';
            minuteStr[1] = minuteStr[0];
            minuteStr[0] = '0';
        }
        sprintf(secondStr, "%d", second);          // 秒。
        if (secondStr[1] == '\0')                  // 如果秒为一位，如5，则需要转换字符串为两位，如05。
        {
            secondStr[2] = '\0';
            secondStr[1] = secondStr[0];
            secondStr[0] = '0';
        }
        char s[20];                                // 定义总日期时间char*变量。
        sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr,
                secondStr);// 将年月日时分秒合并。
        std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。
        return str;                                // 返回转换日期时间后的string变量。
    }

    bool CarrierRobot::IsJsonIllegal(const char *jsoncontent) {
        std::stack<char> jsonstr;
        const char *p = jsoncontent;
        char startChar = jsoncontent[0];
        char endChar = '\0';
        bool isObject = false;//防止 {}{}的判断
        bool isArray = false;//防止[][]的判断

        while (*p != '\0') {
            endChar = *p;
            switch (*p) {
                case '{':
                    if (!isObject) {
                        isObject = true;
                    } else if (jsonstr.empty())//对象重复入栈
                    {
                        return false;
                    }
                    jsonstr.push('{');
                    break;
                case '"':
                    if (jsonstr.empty() || jsonstr.top() != '"') {
                        jsonstr.push(*p);
                    } else {
                        jsonstr.pop();
                    }
                    break;
                case '[':
                    if (!isArray) {
                        isArray = true;
                    } else if (jsonstr.empty())//数组重复入栈
                    {
                        return false;
                    }
                    jsonstr.push('[');
                    break;
                case ']':
                    if (jsonstr.empty() || jsonstr.top() != '[') {
                        return false;
                    } else {
                        jsonstr.pop();
                    }
                    break;
                case '}':
                    if (jsonstr.empty() || jsonstr.top() != '{') {
                        return false;
                    } else {
                        jsonstr.pop();
                    }
                    break;
                case '\\'://被转义的字符,跳过
                    p++;
                    break;
                default:;
            }
            p++;
        }

        if (jsonstr.empty()) {
            //确保是对象或者是数组,之外的都不算有效
            switch (startChar)//确保首尾符号对应
            {
                case '{': {
                    if (endChar == '}') {
                        return true;
                    }
                    return false;
                }
                case '[': {
                    if (endChar == ']') {
                        return true;
                    }
                    return false;
                }
                default:
                    return false;
            }

            return true;
        } else {
            return false;
        }
    }
}