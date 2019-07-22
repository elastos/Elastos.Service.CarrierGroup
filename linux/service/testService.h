//
// Created by luocf on 2019/7/19.
//

#ifndef TESTSERVICE_TESTSERVICE_H
#define TESTSERVICE_TESTSERVICE_H
#include <string>
#include <iostream>
class testService {
public:
    testService();
    ~testService();
    void start(std::string ip, int port, std::string data_root_dir);
    void runCarrier();
    void runCommunicationThread();
private:
    std::thread mCommunicationThread;
    std::string mRootDir;
    std::string mIp;
    int mPort;
};


#endif //TESTSERVICE_TESTSERVICE_H
