//
// Created by luocf on 2019/7/19.
//
#include <stdlib.h>
#include <arpa/inet.h>
#include <iostream>
#include<unistd.h>
#include<sys/types.h>
#include "manager.h"
#include "../../common/json.hpp"
#include "../../common/FileUtils.hpp"
#include "../../common/CommonVar.h"
#include "../../common/Log.hpp"

manager::manager() {
    mDataBaseProxy = std::make_shared<DatabaseProxy>();
}

manager::~manager() {

}

using json = nlohmann::json;
constexpr int MAX_QUEUE_SIZE = 10;

void manager::start(std::string ip, int port, std::string data_root_dir) {
    mRootDir = data_root_dir;
    mIp = ip;
    mPort = port;

    mServiceId = 0;
    mIsReady = false;
    mDataBaseProxy->startDb(mRootDir.c_str());
    printf("manager::start in\n");
    //启动线程接收消息
    mCommunicationThread = std::thread(&manager::runCommunicationThread, this); //引用
    mWorkThread = std::thread(&manager::runWorkThread, this); //引用
    printf("manager::start out\n");
}
std::shared_ptr<std::vector<std::shared_ptr<GroupInfo>>> manager::getGroupList() {
    return mDataBaseProxy->getGroupList();
}
void manager::_recoveryGroupInfo() {
    std::shared_ptr<std::vector<std::shared_ptr<GroupInfo>>> group_list = mDataBaseProxy->getGroupList();
    int service_id_tmp = 0;
    for(int i=0; i<group_list->size(); i++) {
        std::shared_ptr<GroupInfo> groupinfo = group_list->at(i);
        int service_id = groupinfo->getServiceId();
        std::string data_dir = groupinfo->getDataDir();
        std::async(&manager::_bindService, this, service_id, data_dir);
        if (service_id_tmp < service_id) {
            service_id_tmp = service_id;
        }
    }
    mServiceId = service_id_tmp;

}
void manager::stop() {
    mDataBaseProxy->closeDb();
}

void manager::_removeGroup(std::string address) {
    std::cout << address.c_str() << std::endl;
    mDataBaseProxy->removeGroup(address);
}

void manager::_updateGroupNickName(std::string friendid, std::string nick_name) {
    std::cout << nick_name.c_str() << std::endl;
    mDataBaseProxy->updateGroupNickName(friendid, nick_name);
}

void manager::_updateGroupAddress(int service_id, std::string address) {
    std::cout << address.c_str() << std::endl;
    mDataBaseProxy->updateGroupAddress(service_id, address);
}

void manager::_updateGroupMemberCount(std::string friendid, int member_count) {
    std::cout << std::to_string(member_count).c_str() << std::endl;
    mDataBaseProxy->updateGroupMemberCount(friendid, member_count);
}

int manager::createGroup() {
    sendMsgToWorkThread("{\"cmd\":1}");
    return 0;
}

void manager::sendMsgToWorkThread(std::string msg) {
    std::unique_lock<std::mutex> lk(mQueue_lock);
    mWrite_cond.wait(lk, [this] { return mQueue.size() < MAX_QUEUE_SIZE; });
    mQueue.push(std::make_shared<std::string>(msg));
    lk.unlock();
    mQueue_cond.notify_one();
}

int manager::_createGroup() {
    int service_id = ++mServiceId;
    //创建目录data dir
    const std::string data_dir = mRootDir + std::string("/carrierService") + std::to_string(service_id);
    FileUtils::mkdirs(data_dir.c_str(), 0777);
    mDataBaseProxy->addGroup("", "", data_dir, 0, service_id);
    std::async(&manager::_bindService, this, service_id, data_dir);

    return 0;
}

void manager::runWorkThread() {
    printf("manager::runWorkThread in\n");
    while (true) {
        std::unique_lock<std::mutex> lk(mQueue_lock);
        mQueue_cond.wait(lk, [this] { return !mQueue.empty(); });
        if (mQueue.empty()) {
            return;
        }
        std::shared_ptr<std::string> result = mQueue.front();
        try {
            auto msg_json = json::parse(result->c_str());
            int cmd = msg_json["cmd"];
            switch (cmd) {
                case Command_Ready: {
                    mIsReady = true;
                    //recovery db
                    this->_recoveryGroupInfo();
                    //判断是否有临时消息
                    if (!mTmpQueue.empty()) {
                        std::shared_ptr<std::string> tmp = mTmpQueue.front();
                        while (tmp.get() != nullptr) {
                            this->_createGroup();
                            mTmpQueue.pop();
                            tmp = std::move(mTmpQueue.front());
                        }
                    }
                    break;
                }
                case CreateGroup: {
                    if (!mIsReady) {
                        //临时存储消息
                        mTmpQueue.push(std::make_shared<std::string>("create_group"));
                    } else {
                        this->_createGroup();
                    }
                    break;
                }
                case Command_UpdateAddress: {
                    int serviceid = msg_json["serviceid"];
                    std::string address = msg_json["friendid"];
                    this->_updateGroupAddress(serviceid, address);
                    break;
                }
                case Command_UpdateMemberCount: {
                    std::string friendid = msg_json["friendid"];
                    int member_count = msg_json["membercount"];
                    this->_updateGroupMemberCount(friendid, member_count);
                    break;
                }
                case Command_UpdateNickName: {
                    std::string friendid = msg_json["friendid"];
                    std::string nickname = msg_json["nickname"];
                    this->_updateGroupNickName(friendid, nickname);
                    break;
                }
                case Command_UpdateStatus: {
                    std::string friendid = msg_json["friendid"];
                    int status = msg_json["status"];
                    if (status == -1) {
                        this->_removeGroup(friendid);
                    }
                    break;
                }
                case Command_WatchDog: {
                    break;
                }
            }
        } catch (std::exception &e) {
            std::cout << "[exception caught: " << e.what() << "]\n";
        }
        mQueue.pop();
        lk.unlock();
        mWrite_cond.notify_one();
    }
}
void manager::recvServiceMsgThread(int client_fd) {
    char buf[512] = {};
    json msg;
    msg["cmd"] = Command_Ready;
    std::string result = msg.dump();
    write(client_fd, result.c_str(), strlen(result.c_str()));

    while(1) {
        memset(buf, 0, sizeof(buf));
        int res = read(client_fd, buf, sizeof(buf));
        if (res <= 0) {//包括0和-1
            return;
        }
        write(client_fd, buf, strlen(buf));
        //发送消息到工作线程
        char msg[512];
        sprintf(msg, "%s", buf);
        sendMsgToWorkThread(msg);
    }
}
void manager::runCommunicationThread() {
    printf("manager::runCommunicationThread in\n");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(-1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(mPort);
    addr.sin_addr.s_addr = inet_addr(mIp.c_str());

    int reuseaddr = 1;//解决地址已被占用问题
    //端口复用
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
               &reuseaddr, sizeof(reuseaddr));
    printf("manager::runCommunicationThread ip:%s, port:%d\n", mIp.c_str(), mPort);
    int res = bind(sockfd, (struct sockaddr *) &addr,
                   sizeof(addr));
    if (res == -1) {
        perror("bind");
        printf("manager::runCommunicationThread error ip:%s, port:%d\n", mIp.c_str(), mPort);
        exit(-1);
    }
    sendMsgToWorkThread("{\"cmd\":0}");
    pthread_t th =-1 ;
    while (1) {
        listen(sockfd, 100);//监听
        struct sockaddr_in client;
        socklen_t len = sizeof(client);

        int client_fd = accept(sockfd, (struct sockaddr *) &client,
                        &len);//阻塞函数
        if(-1 == client_fd)
        {
            perror("listen");
            continue;
        }
        //save client fd, close client when client shut down
        //TODO
        mThreadList.push_back(std::make_shared<std::thread>(std::thread(&manager::recvServiceMsgThread , this, client_fd)));
    }
    exit(0);
}

void manager::_bindService(int service_id, const std::string data_dir) {
    char ip_str[32];
    snprintf(ip_str, sizeof(ip_str), "%d", mPort);
    char service_id_str[32];
    snprintf(service_id_str, sizeof(service_id_str), "%d", service_id);
    char *nargv[] = {"CarrierService", (char *) mIp.c_str(),
                     (char *) ip_str,
                     (char *) data_dir.c_str(),
                     (char*) service_id_str,
                     (char*) 0}; //命令行参数都以0结尾
    std::cout << "_bindService:"<<nargv[0]<<","<<nargv[1]<<","<<nargv[2]<<","<<nargv[3]<<","<<nargv[4] << std::endl;

    printf("_bindService %d", service_id);
    pid_t pid;
    pid = fork();
    switch (pid) {
        case 0:
            execv("carrierService", nargv);      //指定环境变量，原来的环境变量不起作用
            perror("exec");
            exit(1);
        case -1:
            perror("fork");
            exit(1);
        default:
            printf("exec is completed\n");
            break;

    }
}