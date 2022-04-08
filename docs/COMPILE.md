Compiling
----
#### for Pandora

 `mkdir build; cd build; cmake .. -DPANDORA=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for Pyra

 `mkdir build; cd build; cmake .. -DPYRA=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for Gameshell

`mkdir build; cd build; cmake .. -DGAMESHELL=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for Raspberry Pi

  _a build for model 2, 3 and 4 can be done. Model 1 and 0 cannot (at least not with Dynarec, as they lack NEON support)_
 
```
git clone https://github.com/ptitSeb/box86
cd box86
mkdir build; cd build; cmake .. -DRPI4=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make
sudo make install
sudo systemctl restart systemd-binfmt
```
###### *Note: You can use `make -j2`, `-j3`, etc. to speed up 1st build, but beware of the memory requirement if you go too high.*
 
#### for Raspberry Pi on 64bit OS

  _For Pi4. Change to RPI2 or RPI3 for other models.  Change `-DRPI4=1` to `-DRPI4ARM64=1` for compiling on arm64. (armhf multiarch or chroot required alongside armhf gcc. Install it with 'sudo apt install gcc-arm-linux-gnueabihf'.)_

```
git clone https://github.com/ptitSeb/box86
cd box86
mkdir build; cd build; cmake .. -DRPI4ARM64=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j2
sudo make install
sudo systemctl restart systemd-binfmt
```

#### for ODROID

`mkdir build; cd build; cmake .. -DODROID=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j3`

#### for RK3399

`mkdir build; cd build; cmake .. -DRK3399=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j3`

As most RK3399 devices run AARCH64 OS, you'll need an `armhf` multiarch environment, and an armhf gcc: On debian, install it with `sudo apt install gcc-arm-linux-gnueabihf`. Also, on armbian, you may need to also use `sudo apt install libc6-dev-armhf-cross` or you may have an issue with `crt1.o` and a few other similar file not found at link.

#### for Tinker Board (1/1S) or RK3288

`mkdir build; cd build; cmake .. -DRK3288=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j3`

#### for Allwinner A64

`mkdir build; cd build; cmake .. -DA64=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j3`

As most Allwinner A64 device run AARCH64 OS, you'll need an `armhf` multiarch environment, and an armhf gcc: On debian, install it with `sudo apt install gcc-arm-linux-gnueabihf`. Also, on armbian, you may need to also use `sudo apt install libc6-dev-armhf-cross` or you may have an issue with `crt1.o` and a few other similar file not found at link.

#### for Snapdragon 845

`mkdir build; cd build; cmake .. -DSD845=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j4`

As most Snapdragon 845 device run AARCH64 OS, in most cases you'll need an `armhf` multiarch environment, and an armhf gcc: On mobian, install it with `sudo apt install gcc-arm-linux-gnueabihf`.

#### for Phytium

`mkdir build; cd build; cmake .. -DPHYTIUM=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j4`

As most Phytium (D2000 or FT2000/4) device run AARCH64 OS, you'll need an `armhf` multiarch environment, and an armhf gcc: On debian/ubuntu, install it with `sudo apt install gcc-arm-linux-gnueabihf`. Also, on armbian, you may need to also use `sudo apt install libc6-dev-armhf-cross` or you may have an issue with `crt1.o` and a few other similar file not found at link.

#### for Other ARM Linux platforms

 `mkdir build; cd build; cmake .. -DARM_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j3`

#### for x86 Linux

 `mkdir build; cd build; cmake .. -DCMAKE_C_FLAGS="-m32" -DLD80BITS=1 -DNOALIGN=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j3`

### use ccmake

Alternatively, you can use the curses-bases ccmake (or any other gui frontend for cmake) to select wich platform to use interactively.

### Customize your build

#### use ccache if you have it

Add `-DUSE_CCACHE=1` if you have ccache (it's better if you plan to touch the sources)

#### To have some debug info

The `-DCMAKE_BUILD_TYPE=RelWithDebInfo` argument makes a build that is both optimized for speed, and has debug information embedded. That way, if you have a crash or try to analyse performance, you'll have some symbols.

#### To have a Trace Enabled build

To have a trace enabled build (***the interpreter will be slightly slower***), add `-DHAVE_TRACE=1`. But you will need the [Zydis library](https://github.com/zyantific/zydis) in your `LD_LIBRARY_PATH` or in the system library folders at runtime.

#### to have ARM Dynarec

###### *Note: VFPv3 and NEON are required for Dynarec.*

###### *Note: Compiling with `ARM_DYNAREC` without selecting a hardware profile is not advised.*

Dynarec is only available on the ARM architecture (for the meantime anyways). Activate it by using `-DARM_DYNAREC=1`.

You will most likely need `-marm` in compilation flags (Many compilers default to the Thumb instruction set and Dynarec does not support this).

###### *Note: If you get error building that "target CPU does not support ARM mode", then try to pick a hardware profile (like ODROID for armv7 or PI4 for armv8).*

##### 64bit OS with Dynarec

If you are using a 64bit OS with armhf multiarch, it's much easier to pick one of the hardware profiles.

`RPI4ARM64`, `RK3399`, `PHYTIUM` or `SD845`.

#### Not building from a git clone
If you are not building from a git clone (for example, downloading a release source code zip from github), you need to use `-DNOGIT=1` from cmake to be able to build (box86 uses git SHA1 to show last commit in version number).

----

Testing
----
A few tests are included with box86.

They can be launched using the `ctest` command.

The tests are very basic and only tests some functionality for now.

----

Note about devices with Tegra X1 and newer.

NVIDIA doesn't provide armhf libraries for their GPU drivers at this moment. There is no special variable to compile it for them, as it would be misleading for many people. If you still want to use it wihout GPU acceleration, building it with RPI4 configuration should work just fine. Installation of Mesa can break the NVIDIA driver, so the safest option is to use a chroot environment.

----

Debian Packaging
----
Box86 can also be packaged into a .deb file with `DEB_BUILD_OPTIONS=nostrip dpkg-buildpackage -us -uc -nc`.

