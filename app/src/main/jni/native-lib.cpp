#include <jni.h>
#include <string>

#include "DataBase/DatabaseProxy.h"
#include "CarrierRobot.h"
#include "Tools/JniUtils.hpp"

extern "C" {
std::shared_ptr<chatrobot::CarrierRobot> mCarrierRobot;
JNIEXPORT jstring JNICALL
Java_com_qcode_chatrobot_MainActivity_getAddress(
        JNIEnv *env,
        jobject /* this */) {
    std::string address;
    mCarrierRobot->getAddress(address);
    return env->NewStringUTF(address.c_str());

} ;
JNIEXPORT jstring JNICALL
Java_com_qcode_chatrobot_MainActivity_getUserId(
        JNIEnv *env,
        jobject /* this */) {
    std::string user_id;
    mCarrierRobot->getUserId(user_id);

    return env->NewStringUTF(user_id.c_str());

};

JNIEXPORT jint JNICALL
Java_com_qcode_chatrobot_MainActivity_startChatRobot(
        JNIEnv *env,
        jobject /* this */,
        jstring jdata_dir) {


    std::shared_ptr<const char> data_dir = JniUtils::GetStringSafely(env, jdata_dir);
    mCarrierRobot = chatrobot::CarrierRobot::Factory::Create();

    return mCarrierRobot->start(data_dir.get());
}
JNIEXPORT jint JNICALL
Java_com_qcode_chatrobot_MainActivity_runChatRobot(
        JNIEnv *env,
        jobject /* this */) {

    mCarrierRobot->runCarrier();
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
    std::shared_ptr<std::vector<std::shared_ptr<chatrobot::MemberInfo>>> memberlist = mCarrierRobot->getFriendList();
    std::vector<std::shared_ptr<chatrobot::MemberInfo>> memberlist_vector = *memberlist.get();
    int length = memberlist_vector.size();
    infos = env->NewObjectArray(length, clsMemberInfo, NULL);
    mNickName = env->GetFieldID(clsMemberInfo, "NickName", "Ljava/lang/String;");
    mFriendId = env->GetFieldID(clsMemberInfo, "FriendId", "Ljava/lang/String;");
    mStatus = env->GetFieldID(clsMemberInfo, "Status", "Ljava/lang/String;");
    consID = env->GetMethodID(clsMemberInfo, "<init>", "()V");

    for (i = 0; i < length; i++) {
        std::shared_ptr<chatrobot::MemberInfo> memberInfo = memberlist_vector[i];
        if (memberInfo.get() != nullptr) {
            memberInfo->Lock();
            obj = env->NewObject(clsMemberInfo, consID);
            env->SetObjectField(obj, mNickName,
                                env->NewStringUTF(memberInfo.get()->mNickName.get()->c_str()));
            env->SetObjectField(obj, mFriendId,
                                env->NewStringUTF(memberInfo.get()->mFriendid.get()->c_str()));
            env->SetObjectField(obj, mStatus, env->NewStringUTF(
                    memberInfo.get()->mStatus == 0 ? "online" : "offline"));
            env->SetObjectArrayElement(infos, i, obj);
            memberInfo->UnLock();
        }
    }

    return infos;
}
}

