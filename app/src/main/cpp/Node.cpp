//
// Created by luocf on 2019/6/13.
//

#include "Node.h"
namespace chatrobot {
    Node::Node(std::shared_ptr<std::string> ipv4,
               std::shared_ptr<std::string> port,
               std::shared_ptr<std::string> public_key) {
        mIpv4 = ipv4;
        mPort = port;
        mPublicKey = public_key;
    }
}