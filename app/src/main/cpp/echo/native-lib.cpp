#include <jni.h>
#include <string>


extern "C"
JNIEXPORT void JNICALL
Java_com_example_jni_EchoActivity_createSLEngine(JNIEnv *env, jobject thiz, jint rate,
                                                 jint frames_per_buf, jlong delay_in_ms,
                                                 jfloat decay) {
    // TODO: implement createSLEngine()
}