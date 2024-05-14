# Building ProffieConfig on Linux (for Linux)

This process was written and tested on Debian 12, but the same process is used for development on openSUSE Tumbleweed, and should be directly transferrable to other linux distributions, or transferrable with minimal tweaking.

## Building wxWidgets

In order to build ProffieConfig, you must first build and install the library ProffieConfig is built on, wxWidgets. **wxWidgets must be built custom from source!** Distribution packages **WILL NOT** work, either because they've a wrong version, are compiled with different flags, or simply don't support the features ProffieConfig uses.

Luckily this process is pretty straightforward:

1. Clone/Download wxWidgets source code:
	```
	git clone --recursive https://github.com/wxWidgets/wxWidgets.git
	```
1. Switch Directory to the newly-downloaded folder:
	```
	cd wxWidgets/
	```
1. Ensure wxWidgets submodules are initialized and updated (this shouldn't be required with `--recursive`, but is here for good measure):
	```
	git submodule init
	git submodule update --init
	```
1. Create a new build directory and switch to it:
	```
	mkdir gtk-build
	cd gtk-build
	```
1. Run wxWidgets configuration script:
	```
	../configure --enable-stl --disable-shared
	```
1. Build wxWidgets (Set number of CPU cores you have):
	```
	make -j [NUMBER OF CORES]
	```
1. Install wxWidgets to system directories for use compiling ProffieConfig:
	```
	sudo make install
	```

(NOTE: `--disable-shared` is recommended as per [ryrog25](https://github.com/ryryog25/ProffieConfig/issues/10#issuecomment-2064536605) for distribution, so it is included here. If `--disable-shared` is not used, the command `sudo ldconfig` must be run after `sudo make install` to update the dynamic linker, and your ProffieConfig build will only work on your machine.)

# Building ProffieConfig:

1. Clone/Download ProffieConfig source code:
	```
	git clone --recursive https://github.com/ryryog25/ProffieConfig
	```
1. Switch to ProffieConfig directory, create build directory, then switch to it:
	```
	cd ProffieConfig
	mkdir build && cd build
	```
1. Run `qmake` to configure the ProffieConfig build: 
	(You'll need to install `qmake` first, easiest is via your distribution package manager)
	```
	qmake6 -o Makefile ../src/ProffieConfig.pro
	```
1. Build ProffieConfig (Set number of CPU cores you have):
	```
	make -j [NUMBER OF CORES]
	```
1. Copy ProffieConfig resources:
	```
	cp -r ../resources/linux/arduino-cli resources/
	cp -r ../resources/ProffieOS resources/
	cp -r ../resources/props resources/
	cp -r ../resources/StyleEditor resources/
	```

Now at this point you should have a `ProffieConfig` binary executable alongside its `resources` folder (and probably a lot of other compile artifacts ending with `.o`), and you can run ProffieConfig!
