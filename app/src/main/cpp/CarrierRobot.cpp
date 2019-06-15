//
// Created by luocf on 2019/6/13.
//
#include <cstring>
#include <future>
#include <ela_carrier.h>
#include <ela_session.h>
#include <Tools/Log.hpp>
#include "CarrierRobot.h"
#include "ErrCode.h"

namespace chatrobot {

    CarrierRobot *CarrierRobot::instance = new CarrierRobot();

    CarrierRobot *CarrierRobot::getInstance() {
        return instance;
    }

    CarrierRobot::~CarrierRobot() {
        mCarrierConfig.reset();
    }

    CarrierRobot::CarrierRobot() {
        mCarrierConfig = std::make_shared<CarrierConfig>();
    }

    int CarrierRobot::GetCarrierUsrIdByAddress(const std::string& address, std::string& usrId)
    {
        char buf[ELA_MAX_ID_LEN + 1] = {0};
        auto ret = ela_get_id_by_address(address.c_str(), buf, sizeof(buf));
        if(ret == nullptr) {
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
        if(ret < 0) {
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

        /*channel->mChannelStatus = ( status == ElaConnectionStatus_Connected
                                   ? ChannelListener::ChannelStatus::Online
                                   : ChannelListener::ChannelStatus::Offline);
       Log::I(Log::TAG, "OnCarrierConnection status: %d", channel->mChannelStatus);
       if(channel->mChannelListener.get() != nullptr) {
           channel->mChannelListener->onStatusChanged(carrierUsrId, channel->mChannelType, channel->mChannelStatus);
       }*/
    }

    void CarrierRobot::OnCarrierFriendRequest(ElaCarrier *carrier, const char *friendid,
                                              const ElaUserInfo *info,
                                              const char *hello, void *context) {
        Log::I(Log::TAG, "OnCarrierFriendRequest from: %s", friendid);
        auto channel = reinterpret_cast<CarrierRobot *>(context);
        ela_accept_friend(carrier, friendid);

        /*if(channel->mChannelListener.get() != nullptr) {
            channel->mChannelListener->onFriendRequest(friendid, channel->mChannelType, hello);
        }*/
    }

    void CarrierRobot::OnCarrierFriendConnection(ElaCarrier *carrier, const char *friendid,
                                                 ElaConnectionStatus status, void *context) {
        Log::I(Log::TAG, "OnCarrierFriendConnection from: %s %d", friendid, status);
        auto channel = reinterpret_cast<CarrierRobot *>(context);
        /*
                if(channel->mChannelListener.get() != nullptr) {
                    auto chStatus = ( status == ElaConnectionStatus_Connected
                                      ? ChannelListener::ChannelStatus::Online
                                      : ChannelListener::ChannelStatus::Offline);
                    channel->mChannelListener->onFriendStatusChanged(friendid, channel->mChannelType, chStatus);
                }*/
    }

    void CarrierRobot::OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                              const void *msg, size_t len, void *context) {
        Log::I(Log::TAG, "OnCarrierFriendMessage from: %s len=%d", from, len);

        auto channel = reinterpret_cast<CarrierRobot *>(context);
        auto data = reinterpret_cast<const uint8_t *>(msg);
    /*
        int32_t dataOffset = 0;
        bool dataComplete = true;
        bool isPkgData = true;
        for(auto idx = 0; idx < PkgMagicHeadSize; idx++) {
            if(data[idx] != PkgMagic[idx]) {
                isPkgData = false;
                break;
            }
        }
        if(isPkgData == true) {
            dataOffset = PkgMagicSize;
            dataComplete = (data[PkgMagicDataIdx] == data[PkgMagicDataCnt] - 1 ? true : false);
            Log::I(Log::TAG, "OnCarrierFriendMessage PkgMagicData Idx/Cnt=%d/%d", data[PkgMagicDataIdx], data[PkgMagicDataCnt]);
        }

        auto& dataCache = channel->mRecvDataCache[from];
        dataCache.insert(dataCache.end(), data + dataOffset, data + len);

        if(dataComplete == true) {
            channel->mChannelListener->onReceivedMessage(from, channel->mChannelType, dataCache);
            dataCache.clear();
        }*/
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

    int CarrierRobot::getUserId(std::string &userid){
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
}