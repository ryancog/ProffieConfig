# Blades Page

This is where any blade or LED on your saber will be configured, including any accent LEDs.

# Blades

This is a list of all blades currently set up for your saber, regardless of type. There must be at least one blade at all times.

Clicking "+" will add a new blade, clicking "-" will remove the currently-selected blade.

# SubBlades

This is a list, in the case of NeoPixel blades, where any created subblades will be displayed for configuration. If there are any, there must be at least two (otherwise there's no point).

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
- **"Tri-Star Cree"**: If you have a "baselit" type saber, chances are this is what you'll want. *Power Pins are assigned in order of LED*
  - "Star *x* Color": For each LED on your Star, choose a color.
  - "Resistance": For each LED on your star, enter in the Ohm value used on the LED. This isn't critical, but will help account for differences in brightness when color mixing.
- **"Quad-Star Cree"**: If you have a baselit saber with white (or another color) you probably have a quad-star. The options remain the same, simply with a forth LED to set up.
- **"Single Color"**: If you have a normal type LED (single diode) or something similar, you can use the MOSFETs to control it with a Single Color option. (PWM direct-driven LED config is planned)


## Power Pins

Each blade you setup should have Power Pins selected, though the selection process varies by blade type. This should follow the pins you've soldered to, but here's a quick reference, and a quick explanation.

For NeoPixel blades, most of the time you should have at least two pads for full-length blades. Small accent strips can use just one. If you have seperate NeoPixel blades (Subblades by their nature share power pins) they can share pins (assuming they physically use the same pads), but this comes at the cost of having all the LEDs the pins are connected to turned on, even if they're not being used. With NeoPixels, the power draw while off is non-negligible, so while you might want to share power pins for maybe quillions on a crossguard, it is ill-advised to have an accent share power pins with a blade, as that means the blade would be kept on for as long as the accent is, likely greatly increasing power draw.

For In-Hilt LED blades and Single Color blades, the number of power pins should equal the number of LEDs you're configuring. The power pins are configured in LED order, and while you can select more than the number of LEDs you're setting up, only the first will apply. For example, if I'm setting up a Tri-Star Cree, and I select pins 2, 3, 4, and 5, LED 1 will use Power Pin 2, LED 2 will use Pin 3, LED 3 will use Pin 4, and 5 will not be used, despite being selected. Similarly, if I have a Single-Color LED, only the first numerical Pin will be used, so if I had 3,4, and 5 selected, only 3 would be used.
