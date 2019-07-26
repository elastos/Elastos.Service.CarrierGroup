//
// Created by luocf on 2019/7/20.
//

#include "src/manager.h"
#include "../common/json.hpp"
#include <Python.h>
extern "C" {
manager mManager;
using json = nlohmann::json;
void start(char *ip, int port, char *data_root_dir) {
    mManager.start(ip, port, data_root_dir);
}

void createGroup() {
    int num = 0;
    mManager.createGroup();
}

PyObject* list() {
    printf("list in\n");
    std::shared_ptr<std::vector<std::shared_ptr<GroupInfo>>> group_list = mManager.getGroupList();
    json result_json;
    result_json["code"] = 0;
    result_json["data"] = json::parse("[]");
    for(int i=0; i<group_list->size(); i++) {
        std::shared_ptr<GroupInfo> groupinfo = group_list->at(i);
        groupinfo->Lock();
        int service_id = groupinfo->getServiceId();
        std::string address = groupinfo->getAddress();
        std::string data_dir = groupinfo->getDataDir();
        std::string nickname = groupinfo->getNickName();
        int member_count = groupinfo->getMemberCount();
        json item_json;
        item_json["id"] = std::to_string(service_id);
        item_json["address"] = address;
        item_json["members"] = std::to_string(member_count);
        item_json["nickname"] = nickname;
        result_json["data"].push_back(item_json);
        groupinfo->UnLock();
    }
    return  (PyObject*)Py_BuildValue("s","helloworld");
}
}
