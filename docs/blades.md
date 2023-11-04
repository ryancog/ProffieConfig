# Blades Page

This is where any blade or LED on your saber will be configured, including any accent LEDs.

# Blades

This is a list of all blades currently set up for your saber, regardless of type. There must be at least one blade at all times.

Clicking "+" will add a new blade, clicking "-" will remove the currently-selected blade.

# SubBlades

This is a list of, in the case of NeoPixel blades, where any created subblades will be listed for configuration. If there are any, there must be at least two (otherwise there's no point).

Clicking "+" will add a new subblade, clicking "-" will remove the currently-selected subblade. If there are only two subblades, "-" will remove both, but this will not remove the parent blade or its configuration.

# Blade Type

The drop-down selects a blade's type, which can be one of the following:

- **"NeoPixel (RGB)"**: This will be used for most NeoPixel blades (which use Red, Green, and Blue channels), if your blade has LEDs going up along the inside of the blade, chances are you should choose this type; it supports the follow options.
  - "Color Order": Most of the time this will be "GRB," only change this if you know your color order is different (rare).
  - "Blade Data Pin": Which pin your blade is connected to on the Proffieboard, uses the names found on [the pin table](https://fredrik.hubbe.net/lightsaber/v6/), if you're using a pin not in the dropdown (e.g. one of the Free pins on the V3), you can type in your own pin to use in this box.
  - "Number of Pixels": The number of LEDs in your blade (if using subblades, this is the total number of LEDs for the whole blade, not for one subblade or another)
- **"Neopixel (RGBW)"**: On certain NeoPixel blades, there is an additional white LED which allows for purer white color. If you have a blade which supports this, choose this type; it supports the options of "NeoPixel (RGB)" with the following differences:
  - "Color Order": Has an additional "W" placed in the order to represent that channel. Most of the time this will be "GRBW," only change this if you know your color is different.
  - "Use RGB with White": If this is enabled, whenever "White" is displayed on the blade, the White channel **and** the RGB channels will be turned on. This can make the blade brighter, but at the cost of battery life and a "pure" white color.
- **"Tri-Star Cree"**: If you have a "baselit" 
