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
#include <thread>
#include <chrono>
#include <Log.hpp>
#include <json.hpp>
#include <CommonVar.h>
#include <ThirdParty/CompatibleFileSystem.hpp>
#include <Command/ChatRobotCmd.hpp>
#include "CarrierRobot.h"
#include "ErrCode.h"

namespace chatrobot {
    using json = nlohmann::json;
    std::string CarrierRobot::Factory::sLocalDataDir;

    /***********************************************/
    /***** static function implement ***************/
    /***********************************************/
    int CarrierRobot::Factory::SetLocalDataDir(const std::string &dir) {
        if (dir.empty()) {
            return ErrCode::InvalidArgument;
        }

        std::error_code stdErrCode;
        bool ret;
        ret = std::filesystem::create_directories(dir, stdErrCode);
        if (ret == false
            || stdErrCode.value() != 0) {
            int errCode = ErrCode::StdSystemErrorIndex - stdErrCode.value();
            auto errMsg = ErrCode::ToString(errCode);
            Log::D(Log::TAG, "Failed to set local data dir, errcode: %s", errMsg.c_str());
            return errCode;
        }

        sLocalDataDir = dir;

        return 0;
    }

    std::shared_ptr<CarrierRobot> CarrierRobot::Factory::Create() {
        struct Impl : CarrierRobot {
        };

        return std::make_shared<Impl>();
    }

    CarrierRobot::~CarrierRobot() {
        mCarrierConfig.reset();
        this->stop();
    }

    CarrierRobot::CarrierRobot() {
        mCarrierConfig = std::make_shared<CarrierConfig>();
        mDatabaseProxy = std::make_shared<DatabaseProxy>();
        //anypeer Id format
        std::string reg_str("(\\w{8})-(\\w{4})-(\\w{4})-(\\w{4})-(\\w{13})");
        mMsgReg = std::make_shared<std::regex>(reg_str, std::regex::icase);
        mStatus = 0;
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

    void CarrierRobot::runCarrierInner() {
        int ret = ela_run(mCarrier.get(), 500);
        if (ret < 0) {
            ela_kill(mCarrier.get());
            Log::E(Log::TAG, "Failed to run carrier!");
        }

    }

    void CarrierRobot::runCarrier() {
        mCarrieryThread = std::thread(&CarrierRobot::runCarrierInner, this); //引用
    }

    void CarrierRobot::OnCarrierFriendInfoChanged(ElaCarrier *carrier, const char *friendid,
                                                  const ElaFriendInfo *info, void *context) {
        auto carrier_robot = reinterpret_cast<CarrierRobot *>(context);
        std::shared_ptr<std::string> nickanme = std::make_shared<std::string>(info->user_info.name);
        carrier_robot->updateMemberInfo(std::make_shared<std::string>(friendid), nickanme,
                                        ElaConnectionStatus_Connected);
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
        auto carrierRobot = reinterpret_cast<CarrierRobot *>(context);
        carrierRobot->acceptFriend(std::make_shared<std::string>(friendid));
    }

    std::time_t CarrierRobot::getTimeStamp() {
        return time(0);
    }

    void CarrierRobot::OnCarrierFriendConnection(ElaCarrier *carrier, const char *friendid,
                                                 ElaConnectionStatus status, void *context) {
        Log::I(Log::TAG, "OnCarrierFriendConnection from: %s %d", friendid, status);
        auto carrier_robot = reinterpret_cast<CarrierRobot *>(context);
        if (!carrier_robot->inRemovedList(std::make_shared<std::string>(friendid))) {
            ElaFriendInfo info;
            int ret = ela_get_friend_info(carrier, friendid, &info);
            std::shared_ptr<std::string> nickanme;
            if (ret == 0) {
                nickanme = std::make_shared<std::string>(info.user_info.name);
            } else {
                nickanme = std::make_shared<std::string>("");
            }
            carrier_robot->updateMemberInfo(std::make_shared<std::string>(friendid), nickanme, status);

        }
    }

    bool CarrierRobot::inRemovedList(std::shared_ptr<std::string> friendid) {
        return mDatabaseProxy->inRemovedList(*friendid.get());
    }

    void CarrierRobot::updateMemberInfo(std::shared_ptr<std::string> friendid,
                                        std::shared_ptr<std::string> nickname,
                                        ElaConnectionStatus status) {
        if (mCreaterFriendId.get() == nullptr) {
            mCreaterFriendId = friendid;
        }

        std::shared_ptr<MemberInfo> block_member_info = mDatabaseProxy->getBlockMemberInfo(friendid);
        if (block_member_info.get() == nullptr) {
            mDatabaseProxy->updateMemberInfo(friendid, nickname, status == ElaConnectionStatus_Connected ? 0 : 1, 0);
            //当前状态为上线时，获取该成员offline以后的所以消息，并发送给该人,
            if (status == ElaConnectionStatus_Connected) {
                relayMessages();
            }
        }
        json msg;
        msg["cmd"] = Command_UpdateMemberCount;
        msg["friendid"] = mAddress.c_str();
        msg["membercount"] = getFriendList()->size();
        sendMsgForManager(msg.dump());
    }

    bool CarrierRobot::relayMessages() {
        MUTEX_LOCKER locker_sync_data(_mReplyMessage);
        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> memberlist = mDatabaseProxy->getFriendList();
        for (int i = 0; i < memberlist->size(); i++) {
            std::shared_ptr<MemberInfo> memberInfo = memberlist->at(i);
            if (memberInfo.get() == nullptr) {
                continue;
            }
            bool info_changed = false;
            memberInfo->Lock();
            if (memberInfo->mStatus != 0) {
                memberInfo->UnLock();
                continue;
            }
            std::shared_ptr<std::vector<std::shared_ptr<MessageInfo>>> message_list = mDatabaseProxy->getMessages(
                    memberInfo->mFriendid, memberInfo->mMsgTimeStamp, 100);
            if (message_list.get() != nullptr) {
                for (int i = 0; i < message_list->size(); i++) {
                    std::shared_ptr<MessageInfo> message = message_list->at(i);
                    if (message.get() != nullptr) {
                        char msg[1024];
                        std::shared_ptr<chatrobot::MemberInfo> target_memberInfo = mDatabaseProxy->getMemberInfo(
                                message->mFriendid);
                        if (target_memberInfo.get() != nullptr) {
                            target_memberInfo->Lock();
                            sprintf(msg, "%s: %s \n[%s]",
                                    target_memberInfo->mNickName.get()->c_str(),
                                    message->mMsg.get()->c_str(),
                                    this->convertDatetimeToString(message->mSendTimeStamp).c_str());
                            target_memberInfo->UnLock();
                        }

                        int msg_ret = ela_send_friend_message(mCarrier.get(),
                                                              memberInfo->mFriendid.get()->c_str(),
                                                              msg, strlen(msg));
                        if (msg_ret != 0) {
                            break;
                        }
                        info_changed = true;
                        memberInfo->mMsgTimeStamp = message->mSendTimeStamp;
                    }
                }
            }
            memberInfo->UnLock();
            if (info_changed) {
                mDatabaseProxy->updateMemberInfo(memberInfo->mFriendid, memberInfo->mNickName, memberInfo->mStatus, memberInfo->mMsgTimeStamp);
            }

        }
        return false;
    }

    void CarrierRobot::addMessgae(std::shared_ptr<std::string> friend_id,
                                  std::shared_ptr<std::string> message, std::time_t send_time) {
        std::string errMsg;
        std::string msg = *message.get();
        if (msg.size() > 38 &&
            std::regex_match(msg.substr(0, 37).c_str(), *mMsgReg.get()) == true) {
            msg = msg.substr(38);//包含空格
        }
        std::string pre_cmd = msg + " " + *friend_id.get();//Pretreatment cmd
        int ret = ChatRobotCmd::Do(this, pre_cmd, errMsg);
        if (ret < 0) {
            //save message
            mDatabaseProxy->addMessgae(friend_id, msg, send_time);
            //将该消息转发给其他人
            relayMessages();
        }
    }

    std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> CarrierRobot::getFriendList() {
        return mDatabaseProxy->getFriendList();
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

    void CarrierRobot::stop() {
        mDatabaseProxy->closeDb();
    }

    int CarrierRobot::start(const char *data_dir, int service_id, int my_socket_fd) {
        mMySocketFd = my_socket_fd;
        ElaOptions carrierOpts;
        ElaCallbacks carrierCallbacks;
        mLocalPath = std::make_shared<std::string>(data_dir);
        mDatabaseProxy->startDb(data_dir);
        memset(&carrierOpts, 0, sizeof(carrierOpts));
        memset(&carrierCallbacks, 0, sizeof(carrierCallbacks));
        carrierCallbacks.connection_status = OnCarrierConnection;
        carrierCallbacks.friend_request = OnCarrierFriendRequest;
        carrierCallbacks.friend_connection = OnCarrierFriendConnection;
        carrierCallbacks.friend_message = OnCarrierFriendMessage;
        carrierCallbacks.friend_info = OnCarrierFriendInfoChanged;
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
        ela_log_init(static_cast<ElaLogLevel>(mCarrierConfig->mLogLevel), nullptr, nullptr);

        auto creater = [&]() -> ElaCarrier * {
            ela_log_init(ElaLogLevel::ElaLogLevel_Warning, data_dir, nullptr);
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
        std::shared_ptr<MemberInfo> member_info = mDatabaseProxy->getMemberInfo(1);
        if (member_info.get() != nullptr) {
            mCreaterFriendId = member_info->mFriendid;
        }
        getAddress(mAddress);
        json msg;
        msg["cmd"] = Command_UpdateAddress;
        msg["serviceid"] = service_id;
        msg["friendid"] = mAddress.c_str();
        sendMsgForManager(msg.dump());
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

    int CarrierRobot::acceptFriend(std::shared_ptr<std::string>friendid) {
        std::shared_ptr<MemberInfo> member_info = mDatabaseProxy->getBlockMemberInfo(friendid);
        if (member_info.get() == nullptr) {
            mDatabaseProxy->deleteRemovedListMember(*friendid.get());
            ela_accept_friend(mCarrier.get(), friendid->c_str());
        } else {
            Log::D(TAG, "acceptFriend can't accept friend, block member:%s", friendid->c_str());
        }

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

    void CarrierRobot::helpCmd(const std::vector<std::string> &args, const std::string &message) {
        if (args.size() >= 2) {
            const std::string friend_id = args[1];
            int ela_ret = ela_send_friend_message(mCarrier.get(), friend_id.c_str(),
                                                  message.c_str(), strlen(message.c_str()));
            if (ela_ret != 0) {
                Log::I(Log::TAG,
                       "helpCmd .c_str(): %s errno:(0x%x)",
                       message.c_str(), ela_get_error());
            }
        }

    }

    void CarrierRobot::listCmd(const std::vector<std::string> &args) {
        if (args.size() >= 2) {
            const std::string friend_id = args[1];
            std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> memberlist = mDatabaseProxy->getFriendList();
            std::string ret_msg_str = "";
            for (int i = 0; i < memberlist->size(); i++) {
                std::shared_ptr<MemberInfo> memberInfo = memberlist->at(i);
                if (memberInfo.get() == nullptr) {
                    continue;
                }
                memberInfo->Lock();
                ret_msg_str += std::string(
                        std::to_string(memberInfo->mIndex) + ": " +
                        (*memberInfo->mNickName.get()) + " " +
                        std::string(memberInfo->mStatus == 0 ? "online" : "offline") + "\n");
                memberInfo->UnLock();//同名还没处理
            }

            int ela_ret = ela_send_friend_message(mCarrier.get(), friend_id.c_str(),
                                                  ret_msg_str.c_str(),
                                                  strlen(ret_msg_str.c_str()));
            if (ela_ret != 0) {
                Log::I(Log::TAG,
                       "listCmd .c_str(): %s errno:(0x%x)",
                       ret_msg_str.c_str(), ela_get_error());
            }
        }

    }

    int CarrierRobot::getGroupStatus() {
        return mStatus;
    }

    std::shared_ptr<std::string> CarrierRobot::getGroupNickName() {
        std::shared_ptr<std::string> nick_name = mDatabaseProxy->getGroupNickName();
        return nick_name;
    }

    void CarrierRobot::updateNickNameCmd(const std::vector<std::string> &args) {
        MUTEX_LOCKER locker_sync_data(_mReplyMessage);
        if (args.size() >= 3) {
            const std::string friend_id = args[2];
            if ((*mCreaterFriendId.get()).compare((friend_id)) == 0) {
                const std::string nick_name = args[1];
                mDatabaseProxy->updateGroupNickName(std::make_shared<std::string>(nick_name));
                json msg;
                msg["cmd"] =Command_UpdateNickName;
                msg["friendid"] = mAddress.c_str();
                msg["nickname"] = nick_name.c_str();
                sendMsgForManager(msg.dump());
            }
        }
    }

    void CarrierRobot::deleteGroupCmd(const std::vector<std::string> &args) {
        MUTEX_LOCKER locker_sync_data(_mReplyMessage);
        if (args.size() >= 2) {
            const std::string friend_id = args[1];
            if ((*mCreaterFriendId.get()).compare((friend_id)) == 0) {
                mStatus = -1;
                json msg;
                msg["cmd"] = Command_UpdateStatus;
                msg["friendid"] = mAddress.c_str();
                msg["my_socket_fd"] = mMySocketFd;
                msg["status"] = mStatus;
                sendMsgForManager(msg.dump());
                //好友解除关系
                std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>> memberlist = mDatabaseProxy->getFriendList();
                for (int i = 0; i < memberlist->size(); i++) {
                    std::shared_ptr<MemberInfo> memberInfo = memberlist->at(i);
                    if (memberInfo.get() == nullptr) {
                        continue;
                    }
                    memberInfo->Lock();
                    int ela_ret = ela_remove_friend(mCarrier.get(),
                                                    memberInfo->mFriendid.get()->c_str());
                    if (ela_ret != 0) {
                        Log::I(Log::TAG,
                               "deleteGroupCmd can't delete this user: %s errno:(0x%x)",
                               memberInfo->mFriendid.get()->c_str(), ela_get_error());
                    }
                    memberInfo->UnLock();
                }
            }
        }
    }

    void CarrierRobot::blockFriendCmd(const std::vector<std::string> &args) {
        if (args.size() >= 3) {
            const std::string friend_id = args[2];
            if ((*mCreaterFriendId.get()).compare((friend_id)) == 0) {
                std::string del_userindex = args[1];
                int user_num = std::atoi(del_userindex.c_str());
                std::shared_ptr<MemberInfo> memberInfo = mDatabaseProxy->getMemberInfo(user_num);
                char msg_str[256];
                if (memberInfo.get() != nullptr) {
                    memberInfo->Lock();
                    std::string del_friend_id = *memberInfo->mFriendid.get();
                    memberInfo->UnLock();
                    mDatabaseProxy->addBlockMember(memberInfo);
                    mDatabaseProxy->removeMember(del_friend_id);

                    int ela_ret = ela_remove_friend(mCarrier.get(),
                                                    memberInfo->mFriendid.get()->c_str());
                    sprintf(msg_str, "%s has been blocked!",
                            memberInfo->mNickName.get()->c_str());
                    if (ela_ret != 0) {
                        Log::I(Log::TAG,
                               "delCmd can't delete this user: %s errno:(0x%x)",
                               memberInfo->mFriendid.get()->c_str(), ela_get_error());
                    }
                } else {
                    sprintf(msg_str, "num %s member not exist!", del_userindex.c_str());
                }
                ela_send_friend_message(mCarrier.get(), friend_id.c_str(),
                                        msg_str, strlen(msg_str));
                json msg;
                msg["cmd"] = Command_UpdateMemberCount;
                msg["friendid"] = mAddress.c_str();
                msg["membercount"] = getFriendList()->size();
                sendMsgForManager(msg.dump());
            }
        }
    }

    void CarrierRobot::delCmd(const std::vector<std::string> &args) {
        if (args.size() >= 3) {
            const std::string friend_id = args[2];
            if ((*mCreaterFriendId.get()).compare((friend_id)) == 0) {
                std::string del_userindex = args[1];
                int user_num = std::atoi(del_userindex.c_str());
                std::shared_ptr<MemberInfo> memberInfo = mDatabaseProxy->getMemberInfo(user_num);
                char msg_str[256];
                if (memberInfo.get() != nullptr) {
                    memberInfo->Lock();
                    std::string del_friend_id = *memberInfo->mFriendid.get();
                    memberInfo->UnLock();
                    mDatabaseProxy->removeMember(del_friend_id);
                    int ela_ret = ela_remove_friend(mCarrier.get(),
                                                    memberInfo->mFriendid.get()->c_str());
                    sprintf(msg_str, "%s has been kicked out!",
                            memberInfo->mNickName.get()->c_str());
                    if (ela_ret != 0) {
                        Log::I(Log::TAG,
                               "delCmd can't delete this user: %s errno:(0x%x)",
                               memberInfo->mFriendid.get()->c_str(), ela_get_error());
                    }
                } else {
                    sprintf(msg_str, "num %s member not exist!", del_userindex.c_str());
                }

                ela_send_friend_message(mCarrier.get(), friend_id.c_str(),
                                        msg_str, strlen(msg_str));

                json msg;
                msg["cmd"] = Command_UpdateMemberCount;
                msg["friendid"] = mAddress.c_str();
                msg["membercount"] = getFriendList()->size();
                sendMsgForManager(msg.dump());
            }
        }
    }

    void CarrierRobot::registerCarrierCallBack(const std::function<void (const std::string)> &carrierCallBack) {
        mCarrierCallBack = carrierCallBack;
    }

    void  CarrierRobot::sendMsgForManager(std::string msg) {
        if (mCarrierCallBack != nullptr) {
            mCarrierCallBack(msg);
        }
    }
    void CarrierRobot::showAddressCmd(const std::vector<std::string> &args) {
        if (args.size() >= 2) {
            std::string address;
            this->getAddress(address);

            const std::string friend_id = args[1];
            int ela_ret = ela_send_friend_message(mCarrier.get(), friend_id.c_str(),
                                                  address.c_str(), strlen(address.c_str()));
            if (ela_ret != 0) {
                Log::I(Log::TAG,
                       "showAddressCmd .c_str(): %s errno:(0x%x)",
                       address.c_str(), ela_get_error());
            }
        }

    }

}