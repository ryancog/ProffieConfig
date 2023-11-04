# General Page

The General Page is the first page you'll be greeted by when you open ProffieConfig, and it's where several important settings live.

## Board Setup

Here, select the version of Proffieboard you have from the dropdown. This can easily be changed, but it has to match the board you choose to apply the configuration to later.

There's two other options here: "Enable Mass Storage" and "Enable WebUSB"

**"Enable Mass Storage"** allows you to access the contents of your SD card (to add/remove/change files) when plugging in your Proffieboard with a USB cable, instead of needing to remove the SD card from the board, and plug it directly into your computer.

**"Enable WebUSB"** enables a feature where, if your board is connected via USB to your computer, you can access the [ProffieOS Workbench](https://pod.hubbe.net/tools/workbench.html).

## Options

- **"Number of Buttons"**: The number of buttons connected to your saber. This assumes you've connected your buttons to the respective Button1, Button2, and/or Button3 pads.
- **"Max Volume"**: This setting is an arbitrary value for setting the maximum volume of your saber (you can raise and lower the volume up to this value while using the saber). Generally, 2000 is a good value for most speakers, though it may be possible to increase this value for certain speakers. Caution should be used when increasing the volume, as too high of a volume may damage your speaker. Lower volumes can be set based on preference.
- **"Clash Threshold"**: This sets the intensity of impact that must be experienced by the saber to trigger a clash effect. The value is measured in Gs, and can be modified to user preference.
- **"PLI Timeout"**: This sets the amount of time a PLI will remain active on the saber after the last interaction, assuming one is connected, measured in minutes.
- **"Idle Timeout"**: This sets the amount of time any accent LEDs and OLED animations will remain active on the saber after the last interaction, measured in minutes.
- **"Motion Timeout"**: This sets the amount of time before guesture controls are disabled after the last interaction, measured in minutes.

- **"Save Volume"**: After disconnecting power, choose whether to save the volume the saber was set to or not.
- **"Save Preset"**: After disconnecting power, choose whether to save the last preset the saber was set to or not.
- **"Save Color"**: Choose whether to save color changes after switching preset or not.
- **"Disable Color Change"**: Disable color change functionality.
- **"Disable Talkie"**: Removes the "Talkie" spoken prompts and error messages and replaces them with [beeps](https://pod.hubbe.net/troubleshooting/beep_codes.html). Can be used to save FLASH memory if prompted.
- **"Disable Basic Parser Styles"**: Removes the option to use "Basic Parser Styles" in the ProffieOS Workbench, can be used to save FLASH memory if promted.
- **"Disable Diagnostic Commands"**: Removes certain commands used with the Arduino Serial Monitor, recommended unless commands are needed, can be used to save FLASH memory if prompted.
- **"Enable Developer Commands"**: Enables special commands used for Proffieboard development, this probably shouldn't be enabled. Should be disabled to save FLASH memory.
- **"Neopixel Max LEDs"**: This sets the maximum size of a neopixel blade to be configured in the "Blades" page. Unless you have a blade longer than this, it should not be changed.
