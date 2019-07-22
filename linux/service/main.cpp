#include <iostream>
#include <thread>
#include "testService.h"

int main(int argc,char *argv[]) {
    std::cout << "Hello, World I'm child !" << std::endl;
    if (argc < 4) {
        return -1;
    }

    char* ip = argv[1];
    char* port = argv[2];
    char* data_dir = argv[3];
    std::shared_ptr<testService> service = std::make_shared<testService>();
    service->start(ip, atoi(port), data_dir);

    while(1) {
        std::chrono::milliseconds dur(1000);
        std::this_thread::sleep_for(dur);
    }
    return 0;
}