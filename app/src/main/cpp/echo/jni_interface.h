//
// Created by Administrator on 2020/7/1.
//

#ifndef JNI_INTERFACE_H
#define JNI_INTERFACE_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jboolean JNICALL
Java_com_example_echo_EchoActivity_configureEcho(JNIEnv *env, jobject thiz, jint delay_in_ms,
                                                 jfloat decay);

JNIEXPORT void JNICALL
Java_com_example_echo_EchoActivity_createSLEngine(JNIEnv *env, jobject thiz, jint rate,
                                                  jint frames_per_buf, jlong delay_in_ms,
                                                  jfloat decay) ;
JNIEXPORT jboolean JNICALL
Java_com_example_echo_EchoActivity_createSLBufferQueueAudioPlayer(JNIEnv *env, jobject thiz);


JNIEXPORT jboolean JNICALL
Java_com_example_echo_EchoActivity_createAudioRecorder(JNIEnv *env, jobject thiz);

JNIEXPORT void JNICALL
Java_com_example_echo_EchoActivity_deleteAudioRecorder(JNIEnv *env, jobject thiz);

JNIEXPORT void JNICALL
Java_com_example_echo_EchoActivity_startPlay(JNIEnv *env, jobject thiz);

JNIEXPORT void JNICALL
Java_com_example_echo_EchoActivity_stopPlay(JNIEnv *env, jobject thiz);

JNIEXPORT void JNICALL
Java_com_example_echo_EchoActivity_deleteSLBufferQueueAudioPlayer(JNIEnv *env, jobject thiz);
#ifdef __cplusplus
}
#endif

#endif // JNI_INTERFACE_H


