# Com Ports >= 10 Fail to Upload normally (Require Bootloader Mode)

@TheTomas from the r/lightsabers Discord has discovered a wonderfully annoying little issue. I've not yet fully investigated, but it seems to be either a glaring oversight somewhere in my code (possible, but I don't think so), or something really stupid in the Windows API. You're not here to hear me rant though, so I digress.

If your proffieboard ends up using a `COM` port >= 10 (e.g. `COM10`, `COM11`, etc.), then it seems (on some computers at least), a normal upload will not work, and you'll get the error "Could not connect to proffieboard for upload."

There are two workarounds:
- Manually enter bootloader mode
- Change com port number

The former is the easier. Simply reboot the board into bootloader mode (You can do this via holding the `BOOT` button then pressing `RESET`, or by typing `RebootDFU` into the Serial Monitor), and then uploading using the `BOOTLOADER RECOVERY` option.

Alternatively, and credit for this goes to @TheTomas, you can find the proffieboard in your Device Manager underneath the "COM & LPT Ports" section. There you should find an option for "Port Settings," then "Advanced," and from there you should be able to choose a new port number, with those in use (and thus the ones to avoid) being marked as such.

![image](https://github.com/user-attachments/assets/853084e1-a1e7-4ac5-926d-8d1dfa0711da)
