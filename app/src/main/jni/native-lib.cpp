#include <jni.h>
#include <string>

#include "DataBase/DatabaseProxy.h"
#include "CarrierRobot.h"
#include "Tools/JniUtils.hpp"

extern "C" {

JNIEXPORT jstring
JNICALL
Java_com_qcode_chatrobot_MainActivity_getAddress(
        JNIEnv *env,
        jobject /* this */) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    std::string address;
    int status = carrier_robot->getAddress(address);

    return env->NewStringUTF(address.c_str());

} ;
JNIEXPORT jstring
JNICALL
Java_com_qcode_chatrobot_MainActivity_getUserId(
        JNIEnv *env,
        jobject /* this */) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    std::string user_id;
    int status = carrier_robot->getUserId(user_id);

    return env->NewStringUTF(user_id.c_str());

} ;
JNIEXPORT jint
JNICALL
Java_com_qcode_chatrobot_MainActivity_startChatRobot(
        JNIEnv *env,
        jobject /* this */,
        jstring jdata_dir) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    std::shared_ptr<const char> data_dir = JniUtils::GetStringSafely(env, jdata_dir);
    return carrier_robot->start(data_dir.get());
}
JNIEXPORT jint
JNICALL
Java_com_qcode_chatrobot_MainActivity_runChatRobot(
        JNIEnv *env,
        jobject /* this */) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    carrier_robot->runCarrier();
    return 0;
}
}
