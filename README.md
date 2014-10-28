ckb: Corsair K70/K95 Driver for Linux
=====================================

ckb is divided into two parts: a daemon program, which must be run as root, to communicate with the USB device, and a utility program, which provides several animations and may be run as any user.

The daemon provides devices at `/dev/input/ckb*`, where * is the device number, starting at 1. Up to 9 keyboards may be connected at once (note: not tested...) and controlled independently. Hot-plugging is supported; if you unplug a keyboard while the daemon is running and then plug it back in, the keyboard's previous settings will be restored. The daemon additionally provides `/dev/input/ckb0`, which can be used to control a keyboard when it is not plugged in. Settings are only remembered as long as the daemon is running; if you restart the daemon, all settings will be forgotten.

The user-runnable utility is currently very limited. It only supports one keyboard and only supports a limited selection of animations with little configuration. The plan is to replace it with a more robust Qt-based utility, creating something like Corsair's proprietary Windows controller. However, there are some issues that need to be sorted out first (see Caveats below).

Building
--------

Only tested on Linux so far. It may work on OSX or Windows with some modification. Libusb-1.0 and GCC are required. Check with your package manager to make sure you have the correct libraries/headers installed. After that you can build ckb by running `make` in the directory you downloaded it to. The binaries will be placed in the "bin" directory assuming they compile successfully.

Usage
-----

Run ckb-daemon as root. It will log some status messages to the terminal and you should now be able to access `/dev/input/ckb*`. The easiest way to see it in action is to run ckb (as any user) and specify an effect, foreground, or background color. ckb accepts colors in hexadecimal format or recognizes the names "white", "black", "red", "yellow", "green", "cyan", "blue", and "magenta".

As for the daemon, `/dev/input/ckb0` contains the following files:
- `connected`: A list of all connected keyboards, one per line. Each line contains a device path followed by the device's serial number and its description.
- `led`: LED controller. More information below.

Other ckb devices contain the following:
- `model`: Device description/model.
- `serial`: Device serial number. These are the same as the information listed in `/dev/input/ckb0/connected`
- `led`: LED controller.

The LED controllers are writeable by any user and accept input in the form of text commands. Write one command per line in the following format:
`[serial] command [parameter]`

The serial number is required when issuing commands to ckb0 and specifies which device to control. If the device is not present, the settings will take effect when it is plugged in. When writing to ckb1 and above, the setting is optional, as the default behavior is to control the device specified by the path. `command` may be one of the following:
- `off` turns all lighting off. It does not take a parameter.
- `on` turns lighting on, restoring previously-applied colors (if any). It does not take a parameter.
- `rgb` turns lighting on and also changes key colors. You may specify a single hex color (RRGGBB) to set the whole keyboard; for instance, "rgb ff0000" will turn the whole keyboard red. Alternatively, you may specify key/color combinations in a format like "key1:color1,key2:color2,key3:color3" and so on. See src/ckb_daemon/main.c for a list of key names. Multiple keys may be set to the same color by writing "key1,key2:color" instead of "key1:color,key2:color". The special name "all" may be used to change all keys; for instance, "rgb all:ffffff,w,a,s,d:0000ff,esc:ff0000" will set the WASD keys to blue, the Esc key to red, and all other keys to white.

Caveats
-------

The caps lock and scroll lock LEDs don't work on my computer (Arch Linux). Num Lock is fine, and the actual function of the keys is unimpaired. I don't know if this is a universal problem or not. I noticed that if I run a Windows VM in VMware, the lights work correctly when I move the mouse into the VM screen...even if the keyboard is connected to Linux and not Windows. This may actually be a problem in the kernel or Xorg. I'm still in the process of researching it.

There's some code near line 482 in the daemon source that sends an 07:04:02:00... packet to the device. Without this code, the MR/M1/M2/M3 buttons as well as the Brightness and Win Lock buttons are hardware controlled. The selected M-button is highlighted with the inverse color assigned to it (i.e. if the driver assigns black, it becomes white), and pressing a different M-button will switch profiles, disregarding the colors assigned by the driver unless you switch back or the driver reassigns them. *With* this packet the keyboard seems to activate software control for these buttons, as their colors now display normally. Unfortunately, pressing any one of them causes the keyboard to freeze and stops all input/output. The same thing happens when pressing any of the G-buttons, with or without the packet. I assume the keyboard is waiting for some kind of response from the driver, but I have yet to figure out what it is.
