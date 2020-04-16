Compiling
----
#### for Pandora

 `mkdir build; cd build; cmake .. -DPANDORA=1; make`

#### for Pyra

 `mkdir build; cd build; cmake .. -DPYRA=1; make`

#### for Gameshell

`mkdir build; cd build; cmake .. -DGAMESHELL=1; make`

#### for RaspberryPI

  _a build for model 2, 3 and 4 can be done. Model 1 and 0 cannot (at least not with Dynarec, as they lack NEON support)_
 
`mkdir build; cd build; cmake .. -DRPI4=1; make`
 
  _For Pi4. Change to RPI2 or RPI3 for other models._

#### for ODROID

`mkdir build; cd build; cmake .. -DODROID=1; make`

#### for Other ARM Linux

 `mkdir build; cd build; cmake .. -DARM_DYNAREC=ON; make`

#### for x86 Linux

 `mkdir build; cd build; cmake .. -DLD80BITS=1 -DNOALIGN=1; make`

#### use ccmake

Alternatively, you can use the curses-bases ccmake (or any other gui frontend for cmake) to select wich platform to use interactively.

#### Customize your build

*use ccache if you have it*
Add `-DUSE_CCACHE=1` if you have ccache (it's better if you plan to touch the sources)


*to have a Trace Enabled build*
To have a trace enabled build (warning, it will be slower), add `-DHAVE_TRACE=1` but you will need, at runtime, to have the [Zydis library](https://github.com/zyantific/zydis) library in your `LD_LIBRARY_PATH` or in the system lib folders.

*to have ARM Dynarec*
The Dynarec is only avaiable on ARM Cpu. Notes also that VFPv3 and NEON are required for the Dynarec. Activate it by using `-DARM_DYNAREC=1`. Also, be sure to use `-marm` in compilation flags (because many compileur use Thumb as default, and the dynarec will not work in this mode).

----

Testing
----
A few tests are included.
They can be launched with `ctest`
They are very basic and don't test much for now.

