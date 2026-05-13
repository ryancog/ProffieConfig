#!/bin/bash
# ProffieConfig, All-In-One Proffieboard Management Utility
# Copyright (C) 2024-2026 Ryan Ogurek

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

if [ "${TARGET_PLATFORM}" == "macOS" ]; then
    export CXXFLAGS="-mmacosx-version-min=11.0"
    export CFLAGS="-mmacosx-version-min=11.0"
    export LDFLAGS="-mmacosx-version-min=11.0"
fi

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
check_exec patchelf linux
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
    if ! eval ${2} &> ${3}.log; then
        echo "Error! Check ${3}.log."
        exit 1
    fi
}

patch_rpaths() {
    switch_to_lib_dir() {
        local LIB_DIR_1="${WX_INSTALL_PREFIX}-shared/lib64"
        local LIB_DIR_2="${WX_INSTALL_PREFIX}-shared/lib"
        if [[ -d $LIB_DIR_1 ]]; then
            cd $LIB_DIR_1
        elif [[ -d $LIB_DIR_2 ]]; then
            cd $LIB_DIR_2
        else
            echo "Missing lib dir!"
            exit 1
        fi
    }

    if [ "$TARGET_PLATFORM" == "linux" ]; then
        switch_to_lib_dir

        RENAME_KEY='s/\(lib.*\)-.*/\1.so/p'
        for lib in `ls -1 | grep '.*-.*.so.*'`; do
            if ! [ -L $lib ]; then
                SONAME=`echo $lib | sed -n $RENAME_KEY`
                mv -f $lib $SONAME
                chrpath -r \$ORIGIN/../lib $SONAME &> /dev/null
                patchelf --set-soname $SONAME $SONAME
                for dep in `ldd $SONAME | grep 'libwx' | awk '{print $1}'`; do
                    patchelf --replace-needed $dep `echo $dep | sed -n $RENAME_KEY` $SONAME
                done
            else
                rm $lib
            fi
        done
    elif [ "$TARGET_PLATFORM" == "macOS" ]; then
        switch_to_lib_dir

        # set install_name of libs correctly
        #
        # Basically it's just changing the link names of all the libs (and their dependencies) to @rpath/[name] that way
        # the install location can be dynamically changed when building the actual application(s).

        RENAME_KEY='s/.*\/\(lib[A-Za-z\_]*\).*/\1.dylib/p'
        WX_LIBNAMES=`ls -1 | grep '.*\.dylib$'`
        for lib in ${WX_LIBNAMES}; do
            if ! [ -L $lib ]; then
                NEWNAME=`otool -D ${lib} | sed -n 2$RENAME_KEY`
                mv -f ${lib} $NEWNAME
                install_name_tool -id @rpath/${NEWNAME} ${NEWNAME}
                for dep in `otool -L ${NEWNAME} | grep $'\t.*/libwx' | awk '{print $1}'`; do
                    CLEANED=`echo $dep | sed -n $RENAME_KEY`
                    install_name_tool -change ${dep} @rpath/${CLEANED} ${NEWNAME}
                done
            else
                rm ${lib}
            fi
        done
    fi

    cd ..
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
        if [ "$BUILD_PLATFORM" == "linux" ]; then
            export CROSS_COMPILE=x86_64-w64-mingw32-
        elif [ "$BUILD_PLATFORM" == "macOS" ]; then
            export CROSS_COMPILE=x86_64-w64-mingw32-
            BUILD_FLAGS="CC=\"x86_64-w64-mingw32-gcc -maes\""
        fi
    fi

    make clean &> /dev/null

    if [ "$TARGET_PLATFORM" == "win32" ]; then
        export LDFLAGS="-static-libstdc++ -static-libgcc"
    fi

    OLD_LDFLAGS=$LDFLAGS
    export LDFLAGS="${OLD_LDFLAGS} -s"
    if [ "$TARGET_PLATFORM" == "macOS" ]; then
        do_with_log \
            "Building shared (x86)" \
            "make -f makefile.shared -j`nproc --all` CC=\"clang -maes -target x86_64-apple-darwin\"" \
            build_x86
        make clean &> /dev/null
        mv libtomcrypt.dylib libtomcrypt-x86_64.dylib
        do_with_log \
            "Building shared (arm64)" \
            "make -f makefile.shared -j`nproc --all` CC=\"clang -target arm64-apple-darwin\"" \
            build_arm64
        make clean &> /dev/null
        mv libtomcrypt.dylib libtomcrypt-arm64.dylib
        do_with_log \
            "Melding" \
            "lipo -create -output libtomcrypt.dylib libtomcrypt-x86_64.dylib libtomcrypt-arm64.dylib" \
            meld
        rm -f libtomcrypt-x86_64.dylib libtomcrypt-arm64.dylib
    else
        do_with_log \
            "Building shared" \
            "make -f makefile.shared -j`nproc --all` $BUILD_FLAGS" \
            build
    fi
    export LDFLAGS=$OLD_LDFLAGS

    if [ "$TARGET_PLATFORM" == "macOS" ]; then
        do_with_log \
            "Building static (x86)" \
            "make -j`nproc --all` CC=\"clang -maes -arch x86_64\"" \
            build_x86
        make clean &> /dev/null
        mv libtomcrypt.a libtomcrypt-x86_64.a
        do_with_log \
            "Building static (arm64)" \
            "make -j`nproc --all` CC=\"clang -arch arm64\"" \
            build_arm64
        make clean &> /dev/null
        mv libtomcrypt.a libtomcrypt-arm64.a
        do_with_log \
            "Melding" \
            "lipo -create -output libtomcrypt.a libtomcrypt-x86_64.a libtomcrypt-arm64.a" \
            meld
        rm -f libtomcrypt-x86_64.a libtomcrypt-arm64.a
    else
        do_with_log \
            "Building static" \
            "make -j`nproc --all` $BUILD_FLAGS" \
            build
    fi

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

WX_FLAGS='--disable-exceptions --enable-debug_info --disable-debug --enable-pch '
WX_FLAGS+='--disable-unsafe-conv-in-wxstring --disable-sys-libs '
WX_FLAGS+='--without-opengl --without-expat --disable-html --disable-propgrid '
WX_FLAGS+='--disable-aui --disable-xrc --disable-richtext --disable-webview '

if [ "$TARGET_PLATFORM" == "linux" ]; then
    WX_HOST='x86_64-linux'
    WX_PLATFORM_FLAGS='--with-gtk=3'
elif [ "$TARGET_PLATFORM" == "macOS" ]; then
    WX_HOST='x86_64-apple-darwin --enable-universal_binary=x86_64,arm64 --with-macosx-version-min=11.0'
    WX_PLATFORM_FLAGS='--with-osx'
elif [ "$TARGET_PLATFORM" == "win32" ]; then
    if [ "$BUILD_PLATFORM" == "linux" ]; then
        export LDFLAGS="-static-libstdc++ -static-libgcc"
        WX_HOST='x86_64-w64-mingw32'
    elif [ "$BUILD_PLATFORM" == "macOS" ]; then
        # winpthread dll.a cannot exist because macOS MinGW is stupid.
        # This is probably a skill issue on my part but I refuse to fix it.
        export LDFLAGS="-static-libstdc++ -static-libgcc"
        WX_HOST='x86_64-w64-mingw32'
    fi
    WX_PLATFORM_FLAGS='--with-msw'
    export VENDOR=win32
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
fi

# For wxWidgets
patch_rpaths

cd $ROOT_DIR

echo "Preparing libbacktrace..."
cd 3rdparty/libbacktrace
if [ -d install-$TARGET_PLATFORM ]; then
    echo "    libbacktrace already built, skipping! (Remove install-$TARGET_PLATFORM to reset)"
else
    rm -rf install-$TARGET_PLATFORM
    rm -rf build-$TARGET_PLATFORM
    mkdir -p build-$TARGET_PLATFORM
    cd build-$TARGET_PLATFORM

    if [ "$TARGET_PLATFORM" == "win32" ]; then
        export LDFLAGS="-static-libstdc++ -static-libgcc"
    fi

    do_with_log \
        "Configuring" \
        "../configure --host=$WX_HOST --prefix=$PWD/../install-$TARGET_PLATFORM" \
        configure

    do_with_log \
        "Building" \
        "make -j" \
        build

    do_with_log \
        "Installing" \
        "make install" \
        install
fi

cd $ROOT_DIR

echo "Done!"

