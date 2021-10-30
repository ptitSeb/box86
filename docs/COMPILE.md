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
  Note that you can use `make -j2` or more to speed up 1st build, but beware of the memory requirement if you go to high.
 
#### for Raspberry Pi on 64bits OS

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

`mkdir build; cd build; cmake .. -DODROID=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

#### for RK3399

`mkdir build; cd build; cmake .. -DRK3399=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j4`

As most RK3399 device run AARCH64 OS, you'll need an `armhf` multiarch environment, and an armhf gcc: On debian, install it with `sudo apt install gcc-arm-linux-gnueabihf`.

#### for Allwinner A64

`mkdir build; cd build; cmake .. -DA64=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

As most Allwinner A64 device run AARCH64 OS, you'll need an `armhf` multiarch environment, and an armhf gcc: On debian, install it with `sudo apt install gcc-arm-linux-gnueabihf`.

#### for Snapdragon 845

`mkdir build; cd build; cmake .. -DSD845=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j4`

As most Snapdragon 845 device run AARCH64 OS, in most cases you'll need an `armhf` multiarch environment, and an armhf gcc: On mobian, install it with `sudo apt install gcc-arm-linux-gnueabihf`.

#### for Phytium

`mkdir build; cd build; cmake .. -DPHYTIUM=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j4`

As most Phytium (D2000 or FT2000/4) device run AARCH64 OS, you'll need an `armhf` multiarch environment, and an armhf gcc: On debian/ubuntu, install it with `sudo apt install gcc-arm-linux-gnueabihf`.

#### for Other ARM Linux platforms

 `mkdir build; cd build; cmake .. -DARM_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

#### for x86 Linux

 `mkdir build; cd build; cmake .. -DLD80BITS=1 -DNOALIGN=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j$(nproc)`

#### use ccmake

Alternatively, you can use the curses-bases ccmake (or any other gui frontend for cmake) to select wich platform to use interactively.

#### Customize your build

*use ccache if you have it* 

Add `-DUSE_CCACHE=1` if you have ccache (it's better if you plan to touch the sources)

*have some debug info* 

The `-DCMAKE_BUILD_TYPE=RelWithDebInfo` argument makes a build that is both optimized for speed, and has debug information embedded. That way, if you have a crash or try to analyse performance, you'll have some symbols.

*to have a Trace Enabled build* 

To have a trace enabled build ( ***it will be slower***), add `-DHAVE_TRACE=1` but you will need, at runtime, to have the [Zydis library](https://github.com/zyantific/zydis) library in your `LD_LIBRARY_PATH` or in the system library folders.

*to have ARM Dynarec*

The Dynarec is only available on the ARM architecture(Right now, anyways.). Notes also that VFPv3 and NEON are required for the Dynarec. Activate it by using `-DARM_DYNAREC=1`. Also, be sure to use `-marm` in compilation flags (because many compileur use Thumb as default, and the dynarec will not work in this mode). Note that compiling with `ARM_DYNAREC` but without hardware profile is not advise. If you get error building that "target CPU does not support ARM mode", then try to pick hardware profile (like ODROID for armv7 or PI4 for armv8). Also, if you are using a 64bits with armhf multiarch, it's much easier to pick one of the hardware profile like `RPI4ARM64`, `RK3399`, `PHYTIUM` or `SD845`.

*not building from a git clone*

If you are not building from a git clone (for example, downloading a release source zipped from github), you need to activate `-DNOGIT=1` from cmake to be able to build (normal process include git sha1 of HEAD in the version that box86 print).

----

Testing
----
A few tests are included.
They can be launched with `ctest`
They are very basic and don't test much for now.

----

Note about devices with Tegra X1 and newer.

Nvidia don't provide armhf libraries for their GPU drivers at this moment so there is no special variable to compile it for them as it would be missleading for many people. If you still want to use it wihout GPU acceleration, building it with RPI4 configuration should work just fine. As instalation of Mesa can break Nvidia driver, safest option is usage of chroot.
