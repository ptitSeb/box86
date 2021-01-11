Compiling
----
#### for Pandora

 `mkdir build; cd build; cmake .. -DPANDORA=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for Pyra

 `mkdir build; cd build; cmake .. -DPYRA=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for Gameshell

`mkdir build; cd build; cmake .. -DGAMESHELL=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for RaspberryPI

  _a build for model 2, 3 and 4 can be done. Model 1 and 0 cannot (at least not with Dynarec, as they lack NEON support)_
 
```
git clone https://github.com/ptitSeb/box86
cd box86
mkdir build; cd build; cmake .. -DRPI4=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j4
sudo make install
sudo systemctl restart systemd-binfmt
```
 
  _For Pi4. Change to RPI2 or RPI3 for other models._

#### for ODROID

`mkdir build; cd build; cmake .. -DODROID=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for RK3399

`mkdir build; cd build; cmake .. -DRK3399=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make -j4`

As most RK3399 device run AARCH64 OS, you'll need an `armhf` multiarch environment, and an armhf gcc: On debian, install it with `sudo apt install gcc-arm-linux-gnueabihf`.

#### for Other ARM Linux platforms

 `mkdir build; cd build; cmake .. -DARM_DYNAREC=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

#### for x86 Linux

 `mkdir build; cd build; cmake .. -DLD80BITS=1 -DNOALIGN=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo; make`

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

The Dynarec is only available on the ARM architecture(Right now, anyways.). Notes also that VFPv3 and NEON are required for the Dynarec. Activate it by using `-DARM_DYNAREC=1`. Also, be sure to use `-marm` in compilation flags (because many compileur use Thumb as default, and the dynarec will not work in this mode).

*not building from a git clone*

If you are not building from a git clone (for example, downloading a release source zipped from github), you need to activate `-DNOGIT=1` from cmake to be able to build (normal process include git sha1 of HEAD in the version that box86 print).

----

Testing
----
A few tests are included.
They can be launched with `ctest`
They are very basic and don't test much for now.

