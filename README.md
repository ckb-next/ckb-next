ckb: Corsair K70/K95 Driver for Linux
=====================================

ckb is divided into two parts: a daemon program which must be run as root and communicates with the USB device, and a utility program, which provides several animations and may be run as any user.

The daemon provides devices at `/dev/input/ckb*`, where * is the device number, starting at 1. Up to 9 keyboards may be connected at once (note: not tested...) and controlled independently. Hot-plugging is supported; if you unplug a keyboard while the daemon is running and then plug it back in, the keyboard's previous settings will be restored. The daemon additionally provides `/dev/input/ckb0`, which can be used to control keyboards when they are not plugged in. Settings are only remembered as long as the daemon is running; if you restart the daemon, all settings will be forgotten.

The G-keys on the K95 cannot currently be remapped, but using this driver WILL fix the bug where pressing a G-key will cause the entire keyboard to freeze. It is also not currently possible to use the Brightness or Win Lock keys, or the K95's memory keys, with this driver. Support for those is coming soon.

The user-runnable utility is currently very limited. It only supports one keyboard and has a limited selection of animations with little configuration. The plan is to replace it with a more robust Qt-based utility, creating something like Corsair's proprietary Windows controller.

Building
--------

ckb only supports Linux. OSX support is planned (tentatively). `libusb-1.0` and `gcc` are required. Check with your package manager to make sure you have the correct libraries/headers installed. After that you can build ckb by running `make` in the directory you downloaded it to. The binaries will be placed in `bin` assuming they compile successfully.

Usage
-----

Run `ckb-daemon` as root. It will log some status messages to the terminal and you should now be able to access `/dev/input/ckb*`. The easiest way to see it in action is to run `ckb` (as any user) and specify an effect and foreground/background colors. `ckb` accepts colors in hexadecimal format (`RRGGBB`) or recognizes the names `white`, `black`, `red`, `yellow`, `green`, `cyan`, `blue`, and `magenta`.

`/dev/input/ckb0` contains the following files:
- `connected`: A list of all connected keyboards, one per line. Each line contains a device path followed by the device's serial number and its description.
- `cmd`: Keyboard controller. More information below.

Other `ckb*` devices contain the following:
- `model`: Device description/model.
- `serial`: Device serial number. `model` and `serial` will match the info found in `ckb0/connected`
- `cmd`: Keyboard controller.

Commands
--------

The `/dev/input/ckb*/cmd` nodes accept input in the form of text commands. They may be written by any user. Commands should be given in the following format:
`[serial] command1 [paramter1] [command2] [parameter2] [command3] [parameter3] ...`

In a terminal shell, you can do this with e.g. `echo foo > /dev/input/ckb1/cmd`. Programmatically, you can open and write them as regular files. When programming, you must append a newline character and flush the output before your command(s) will actually be read.

The `serial` parameter, representing a keyboard's serial number, is required when issuing commands to `ckb0`. It is unnecessary if writing to `ckb1` or any other path with an actual keyboard. If a keyboard with the given serial number isn't connected, the settings will be applied to that keyboard when it is plugged in.

LED commands
------------

The backlighting is controlled by the `rgb` commands. Any of the following combinations may be used:
- `rgb off` turns lighting off. No further color changes will take effect until you issue `rgb on`.
- `rgb on` turns lighting on.
- `rgb <RRGGBB>` sets the entire keyboard to the color specified by the hex constant RRGGBB.
- `rgb <key>:<RRGGBB>` sets the specified key to the specified hex color. See `src/ckb-daemon/keyboard.c` for a list of key names.

**Examples:**
- `rgb ffffff` makes the whole keyboard white.
- `rgb 000000` makes the whole keyboard black. This is NOT equivalent to `rgb off` as the keys are still considered "on" but are simply not lit.
- `rgb esc:ff0000` sets the Esc key red but leaves the rest of the keyboard unchanged.
Multiple keys may be changed to one color when separated with commas, for instance:
- `rgb w,a,s,d:0000ff` sets the WASD keys to blue.
Additionally, multiple commands may be combined into one, for instance:
- `rgb ffffff esc:ff0000 w,a,s,d:0000ff` sets the Esc key red, the WASD keys blue, and the rest of the keyboard white (note the lack of a key name before `ffffff`, implying the whole keyboard is to be set).

Binding keys
------------

Keys may be rebound through use of the `bind` commands. Binding is a 1-to-1 operation that translates one keypress to a different keypress, regardless of circumstance; simple, but inflexible.
- `bind <key1>:<key2>` remaps key1 to key2. Again, see `src/ckb-daemon/keyboard.c` for a list of key names.
- `unbind <key>` unbinds a key, causing it to lose all function.
- `rebind <key>` resets a key, returning it to its default binding.

**Examples:**
- `bind g1:esc` makes G1 become an alternate Esc key (the actual Esc key is not changed).
- `bind caps:tab tab:caps` switches the functions of the Tab and Caps Lock keys.
- `unbind lwin rwin` disables both Windows keys, even without using the keyboard's Windows Lock function.
- `rebind all` resets the whole keyboard to its default bindings.

Key macros
----------

Macros are a more advanced form of key binding, controlled with the `macro` command.
- `macro <keys>:<command>` binds a key combination to a command, where the command is a series of key presses. To combine keys, separate them with `+`; for instance, `lctrl+a` binds a macro to (left) Ctrl+A. In the command field, enter `+<key>` to trigger a key down or `-<key>` to trigger a key up. To simulate a key press, use `+<key>,-<key>`.
- `macro <keys>:clear` clears commands associated with a key combination. Only one macro may be assigned per combination; assigning a second one will overwrite the first.
- `macro clear` clears all macros.

**Examples:**
- `macro g1:+lctrl,+a,-a,-lctrl` triggers a Ctrl+A when G1 is pressed.
- `macro g2+g3:+lalt,+f4,-f4,-lalt` triggers an Alt+F4 when both G1 and G2 are pressed.

Assigning a macro to a key will cause its binding to be ignored; for instance, `macro a:+b,-b` will cause A to generate a B character regardless of its binding. However, `macro lctrl+a:+b,-b` will cause A to generate a B only when Ctrl is also held down. Macros currently do not have any repeating options and will be triggered only once, when the key is pressed down. This feature will be added soon.

Caveat emptor
-------------

The firmware on these boards is extremely buggy and your keyboard may not work immediately or at all. I'm still trying to sort this out, but it may not be fixable.
