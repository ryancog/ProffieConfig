# Building ProffieConfig

**There's usually no reason to build ProffieConfig, and it's an advanced procedure. These are instructions to build the application itself, not anything for your proffieboard.**

If you're simply trying to download and use ProffieConfig, go [here](https://proffieconfig.kafrenetrading.com).

---
### Notes and Disclaimers

ProffieConfig's build *stuff* has only been tested on (all x86) macOS (Monterey through Sequoia), Windows 10, and Linux (OpenSUSE Tumbleweed, Debian Trixie). Other platforms and/or architectures will probably require some (potentially significant) additional effort and modification to make things work.

#### Regarding MinGW (Building on macOS or Linux for Windows)

*MinGW is strongly recommended against.* Here's a few reasons why, lol:

Building on Windows w/ MSVC is, probably easier, and MinGW currently seems to have issues. It produces binaries that have severe performance and memory usage issues, making it impractical to use.

Some commonly distributed versions of MinGW (e.g. via mxe) are out of date as of the time of writing and cannot be used to compile ProffieConfig.

I've encountered an issue with a typeinfo operator== being duplicated (added to stdc++ static lib and compiled into application code), despite being marked inline. To fix this, I simply added `__attribute__((always_inline))` to its signature in typeinfo header, as seems to be suggested from discussions around it. This means modifying system headers.

## Prerequisites

1. You'll need a C++ build toolchain. AppleClang on macOS (Xcode command line utilities), MSVC on Windows (Visual Studio 2022), and GCC or Clang on Linux (or MinGW for Windows cross-compile, but see the notes that it's not recommended).

2. On Linux especially, (and least so on Windows) building ProffieConfig's prerequisites may require additional 3rd party "devel" packages to be installed. `init.sh` will fail without them, but there's otherwise not a formal list.

3. In order to download the ProffieConfig repository properly, you'll need to install [git](https://git-scm.com). You'll also need [git-lfs](git-lfs.com), otherwise some files won't be downloaded correctly and **you'll run into issues later**.

## Get the Code

Open a terminal, move to the directory you want to save the ProffieConfig sources to, and run:
```
git clone https://github.com/ryancog/ProffieConfig.git
```

Enter the ProffieConfig directory:
```
cd ProffieConfig
```

## Set up the build environment

ProffieConfig includes `init.sh` and `init.ps1` scripts to automatically setup and verify what's needed for building. It's not meant to be totally foolproof and catch-all, you may find some rough edges.

Its job is primarily to setup 3rd-party dependencies ProffieConfig requires.

There are two ways to run the init script:
1. Building for Native (i.e. On Windows for Windows, on Linux for Linux or on macOS for macOS)
2. Cross-Compiling for Windows (i.e Build on Linux/macOS for Windows)

The former is what the init script configures for by default. The latter is triggred by passing `--CROSS_WIN32` as a command line argument. If you intend to build both ways, the script must be run twice (once each way).

The `init.ps1` only supports building on Windows for Windows with MSVC, and of course it needs to be ran in a PowerShell terminal (or invoked with powershell), not `cmd.exe`.

Run the ProffieConfig init script for a native build (Linux/macOS):
```
./init.sh
```
Run the ProffieConfig init script for a cross-compile Windows build:
```
./init.sh --CROSS_WIN32
```

Run the ProffieConfig init script for a native build (Windows):
```
.\init.ps1
```

- Go read a book, take a walk, or just stare at the wall. This takes a while (as long as an hour or maybe more on a low-end computer, it's usually <30 minutes on my machines)

## Configure CMake

Love it or hate it, ProffieConfig uses CMake now. At least it's better than `qmake`.

For the sake of convention, I structure the directories such that at ProffieConfig root there should be:
- `build`
- `deploy`

Inside each I create folders in the convention of `[platform]-[buildtype]`. e.g. `build/macOS-rel`.
You should build for `rel` or "Release" mode almost always.

In case you haven't picked up on it by now, there's only really 4(.5) ways ProffieConfig can be built.
1. Linux for Linux
2. macOS for macOS
3. Windows for Windows
4. Linux/macOS for Windows (there's a .5 in here somewhere)

For the former two, for example:
```
cmake -S . -B build/[macOS/linux]-rel -DCMAKE_BUILD_TYPE=Release -DLOCAL_BUILD=ON
```

For Windows native w/ MSVC:
```
cmake -G "Visual Studio 17 2022" -S . -B build\win32-rel -DCMAKE_BUILD_TYPE=Release -DLOCAL_BUILD=ON
```

For Windows w/ MinGW:
```
cmake -S . -B build/win32-rel -DCMAKE_BUILD_TYPE=Release -DLOCAL_BUILD=ON --toolchain mingw-tc.cmake
```
*for MinGW on macOS `-G Ninja` must also be added. Makefiles don't work on macOS for Windows*

Once the configuration completes, `cd` to the newly-created build directory. (e.g. `build/linux-rel`)

### An Aside on Local Builds

When ProffieConfig is built by me for people to use, the binaries ultimately are distributed onto my server in order for the ProffieConfig Launcher to manage installation and updates.

This is convenient for end users, but inconvenient for a custom build or for my own development/testing.

To build in a local way, you can use the `LOCAL_BUILD` flag. This is default in Debug mode, and can be specified in Release explicitly. Most people probably want to do this.

Otherwise ProffieConfig is configured to expect a Launcher-installed directory structure and to be in the usual installation path. Installing (and therefore running) ProffieConfig without local configuration is beyond the scope of this document.

## Build ProffieConfig

In the build directory, the actual build system must be called.

For macOS/Linux (native or cross-compile):
```
cmake --build . --parallel && cmake --install .
```
For Windows:
```
cmake --build . --config Release
cmake --install . --config Release
```

The installation will, after compilation is complete, move the compiled executable(s) and libraries into their correct locations in `deploy/[platform]-[buildtype]`

## Profit

That's pretty much all there is to building ProffieConfig.

At this point, you can simply run the `ProffieConfig[.exe]` binary in `deploy/[platform]-[buildtype]/items/bin`

