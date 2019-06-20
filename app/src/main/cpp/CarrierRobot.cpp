//
// Created by luocf on 2019/6/13.
//
#include <cstring>
#include <future>
#include <stack>
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
        std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now());
        return tp.time_since_epoch().count();
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
            if (memberInfo->mStatus != ElaConnectionStatus_Connected) {
                continue;
            }

            std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> message_list = mDatabaseProxy->getMessages();
            for (int i = 0; i < message_list->size(); i++) {
                std::shared_ptr<MessageInfo> message = message_list->at(i);
                if (message.get() != nullptr
                    && !(*memberInfo->mFriendid.get()).compare((*message->mFriendid.get())) == 0
                    && memberInfo->mMsgTimeStamp < message->mSendTimeStamp) {
                    if (100+i < message_list->size()) {
                        i+= message_list->size() - 100;
                        Log::I(Log::TAG, "relayMessages last 100 messages, i:%d, message_list->size():%d", i, message_list->size());
                        continue;
                    }

                    Json msg_json = Json::object();
                    msg_json["from"] = message->mFriendid.get()->c_str();
                    msg_json["nickname"] = memberInfo->mNickName.get()->c_str();
                    msg_json["sendtime"] = message->mSendTimeStamp;
                    msg_json["content"] = message->mMsg.get()->c_str();
                    const char* ret_msg = msg_json.dump().c_str();
                    int msg_ret = ela_send_friend_message(mCarrier.get(), memberInfo->mFriendid.get()->c_str(),
                                                          ret_msg, strlen(ret_msg));
                    if (msg_ret != 0) {
                        break;
                    }
                    memberInfo->mMsgTimeStamp = message->mSendTimeStamp;
                }
            }
        }
        return false;
    }

    bool CarrierRobot::handleSpecialMessage(std::shared_ptr<std::string> friend_id,
                                            const std::string &message) {
        bool ret = false;
        //Test
        if ((*mCreaterFriendId.get()).compare((*friend_id.get())) == 0
            && this->IsJsonIllegal(message.c_str())) {
            //群主时，解析特殊指令,若有特殊指令，执行相应的任务，如踢人、退群等
            Json jsonInfo = Json::parse(message);
            std::string cmd = jsonInfo["cmd"];
            Log::I(Log::TAG, "handleSpecialMessage message: %s", message.c_str());
            if (cmd.empty() == false) {
                Json ret_json = Json::object();
                Json result_json = Json::object();
                ret_json["ack"] = cmd;
                if (cmd.compare("del") == 0) {
                    Json params = jsonInfo["params"];
                    std::string del_userid = params[0];
                    bool ret_del = mDatabaseProxy->removeMember(del_userid);
                    if (ret_del == true) {
                        int ela_ret = ela_remove_friend(mCarrier.get(), del_userid.c_str());
                        if (ela_ret == 0) {
                            result_json["code"] = 0;
                            ret_json["result"] = result_json;
                            const char* msg_str =  ret_json.dump().c_str();
                            ela_ret = ela_send_friend_message(mCarrier.get(), friend_id->c_str(),
                                                    msg_str, strlen(msg_str));
                        }
                        if (ela_ret != 0) {
                            Log::I(Log::TAG,
                                   "handleSpecialMessage can't delete this user: %s errno:(0x%x)",
                                   del_userid.c_str(), ela_get_error());
                        }
                    }
                    ret = true;
                } else if (cmd.compare("getmemberlist") == 0) {
                    result_json["code"] = 0;
                    Json data_json = Json::array();
                    std::map<std::string, std::shared_ptr<MemberInfo>> mMemberList = mDatabaseProxy->getFriendList();
                    std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> friendlist = std::make_shared<std::vector<std::shared_ptr<MemberInfo>>>();
                    for (auto item = mMemberList.begin(); item != mMemberList.end(); item++) {
                        std::shared_ptr<MemberInfo> value = item->second;
                        if ((*mCreaterFriendId.get()).compare((*value->mFriendid.get())) == 0) {
                            continue;
                        }
                        Json member_json = Json::object();
                        member_json["userid"] = value->mFriendid.get()->c_str();
                        member_json["nickname"] = value->mNickName.get()->c_str();
                        member_json["status"] = value->mStatus;
                        data_json.push_back(member_json);
                    }

                    result_json["data"] = data_json;
                    ret_json["result"] = result_json;
                    const char* ret_msg_str =  ret_json.dump().c_str();
                    int ela_ret = ela_send_friend_message(mCarrier.get(), friend_id->c_str(),
                                            ret_msg_str, strlen(ret_msg_str));
                    if (ela_ret != 0) {
                        Log::I(Log::TAG,
                               "handleSpecialMessage .c_str(): %s errno:(0x%x)",
                               ret_msg_str, ela_get_error());
                    }
                    ret = true;
                } else {
                    Log::I(Log::TAG, "handleSpecialMessage not support this command: %s",
                           cmd.c_str());
                }
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
            auto value = item->second;
            friendlist->push_back(value);
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