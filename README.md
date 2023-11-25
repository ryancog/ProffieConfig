# ProffieConfig 

<img align="left" src=/resources/icons/icon.svg width=200> 
  
All-In-One ProffieBoard (Created by Fredrik Hubbinette) utility for customization. Handles the installation of all tools needed in the background, and presents an easy to use GUI for configuration and applying changes (flashing) to the board.

Using this tool, there is no need for the Arduino IDE, Zadig, etc. in an attempt to make using a Proffieboard as beginner-friendly as possible, while still retaining as much of the powerful customization which makes proffie so popular.

Utilzes `arduino-cli` and `proffie-dfu-setup` for driver installation and flashing.

## Usage

To get started, head over to [the docs](/docs/README.md) which contain all the information needed to setup and use ProffieConfig.
The tool is fairly straightforward, however it's recommended to read these over to ensure there is no confusion or unexpected issues when using the tool.

## To-Do (Non-Exhaustive)
- Implement Blade Detect and Blade ID
- Implement SSD1306
- Implement tooltips for more detailed setting explanation
- Implement dialog showing current button configuration (depending on prop and `NUM_BUTTONS`)
- Add a confirmation on close to prevent losing changes
- Add an option to rearrange preset order
- Implement interface for `using` syntax (not just the ability to parse)

## Gallery

<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/f88be6ab-cd8b-4cd9-9d04-35a0732d2d71 height=500>
<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/4714b8e5-8267-44b8-9524-a8a4ef7e9a58 width=500>
<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/497ea717-481e-4a39-86b1-0f82ee49632c width=500>
<img src=https://github.com/ryryog25/ProffieConfig/assets/60193408/98c8bd1b-4b09-401f-a3e8-5a6f0b123d24 width=500>
