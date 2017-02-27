## CMake install options

To pass an option to CMake use `-DOPTION=VALUE` syntax.

For example, to disable the animations and to build a release configuration one would write:

```cmake
cmake -DWITH_ANIMATIONS=OFF -DCMAKE_BUILD_TYPE=Debug ..
```

You can also configure the project in a GUI using `cmake-gui` and in a TUI using `ccmake` (if they are installed).

### Linux only

| Option                | Description                   | Default value   |
| :---------:           | :-------------:               | :-------------: |
|LINUX_CUSTOM_INSTALL   |Manually install project files |OFF              |

### macOS only

| Option                | Description                   | Default value   |
| :---------:           | :-------------:               | :-------------: |
| BREW_QT5<sup>*</sup>  |Use Homebrew package for Qt5   |ON               |

<sup>*</sup>if you turn `BREW_QT5` off, you must populate `CMAKE_PREFIX_PATH` option with a path to your Qt5 installation.

### Both

| Option            | Description                   | Default value   |
| :---------:       | :-------------:               | :-------------: |
| CMAKE_BUILD_TYPE  | Type of the build. Possible values: "Debug" "Release" "MinSizeRel" "RelWithDebInfo" | RelWithDebInfo |
| WITH_ANIMATIONS   | Build and install animations  | ON              |
| LOCAL_QUAZIP      | Use system's QuaZip library   | ON              |
| ENFORCE_QUAZIP    | Use only system's QuaZip library without backtracking to the hardcoded one | OFF |
