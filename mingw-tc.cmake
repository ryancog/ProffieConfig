set(CMAKE_SYSTEM_NAME Windows)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
    # The cstd lib stuff had *issues* linking shared and I don't feel like troubleshooting them.
    # They are non-trivial in size so building shared and adding them as DLLs would be better, maybe look at this later
    # set(CMAKE_FIND_ROOT_PATH /opt/mxe/usr/x86_64-w64-mingw32.shared)
    # set(CMAKE_C_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.shared-gcc)
    # set(CMAKE_CXX_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.shared-g++)
    set(CMAKE_FIND_ROOT_PATH /opt/mxe/usr/x86_64-w64-mingw32.static)
    set(CMAKE_C_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.static-gcc)
    set(CMAKE_CXX_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.static-g++)
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
    # Ninja generator required on macOS

    set(CMAKE_FIND_ROOT_PATH /usr/local/opt/mingw-w64/toolchain-x86_64/x86_64-w64-mingw32)
    set(CMAKE_C_COMPILER /usr/local/bin/x86_64-w64-mingw32-gcc)
    set(CMAKE_CXX_COMPILER /usr/local/bin/x86_64-w64-mingw32-g++)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libstdc++ -static-libgcc -static -lpthread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc -static -lpthread")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

