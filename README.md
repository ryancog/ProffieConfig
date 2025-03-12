# ProffieConfig 

<img align="left" src=https://github.com/ryancog/ProffieConfig/blob/master/resources/icons/icon.svg width=200> 
  
All-In-One ProffieBoard Management Utility. 

Incorporates extensive configuration generation, compiling and applying changes to the ProffieBoard, the Serial Monitor, handles driver installation, and some sanity checks for good measure; All without the Arduino IDE. 

Using this tool, there is no need for dealing with the Arduino IDE, Zadig, etc. in an attempt to make using a Proffieboard as beginner-friendly as possible, while still retaining as much of the powerful customization which makes Proffie so popular.

## Getting Started

Head over to the [latest ProffieConfig release](https://github.com/ryancog/ProffieConfig/releases/latest), grab either the macOS or Windows (Win32) version, and run it! 

ProffieConfig will complete its installation and guide you through the setup and all of its features, so whether you're a first-time proffieboard user or have been around the block a few times, you'll be ready before you know it!

***NOTE:** ProffieConfig is relatively new software, and because I do not pay for an Apple Developer License or Microsoft Developer account I cannot "sign" the application, chances are something will tell you the program is unknown or "dangerous." See the info below about how you can address this if it comes up:*
- ***macOS:** Please visit this Apple Support page to read about macOS' security and how to launch unsigned apps [here](https://support.apple.com/en-us/102445)*
- ***Windows:** A SmartScreen dialog may pop up warning about the program origins, clicking "More Info..." and then "Run Anyways" should allow you to open the app.*
*The Chrome browser (and Chromium-based browsers) may block the program, which requires you to visit the "Show All Downloads" page to "keep" the file.*

## To-Do (Non-Exhaustive)
- Expand pre-checker capabilities (ongoing)
- Implement interface for `using` syntax (not just the ability to parse)
- Add blade nicknames
- Update power pin selection to new list style for blade awareness
- Add support for `DimBlade()`
- Add progress percentage while applying
- Add config percentage of usage
- Add warnings for soft config fails.
- Take over the world ;)

## Additional Info

- [COM ports >= 10 causing issues?](docs/com-ports.md)
- [Creating/Editing ProffieConfig's `pconf` files](docs/pconfs.md)
- [Building ProffieConfig for Linux](docs/linux-build.md)

## Gallery

<img src=https://github.com/ryancog/ProffieConfig/blob/master/screenshots/mainmenu.png width=200> 
<img src=https://github.com/ryancog/ProffieConfig/blob/master/screenshots/editor-general.png width=300>
<img src=https://github.com/ryancog/ProffieConfig/blob/master/screenshots/editor-propfile-fett263.png width=500>
<img src=https://github.com/ryancog/ProffieConfig/blob/master/screenshots/editor-bladearrays.png width=400>
<img src=https://github.com/ryancog/ProffieConfig/blob/master/screenshots/editor-bladeawareness.png width=400>
<img src=https://github.com/ryancog/ProffieConfig/blob/master/screenshots/editor-presetsstyles.png width=500>
