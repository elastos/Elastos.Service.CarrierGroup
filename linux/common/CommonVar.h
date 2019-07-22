//
// Created by luocf on 2019/7/17.
//

#ifndef CHATROBOT_COMMON_VAR_H
#define CHATROBOT_COMMON_VAR_H

class CommonVar {
public:
    static constexpr int CONST_SERVICE_LIMIT_NUM = 12;
    static constexpr int Command_Ready = 0;
    static constexpr int CreateGroup = 1;
    static constexpr int Command_UpdateAddress = 2;
    static constexpr int Command_UpdateMemberCount = 3;
    static constexpr int Command_UpdateNickName = 4;
    static constexpr int Command_UpdateStatus = 5;
    static constexpr int Command_WatchDog = 6;
    static constexpr int CONST_ERROR_CODE_MAX_NUM = -1;
};

#endif //CHATROBOT_COMMON_VAR_H
