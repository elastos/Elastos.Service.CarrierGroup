//
// Created by luocf on 2019/6/13.
//

#ifndef CHATROBOT_CARRIERROBOT_H
#define CHATROBOT_CARRIERROBOT_H

#include <stdlib.h>
#include <functional>
#include <memory> // std::unique_ptr
#include <ctime>
#include <thread>
#include <regex>
#include <ela_carrier.h>
#include <ela_session.h>

#include "DataBase/DatabaseProxy.h"
#include <CarrierConfig.h>

namespace chatrobot {
    static const char *TAG = "CarrierRobot";

    class CarrierRobot:std::enable_shared_from_this<CarrierRobot> {
    public:
        class Factory {
        public:
            static int SetLocalDataDir(const std::string& dir);
            static std::shared_ptr<CarrierRobot> Create();

        private:
            static std::string sLocalDataDir;
            friend class CarrierRobot;
        };
        ~CarrierRobot();
        int start(const char* data_dir);
        void stop();
        int getAddress(std::string& address);
        int getUserId(std::string& userid);
        static void OnCarrierConnection(ElaCarrier *carrier,
                                               ElaConnectionStatus status, void *context);
        static void OnCarrierFriendRequest(ElaCarrier *carrier, const char *friendid,
                                                  const ElaUserInfo *info,
                                                  const char *hello, void *context);
        static void OnCarrierFriendInfoChanged(ElaCarrier *carrier, const char *friendid,
                                               const ElaFriendInfo *info, void *context);
        static void OnCarrierFriendConnection(ElaCarrier *carrier,const char *friendid,
                                                     ElaConnectionStatus status, void *context);

        static void OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                                  const void *msg, size_t len, void *context);

        static int GetCarrierUsrIdByAddress(const std::string& address, std::string& usrId);
        void runCarrierInner();
        void runCarrier();

        void updateMemberInfo(std::shared_ptr<std::string> friendid, std::shared_ptr<std::string> nickname,
                              ElaConnectionStatus status);
        void addMessgae(std::shared_ptr<std::string> friend_id, std::shared_ptr<std::string> message, std::time_t send_time);
        std::shared_ptr<std::vector<std::shared_ptr<MemberInfo>>>getFriendList();
        void helpCmd(const std::vector<std::string> &args, const std::string& message);
        void listCmd(const std::vector<std::string> &args);
        void delCmd(const std::vector<std::string> &args);
    private:
        static CarrierRobot* instance;

        explicit CarrierRobot();
        std::time_t getTimeStamp();
        std::string convertDatetimeToString(std::time_t time);
        bool relayMessages();
        std::shared_ptr<std::regex> mMsgReg;
        std::mutex _mReplyMessage;
        std::thread mCarrieryThread;
        bool mQuit;
        std::shared_ptr<std::string> mCreaterFriendId;
        std::unique_ptr<ElaCarrier, std::function<void(ElaCarrier*)>> mCarrier;
        std::shared_ptr<CarrierConfig> mCarrierConfig;
        std::shared_ptr<DatabaseProxy> mDatabaseProxy;
    };
}

#endif //CHATROBOT_CARRIERROBOT_H
