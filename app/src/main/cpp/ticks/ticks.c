#include <jni.h>
#include <pthread.h>
#include <sys/types.h>
#include <assert.h>
#include <android/log.h>
#include <inttypes.h>
#include <string.h>
//
// Created by Administrator on 2020/7/9.
//
// Android log function wrappers
static const char *kTAG = "hello-jniCallback";
#define LOGI(...) \
  ((void)__android_log_print(ANDROID_LOG_INFO, kTAG, __VA_ARGS__))
#define LOGW(...) \
  ((void)__android_log_print(ANDROID_LOG_WARN, kTAG, __VA_ARGS__))
#define LOGE(...) \
  ((void)__android_log_print(ANDROID_LOG_ERROR, kTAG, __VA_ARGS__))

typedef struct tick_cotext {
    JavaVM *javaVM;
    jclass jniHelperClz;
    jobject jniHelperObj;
    jclass mainActivityClz;
    jobject mainActivityObj;
    pthread_mutex_t lock;
    int done;
} TickContext;
TickContext tickContext;

void   sendJavaMsg(JNIEnv *env, jobject instance,
                   jmethodID func,const char* msg) {
    jstring javaMsg = (*env)->NewStringUTF(env, msg);
    (*env)->CallVoidMethod(env, instance, func, javaMsg);
    (*env)->DeleteLocalRef(env, javaMsg);
}
void* UpdateTicks(void* context) {
    TickContext *pctx = (TickContext*) context;
    JavaVM *javaVm = pctx->javaVM;
    JNIEnv *env;
    jint res = (*javaVm)->GetEnv(javaVm, (void **) &env, JNI_VERSION_1_6);

    if (res != JNI_OK) {
        res = (*javaVm)->AttachCurrentThread(javaVm, &env, NULL);
        if (res != JNI_OK) {
            LOGE("Failed to AttachCurrentThread, ErrorCode = %d", res);
            return NULL;
        }
    }

    jmethodID statusId = (*env)->GetMethodID(env, pctx->jniHelperClz, "updateStatus",
                                             "(Ljava/lang/String;)V");
    sendJavaMsg(env, pctx->jniHelperObj, statusId, "TickerThread status: initializing...");

    jmethodID timerId = (*env)->GetMethodID(env, pctx->mainActivityClz, "updateTimer", "()V");

    struct timeval beginTime, curTime, usedTime, leftTime;
    const struct timeval kOneSecond = {
            (__kernel_time_t) 1,
            (__kernel_suseconds_t) 0
    };
    sendJavaMsg(env, pctx->jniHelperObj, statusId,
                "TickerThread status: start ticking ...");

    while (1) {
        gettimeofday(&beginTime, NULL);
        pthread_mutex_lock(&pctx->lock);
        int done = pctx->done;
        if (pctx->done) {
            pctx->done = 0;
        }
        pthread_mutex_unlock(&pctx->lock);
        if (done) {
            break;
        }
        (*env)->CallVoidMethod(env, pctx->mainActivityObj, timerId);

        gettimeofday(&curTime, NULL);
        timersub(&curTime, &beginTime, &usedTime);
        timersub(&kOneSecond, &usedTime, &leftTime);\
         struct timespec sleepTime;
        sleepTime.tv_sec = leftTime.tv_sec;
        sleepTime.tv_nsec = leftTime.tv_usec * 1000;

        if (sleepTime.tv_sec <= 1) {
            nanosleep(&sleepTime, NULL);
        } else {
            sendJavaMsg(env, pctx->jniHelperObj, statusId,
                        "TickerThread error: processing too long!");
        }
    }
    sendJavaMsg(env, pctx->jniHelperObj, statusId,
                "TickerThread status: ticking stopped");
    (*javaVm)->DetachCurrentThread(javaVm);
    return context;
}


void queryRuntimeInfo(JNIEnv *env, jobject instance) {
    // Find out which OS we are running on. It does not matter for this app
    // just to demo how to call static functions.
    // Our java JniHelper class id and instance are initialized when this
    // shared lib got loaded, we just directly use them
    //    static function does not need instance, so we just need to feed
    //    class and method id to JNI
    jmethodID versionFunc = (*env)->GetStaticMethodID(
            env, tickContext.jniHelperClz,
            "getBuildVersion", "()Ljava/lang/String;");
    if (!versionFunc) {
        LOGE("Failed to retrieve getBuildVersion() methodID @ line %d",
             __LINE__);
        return;
    }
    jstring buildVersion = (*env)->CallStaticObjectMethod(env,
                                                          tickContext.jniHelperClz, versionFunc);
    const char *version = (*env)->GetStringUTFChars(env, buildVersion, NULL);
    if (!version) {
        LOGE("Unable to get version string @ line %d", __LINE__);
        return;
    }
    LOGI("Android Version - %s", version);
    (*env)->ReleaseStringUTFChars(env, buildVersion, version);

    // we are called from JNI_OnLoad, so got to release LocalRef to avoid leaking
    (*env)->DeleteLocalRef(env, buildVersion);

    // Query available memory size from a non-static public function
    // we need use an instance of JniHelper class to call JNI
    jmethodID memFunc = (*env)->GetMethodID(env, tickContext.jniHelperClz,
                                            "getRuntimeMemorySize", "()J");
    if (!memFunc) {
        LOGE("Failed to retrieve getRuntimeMemorySize() methodID @ line %d",
             __LINE__);
        return;
    }
    jlong result = (*env)->CallLongMethod(env, instance, memFunc);
    LOGI("Runtime free memory size: %" PRId64, result);
    (void)result;  // silence the compiler warning
}
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    memset(&tickContext, 0, sizeof(tickContext));

    tickContext.javaVM = vm;
    if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR; // JNI version not supported.
    }

    jclass  clz = (*env)->FindClass(env,
                                    "com/example/echo/JniHandler");
    tickContext.jniHelperClz = (*env)->NewGlobalRef(env, clz);

    jmethodID  jniHelperCtor = (*env)->GetMethodID(env, tickContext.jniHelperClz,
                                                   "<init>", "()V");
    jobject    handler = (*env)->NewObject(env, tickContext.jniHelperClz,
                                           jniHelperCtor);
    tickContext.jniHelperObj = (*env)->NewGlobalRef(env, handler);
    queryRuntimeInfo(env, tickContext.jniHelperObj);

    tickContext.done = 0;
    tickContext.mainActivityObj = NULL;
    return  JNI_VERSION_1_6;
}
JNIEXPORT void JNICALL
Java_com_example_echo_TicksActivity_startTicks(JNIEnv *env, jobject thiz) {
    pthread_t threadInfo;
    pthread_attr_t pthreadAttr;

    pthread_attr_init(&pthreadAttr);
    pthread_attr_setdetachstate(&pthreadAttr, PTHREAD_CREATE_DETACHED);
    pthread_mutex_init(&tickContext.lock, NULL);

    jclass clz = (*env)->GetObjectClass(env, thiz);
    tickContext.mainActivityClz = (*env)->NewGlobalRef(env, clz);
    tickContext.mainActivityObj = (*env)->NewGlobalRef(env, thiz);

    int result = pthread_create(&threadInfo, &pthreadAttr, UpdateTicks, &tickContext);
    assert(result == 0);

    pthread_attr_destroy(&pthreadAttr);

    (void) result;
}






JNIEXPORT void JNICALL
Java_com_example_echo_TicksActivity_StopTicks(JNIEnv *env, jobject thiz) {

}
/*
 *  A helper function to show how to call
 *     java static functions JniHelper::getBuildVersion()
 *     java non-static function JniHelper::getRuntimeMemorySize()
 *  The trivial implementation for these functions are inside file
 *     JniHelper.java
 */
