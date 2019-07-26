#include <iostream>
#include <string>
#include "src/manager.h"
//引入字符串
using namespace std;
int main(int argc, char** argv) {
    if (argc != 4) {
        cout<<"params error, need ip port data_dir"<<endl;
        return -1;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);
    char* data_dir = argv[3];

    std::shared_ptr<manager> m = std::make_shared<manager>();
    m->start(ip, port, data_dir);
    bool quite = false;

    while(!quite) {
        printf("please input command, help can show all command!\n");
        char cmd[100];
        scanf("%s",cmd);
        printf("input command:%s\n", (char*)cmd);
        if (strcmp((char*)cmd, "help") == 0) {
            std::string result = "";
            result += "list: show the group  list \n";
            result += "create: create a new group \n";
            result += "quite: exit\n";
            printf("help:\n%s\n",result.c_str());
        } else if (strcmp((char*)cmd, "list") == 0) {
            std::shared_ptr<std::vector<std::shared_ptr<GroupInfo>>> group_list = m->getGroupList();
            std::string result = "";
            for(int i=0; i<group_list->size(); i++) {
                std::shared_ptr<GroupInfo> groupinfo = group_list->at(i);
                groupinfo->Lock();
                int service_id = groupinfo->getServiceId();
                std::string address = groupinfo->getAddress();
                std::string data_dir = groupinfo->getDataDir();
                std::string nickname = groupinfo->getNickName();
                int member_count = groupinfo->getMemberCount();
                result += "id:"+std::to_string(service_id) +"   ";
                result += "address:"+address +"   ";
                result += "members count:"+std::to_string(member_count) +"   ";
                result += "nickname:"+nickname +"   \n";
                groupinfo->UnLock();
            }
            printf("group list:\n%s\n",result.c_str());
        } else if (strcmp((char*)cmd, "create") == 0) {
            m->createGroup();

        } else if (strcmp((char*)cmd, "quite") == 0) {
            quite = true;
            break;
        }
    }
    return 0;
}