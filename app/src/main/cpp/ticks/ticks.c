#include <jni.h>
#include <pthread.h>
#include <sys/types.h>

//
// Created by Administrator on 2020/7/9.
//
// Android log function wrappers
static const char* kTAG = "hello-jniCallback";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

typedef struct tick_cotext{
    JavaVM  *javaVM;
    jclass   jniHelperClz;
    jobject  jniHelperObj;
    jclass   mainActivityClz;
    jobject  mainActivityObj;
    pthread_mutex_t  lock;
    int      done;
} TickContext;
TickContext tickContext;


JNIEXPORT void JNICALL
Java_com_example_echo_TicksActivity_startTicks(JNIEnv *env, jobject thiz) {
pthread_t threadInfo;
pthread_attr_t pthreadAttr;

pthread_attr_init(&pthreadAttr);
pthread_attr_setdetachstate(&pthreadAttr,PTHREAD_CREATE_DETACHED);
pthread_mutex_init(&tickContext.lock, NULL);

jclass clz = (*env)->GetObjectClass(env,thiz);
tickContext.mainActivityClz = (*env)->NewGlobalRef(env,clz);

}

JNIEXPORT void JNICALL
Java_com_example_echo_TicksActivity_StopTicks(JNIEnv *env, jobject thiz) {

}