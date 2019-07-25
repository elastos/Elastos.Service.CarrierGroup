//
// Created by luocf on 2019/7/20.
//

#include "src/manager.h"

extern "C" {
manager mManager;
void start(char *ip, int port, char *data_root_dir) {
    mManager.start(ip, port, data_root_dir);
}

void createGroup() {
    int num = 0;
    /*while(1) {
        std::chrono::milliseconds dur(5000);
        std::this_thread::sleep_for(dur);
        if (num++ < 5) {
            mManager.createGroup();
        }
    }*/
}
}
