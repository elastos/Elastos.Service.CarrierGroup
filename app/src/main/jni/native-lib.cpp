#include <jni.h>
#include <string>

#include "DatabaseProxy.h"
#include "CarrierRobot.h"
#include "Tools/JniUtils.hpp"
extern "C" {
    JNIEXPORT jstring
    JNICALL
    Java_com_qcode_chatrobot_MainActivity_stringFromJNI(
            JNIEnv *env,
            jobject /* this */) {
        std::shared_ptr<chatrobot::DababaseProxy> db = std::make_shared<chatrobot::DababaseProxy>();
        std::string hello = db->getString();
        return env->NewStringUTF(hello.c_str());
    };

    JNIEXPORT jstring
    JNICALL
    Java_com_qcode_chatrobot_MainActivity_getAddress(
            JNIEnv *env,
            jobject /* this */) {
        std::shared_ptr<chatrobot::DababaseProxy> db = std::make_shared<chatrobot::DababaseProxy>();
        std::string hello = db->getString();
        chatrobot::CarrierRobot* carrier_robot = chatrobot::CarrierRobot::getInstance();
        std::string address;
        int status = carrier_robot->getAddress(address);

        return env->NewStringUTF(address.c_str());

    };
    JNIEXPORT jint
    JNICALL
    Java_com_qcode_chatrobot_MainActivity_startChatRobot(
            JNIEnv *env,
            jobject /* this */,
            jstring jdata_dir) {
        chatrobot::CarrierRobot* carrier_robot = chatrobot::CarrierRobot::getInstance();
        std::shared_ptr<const char> data_dir = JniUtils::GetStringSafely(env, jdata_dir);
        return carrier_robot->start(data_dir.get());
    }

}
