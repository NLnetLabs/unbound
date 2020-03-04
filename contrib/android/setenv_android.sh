#!/usr/bin/env bash

# Error checking
if [ ! -d "$ANDROID_NDK_ROOT" ]; then
    echo "ERROR: ANDROID_NDK_ROOT is not a valid path. Please set it."
    echo "NDK root is $ANDROID_NDK_ROOT"
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

#####################################################################

# Need to set THIS_HOST to darwin-x86_64, linux-x86_64,
# windows-x86_64 or windows.

if [[ "$(uname -s | grep -i -c darwin)" -ne 0 ]]; then
    THIS_HOST=darwin-x86_64
elif [[ "$(uname -s | grep -i -c linux)" -ne 0 ]]; then
    THIS_HOST=linux-x86_64
else
    echo "ERROR: Unknown host"
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

AOSP_TOOLCHAIN_ROOT="$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/$THIS_HOST"
AOSP_TOOLCHAIN_PATH="$AOSP_TOOLCHAIN_ROOT/bin"
AOSP_SYSROOT="$AOSP_TOOLCHAIN_ROOT/sysroot"

# Error checking
if [ ! -d "$AOSP_TOOLCHAIN_ROOT" ]; then
    echo "ERROR: AOSP_TOOLCHAIN_ROOT is not a valid path. Please set it."
    echo "Root is $AOSP_TOOLCHAIN_ROOT"
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Error checking
if [ ! -d "$AOSP_TOOLCHAIN_PATH" ]; then
    echo "ERROR: AOSP_TOOLCHAIN_PATH is not a valid path. Please set it."
    echo "Path is $AOSP_TOOLCHAIN_PATH"
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Error checking
if [ ! -d "$AOSP_SYSROOT" ]; then
    echo "ERROR: AOSP_SYSROOT is not a valid path. Please set it."
    echo "Path is $AOSP_SYSROOT"
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

#####################################################################

AOSP_CPU=$(tr '[:upper:]' '[:lower:]' <<< "$ANDROID_CPU")

# https://developer.android.com/ndk/guides/abis.html
case "$AOSP_CPU" in
  armeabi|armv7a|armv7-a|armeabi-v7a)
    CC="armv7a-linux-androideabi$ANDROID_API-clang"
    CXX="armv7a-linux-androideabi$ANDROID_API-clang++"
    LD="arm-linux-androideabi-ld"
    AS="arm-linux-androideabi-as"
    AR="arm-linux-androideabi-ar"
    RANLIB="arm-linux-androideabi-ranlib"
    STRIP="arm-linux-androideabi-strip"

    CFLAGS="-march=armv7-a -mthumb -mfloat-abi=softfp -funwind-tables -fexceptions"
    CXXFLAGS="-march=armv7-a -mthumb -mfloat-abi=softfp -funwind-tables -fexceptions -frtti"
    ;;

  armv8|armv8a|aarch64|arm64|arm64-v8a)
    CC="aarch64-linux-android$ANDROID_API-clang"
    CXX="aarch64-linux-android$ANDROID_API-clang++"
    LD="aarch64-linux-android-ld"
    AS="aarch64-linux-android-as"
    AR="aarch64-linux-android-ar"
    RANLIB="aarch64-linux-android-ranlib"
    STRIP="aarch64-linux-android-strip"

    CFLAGS="-funwind-tables -fexceptions"
    CXXFLAGS="-funwind-tables -fexceptions -frtti"
    ;;

  x86)
    CC="i686-linux-android$ANDROID_API-clang"
    CXX="i686-linux-android$ANDROID_API-clang++"
    LD="i686-linux-android-ld"
    AS="i686-linux-android-as"
    AR="i686-linux-android-ar"
    RANLIB="i686-linux-android-ranlib"
    STRIP="i686-linux-android-strip"

    CFLAGS="-mtune=intel -mssse3 -mfpmath=sse -funwind-tables -fexceptions"
    CXXFLAGS="-mtune=intel -mssse3 -mfpmath=sse -funwind-tables -fexceptions -frtti"
    ;;

  x86_64|x64)
    CC="x86_64-linux-android$ANDROID_API-clang"
    CXX="x86_64-linux-android$ANDROID_API-clang++"
    LD="x86_64-linux-android-ld"
    AS="x86_64-linux-android-as"
    AR="x86_64-linux-android-ar"
    RANLIB="x86_64-linux-android-ranlib"
    STRIP="x86_64-linux-android-strip"

    CFLAGS="-march=x86-64 -msse4.2 -mpopcnt -mtune=intel -funwind-tables -fexceptions"
    CXXFLAGS="-march=x86-64 -msse4.2 -mpopcnt -mtune=intel -funwind-tables -fexceptions -frtti"
    ;;

  *)
    echo "ERROR: Unknown architecture $ANDROID_CPU"
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
    ;;

esac

#####################################################################

# Error checking
if [ ! -e "$AOSP_TOOLCHAIN_PATH/$CC" ]; then
    echo "ERROR: Failed to find Android clang. Please edit this script."
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Error checking
if [ ! -e "$AOSP_TOOLCHAIN_PATH/$CXX" ]; then
    echo "ERROR: Failed to find Android clang++. Please edit this script."
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Error checking
if [ ! -e "$AOSP_TOOLCHAIN_PATH/$RANLIB" ]; then
    echo "ERROR: Failed to find Android ranlib. Please edit this script."
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Error checking
if [ ! -e "$AOSP_TOOLCHAIN_PATH/$AR" ]; then
    echo "ERROR: Failed to find Android ar. Please edit this script."
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Error checking
if [ ! -e "$AOSP_TOOLCHAIN_PATH/$AS" ]; then
    echo "ERROR: Failed to find Android as. Please edit this script."
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Error checking
if [ ! -e "$AOSP_TOOLCHAIN_PATH/$LD" ]; then
    echo "ERROR: Failed to find Android ld. Please edit this script."
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

#####################################################################

LENGTH=${#AOSP_TOOLCHAIN_PATH}
SUBSTR=${PATH:0:$LENGTH}
if [ "$SUBSTR" != "$AOSP_TOOLCHAIN_PATH" ]; then
    export PATH="$AOSP_TOOLCHAIN_PATH:$PATH"
fi

#####################################################################

export CPP CC CXX LD AS AR RANLIB STRIP
export ANDROID_SYSROOT="$AOSP_SYSROOT"
export CFLAGS="-D__ANDROID_API__=$ANDROID_API $CFLAGS --sysroot=$AOSP_SYSROOT"
export CXXFLAGS="-D__ANDROID_API__=$ANDROID_API $CXXFLAGS --sysroot=$AOSP_SYSROOT"

#####################################################################

echo "AOSP_TOOLCHAIN_PATH: $AOSP_TOOLCHAIN_PATH"

echo "CC: $(command -v "$CC")"
echo "CXX: $(command -v "$CXX")"
echo "LD: $(command -v "$LD")"
echo "AS: $(command -v "$AS")"
echo "AR: $(command -v "$AR")"

echo "ANDROID_SYSROOT: $ANDROID_SYSROOT"

echo "CFLAGS: $CFLAGS"
echo "CXXFLAGS: $CXXFLAGS"

[ "$0" = "${BASH_SOURCE[0]}" ] && exit 0 || return 0
