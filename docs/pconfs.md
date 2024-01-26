# Prop Files in ProffieConfig

One thing I realized fairly early on was that having prop files hard-coded into my tool was silly. 

I'm not the one making these prop files, what if someone wants to add another prop file (like the blaster prop?), what if whomever makes one of the prop files I support by default wants to change something?

Understanding the C++ the tool is written in would be challenging just to add a prop file, and at the time the code didn't lend itself to that.

Now, new props can be added and existing props can be modified through the use of what I'm calling `pconf` files.

## What is a `pconf` File?

`.pconf` is the file extension for ProffieConfig's configs (nice).

They mostly follow the same syntax, but the ones we're interested in are those under ProffieConfig's `props` folder, in `resources`.

NOTE: In macOS, this folder is inside the ProffieConfig app. Right Click on the app and click "Show Package Contents", then you can find the "Resources" folder in there.

## What does a `pconf` look like?

Each `pconf` file's syntax follows the same basic idea. There are "entries" and there are "sections," and both can have "labels."

Comments in `pconf` files can be used, are single-line only, and use the `//` style syntax. `//` will mark it and everything after it in the line as a comment, and will be ignored by the parser.

### What is an Entry?

An entry is pretty simple. It's a token. Entries don't actually need anything else (we'll get to this). However, in most cases the token is followed by `:`, and then some info.

For example: `NAME: "Caiwyn"`

It's important to note `pconf`s *are* fundamentally line-based, so each entry must stay on it's own line. (I recommend turning on word wrap in your editor if you need it...)

### What is a Section?

A section is a collection of other sections or entries. This is easiest to show alongside a `pconf` snippet:

```
LAYOUT {
	HORIZONTAL {
		VERTICAL("Controls") {
			OPTION("CAIWYN_BUTTON_LOCKUP")
			OPTION("CAIWYN_BUTTON_CLASH")
		}
		VERTICAL("Settings") {
			OPTION("CAIWYN_SAVE_TRACKS")
			OPTION("CAIWYN_SAVE_TRACK_MODE")
		}
	}
}
```

Here, `LAYOUT`, `HORIZONTAL`, and the two `VERTICAL`s are all sections, and at the very end we find some `OPTION` entries. This shows how sections and entries can be nested within other sections.

It's worth noting that, although it's not shown here, each "level" of a section can contain both entries and sections, the two types are not mutually exclusive.

### So then, what about labels?

In the previous code snippet, both the `VERTICAL` sections and `OPTION` entries had labels, I'll put part of that again here:

```
VERTICAL("Controls") {
	OPTION("CAIWYN_BUTTON_LOCKUP")
	OPTION("CAIWYN_BUTTON_CLASH")
}
```

Here, we can see the usage of labels. Labels exist (mostly) in the form of parenthesis directly after the entry name (no whitespace), and then the label itself.

There's a "substyle" to this also. Labels specified with parenthesis denote a string label (hence the `"` above), but labels can also be specified with `{}` in a similar fashion, and these denote numerical labels.

The usage of labels in `pconf`s is pretty widespread, and they're used in a variety of ways, but they always follow these two styles.

It's also important to note, in the case of entries, a `:`, if used, should go *after* the label, and info can be added as normal.

## How Do I Edit A Prop File `pconf`?

First, let's open a `pconf`, I'll use Caiwyn's as an example for now.

NOTE: `pconf` files can be open with any normal text editor. Personally I like Sublime Text, and the "Groovy" syntax highlighting seems to work pretty well ;)

### The `NAME` Entry

This one's pretty simple, and required:

```
NAME: "Caiwyn"
```

The entry is `NAME`, and the token is the name which will appear in the drop-down in ProffieConfig. Ideally this will be *pretty* and easily identify the prop.

### The `FILENAME` Entry

This one's also pretty simple, and also required:

```
FILENAME: "saber_caiwyn_buttons.h"
```

The entry is `FILENAME`, and the token is the name of the prop header file in ProffieOS.

Notice that, unlike in the config file, this doesn't have the `../props`, that part is filled in by ProffieConfig automatically.

### The `INFO` Section

Here we've reached our first optional part of the `pconf`.

The Info section is unique in the `pconf` syntax because unlike any other section, it is comprised of neither other entries nor sections, instead the text is directly placed in, line by line: 

```
INFO {
	"Caiwyn's Lightsaber Controls"
	"This config is designed to provide fast, low-latency response to button"
	"presses for the most basic functions (blaster blocks, lockups, and clashes)"
	...
}
```

This section will be displayed whenever a user clicks the "Info" button within ProffieConfig, and the idea is to provide a description of the prop file and include any important information for the prop.

Each line should be surrounded by `"`.

NOTE: Currently the `pconf` parser will treat links with `https://` as comments due to the `//`.

### The `SETTINGS` Section

Another optional section, this is where it gets interesting ;)

The settings section contains all the `#define`s a prop file supports, and there's 4 different kinds of settings: `TOGGLE`, `OPTION`, `NUMERIC`, and `DECIMAL`

Here's an example:
```
TOGGLE("CAIWYN_BUTTON_LOCKUP") {
	NAME: "Lockup With Button"
	DESCRIPTION: "Trigger a lockup by holding the Power button"
}
```

We can see the setting type is the key for the section, and it's followed by a label which holds the name of the `#define`.

#### Setting Entries

Every setting requires the `NAME` entry, along with the aforementioned setup.

**`NAME`** is the name of the setting to be displayed in ProffieConfig. Most of the time this is just the define name with `_` removed and proper casing/spacing.

Following that, every setting supports the following entries: `DESCRIPTION`, `REQUIRE`, and `REQUIREANY`

**`DESCRIPTION`** is the text to be shown in the tooltip when a user hovers over the option, and is ***HIGHLY RECOMMENDED*** to fully explain the setting to the user and remove any potential confusion. 

*NOTE: Tooltips are an important gateway to understanding in ProffieConfig's design philosophy (to fill in the gaps in simply, well-designed UX to be self-explanatory)*

`REQUIRE` and `REQUIREANY` are mutually-exclusive entries; only one can be used at a time.

**`REQUIRE`** is a list of other `TOGGLE` and/or `OPTION` type `#define`s in the prop which **must** be selected in order to enable the given setting. All `#define`s are placed on the same line, and should be comma separated.

**`REQUIREANY`** is similar to `REQUIRE` in that it is also a list of specifically `TOGGLE` and/or `OPTION` type `#define`s in the prop. However, for these, if **any** of them are selected the given setting will be enabled.

Note that for both `REQUIRE` and `REQUIREANY`, there can be one or many `#define`s in the list.

#### The `TOGGLE` Setting

This is the most common type of setting, and it's pretty simple. It's a checkbox in ProffieConfig, and if enabled, the define will be put into the config file, if not, it simply won't be added.

The `TOGGLE` section supports an additional entry: `DISABLE`, with identical syntax to `REQUIRE`/`REQUIREANY`, it specifies setting(s) which will be disabled if this setting is checked.

#### The `OPTION` Setting

This one's kind of unique. This is a section which contains `SELECTION` sections, each of which are "Radio" checkboxes, meaning that, out of the list, only one can be selected at a time.

An Example:
```
OPTION {
	SELECTION("NO_EDIT_MODE") {
		NAME: "No Edit Mode"
		OUTPUT: FALSE
	}
	SELECTION("FETT263_EDIT_MODE_MENU") {
		NAME: "Edit Mode"
		DESCRIPTION: "Enable Edit Mode Menu System\nRequires ENABLE_ALL_EDIT_OPTIONS"
	}
	SELECTION("FETT263_EDIT_SETINGS_MENU") {
		NAME: "Edit Settings"
		DESCRIPTION: "Enable Edit Settings Menu (Volume, Clash Threshold, Blade Length, Gestures/Controls, Brightness)\nRequires ENABLE_ALL_EDIT_OPTIONS" 
	}
}
```

The `OPTION` setting itself doesn't take a label, instead each `SELECTION` does. 

A `SELECTION` should be thought of as a `TOGGLE`, just within an `OPTION`. `SELECTION`s have all the same possible entries as `TOGGLE`.

`SELECTION`s support one additional entry, however: `OUTPUT`.

The default for `OUTPUT` is `TRUE` if not specified, and essentially means whether ProffieConfig should actually output this define to the config file. (`FALSE` should be used if we don't want output)

The reason for this is to allow for logically laying out options to users. In the example above, which is for Fett263's "Edit Modes," the default is to have neither mode active, but given the modes are mutually exclusive and in the same vein, it makes sense to group them in an `OPTION`.

However, since the default is for an "Edit Mode" to be "off", there's no `#define` for it, so we specify a label (in this case, `NO_EDIT_MODE`) purely in order to keep track of it in the `pconf` (and within ProffieConfig), but then we don't output anything if that option is set.

#### The `NUMERIC` Setting

This setting is for defines which have a numeric value associated with them, for example:

```
NUMERIC("FETT263_SWING_ON_SPEED") {
	NAME: "Swing on Speed"
	DESCRIPTION: "Adjust Swing Speed required for Ignition\n250 ~ 500 recommended"
	MIN: 0
	MAX: 1000
	DEFAULT: 250
	INCREMENT: 10
	REQUIREANY: "FETT263_SWING_ON", "FETT263_SWING_ON_PREON"
}
```

Most of the time these will probably only be specified along with another `TOGGLE` or `SELECTION`, so `REQUIRE` or `REQUIREANY` is pretty common here, but not required (haha, punny).

`NUMERIC` settings support the following additional entries: `MIN`, `MAX`, `DEFAULT`, and `INCREMENT`.

`MIN` is the minimum value for the define. (defaults to 0)

`MAX` is the maximum value for the define. (defaults to 100)

`DEFAULT` is the default value for the define. (defaults to `MIN`, this is the value the setting will start at)

`INCREMENT` is the amount the setting should increment/decrement when a user clicks the `+` and `-` buttons beside the setting in ProffieConfig. (defaults to 1)

Users can still enter their own value into the box directly, but `INCREMENT` is intended for making settings more pleasant to click through when, as is the case in the given example, the range of values is very large.

#### The `DECIMAL` Setting

Completely identical to `NUMERIC`, but supports decimal value input from the user and when specifying the entries.

## How do I make a `pconf`

Assuming the preceding information has been read, then it's as simple as either creating a new text file with the `pconf` extension (or copying one of the existing prop `pconf`s), and adding all the necessary *stuff* into it.

However, this isn't enough for ProffieConfig to recognize and use it.
This may change in the future, but as of now, you must find the `.state.pconf` file in the same "resources" folder (You must have hidden files shown in your file explorer), and add the `pconf` filename in the associated `PROPS` section.

### Properly Add A Prop `pconf` to ProffieConfig

If you create a `pconf` or (if you're the prop file maker) edit one, and add it to your ProffieConfig, it'll only be there for you.

I would highly recommend and appreciate it if you "fork" the ProffieConfig GitHub repository, add or make your changes there in the `resources/props` folder, and create a PR.

I can then add your changes into ProffieConfig properly and create a new release with the updates. Additionally, if you're creating a new prop file, I can add it as a default into ProffieConfig's source code, so `.state.pconf` doesn't need to be manually edited.