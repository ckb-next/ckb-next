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
- `led`: LED controller. More information below.

Other `ckb*` devices contain the following:
- `model`: Device description/model.
- `serial`: Device serial number. `model` and `serial` will match the info found in `ckb0/connected`
- `led`: LED controller.

The LED controllers may be written by any user and accept input in the form of text commands. Write one command per line in the following format:
`[serial] command [parameter]`

In a terminal shell, you can do this with `echo foo > /dev/input/ckb0/led`. Programmatically, you can open and write them as regular files. Be sure to call `fflush()` or an equivalent function so that your output is actually written.

The `serial` parameter, representing a keyboard's serial number, is required when issuing commands to `ckb0`. If a keyboard with that number isn't connected, the settings will take effect when it is plugged in. When writing to `ckb1` and above, the setting is optional; by default, it will use the serial number at that path. `command` may be one of the following:
- `off` turns all lighting off. It does not take a parameter.
- `on` turns lighting on, restoring previously-applied colors (if any). It does not take a parameter.
- `rgb` turns lighting on and also changes key colors. The parameter may be a single hex color; for instance, `rgb ff0000` will turn the whole keyboard red. Alternatively, you may specify key/color combinations, in a format like `key1:color1,key2:color2,key3:color3` and so on. For instance, `rgb esc:00ff00` will turn Esc green without affecting other keys. See `src/ckb_daemon/keyboard.c` for a list of key names. Multiple keys may be set to the same color by writing `key1,key2:color` instead of `key1:color,key2:color`. The special name `all` may be used to change all keys; for instance, `rgb all:ffffff,w,a,s,d:0000ff,enter:ff0000` will set the WASD keys to blue, the enter key to red, and all other keys to white.

Caveat emptor
-------------

The firmware on these boards is extremely buggy and your keyboard may not work immediately or at all. I'm still trying to sort this out, but it may not be fixable.
