# Android test runner

It is meant to be an Android app that runs those fluidsynth tests under `../test` directory.

It is not immediately doable because everything is based on ctest where each source has `main()` function that cannot be more than one within a shared library. Therefore, we generate the modified test sources into this standalone Android tester app.

Also, since those tests have to access to libfluidsynth non-public implementation, we have to build the entire native test libraries (for each ABI), not with `libfluidsynth.so`.

The application was based on Android Studio 4.2 (when it was created).

## Building

Here is a brief task list to generate, build, and run Android tests:

- run `./convert-tests.sh` to generate runnable tests.
- run `build-scripts/download.sh` to download fluidsynth dependency archives to build from sources as Android dependencies.
- run `build-scripts/build-all-archs.sh` to build fluidsynth native libraries for Android.
- copy `build-scripts/build-artifacts/lib` contents into `app/src/main/jniLibs`.
- run `./gradlew connectedCheck` to build and run Android tests.

Detailed explanation follows.

(1) First of all, you have to generate the modified test sources as well as the native test runner from `../test` directory. `./convert-tests.sh` does this work for you.

It is a simple sed script that expects various preconditions that those existing ctests meet at the moment when this script was created (e.g. test source filename can be used to construct a valid C function name, it must have `int main()` literally, it must be compilable among with other sources, etc.). This also overwrites `app/src/main/cpp/run_all_tests.c`

You are supposed to run this every time the set of test files get updated. On CI builds it has to be run every time. This also applies when you want to try re-enabling those failing tests, adding exceptional tests that this converter cannot cover, or adding more failing tests.

(2) Once you are done with generating tests, then the next step is to download all fluidsynth runtime dependencies. This can be done by `./download.sh`. It is based on (but not a complete copy of) [Azure DevOps CI build setup](https://github.com/FluidSynth/fluidsynth/blob/master/.azure/azure-pipelines-android.yml). You have to do once until the list of dependencies changes. On Azure DevOps this step is therefore already handled (when this notes were written).

(3) Once you are done with downloading all those dependencies, the next step is to build fluidsynth for Android. Locally it can be achieved by `build-scripts/build-all-archs.sh`. It is (again) based on the Azure DevOps setup and therefore it is already handled there (when this notes were written).

It will end up with `build-scripts/build-artifacts/` that contains a `include` directory and `lib/*` directories for each ABI, on local builds. (Azure DevOps builds it is `$(Build.ArtifactStagingDirectory)/lib/*`.)

Once you have finished building fluidsynth for Android. there will be `$(topdir)/build_(ABI)` directories. While you want to build the Android tester app, you cannot remove them because those intermediate files (OBJ files) are referenced by this app's `CMakeLists.txt`.

(4) The `lib` part from the above has to be copied into `app/src/main/jniLibs`. There should be `armeabi-v7a`, `arm64-v8a`, `x86`, and `x86_64` subdirectories.

(5) After all the steps above are done, the Android tester app is ready to build and run. You can either open this directory as a project on Android Studio, or run `./gradlew build` to build the app, or run `./gradlew connectedCheck` to build and run the tests on a connected Android target (emulator or device). You can also run the tests by simply launching the MainActivity as it run there before showing the UI.

Note that those tests have to run on an Android target otherwise it does not make sense. `./gradlew build` or `./gradlew check` runs "test" in the project, but it does not mean they run on an Android target.
