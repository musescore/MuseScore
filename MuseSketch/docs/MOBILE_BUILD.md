# MuseSketch Mobile Build Guide

This guide explains how to build MuseSketch for iOS and Android.

## Prerequisites

### Common Requirements

1. **Qt 6.5+** with mobile components:
   - Qt for Android (ARM64)
   - Qt for iOS
   
   Install via [Qt Online Installer](https://www.qt.io/download-qt-installer) or [aqtinstall](https://github.com/miurahr/aqtinstall):
   ```bash
   # Using aqtinstall
   pip install aqtinstall
   
   # Install Qt 6.6.1 for macOS (host), Android, and iOS
   aqt install-qt mac desktop 6.6.1
   aqt install-qt mac android 6.6.1 android_arm64_v8a
   aqt install-qt mac ios 6.6.1 ios
   ```

2. **CMake 3.16+** and **Ninja** build system
   ```bash
   brew install cmake ninja
   ```

---

## iOS Build

### Requirements

- macOS 12+ with Xcode 14+
- Apple Developer account (free for simulator, paid for device deployment)

### Steps

1. **Install Qt for iOS**
   
   Make sure you have Qt 6.5+ for iOS installed at `~/Qt/6.x.x/ios/`

2. **Configure the build**
   
   ```bash
   cd MuseSketch
   ./scripts/build-ios.sh simulator debug
   ```

3. **Open in Xcode**
   
   ```bash
   open build-ios-simulator-debug/MuseSketch.xcodeproj
   ```

4. **Configure signing** (for device builds)
   - Select the `musesketch` target
   - Go to Signing & Capabilities
   - Select your development team
   - Update bundle identifier if needed

5. **Build and run**
   - Select iPhone simulator or connected device
   - Press Cmd+R to build and run

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `QT_IOS_PATH` | Qt for iOS installation | `~/Qt/6.6.1/ios` |
| `DEVELOPMENT_TEAM` | Apple Developer Team ID | (none) |

---

## Android Build

### Requirements

- Android SDK (API 24+)
- Android NDK r25+
- Java 17 (for Gradle)

### Setup Android SDK

1. **Install Android Studio** or standalone SDK:
   ```bash
   # SDK typically at:
   # macOS: ~/Library/Android/sdk
   # Linux: ~/Android/Sdk
   ```

2. **Install required components** via SDK Manager:
   - Android SDK Platform 34
   - Android SDK Build-Tools 34
   - Android NDK 25.x or newer
   - CMake (from SDK)

3. **Set environment variables**:
   ```bash
   export ANDROID_SDK_ROOT=~/Library/Android/sdk
   export ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/25.2.9519653
   ```

### Steps

1. **Install Qt for Android**
   
   Make sure you have Qt 6.5+ for Android installed at `~/Qt/6.x.x/android_arm64_v8a/`

2. **Build the APK**
   
   ```bash
   cd MuseSketch
   ./scripts/build-android.sh release
   ```

3. **Find the APK**
   
   ```bash
   ls build-android-release/app/android-build/build/outputs/apk/
   ```

4. **Install on device**
   
   ```bash
   adb install build-android-release/app/android-build/build/outputs/apk/release/android-build-release.apk
   ```

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `QT_ANDROID_PATH` | Qt for Android installation | `~/Qt/6.6.1/android_arm64_v8a` |
| `ANDROID_SDK_ROOT` | Android SDK path | `~/Library/Android/sdk` |
| `ANDROID_NDK_ROOT` | Android NDK path | `$ANDROID_SDK_ROOT/ndk/25.2.9519653` |

---

## Troubleshooting

### iOS

**"Code signing required" error**
- Ensure you have a valid Apple Developer account
- Set DEVELOPMENT_TEAM or configure in Xcode

**"Qt for iOS not found"**
- Verify Qt iOS installation: `ls ~/Qt/6.6.1/ios/`
- Update QT_IOS_PATH if using different version

### Android

**"SDK/NDK not found"**
- Verify paths with `echo $ANDROID_SDK_ROOT` and `echo $ANDROID_NDK_ROOT`
- Check NDK version matches installed version

**Gradle build fails**
- Ensure Java 17 is installed: `java -version`
- Set JAVA_HOME if needed: `export JAVA_HOME=$(/usr/libexec/java_home -v 17)`

**"No toolchain file found"**
- Verify Qt Android installation
- Check that `qt.toolchain.cmake` exists in Qt lib/cmake/Qt6/

---

## App Store / Play Store Distribution

### iOS (App Store)

1. Build with release configuration
2. Archive in Xcode (Product → Archive)
3. Upload to App Store Connect
4. Submit for review

### Android (Play Store)

1. Generate signing key:
   ```bash
   keytool -genkey -v -keystore musesketch.keystore -alias musesketch -keyalg RSA -keysize 2048 -validity 10000
   ```

2. Configure signing in `app/android/build.gradle`

3. Build signed release APK:
   ```bash
   ./scripts/build-android.sh release
   ```

4. Upload AAB (Android App Bundle) to Play Console

---

## Platform-Specific Notes

### File Storage

- **iOS**: Uses app sandbox, files accessible via Files app (UIFileSharingEnabled)
- **Android**: Uses scoped storage, exports to Downloads folder

### Audio

The app uses Qt Multimedia for audio playback. Both platforms support:
- Real-time audio synthesis
- Background audio (requires additional configuration)

### Permissions

**Android** (`AndroidManifest.xml`):
- `WRITE_EXTERNAL_STORAGE` - for exports
- `READ_EXTERNAL_STORAGE` - for imports

**iOS** (`Info.plist`):
- `UIFileSharingEnabled` - file sharing via iTunes/Finder
- `NSMicrophoneUsageDescription` - for future recording features

