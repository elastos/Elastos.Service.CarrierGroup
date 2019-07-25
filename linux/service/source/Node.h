//
// Created by luocf on 2019/6/13.
//

#ifndef CHATROBOT_NODE_H
#define CHATROBOT_NODE_H


#include <memory>
namespace chatrobot {
    class Node {
    public:
        Node(std::shared_ptr<std::string> ipv4,
             std::shared_ptr<std::string> port,
             std::shared_ptr<std::string> public_key);
        std::shared_ptr<std::string> mIpv4;
        std::shared_ptr<std::string> mPort;
        std::shared_ptr<std::string> mPublicKey;
    };
}

#endif //CHATROBOT_NODE_H
