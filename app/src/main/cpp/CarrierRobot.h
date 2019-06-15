//
// Created by luocf on 2019/6/13.
//

#ifndef CHATROBOT_CARRIERROBOT_H
#define CHATROBOT_CARRIERROBOT_H

#include <stdlib.h>
#include <functional>
#include <memory> // std::unique_ptr
#include <ela_carrier.h>
#include <ela_session.h>
#include <CarrierConfig.h>

namespace chatrobot {
    static const char *TAG = "CarrierRobot";
    class CarrierRobot:std::enable_shared_from_this<CarrierRobot> {
    public:
        static chatrobot::CarrierRobot* getInstance();

        ~CarrierRobot();
        int start(const char* data_dir);
        int getAddress(std::string& address);
        int getUserId(std::string& userid);
        static void OnCarrierConnection(ElaCarrier *carrier,
                                               ElaConnectionStatus status, void *context);
        static void OnCarrierFriendRequest(ElaCarrier *carrier, const char *friendid,
                                                  const ElaUserInfo *info,
                                                  const char *hello, void *context);

        static void OnCarrierFriendConnection(ElaCarrier *carrier,const char *friendid,
                                                     ElaConnectionStatus status, void *context);

        static void OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                                  const void *msg, size_t len, void *context);
        static int GetCarrierUsrIdByAddress(const std::string& address, std::string& usrId);
        void runCarrier();
    private:
        static CarrierRobot* instance;
        CarrierRobot();

        std::unique_ptr<ElaCarrier, std::function<void(ElaCarrier*)>> mCarrier;
        std::shared_ptr<CarrierConfig> mCarrierConfig;
    };
}

#endif //CHATROBOT_CARRIERROBOT_H
