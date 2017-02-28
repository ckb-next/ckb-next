## CMake install options

To pass an option to CMake use `-DOPTION=VALUE` syntax.

For example, to disable the animations and to build a release configuration one would write:

```cmake
cmake -DWITH_ANIMATIONS=OFF -DCMAKE_BUILD_TYPE=Debug ..
```

You can also configure the project in a GUI using `cmake-gui` and in a TUI using `ccmake` (if they are installed).

### Linux only

| Option                | Description                           | Default value   |
| :---------:           | :-------------:                       | :-------------: |
| LINUX_CUSTOM_INSTALL  | Manually install project files        | OFF             |
| WITH_MVIZ<sup>*</sup> | Build with "Music Visualizer"         | OFF             |

<sup>*</sup>requires PulseAudio

### macOS only

| Option                | Description                   | Default value   |
| :---------:           | :-------------:               | :-------------: |
| BREW_QT5<sup>*</sup>  | Use Homebrew package for Qt5  | ON              |

<sup>*</sup>if you turn `BREW_QT5` off, you must populate `CMAKE_PREFIX_PATH` option with a path to your Qt5 installation.

### Both

| Option            | Description                     | Default value   |
| :---------:       | :-------------:                 | :-------------: |
| CMAKE_BUILD_TYPE  | Type of the build. Possible values: "Debug" "Release" "MinSizeRel" "RelWithDebInfo" | RelWithDebInfo |
| LOCAL_QUAZIP      | Use system's QuaZip library     | ON              |
| ENFORCE_QUAZIP    | Use *only* system's QuaZip library without backtracking to the hardcoded one | OFF |
| WITH_GRADIENT     | Build with "Gradient" animation | ON              |
| WITH_HEAT         | Build with "Heat" animation     | ON              |
| WITH_PINWHEEL     | Build with "Pinwheel" animation | ON              |
| WITH_RAIN         | Build with "Rain" animation     | ON              |
| WITH_RANDOM       | Build with "Random" animation   | ON              |
| WITH_RIPPLE       | Build with "Ripple" animation   | ON              |
| WITH_WAVE         | Build with "Wave" animation     | ON              |
