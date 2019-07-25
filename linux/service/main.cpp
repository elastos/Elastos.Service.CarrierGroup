#include <iostream>
#include <thread>
#include "carrierService.h"

int main(int argc,char *argv[]) {
    std::cout << "I'm child !" << std::endl;
    if (argc < 5) {
        return -1;
    }

    char* ip = argv[1];
    char* port = argv[2];
    char* data_dir = argv[3];
    char* service_id = argv[4];
    std::cout << "I'm child !:"<<ip<<","<<port<<","<<data_dir<<","<<service_id << std::endl;
    std::shared_ptr<carrierService> service = std::make_shared<carrierService>();
    service->start(ip, atoi(port), data_dir, atoi(service_id));
    while(1) {
        std::chrono::milliseconds dur(1000);
        std::this_thread::sleep_for(dur);
    }
    return 0;
}