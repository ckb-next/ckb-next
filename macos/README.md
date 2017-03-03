# General information

All files required to create a .pkg for macOS lie here. The package is created on macOS with [Packages](http://s.sudre.free.fr/Software/Packages/about.html) tool, its template is saved under `ckb-next/ckb-next.pkgproj`.

# Preparation

```bash
git clone https://github.com/mattanger/ckb-next.git
cd ckb-next
mkdir build
cd build
sudo ../macos/uninstall.sh
cmake -G Ninja .. && ninja && sudo ninja install  # Ninja is superior
# cmake .. && make && sudo make install  # to use Makefiles
sudo chown -R $(whoami):staff /Applications/ckb-next.app  # cmake can't chown
```

# Building

After executing the commands above you are ready to proceed with the Packages application. To generate the package, open the template using Packages and choose `Build -> Build` in the Menu Bar. However the testing process is inevitable, so you can choose `Build -> Build and Run` immediately.

Don't forget to:
* run `Build -> Clean`. You don't want a package to report an error just because you didn't clean up and the settings interfere
* update `VERSION_MAJOR`, `VERSION_MAJOR` and `VERSION_PATCH` **manually** in the main `CMakeLists.txt` after every annotated tag created in git

As a result you'll see `ckb-next.pkg` in `ckb-next/build/ckb-next.pkg`.
