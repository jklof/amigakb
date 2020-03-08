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
- Use Arduino IDE v1.6.5-r5 (needed for the changes below - the last version before a refactoring of the USB stack)
- Apply the change from https://github.com/tkoecker/Arduino/commit/e6783cb5e4b7e1dc8abc4932b3e3543ad793c6ff to
  get an USB core that uses low-speed USB, with only one boot protocol keyboard HID device

TODO:
- Add reset handling for A500 keyboard (i.e. keyboard with reset line) (optional as the current reset handling also is sufficient for A500 keyboards)
- Fix mapping of numpad '(' and ')' keys (Amiga keycodes 0x5a, and 0x5b) (these seem currently not to be available on V4)
- A500 keyboard: make LEDs controllable via USB (i.e. switch drive led on/off, switch power led bright/dim)
- Create a 3D printable case for the boards
