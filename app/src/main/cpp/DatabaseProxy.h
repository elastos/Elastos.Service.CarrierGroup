//
// Created by luocf on 2019/6/12.
//

#ifndef CHATROBOT_DB_PROXY_H
#define CHATROBOT_DB_PROXY_H

#include <string>
namespace chatrobot {
    class DababaseProxy {
    public:
        DababaseProxy();
        virtual ~DababaseProxy();
        std::string getString();
    };

}
#endif //CHATROBOT_DB_PROXY_H
