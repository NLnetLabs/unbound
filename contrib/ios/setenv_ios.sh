#!/usr/bin/env bash

#####################################################################

# Allow a user override? I think we should be doing this. The use case is:
# move /Applications/Xcode somewhere else for a side-by-side installation.
if [ -z "${XCODE_DEVELOPER-}" ]; then
  XCODE_DEVELOPER=$(xcode-select -print-path 2>/dev/null)
fi

if [ ! -d "$XCODE_DEVELOPER" ]; then
  echo "ERROR: unable to find XCODE_DEVELOPER directory."
  [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# Default toolchain location
XCODE_TOOLCHAIN="$XCODE_DEVELOPER/usr/bin"

if [ ! -d "$XCODE_TOOLCHAIN" ]; then
  echo "ERROR: unable to find XCODE_TOOLCHAIN directory."
  [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# XCODE_DEVELOPER_TOP is the top of the development tools tree
XCODE_DEVELOPER_TOP="$XCODE_DEVELOPER/Platforms/$IOS_SDK.platform/Developer"

if [ ! -d "$XCODE_DEVELOPER_TOP" ]; then
  echo "ERROR: unable to find XCODE_DEVELOPER_TOP directory."
  [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

# IOS_TOOLCHAIN_PATH is the location of the actual compiler tools.
if [ -d "$XCODE_DEVELOPER/Toolchains/XcodeDefault.xctoolchain/usr/bin/" ]; then
  IOS_TOOLCHAIN_PATH="$XCODE_DEVELOPER/Toolchains/XcodeDefault.xctoolchain/usr/bin/"
elif [ -d "$XCODE_DEVELOPER_TOP/usr/bin/" ]; then
  IOS_TOOLCHAIN_PATH="$XCODE_DEVELOPER_TOP/usr/bin/"
fi

if [ -z "$IOS_TOOLCHAIN_PATH" ] || [ ! -d "$IOS_TOOLCHAIN_PATH" ]; then
  echo "ERROR: unable to find Xcode cross-compiler tools."
  [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

#####################################################################

# XCODE_SDK is the SDK name/version being used. Adjust the list as appropriate.
# For example, remove 4.3, 6.2, and 6.1 if they are not installed. We go back to
# the 1.0 SDKs because Apple WatchOS uses low numbers, like 2.0 and 2.1.
unset XCODE_SDK
for i in $(seq -f "%.1f" 30.0 -0.1 1.0)
do
    if [ -d "$XCODE_DEVELOPER/Platforms/$IOS_SDK.platform/Developer/SDKs/$IOS_SDK$i.sdk" ]; then
        XCODE_SDK="$IOS_SDK$i.sdk"
        break
    fi
done

# Error checking
if [ -z "$XCODE_SDK" ]; then
    echo "ERROR: unable to find a SDK."
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

IOS_SYSROOT="$XCODE_DEVELOPER_TOP/SDKs/$XCODE_SDK"

if [ -z "$IOS_SYSROOT" ] || [ ! -d "$IOS_SYSROOT" ]; then
  echo "ERROR: unable to find IOS_SYSROOT directory."
  [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
fi

#####################################################################

XCODE_SDK=$(tr '[:upper:]' '[:lower:]' <<< "$IOS_SDK")

case "$XCODE_SDK" in
  iphone|iphoneos)
    CPP="cpp"
    CC="clang"
    CXX="clang++"
    LD="ld"
    AS="as"
    AR="ar"
    RANLIB="ranlib"
    STRIP="strip"

    # Default armv7. Also allowed armv7a, armv7s, arm64
    if [ -z "$IOS_CPU" ]; then IOS_CPU=armv7; fi

    if [ "$IOS_CPU" = "arm64" ]; then
      CFLAGS="-arch $IOS_CPU -mios-version-min=7"
      CXXFLAGS="-arch $IOS_CPU -stdlib=libc++ -mios-version-min=7"
    else
      CFLAGS="-arch $IOS_CPU -mios-version-min=6"
      CXXFLAGS="-arch $IOS_CPU -stdlib=libc++ -mios-version-min=6"
    fi
    ;;

  iphonesimulator)
    CPP="cpp"
    CC="clang"
    CXX="clang++"
    LD="ld"
    AS="as"
    AR="ar"
    RANLIB="ranlib"
    STRIP="strip"

    # Default i386.
    if [ -z "$IOS_CPU" ]; then IOS_CPU=i386; fi

    CFLAGS="-arch $IOS_CPU -mios-version-min=5"
    CXXFLAGS="-arch $IOS_CPU -stdlib=libc++ -mios-version-min=5"
    ;;

  watch|watchos|applewatch)
    CPP="cpp"
    CC="clang"
    CXX="clang++"
    LD="ld"
    AS="as"
    AR="ar"
    RANLIB="ranlib"
    STRIP="strip"

    # Default armv7. Also allowed armv7a, armv7s
    if [ -z "$IOS_CPU" ]; then IOS_CPU=armv7; fi

    CFLAGS="-arch $IOS_CPU -mios-version-min=7"
    CXXFLAGS="-arch $IOS_CPU -stdlib=libc++ -mios-version-min=7"
    ;;

  watchsimulator|watchossimulator)
    CPP="cpp"
    CC="clang"
    CXX="clang++"
    LD="ld"
    AS="as"
    AR="ar"
    RANLIB="ranlib"
    STRIP="strip"

    # Default i386.
    if [ -z "$IOS_CPU" ]; then IOS_CPU=i386; fi

    CFLAGS="-arch $IOS_CPU"
    CXXFLAGS="-arch $IOS_CPU -stdlib=libc++"
    ;;

  appletv|appletvos)
    CPP="cpp"
    CC="clang"
    CXX="clang++"
    LD="ld"
    AS="as"
    AR="ar"
    RANLIB="ranlib"
    STRIP="strip"

    # Default arm64.
    if [ -z "$IOS_CPU" ]; then IOS_CPU=arm64; fi

    CFLAGS="-arch $IOS_CPU"
    CXXFLAGS="-arch $IOS_CPU -stdlib=libc++"
    ;;

  appletvsimulator|appletvossimulator)
    CPP="cpp"
    CC="clang"
    CXX="clang++"
    LD="ld"
    AS="as"
    AR="ar"
    RANLIB="ranlib"
    STRIP="strip"

    # Default x86_64,
    if [ -z "$IOS_CPU" ]; then IOS_CPU=x86_64; fi

    CFLAGS="-arch $IOS_CPU"
    CXXFLAGS="-arch $IOS_CPU -stdlib=libc++"
    ;;

  *)
    echo "ERROR: Unknown architecture $IOS_SDK"
    [ "$0" = "${BASH_SOURCE[0]}" ] && exit 1 || return 1
    ;;

esac

#####################################################################

TOOLCHAIN_PATH="$IOS_TOOLCHAIN_PATH:$XCODE_TOOLCHAIN"
LENGTH=${#TOOLCHAIN_PATH}
SUBSTR=${PATH:0:$LENGTH}
if [ "$SUBSTR" != "$TOOLCHAIN_PATH" ]; then
    export PATH="$TOOLCHAIN_PATH":"$PATH"
fi

#####################################################################

export CPP CC CXX LD AS AR RANLIB STRIP
export IOS_SYSROOT
export CFLAGS="$CFLAGS --sysroot=$IOS_SYSROOT"
export CXXFLAGS="$CXXFLAGS --sysroot=$IOS_SYSROOT"

#####################################################################

echo "IOS_TOOLCHAIN_PATH: $IOS_TOOLCHAIN_PATH"

echo "CC: $(command -v "$CC")"
echo "CXX: $(command -v "$CXX")"
echo "LD: $(command -v "$LD")"
echo "AS: $(command -v "$AS")"
echo "AR: $(command -v "$AR")"

echo "IOS_SYSROOT: $IOS_SYSROOT"

echo "CFLAGS: $CFLAGS"
echo "CXXFLAGS: $CXXFLAGS"

[ "$0" = "${BASH_SOURCE[0]}" ] && exit 0 || return 0
