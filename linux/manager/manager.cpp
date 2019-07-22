//
// Created by luocf on 2019/7/19.
//

#include <arpa/inet.h>
#include <iostream>
#include<unistd.h>
#include<sys/types.h>
#include "manager.h"
#include "common/json.hpp"
#include "common/FileUtils.hpp"
#include "common/CommonVar.h"
#include "common/Log.hpp"

manager::manager() {
}

manager::~manager() {

}

using json = nlohmann::json;
constexpr int MAX_QUEUE_SIZE = 10;

void manager::start(std::string ip, int port, std::string data_root_dir) {
    mRootDir = data_root_dir;
    mIp = ip;
    mPort = port;
    mIsReady = false;
    //启动线程接收消息
    mCommunicationThread = std::thread(&manager::runCommunicationThread, this); //引用
    mWorkThread = std::thread(&manager::runWorkThread, this); //引用
}

void manager::stop() {

}

void manager::_removeGroup(std::string address) {
    std::cout << address.c_str() << std::endl;
}

void manager::_updateGroupNickName(int id, std::string nick_name) {
    std::cout << nick_name.c_str() << std::endl;
}

void manager::_updateGroupAddress(int id, std::string address) {
    std::cout << address.c_str() << std::endl;
    sendMsgToWorkThread("{\"cmd\":6}");
}

void manager::_updateGroupMemberCount(int id, int member_count) {
    std::cout << std::to_string(member_count).c_str() << std::endl;
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
int ServiceId = 0;
int manager::_createGroup() {
    int service_id = ServiceId++;
    //创建目录data dir
    const std::string data_dir = mRootDir + std::string("./carrierService") + std::to_string(service_id);
    FileUtils::mkdirs(data_dir.c_str(), 0777);
    std::async(&manager::_bindService, this, service_id, data_dir);
    //this->_bindService(service_id, data_dir);
    return 0;
}

void manager::runWorkThread() {
    while (true) {
        std::unique_lock<std::mutex> lk(mQueue_lock);
        Log::V("manager", "runWorkThread wait");
        mQueue_cond.wait(lk, [this] { return !mQueue.empty(); });
        if (mQueue.empty()) {
            return;
        }
        std::shared_ptr<std::string> result = mQueue.front();
        Log::V("manager", "runWorkThread msg:%s", result->c_str());
        try {
            auto msg_json = json::parse(result->c_str());
            int cmd = msg_json["cmd"];
            switch (cmd) {
                case CommonVar::Command_Ready: {
                    mIsReady = true;
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
                case CommonVar::CreateGroup: {
                    if (!mIsReady) {
                        //临时存储消息
                        mTmpQueue.push(std::make_shared<std::string>("create_group"));
                    } else {
                        this->_createGroup();
                    }
                    break;
                }
                case CommonVar::Command_UpdateAddress: {
                    int id = msg_json["id"];
                    std::string friendido = msg_json["friendid"];
                    this->_updateGroupAddress(id, friendid);
                    break;
                }
                case CommonVar::Command_UpdateMemberCount: {
                    int id = msg_json["id"];
                    int member_count = msg_json["memberCount"];
                    this->_updateGroupMemberCount(id, member_count);
                    break;
                }
                case CommonVar::Command_UpdateNickName: {
                    int id = msg_json["id"];
                    std::string nickname = msg_json["nickName"];
                    this->_updateGroupNickName(id, nickname);
                    break;
                }
                case CommonVar::Command_UpdateStatus: {
                    int status = msg_json["status"];
                    if (status == -1) {
                        std::string friendid = msg_json["friendid"];
                        this->_removeGroup(friendid);
                    }
                    break;
                }
                case CommonVar::Command_WatchDog: {
                    //int id = msg_json["id"];
                    break;
                }
            }
        } catch (std::exception &e) {
            std::cout << "[exception caught: " << e.what() << "]\n";
        }
        Log::V("manager", "runWorkThread mQueue.pop");
        mQueue.pop();
        lk.unlock();
        Log::V("manager", "runWorkThread lk.unlock");
        mWrite_cond.notify_one();
        Log::V("manager", "runWorkThread mWrite_cond.notify_one");
    }
}

void manager::runCommunicationThread() {
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

    int res = bind(sockfd, (struct sockaddr *) &addr,
                   sizeof(addr));
    if (res == -1) {
        perror("bind");
        exit(-1);
    }
    printf("bind ok\n");
    listen(sockfd, 100);//监听
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    sendMsgToWorkThread("{\"cmd\":0}");
    int fd = accept(sockfd, (struct sockaddr *) &client,
                    &len);//阻塞函数
    char *from = inet_ntoa(client.sin_addr);
    printf("%s连接成功\n", from);

    char buf[512] = {};
    while (1) {
        res = read(fd, buf, sizeof(buf));
        printf("接受了%d字节，内容:%s\n", res, buf);
        if (res <= 0) {//包括0和-1
            break;
        }
        if (strcmp(buf, "bye") == 0) {
            break;
        }
        write(fd, buf, strlen(buf));
        //发送消息到工作线程
        char msg[512];
        sprintf(msg, "%s", buf);
        sendMsgToWorkThread(msg);

        memset(buf, 0, sizeof(buf));
    }
    close(fd);
    exit(0);
}

void manager::_bindService(int service_id, const std::string data_dir) {
    char *nargv[] = {"CarrierService", (char *) mIp.data(),
                     (char *) std::to_string(mPort).data(),
                     (char *) data_dir.data(), (char *) 0}; //命令行参数都以0结尾
    Log::V("manager", "_bindService %d", service_id);
    pid_t pid;
    pid = fork();
    switch (pid) {
        case 0:
            Log::V("manager", "_bindService child process");
            execv("../testService/testService", nargv);      //指定环境变量，原来的环境变量不起作用
            perror("exec");
            exit(1);
        case -1:
            perror("fork");
            exit(1);
        default:
            Log::V("manager", "_bindService parent process");
            printf("exec is completed\n");
            break;

    }
}