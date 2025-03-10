set(CMAKE_SYSTEM_NAME Windows)

# The cstd lib stuff had *issues* linking shared and I don't feel like troubleshooting them.
# They are non-trivial in size so building shared and adding them as DLLs would be better, maybe look at this later
# set(CMAKE_FIND_ROOT_PATH /opt/mxe/usr/x86_64-w64-mingw32.shared)
# set(CMAKE_C_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.shared-gcc)
# set(CMAKE_CXX_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.shared-g++)
set(CMAKE_FIND_ROOT_PATH /opt/mxe/usr/x86_64-w64-mingw32.static)
set(CMAKE_C_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.static-gcc)
set(CMAKE_CXX_COMPILER /opt/mxe/usr/bin/x86_64-w64-mingw32.static-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

