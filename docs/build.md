# Building ProffieConfig

**There is no reason to build ProffieConfig unless you want to make your own changes.**

If you're simply trying to download and use ProffieConfig, go [here](https://proffieconfig.kafrenetrading.com).

---
### Notes and Disclaimers

*If you're on Windows, you'll need a better operating system. (I'm joking, kind of)
ProffieConfig is not set up to be built on Windows. It may be possible with WSL, but it probably won't be fun.*

ProffieConfig's build system(s) have only been tested on OpenSUSE Tumbleweed and Intel macOS. Other Linux distributions may have quirks with their build tools that require troubleshooting.

Building for Windows requires MinGW. MinGW is recommended to be installed via homebrew (macOS) or with mxe (Linux). MinGW for Apple Silicon does not have paths configured in the ProffieConfig toolchain. It will require (possibly significant) manual work.

## Get the Code

First things first, you'll need the code installed onto your computer. It's best to use `git` for this:

Open a terminal, move to the directory you want to save the ProffieConfig sources to, and run:
```
git clone https://github.com/ryancog/ProffieConfig.git
```

Enter the ProffieConfig directory:
```
cd ProffieConfig
```

## Set up the build environment

ProffieConfig includes an `init.sh` script to automatically setup and verify what's needed for building. It's not exhaustively tested, but it works pretty well thus far.

Its job is to ensure necessary command line utilities are installed, make sure the 3rd party code ProffieConfig requires is downloaded, and compile the 3rd party libraries.

There are two ways to run the init script, and they are not mutually exclusive.
1. Building for Native (i.e. Build on Linux for Linux or Build on macOS for macOS)
2. Cross-Compiling for Windows (i.e Build on Linux/macOS for Windows)

The former is what the init script configures for by default. The latter is triggred by passing `--CROSS_WIN32` as a command line argument. If you intend to build both ways, the script must be run twice (once each way).

Run the ProffieConfig init script for a native build:
```
./init.sh
```
Run the ProffieConfig init script for a Windows build:
```
./init.sh --CROSS_WIN32
```

- Go read a book, take a walk, or just stare at the wall. This takes a while (as long as an hour or maybe more on a low-end computer, it's usually <30 minutes on my machines)

## Configure CMake

Love it or hate it, ProffieConfig uses CMake now. At least it's better than `qmake`.

For the sake of convention, I structure the directories such that at ProffieConfig root there should be:
- `build`
- `deploy`

Inside each I create folders in the convention of `[platform]-[buildtype]`. e.g. `build/macOS-debug`.

In case you haven't picked up on it by now, there's only really 3(.5) ways ProffieConfig can be built.
1. Linux for Linux
2. macOS for macOS
3. Linux/macOS for Windows (there's a .5 in here somewhere)

For the former two:
```
cmake -S . -B build/[macOS/linux]-debug -DCMAKE_BUILD_TYPE=[Debug/Release]
```

For Windows:
```
cmake -S . -B build/win32-debug -DCMAKE_BUILD_TYPE=[Debug/Release] --toolchain mingw-tc.cmake
```
*on macOS `-G Ninja` must also be added. Makefiles don't work on macOS for Windows*

Once the configuration completes, `cd` to the newly-created build directory. (e.g. `build/linux-debug`)

### Debug/Release

When ProffieConfig is built by me for people to use, the binaries ultimately are distributed onto my server in order for the ProffieConfig Launcher to manage installation and updates.

This is convenient for end users, but inconvenient for a custom build or for my own development/testing.

Setting the build type to `Debug` configures things in a way that allows ProffieConfig to be run without a formal installation. This is easiest.

Setting the build type to `Release` configures ProffieConfig to expect a Launcher-installed directory structure and to be in the usual installation path. Installing (and therefore running) ProffieConfig with `Release` configuration is beyond the scope of this document.

A to-do of mine is to make some kind of `LOCAL` flag that does what Debug does but without `Debug` mode. I haven't bothered as of writing.

## Build ProffieCOnfig

In the build directory, the actual build system must be called.

Normally, this is `make`:
```
make -j`nproc --all` install
```

If the `Ninja` option above was used:
```
cmake --build . && cmake --install .
```

The installation will, after compilation is complete, move the compiled executable(s) and libraries into their correct locations in `deploy/[platform]-[buildtype]`

## Profit

That's pretty much all there is to building ProffieConfig.

At this point, provided the `Debug` mode was used, you can simply run the `ProffieConfig[.exe]` binary in `deploy/[platform]-[buildtype]/items/bin`
