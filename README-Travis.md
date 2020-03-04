# Travis Testing

Unbound 1.10 and above leverage Travis CI to increase coverage of compilers and platforms. Compilers include Clang and GCC; while platforms include Android, Linux, and OS X on AMD64, Aarch64, PowerPC and s390x hardware.

Android is tested on armv7a, aarch64, x86 and x86_64. Mips and Mips64 is no longer supported under current NDKs. The Android recipes build and install OpenSSL and Expat, and then builds Unbound. The testing is tailored for Android NDK-r19 and above, and includes NDK-r20 and NDK-r21.

The Unbound Travis configuration file `.travis.yml` does not use top-level keys like `os:` and `compiler:` so there is no matrix expansion. Instead Unbound specifies the exact job to run under the `jobs:` and `include:` keys.

## Typical recipe

A typical recipe tests Clang and GCC on various hardware. The hardware includes AMD64, Aarch64, PowerPC and s390x. PowerPC is a little-endian platform, and s390x is a big-endian platform. There are pairs of recipes that are similar to the following.

```
- os: linux
  name: GCC on Linux, Aarch64
  compiler: gcc
  arch: arm64
  dist: bionic
- os: linux
  name: Clang on Linux, Aarch64
  compiler: clang
  arch: arm64
  dist: bionic
```

OS X provides a single recipe to test Clang. GCC is not tested because GCC is an alias for Clang.

## Sanitizer builds

Two sanitizer builds are tested using Clang and GCC, for a total of four builds. The first sanitizer is Undefined Behavior sanitizer (UBsan), and the second is Address sanitizer (Asan). The sanitizers are only run on AMD64 hardware. Note the environment includes `TEST_UBSAN=yes` or `TEST_ASAN=yes` for the sanitizer builds.

The recipes are similar to the following.

```
- os: linux
  name: UBsan, GCC on Linux, Amd64
  compiler: gcc
  arch: amd64
  dist: bionic
  env: TEST_UBSAN=yes
- os: linux
  name: UBsan, Clang on Linux, Amd64
  compiler: clang
  arch: amd64
  dist: bionic
  env: TEST_UBSAN=yes
```

When the Travis script encounters a sanitizer it uses different `CFLAGS` and configuration string.

```
if [ "$TEST_UBSAN" = "yes" ]; then
  export CFLAGS="-DNDEBUG -g2 -O3 -fsanitize=undefined -fno-sanitize-recover"
  ./configure
elif [ "$TEST_ASAN" = "yes" ]; then
  export CFLAGS="-DNDEBUG -g2 -O3 -fsanitize=address"
  ./configure
...
```

## Android builds

Travis tests Android builds for the armv7a, aarch64, x86 and x86_64 architectures. The builds are trickier than other builds for several reasons. The testing requires installation of the Android NDK and SDK, it requires a cross-compile, and requires OpenSSL and Expat prerequisites. The Android cross-compiles also require care to set the Autotools triplet, the OpenSSL triplet, the toolchain path, the tool variables, and the sysroot. The steps below detail the pieces of the Android recipes.

### ANDROID_NDK_ROOT

The first step for Android is to set the environmental variables `ANDROID_NDK_ROOT` and `ANDROID_SDK_ROOT`. This is an important step because the NDK and SDK use the variables internally to locate their own tools. Also see [Recommended NDK Directory?](https://groups.google.com/forum/#!topic/android-ndk/qZjhOaynHXc) on the android-ndk mailing list. (Many folks botch this step, and use incorrect variables like `ANDROID_NDK_HOME` or `ANDROID_SDK_HOME`).

Unbound exports the variables in the Travis configuration script for the Android recipe:

```
export ANDROID_SDK_ROOT="$HOME/android-sdk"
export ANDROID_NDK_ROOT="$HOME/android-ndk"
```

### NDK installation

The second step installs the NDK and SDK. This step is handled in by the script `contrib/android/install_ndk.sh`. The script uses `ANDROID_NDK_ROOT` and `ANDROID_SDK_ROOT` to place the NDK and SDK in the `$HOME` directory.

If you are working from a developer machine you probably already have a NDK and SDK installed.

### Android environment

The third step sets the Android cross-compile environment using the script `contrib/android/setenv_android.sh`. The script is `sourced` so the variables in the script are available to the calling shell. The script sets variables like `CC`, `CXX`, `AS` and `AR`; sets `CFLAGS` and `CXXFLAGS`; sets a `sysroot` so Android headers and libraries are found; and adds the path to the toolchain to `PATH`.

`contrib/android/setenv_android.sh` knows which toolchain and architecture to select by inspecting environmental variables set by Travis for the job. In particular, the variables `ANDROID_CPU` and `ANDROID_API` tell `contrib/android/setenv_android.sh` what tools and libraries to select. For example, below is part of the Aarch64 recipe.

```
- os: linux
  name: Android aarch64, Linux, Amd64
  compiler: clang
  arch: amd64
  dist: bionic
  env:
    - TEST_ANDROID=yes
    - AUTOTOOLS_HOST=aarch64-linux-android
    - OPENSSL_HOST=android-arm64
    - ANDROID_CPU=aarch64
    - ANDROID_API=23
```

The `contrib/android/setenv_android.sh` script specifies the tools in a `case` statement like the following. There is a case for each of the architectures armv7a, aarch64, x86 and x86_64.

```
armv8a|aarch64|arm64|arm64-v8a)
  CC="aarch64-linux-android$ANDROID_API-clang"
  CXX="aarch64-linux-android$ANDROID_API-clang++"
  LD="aarch64-linux-android-ld"
  AS="aarch64-linux-android-as"
  AR="aarch64-linux-android-ar"
  RANLIB="aarch64-linux-android-ranlib"
  STRIP="aarch64-linux-android-strip"

  CFLAGS="-funwind-tables -fexceptions"
  CXXFLAGS="-funwind-tables -fexceptions -frtti"
```

### OpenSSL and Expat

OpenSSL and Expat are built for Android using the scripts `contrib/android/install_openssl.sh` and `contrib/android/install_expat.sh`. The scripts download, configure and install the latest release version of the libraries. The libraries are configured with `--prefix="$ANDROID_SYSROOT"` so the output artifacts are placed in the sysroot `$ANDROID_SYSROOT/usr/lib` directory. (Also see *Developer Workstation* section below).

OpenSSL also uses a custom configuration file called `15-android.conf`. It is a copy of the OpenSSL's project file and located at `contrib/android/15-android.conf`. The Unbound version is copied to the OpenSSL source files after unpacking the OpenSSL distribution. The Unbound version has legacy NDK support removed and some other fixes, like `ANDROID_NDK_ROOT` awareness. The changes mean Unbound's `15-android.conf` will only work with Unbound and with NDK-r19 and above.

OpenSSL is configured with `no-engine`. If you want to include OpenSSL engines then edit `contrib/android/install_openssl.sh` and remove the config option.

### Android build

Once all the variables are set and OpenSSL and Expat are built, then the Travis script configures and builds Unbound. The recipe looks as follows.

```
elif [ "$TEST_ANDROID" = "yes" ]; then
  export AUTOTOOLS_BUILD="$(./config.guess)"
  ./contrib/android/install_ndk.sh
  source ./contrib/android/setenv_android.sh "$ANDROID_CPU"
  ./contrib/android/install_openssl.sh
  ./contrib/android/install_expat.sh
  ./configure \
    --build="$AUTOTOOLS_BUILD" --host="$AUTOTOOLS_HOST" \
    --prefix="$ANDROID_SYSROOT" \
    --with-ssl="$ANDROID_SYSROOT" --disable-gost \
    --with-libexpat="$ANDROID_SYSROOT";
  make -j 2
```

Travis only smoke tests an Android build using a compile and link. The self tests are not run. TODO: figure out how to fire up an emulator, push the tests to the device and run them.

Note the `--prefix="$ANDROID_SYSROOT"` used by OpenSSL, Expat and Unbound. This is a hack that makes it easy to find libraries and headers because `CFLAGS` and `CXXFLAGS` already use `--sysroot="$ANDROID_SYSROOT"`. By performing a `make install` and installing into `$ANDROID_SYSROOT`, all the libraries needed by Unbound are present without extra flags or searching. (Also see *Developer Workstation* section below).

### Android flags

`contrib/android/setenv_android.sh` uses specific flags for `CFLAGS` and `CXXFLAGS`. The flags are not arbitrary, and they are taken from the `ndk-build` tool. It is important to use the same flags across projects to avoid subtle problems due to mixing and matching different flags.

`CXXFLAGS` includes `-fexceptions` and `-frtti` because exceptions and runtime type info are disabled by default. `CFLAGS` include `-funwind-tables` and `-fexceptions` to ensure C++ exceptions pass through C code, if needed. Also see `docs/CPLUSPLUS—SUPPORT.html` in the NDK docs.

To inspect the flags used by `ndk-build` for a platform clone ASOP's [ndk-samples](https://github.com/android/ndk-samples/tree/master/hello-jni) and build the `hello-jni` project. Use the `V=1` flag to see the full compiler output from `ndk-build`.

### Developer workstation

If you are setting up a developer workstation then `--prefix="$ANDROID_SYSROOT"` trick will not work. It will not work on a workstation because a previous install gets overwritten by the next install in `$ANDROID_SYSROOT/usr/lib`. In practice OpenSSL and Expat need to be installed in an architecture specific location. The sysroot-based locations are listed below.

* `$ANDROID_SYSROOT/usr/lib/aarch64-linux-android/`
* `$ANDROID_SYSROOT/usr/lib/arm-linux-androideabi/`
* `$ANDROID_SYSROOT/usr/lib/i686-linux-android/`
* `$ANDROID_SYSROOT/usr/lib/x86_64-linux-android/`

A second concern is the OpenSSL header files. At least two different defines are used in the OpenSSL headers based on the architecture. The first pair of defines are `RC4_CHAR` and `RC4_INT`. The second pair of defines are `SIXTY_FOUR_BIT_LONG` and `BN_LLONG`. As with libraries, a previous install gets overwritten by the next install in `$ANDROID_SYSROOT/usr/include`. To work around the problem see [Build Multiarch OpenSSL on OS X](https://stackoverflow.com/q/25530429) on Stack Overflow. Though it is a MacOS question, the same applies to Android in this situation.
