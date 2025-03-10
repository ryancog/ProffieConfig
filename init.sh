#!/bin/sh
# ProffieConfig, All-In-One Proffieboard Management Utility
# Copyright (C) 2024 Ryan Ogurek

ROOT_DIR=`dirname -- \` readlink -f -- "$0" \``
cd $ROOT_DIR

echo "Detecting platform..."
if [ "$OSTYPE" == "linux-gnu" ]; then
    BUILD_PLATFORM=linux
elif [[ "$OSTYPE" == "darwin"* ]]; then
    BUILD_PLATFORM=macOS
else
    echo "Unsupported build platform: $OSTYPE"
    exit 1
fi
echo "Platform: ${BUILD_PLATFORM}"

if [ "$1" == "--CROSS_WIN32" ]; then
    TARGET_PLATFORM=win32
else
    TARGET_PLATFORM=${BUILD_PLATFORM}
fi
if [ "${TARGET_PLATFORM}" != "${BUILD_PLATFORM}" ]; then echo "Target Platform: ${TARGET_PLATFORM}"; fi

check_exec() {
    if [ ! -z "$2" ] && [ "$BUILD_PLATFORM" != "$2" ]; then
        return
    fi

    if ! command -v $1 &> /dev/null; then
        echo "Err: ${1} is missing, please install it!"
        exit 1
    fi
}

# Linux, macOS, Windows
SWITCH_VAL=""
platform_switch() {
    if [ "$TARGET_PLATFORM" == "linux" ]; then
        SWITCH_VAL=$1
    elif [ "$TARGET_PLATFORM" == "macOS" ]; then
        SWITCH_VAL=$2
    elif [ "$TARGET_PLATFORM" == "win32" ]; then
        SWITCH_VAL=$3
    fi
}

check_exec chrpath linux
check_exec otool macOS
check_exec git
check_exec libtool
check_exec automake
check_exec autoconf
check_exec autogen
check_exec nasm

do_with_log() {
    # 1: Human process name
    # 2: Command
    # 3: log
    echo "    ${1}..."
    if ! ${2} &> ${3}.log; then
        echo "Error! Check ${3}.log."
        exit 1
    fi
}

patch_rpaths() {
    if [ "$TARGET_PLATFORM" == "linux" ]; then
        for lib in `ls -1 | grep '.*.so.*'`; do
            if ! [ -L ${lib} ]; then
                SONAME=`readelf -d ${lib} | grep SONAME | sed -n 's/.*\[\([^]]*\)\].*/\1/p'`
                mv -f ${lib} ${SONAME}
                ln -sf ${SONAME} `echo ${SONAME} | sed -n 's/\(lib.*\.so\).*/\1/p'`
                chrpath -r '$ORIGIN/../lib' $SONAME &> /dev/null
            else
                rm ${lib}
            fi
        done
    elif [ "$TARGET_PLATFORM" == "macOS" ]; then
        # set install_name of libs correctly
        #
        # Basically it's just changing the link names of all the libs (and their dependencies) to @rpath/[name] that way
        # the install location can be dynamically changed when building the actual application(s).

        WX_LIBNAMES=`ls -1 | grep '.*\.dylib$'`
        for lib in ${WX_LIBNAMES}; do
            if ! [ -L $lib ]; then
                NEWNAME=`otool -D ${lib} | sed -n '2s/.*\/\(lib[A-Za-z\_]*\).*/\1.dylib/p'`
                mv -f ${lib} $NEWNAME
                install_name_tool -id @rpath/${NEWNAME} ${NEWNAME}
                for dep in `otool -L ${NEWNAME} | grep '/libwx' | sed -n 's/\t\(.*\) (.*/\1/p'`; do
                    CLEANED_DEP_NAME=`echo "${dep}" | sed -n 's/.*\/\(lib[A-Za-z\_]*\).*/\1.dylib/p'`
                    install_name_tool -change ${dep} @rpath/${CLEANED_DEP_NAME} ${NEWNAME}
                done
            else
                rm ${lib}
            fi
        done
    fi
}

echo "Initializing 3rd party repositories..."
git submodule update --init

echo "Setting up libtomcrypt..."
cd 3rdparty/tomcrypt
platform_switch "build-linux" "build-macOS" "build-win32"
if [ -d $SWITCH_VAL ]; then
    echo "Tomcrypt already built, skipping! (Remove 'build' directory to reset)"
else
    unset CROSS_COMPILE
    if [ "$TARGET_PLATFORM" == "win32" ]; then
        export CROSS_COMPILE=x86_64-w64-mingw32.static-
    fi

    make clean &> /dev/null
    export LDFLAGS="-s"
    do_with_log \
        "Building shared" \
        "make -f makefile.shared -j`nproc --all`" \
        build
    unset LDFLAGS

    do_with_log \
        "Building static" \
        "make -j`nproc --all`" \
        build

    mkdir -p $SWITCH_VAL
    if [ "$TARGET_PLATFORM" == "win32" ]; then
        unset CROSS_COMPILE
        mv tomcrypt.dll build-win32/tomcrypt.dll
        mv libtomcrypt.a build-win32/libtomcrypt.a
    elif [ "$TARGET_PLATFORM" == "linux" ]; then
        mv libtomcrypt.so build-linux/libtomcrypt.so
        mv libtomcrypt.a build-linux/libtomcrypt.a
    elif [ "$TARGET_PLATFORM" == "macOS" ]; then
        mv libtomcrypt.dylib build-macOS/libtomcrypt.dylib
        mv libtomcrypt.a build-macOS/libtomcrypt.a
        echo "    Patching tomcrypt lib with @rpath..."
        install_name_tool -id "@rpath/libtomcrypt.dylib" build-macOS/libtomcrypt.dylib
    fi

fi
cd $ROOT_DIR

# echo "Setting up ffmpeg..."
# cd 3rdparty/ffmpeg
#
# if [ -d install-${TARGET_PLATFORM} ]; then
#     echo "ffmpeg already built, skipping! (Remove install-${TARGET_PLATFORM} to reset)"
# else
#     do_with_log \
#         "Configuring" \
#         "./configure --disable-programs --disable-doc --disable-static --enable-shared --enable-rpath --disable-filters --disable-bsfs --disable-network --disable-indevs --disable-outdevs --disable-protocols --enable-protocol=file --enable-lto --prefix=`pwd`/install-${TARGET_PLATFORM}" \
#         configure
#
#     do_with_log \
#         "Building" \
#         "make -j`nproc --all`" \
#         build
#
#     do_with_log \
#         "Installing" \
#         "make install" \
#         install
#
#     platform_switch "install-linux/lib" "" ""
#     cd $SWITCH_VAL
#     patch_rpaths
# fi
# cd $ROOT_DIR

echo "Preparing wxWidgets..."
cd 3rdparty/wxWidgets

do_with_log \
    "Initializing repository" \
    "git submodule update --init" \
    repoinit

WX_INSTALL_PREFIX=`pwd`/install-$TARGET_PLATFORM
WX_FLAGS='--without-opengl --disable-unsafe-conv-in-wxstring --disable-sys-libs'
if [ "$TARGET_PLATFORM" == "linux" ]; then
    WX_HOST='x86_64-linux'
    WX_PLATFORM_FLAGS='--with-gtk=3'
elif [ "$TARGET_PLATFORM" == "macOS" ]; then
    WX_HOST='x86_64-apple-darwin'
    WX_PLATFORM_FLAGS='--with-osx'
elif [ "$TARGET_PLATFORM" == "win32" ]; then
    WX_HOST='x86_64-w64-mingw32.static'
    WX_PLATFORM_FLAGS='--with-msw'
    VENDOR=win32
fi

if [ "$BUILD_PLATFORM" == "linux" ]; then
    WX_BUILD='x86_64-linux'
elif [ "$BUILD_PLATFORM" == "macOS" ]; then
    WX_BUILD='x86_64-apple-darwin'
fi

WX_BUILD_DIR=build-$TARGET_PLATFORM

if [ -d install-${TARGET_PLATFORM}-static ]; then
    echo "    wxWidgets static already built, skipping! (Remove install-${TARGET_PLATFORM}-static to reset)"
else
    rm -rf $WX_BUILD_DIR-static
    rm -rf $WX_INSTALL_PREFIX-static
    mkdir -p $WX_BUILD_DIR-static
    cd $WX_BUILD_DIR-static

    do_with_log \
        "Configuring static" \
        "../configure --prefix=$WX_INSTALL_PREFIX-static --host=$WX_HOST --build=$WX_BUILD --disable-shared $WX_FLAGS $WX_PLATFORM_FLAGS" \
        configure

    do_with_log \
        "Building static" \
        "make -j`nproc --all`" \
        build

    do_with_log \
        "Installing static" \
        "make install" \
        install

    cd ..
fi

if [ -d install-${TARGET_PLATFORM}-shared ]; then
    echo "    wxWidgets shared already built, skipping! (Remove install-${TARGET_PLATFORM}-shared to reset)"
else
    rm -rf $WX_BUILD_DIR-shared
    rm -rf $WX_INSTALL_PREFIX-shared
    mkdir -p $WX_BUILD_DIR-shared
    cd $WX_BUILD_DIR-shared

    do_with_log \
        "Configuring shared" \
        "../configure --prefix=$WX_INSTALL_PREFIX-shared --host=$WX_HOST --build=$WX_BUILD --enable-shared $WX_FLAGS $WX_PLATFORM_FLAGS" \
        configure

    do_with_log \
        "Building shared" \
        "make -j`nproc --all`" \
        build

    do_with_log \
        "Installing shared" \
        "make install" \
        install

    cd ..
    platform_switch "${WX_INSTALL_PREFIX}-shared/lib64" "${WX_INSTALL_PREFIX}-shared/lib" ""
    cd $SWITCH_VAL
    patch_rpaths
fi
cd $ROOT_DIR

echo "Done!"

