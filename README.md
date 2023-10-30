# ProffieConfig

![image](https://github.com/ryryog25/ProffieConfig/assets/60193408/e8f168d3-ae67-4a2b-bb99-146c4cff4b53)

All-In-One ProffieBoard (Created by Fredrik Hubbinette) configuration, driver installer, and flashing utility.

Utilzes `arduino-cli` and `proffie-dfu-setup` for driver installation and flashing.

## Installation

The latest release can be found [here](https://github.com/ryryog25/ProffieConfig/releases/latest), and once it's downloaded, just extract and run it!

Do note, on Windows the executable must be kept in it's folder in order to work. On macOS this is a non-issue due to the app bundle.

## Usage

The first time the program is run, it will prompt you to import a configuration file, you can choose to do so or not, and once you're at the main page, you should head on over to File->Install Dependencies... in order to make sure your computer is ready, especially if you've never programmed a proffieboard on your computer before.

The drop down on the top left allows you to switch between the configuration sections, and from here you'll set up the proffieboard. If you're the kind of person who's paranoid about it like I am, hitting CTRL+S will save the config, so you know it's safe if you close the program, and it'll be there the next time you open it up.

![image](https://github.com/ryryog25/ProffieConfig/assets/60193408/276dcc4f-7344-4854-8aa8-e2246c07e2ca)

Under "Config" there's the menu option to save your config, import and export it, and "verify" which will compile your config as it is, without a board needing to be connected, to check for any errors. (This is similar to what Arduino IDE does if you're familiar)

![image](https://github.com/ryryog25/ProffieConfig/assets/60193408/69dfde51-ec0d-4213-88be-5a2800e30fa7)

Once you're done, plug in your Proffieboard, click "Refresh..." and then your board's port should show up in the "Select Device..." dropdown. If it doesn't make sure you've installed dependencies. Now, click "Apply to Board..." Your configuration will be flashed to the proffieboard, and that's it! You're done. You can move put the necessary font/effect sound files onto your SD card, and you're off to the races!

## To-Do (Non-Exhaustive)
- Implement Blade Detect and Blade ID
- Implement `DISABLE_TALKIE`
- Implement SSD1306
- Implement Smart Suggestions on FLASH overflow
- Implement tooltips for more detailed setting explanation
- Implement dialog showing current button configuration (depending on prop and `NUM_BUTTONS`)

## Gallery

<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/e8f168d3-ae67-4a2b-bb99-146c4cff4b53 width=500>

<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/0be9170f-250d-4a1a-8be1-9b2c3ffa2358 width=500>

<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/8b0ae768-cd27-412b-a701-3f9f3f0857a1 width=500>

<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/d336e95f-ac19-4369-b046-a331719d1f0f width=500>
