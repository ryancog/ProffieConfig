# ProffieConfig FAQ

## What is a config file?

The ProffieOS config file is what holds all the information about your saber setup. It contains (mostly) all the settings you configure within ProffieConfig, and then some.

The code for your styles, the layout/setup of your blades, the the options you choose for things like volume, the prop options, etc. all get stored in the config file, which is then read when the ProffieOS code is compiled (fancy computer term for "turned into a program," in this case, for the Proffieboard to run).

## Where do I get blade styles?

- If you have an existing config, try importing it into ProffieConfig! That's the easiest way to import all your settings, including the blade styles.
- Some sound fonts you buy may have a blade style included with them, see if yours does.
- For some very cool, premade bladestyles, check out [Fett263's Style Library](https://fett263.com/fett263-proffieOS7-style-library.html).
- For advanced use, the [ProffieOS Style Editor](https://profezzorn.github.io/ProffieOS-StyleEditor/style_editor.html) offers unparalleled customization, but has a very steep learning curve and requires quite a bit of effort to get going.

## What if my board doesn't show up?

If your Proffieboard isn't showing up in the device drop down, please make sure you've run the [first-time setup](/docs/firstsetup.md).

If the Proffieboard is unresponsive or it was accidentally unplugged during configuration application, locate the [boot and reset buttons](https://fredrik.hubbe.net/lightsaber/v6/pinout.svg) on the Proffieboard. Now, with the board plugged into your computer, hold BOOT, push on RESET, release BOOT, and try refreshing again. You should see an entry labeled "BOOTLOADER" and will be able to reapply your config and be back off to the races!

## Still Need Help?

[Open an issue](https://github.com/ryryog25/ProffieConfig/issues/new) to get support.
