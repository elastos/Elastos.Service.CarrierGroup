#include <iostream>
#include "manager.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    std::shared_ptr<manager> m = std::make_shared<manager>();
    m->start("127.0.0.1", 2222, "/Users/luocf/workspace/ChatRobotLinux/testManager/");
    //m->createGroup();
    int num = 0;
    while(1) {
        std::chrono::milliseconds dur(5000);
        std::this_thread::sleep_for(dur);
        if (num++ < 5) {
            m->createGroup();
        }
    }
    return 0;
}