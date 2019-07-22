//
// Created by luocf on 2019/7/19.
//

#include <arpa/inet.h>
#include<unistd.h>
#include<sys/types.h>
#include <thread>
#include <cstring>
#include <iostream>
#include "testService.h"

#include "common/json.hpp"

using json = nlohmann::json;
testService::testService() {

}
testService::~testService() {

}
void testService::runCommunicationThread() {
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if (sockfd == -1)
    {
        perror("socket");exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(mPort);//连接端口
    addr.sin_addr.s_addr = inet_addr(mIp.c_str());//都是服务器的，改成连接IP
    int res = connect(sockfd,(struct sockaddr*)&addr,sizeof(addr));
    if (res == -1)
    {
        perror("connect"),exit(-1);
    }
    printf("连接成功\n");
    char buf[512] = {};
    while (1)
    {
        memset(buf,0,sizeof(buf));//buf清0
        printf("请输入要说的话\n");
        //scanf("%s",buf);
        sprintf(buf, "%s", "{\"cmd\":3,\"id\":2323,\"friendid\":\"abcdef123\"}");
        json json_result;
        json_result["cmd"] = 3;
        json_result["id"] = 23234;
        json_result["friendid"] = "abcdef123";
        std::string result = json_result.dump();
        write(sockfd,result.c_str(),strlen(result.c_str()));
        if (strcmp(buf,"bye") == 0) //退出的合适位置
        {
            break;
        }
        sleep(1);
        memset(buf,0,sizeof(buf));//buf清0
        res = read(sockfd,buf,sizeof(buf));
        printf("读到了%d字节，内容：%s\n",res,buf);
    }

    close(sockfd);
}
void testService::start(std::string ip, int port, std::string data_root_dir) {
    mRootDir = data_root_dir;
    mIp = ip;
    mPort = port;
    //启动线程接收消息
    mCommunicationThread = std::thread(&testService::runCommunicationThread, this); //引用
}

void testService::runCarrier() {

}