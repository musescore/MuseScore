#include <jni.h>
#include <string>
#include <jni.h>

extern "C" int run_all_fluidsynth_tests();

extern "C" JNIEXPORT void JNICALL Java_org_fluidsynth_fluidsynthtests_TestRunner_00024Companion_runTests(JNIEnv* env, jobject thiz) {
    run_all_fluidsynth_tests();
}
