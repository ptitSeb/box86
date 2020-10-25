# The directory containing all PKGBUILDs for BOX86
## FAQ
### 1. How to use it?
You just jump to this directory in your local git repository and type this command:
```sh
cp ./PKGBUILD-<your-platform> ../PKGBUILD
cd ..
makepkg -si
```
This will copy the PKGBUILD for your platform (where you type your platform name instead of <your-platform>), build it and install the PKGBUILD on 32-bit Archlinux ARM. If you don't know your platform name, just check the directory for all PKGBUILD and find the one you want to install the box86 with.

In the future, I'll try to do an unified PKGBUILD for all platforms that will recognize your platform automatically.
### 2. What platforms are currently supported.
Only RPi4B right now, but that will change in the nearest future.
### 3. Any advantages with that sollution?
This might give you some advantages like those:
- automatic updates (with some AUR frontends like [`pamac-aur`](https://aur.archlinux.org/packages/pamac-aur/));
- easily uninstall with `pacman -R` when no needed (no need other scripts nor Makefile to do this);
- your `box86` installation is recognized as a package (where it wasn't when installing with `make install`).
