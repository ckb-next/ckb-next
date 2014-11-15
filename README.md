ckb: Corsair K70/K95 Driver for Linux and OSX
=============================================

ckb is divided into two parts: a daemon program which must be run as root and communicates with the USB device, and a utility program, which provides several animations and may be run as any user.

The daemon provides devices at `/dev/input/ckb*`, where * is the device number, starting at 1. Up to 9 keyboards may be connected at once (note: not tested...) and controlled independently. Hot-plugging is supported; if you unplug a keyboard while the daemon is running and then plug it back in, the keyboard's previous settings will be restored. If a keyboard is plugged in which has not yet been assigned any settings, its saved settings will be loaded from the hardware. The daemon additionally provides `/dev/input/ckb0`, which can be used to control keyboards when they are not plugged in. Settings are only remembered as long as the daemon is running; if you restart the daemon, all settings will be forgotten.

The user-runnable utility is currently very limited. It only supports one keyboard and has a limited selection of animations with little configuration. The plan is to replace it with a more robust Qt-based utility, creating something like Corsair's proprietary Windows controller.

Building for Linux
------------------

`libudev`, `gcc`, and `glibc` are required. Check with your package manager to make sure you have the correct libraries/headers installed (Note: on Ubuntu you need `libudev-dev`). You also need a kernel with uinput support (all distros should ship with this by default; don't worry about it unless you're running a custom kernel). You can build ckb by running `make` in the directory you downloaded it to. The binaries will be placed in `bin` assuming they compile successfully.

Building for OSX
----------------

You can build ckb by running `make` from Terminal in the directory you downloaded it to. You should be prompted to install developer tools if you don't have them already (you can also get them by downloading Xcode from the App Store). The binaries will be placed in `bin` assuming they compile successfully.

**Mac notes:**
- The keyboard devices are located at `/tmp/ckb*` and not `/dev/input/ckb*`. So wherever you see `/dev/input/ckb` in this document, replace it with `/tmp/ckb`.
- If you've rebound your modifier keys in System Preferences, those changes won't work anymore. You have to switch them using ckb's `bind` command. For instance, to switch Cmd and Ctrl, run: `echo bind lctrl:lwin lwin:lctrl rctrl:rwin rwin:rctrl > /tmp/ckb1/cmd`

Usage
-----

After building, run `sudo bin/ckb-daemon` to start the daemon. It will log some status messages to the terminal and you should now be able to access `/dev/input/ckb*`. The easiest way to see it in action is to open a new terminal in the same directory and run `bin/ckb [animation] [foreground] [background]` (or `bin/ckb` by itself to see a list of animations). `ckb` accepts colors in hexadecimal format (`RRGGBB`) or recognizes the names `white`, `black`, `red`, `yellow`, `green`, `cyan`, `blue`, and `magenta`.

`/dev/input/ckb0` contains the following files:
- `connected`: A list of all connected keyboards, one per line. Each line contains a device path followed by the device's serial number and its description.
- `cmd`: Keyboard controller. More information below.
- `notify0`: Keyboard notifications. See Notification section.

Other `ckb*` devices contain the following:
- `model`: Device description/model.
- `serial`: Device serial number. `model` and `serial` will match the info found in `ckb0/connected`
- `cmd`: Keyboard controller.
- `notify0`: Keyboard notifications.

Commands
--------

The `/dev/input/ckb*/cmd` nodes accept input in the form of text commands. They may be written by any user. Commands should be given in the following format:
`[device <serial>] [mode <n>] command1 [paramter1] [command2] [parameter2] [command3] [parameter3] ...`

In a terminal shell, you can do this with e.g. `echo foo > /dev/input/ckb1/cmd`. Programmatically, you can open and write them as regular files. When programming, you must append a newline character and flush the output before your command(s) will actually be read.

The `device` command, followed by the keyboard's serial number, specifies which keyboard to control. It is only required when issuing commands to `ckb0` or when controlling a keyboard that is not plugged in. In all other cases, the device is inferred from the control path. Additionally, the following commands may be issued to `ckb0` without any device: `layout`, `fps`, `notifyon`, and `notifyoff`. See below for documentation.

Keyboard layout
---------------

ckb currently supports both US and UK keyboard layouts. By default, the keyboard layout will be detected from the system locale. You can override this by specifying `--layout=<country>` at the command-line. For instance, `ckb-daemon --layout=uk` makes the UK layout default regardless of locale. You can also change it after starting the daemon by issuing `layout <country>` to `/dev/input/ckb0/cmd` (without including any device ID). It can be overwritten on a per-keyboard basis by issuing the `layout` command to `ckb*/cmd`. Sending the command to `ckb0` does not change the layout of any already-connected keyboards, it only changes the default.

Note that changing a keyboard's layout will reset all bindings and remove all macros associated with the keyboard. This is because not all keys are supported by all layouts (for instance, the UK has a separate hash key which is not present on the US layout). You can re-add the bindings manually after resetting the layout.

Profiles and modes
------------------

Keyboard settings are grouped into modes, where each mode has its own independent binding and lighting setup. By default, all commands will update the currently selected mode. The `mode <n>` command may be used to change the settings for a different mode. Up to 64 modes are available. Each keyboard has one profile, which may be given a name. Modes 1 through 3 may be saved to the device hardware (only mode 1 for K70s). Only the RGB settings can be saved, not the bindings or any other info. Commands are as follows:
- `profilename <name>` sets the profile's name. The name must be written without spaces; to add a space, use `%20`.
- `name <name>` sets the current mode's name. Use `mode <n> name <name>` to set a different mode's name.
- `mode <n> switch` switches the keyboard to mode N.
- `hwload` loads the RGB profile from the hardware. The profile's bindings are not affected.
- `hwsave` saves the RGB profile to the hardware.
- `erase` erases the current mode, resetting its lighting and bindings. Use `mode <n> erase` to erase a different mode.
- `eraseprofile` resets the entire profile, erasing its name and all of its modes.

LED commands
------------

The backlighting is controlled by the `rgb` commands. Any of the following combinations may be used:
- `rgb off` turns lighting off. No further color changes will take effect until you issue `rgb on`.
- `rgb on` turns lighting on.
- `rgb <RRGGBB>` sets the entire keyboard to the color specified by the hex constant RRGGBB.
- `rgb <key>:<RRGGBB>` sets the specified key to the specified hex color. See `src/ckb-daemon/keyboard_*.c` for a list of key names in each layout.

**Examples:**
- `rgb ffffff` makes the whole keyboard white.
- `rgb 000000` makes the whole keyboard black. This is NOT equivalent to `rgb off` as the keys are still considered "on" but are simply not lit.
- `rgb esc:ff0000` sets the Esc key red but leaves the rest of the keyboard unchanged.
Multiple keys may be changed to one color when separated with commas, for instance:
- `rgb w,a,s,d:0000ff` sets the WASD keys to blue.
Additionally, multiple commands may be combined into one, for instance:
- `rgb ffffff esc:ff0000 w,a,s,d:0000ff` sets the Esc key red, the WASD keys blue, and the rest of the keyboard white (note the lack of a key name before `ffffff`, implying the whole keyboard is to be set).

By default, the controller runs at 60FPS, meaning that attempts to animate the LEDs faster than that will be ignored. The keyboard's internal LED controller runs at 60Hz, so this should be a sensible maximum. If you wish to set it lower, you can start `ckb-daemon` with the `--fps=<rate>` option. You can also issue `fps <rate>` to `/dev/input/ckb0/cmd` after starting the daemon. Note that the FPS is global and cannot be set on a per-keyboard basis.

Indicators
----------

The indicator LEDs (Num Lock, Caps Lock, Scroll Lock) are controlled with the `i` commands.
- `ioff <led>` turns an indicator off permanently. Valid LED names are `num`, `caps`, and `scroll`.
- `ion <led>` turns an indicator on permanently.
- `iauto <led>` turns an indicator off or on automatically (default behavior).

Binding keys
------------

Keys may be rebound through use of the `bind` commands. Binding is a 1-to-1 operation that translates one keypress to a different keypress, regardless of circumstance; simple, but inflexible.
- `bind <key1>:<key2>` remaps key1 to key2. Again, see `src/ckb-daemon/keyboard_*.c` for a list of key names.
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

Notifications
-------------

The keyboard can be configured to generate user-readable notifications on keypress events. These are controlled with the `notify` commands. In order to see events, read from `/dev/input/ckb*/notify0`. In a terminal, you can do this like `cat /dev/input/ckb1/notify0`. Programmatically, you can open it for reading like a regular file.

Note that the file can only reliably be read by one application: if you try to open it in two different programs, they may both fail to get data. Data will be buffered as long as no programs are reading, so you will receive all unread notifications as soon as you open the file. If you'd like to read notifications from two separate applications, send the command `notifyon <n>` to the keyboard you wish to receive notifications from, where N is a number between 1 and 9. If `/dev/input/ckb*/notify<n>` does not already exist, it will be created, and you can read notifications from there without disrupting any other program currently reading them. To close a notification node, send `notifyoff <n>`.

Notifications are printed in the format of one notification per line. If you are reading from `ckb0/notify*` you will see notifications for all keyboards, with `device <serial>` printed at the beginning of each line. Reading from `ckb1` or above will show you only notifications for that keyboard, with no serial number.

Notification commands are as follows:
- `notify <key>:on` or simply `notify <key>` enables notifications for a key. Each key will generate two notifications: `key +<key>` when the key is pressed, and `key -<key>` when it is released.
- `notify <key>:off` turns notifications off for a key.

**Examples:**
- `notify w a s d` sends notifications whenever W, A, S, or D is pressed.
- `notify g1 g2 g3 g4 g5 g6 g7 g8 g9 g10 g11 g12 g13 g14 g15 g16 g17 g18 mr m1 m2 m3 light lock` prints a notification whenever a non-standard key is pressed.
- `notify all:off` turns all key notifications off.

**Note:** Key notifications are _not_ affected by bindings. For instance, if you run `echo bind a:b notify a > /dev/input/ckb1/cmd` and then press the A key, the notifications will read `key +a` `key -a`, despite the fact that the character printed on screen will be `b`. Likewise, unbinding a key or assigning a macro to a key does not affect the notifications.

Additionally, the following notifications will be generated at `ckb0/notify*` regardless of circumstance:
- `device <serial> added at <path>` whenever a device is connected.
- `device <serial> removed from <path>` whenever a device is disconnected. These messages are only generated at `ckb0/notify*`, not at the node for the actual device.
- `fps <rate>` when the FPS is changed.
- `layout <country>` when the default layout is changed.

Getting parameters
------------------

Parameters can be retrieved using the `get` command. The data will be sent out as a notification. Generally, the syntax to get the data associated with a command is `get :<command>` (note the colon), and the associated data will be returned in the form of `<command> <data>`. The following data may be gotten:
- `get :hello` simply prints `hello` to the notification nodes. This may be useful to determine whether or not the daemon is responding. It can only be issued to `ckb0` with no `device` command; in any other circumstance, it will be ignored.
- `get :fps` gets the current frame rate. Returns `fps <rate>`. Like `:hello`, this will be ignored if it is issued to an actual keyboard.
- `get :layout` gets the current keyboard layout. Returns `layout <country>`. This may be issued to `ckb0` to get the default layout or to any keyboard to get the keyboard's layout.

Security
--------

By default, all of the `ckb*` nodes may be accessed by any user. For most single-user systems this should not present any security issues, since only one person will have access to the computer anyway. However, if you'd like to restrict the users that can write to the `cmd` nodes or read from the `notify` nodes, you can specify the `--gid=<group>` option at start up. For instance, on most systems you could run `ckb-daemon --gid=1000` make them accessible only by the system's primary user. `ckb-daemon` must still be run as root, regardless of which `gid` you specify. The `gid` option may only be set at startup and cannot be changed while the daemon is running.

Known issues
------------

- When loading settings from the hardware, all three modes end up with the same RGB configuration. This is a hardware bug and it affects CUE as well.
- It's not possible for the daemon to determine which hardware mode was in use, so the keyboard always starts on mode 1. Again, this is a hardware issue (although thanks to the above problem, it doesn't really matter anyway).
- The Linux driver doesn't work 100% consistently and can be very slow to start up or shut down. This seems to be a driver/firmware conflict at an OS level, so it probably can't be fixed in ckb.
