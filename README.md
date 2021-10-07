# amigakb
Arduino Amiga keyboard interface

This is a bit of code that allows you to use Amiga hw keyboard as USB HID keyboard.
Useful for projects such as replacing Amiga internals with Raspberry.

---

This is a fork of jklof/amigakb adjusted for Vampire V4-SA

This should work with all Amiga keyboards that have their own controller (i.e. all keyboards except A600 and A1200):
- A500
- A1000 (early and later models)
- A2000/A3000
- A4000/CD32
- CDTV

The repository also contains [schematics and premade board files](boards) for building a "Pro Micro" based adapter that runs this code.
3D printable cases for the boards can be found at https://www.thingiverse.com/thing:4593520

Changes to the original code:
- Merged in changes from https://forum.arduino.cc/index.php?topic=139358.90
- Changed Backslash, Hash and Help key mappings
- Added reset handling for keyboards without reset line
- Made use of the caps lock special handling (for compensating differences between Amiga and USB keyboard protocols)
  configurable via a jumper (V4 expects key events differently than usual for USB keyboards)
  This can be switched with a jumper between pin 2 and ground.
- replaced digitalRead() with direct register usage to meet timing requirements for early A1000 keyboards
- changed handshake to work with early A1000 keyboards
- send data via USB during handshake pulse, in order to not miss next key event
- Added option to use a switch between pin 4 and ground to select an alternative keymap in order to make codes of keys available that do not exist on the amiga keyboard (currently only used for providing F11 and F12 (which are used by V4SA to toggle options) on F1 and F2)

Building for V4-SA:
- Use Arduino IDE v1.6.5-r5 (needed for the changes below - the last version before a refactoring of the USB stack).
  Note that if you have used a newer version, such as 1.8.x, it might prevent the 1.6.5 version from working due to it using
  incompatible compiler switches or adding incompatible packages.  If you get a linker error mentioning LTO (Link Time Optimisation)
  it will certainly be caused by this.  To resolve the issue, on Windows, search in your c:\Users\<UserName>\AppData directory
  for a directory called "Arduino15" and delete it.  There will probably be one in AppData\Local and one in AppData\Roaming.
  You might have to repeat this process to switch back to the newer Arduino IDE version.  On MacOS the directory can be found in
  ~/Library/Arduino15 and on Linux it is in ~/.arduino15.
- When installing the Pro Micro board add-on for Arduino IDE, do *not* use the automated installation method or it will recreate
  the Arduino15 directory and result in a non working build (as it installs the add-on for the latest 1.8.x version of the Arduino
  IDE).  Instead, download and install the board add-on manually, as described on the Sparkfun website at
  https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide/installing-windows under the section entitled "Installing the
  .brd Files Manually".  This is an older version of the add-on designed for older versions of the Arduino IDE
- Apply the change from https://github.com/tkoecker/Arduino/commit/e6783cb5e4b7e1dc8abc4932b3e3543ad793c6ff.patch to
  get an USB core that uses low-speed USB, with only one boot protocol keyboard HID device.  Experience has shown that some versions
  of patch do not work properly (at least on Windows), so look out for error messages such as "Hunk #X FAILED at XXX." or you'll
  be building an unpatched USB stack and will wonder why it doesn't work on the Vampire!

TODO:
- Add reset handling for A500 keyboard (i.e. keyboard with reset line) (optional as the current reset handling also is sufficient for A500 keyboards)
- Fix mapping of numpad '(' and ')' keys (Amiga keycodes 0x5a, and 0x5b) (these seem currently not to be available on V4)
- A500 keyboard: make LEDs controllable via USB (i.e. switch drive led on/off, switch power led bright/dim)
