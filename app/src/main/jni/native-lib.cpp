#include <jni.h>
#include <string>

#include "DataBase/DatabaseProxy.h"
#include "CarrierRobot.h"
#include "Tools/JniUtils.hpp"

extern "C" {

JNIEXPORT jstring JNICALL
Java_com_qcode_chatrobot_MainActivity_getAddress(
        JNIEnv *env,
        jobject /* this */) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    std::string address;
    int status = carrier_robot->getAddress(address);

    return env->NewStringUTF(address.c_str());

} ;
JNIEXPORT jstring JNICALL
Java_com_qcode_chatrobot_MainActivity_getUserId(
        JNIEnv *env,
        jobject /* this */) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    std::string user_id;
    int status = carrier_robot->getUserId(user_id);

    return env->NewStringUTF(user_id.c_str());

} ;
JNIEXPORT jint JNICALL
Java_com_qcode_chatrobot_MainActivity_startChatRobot(
        JNIEnv *env,
        jobject /* this */,
        jstring jdata_dir) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    std::shared_ptr<const char> data_dir = JniUtils::GetStringSafely(env, jdata_dir);
    return carrier_robot->start(data_dir.get());
}
JNIEXPORT jint JNICALL
Java_com_qcode_chatrobot_MainActivity_runChatRobot(
        JNIEnv *env,
        jobject /* this */) {
    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    carrier_robot->runCarrier();
    return 0;
}

JNIEXPORT jobjectArray JNICALL
Java_com_qcode_chatrobot_MainActivity_getMemberList(JNIEnv *env, jobject jobj) {
    jobjectArray infos = NULL;    // jobjectArray 为指针类型
    jclass clsMemberInfo = NULL;        // jclass 为指针类型
    jobject obj;
    jfieldID mNickName;
    jfieldID mFriendId;
    jfieldID mStatus;
    jmethodID consID;
    int i;

    clsMemberInfo = env->FindClass("com/qcode/chatrobot/ui/MemberInfo");

    chatrobot::CarrierRobot *carrier_robot = chatrobot::CarrierRobot::getInstance();
    std::shared_ptr<std::vector<std::shared_ptr<chatrobot::MemberInfo>>> memberlist = carrier_robot->getFriendList();
    std::vector<std::shared_ptr<chatrobot::MemberInfo>> memberlist_vector = *memberlist.get();
    int length = memberlist_vector.size();
    infos = env->NewObjectArray(length, clsMemberInfo, NULL);
    mNickName = env->GetFieldID(clsMemberInfo, "NickName", "Ljava/lang/String;");
    mFriendId = env->GetFieldID(clsMemberInfo, "FriendId", "Ljava/lang/String;");
    mStatus = env->GetFieldID(clsMemberInfo, "Status", "Ljava/lang/String;");
    consID = env->GetMethodID(clsMemberInfo, "<init>", "()V");

    for (i = 0; i < length; i++) {
        std::shared_ptr<chatrobot::MemberInfo> memberInfo = memberlist_vector[i];
        obj = env->NewObject(clsMemberInfo, consID);
        env->SetObjectField(obj, mNickName,
                            env->NewStringUTF(memberInfo.get()->mNickName.get()->c_str()));
        env->SetObjectField(obj, mFriendId,
                            env->NewStringUTF(memberInfo.get()->mFriendid.get()->c_str()));
        env->SetObjectField(obj, mStatus, env->NewStringUTF(
                memberInfo.get()->mStatus == ElaConnectionStatus_Connected ? "1" : "0"));
        env->SetObjectArrayElement(infos, i, obj);
    }

    return infos;
}
}

