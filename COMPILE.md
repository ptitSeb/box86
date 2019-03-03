Compiling
----
*for Pandora*

 `mkdir build; cd build; cmake .. -DPANDORA=1; make`
    
*or for Other ARM Linux*

 `mkdir build; cd build; cmake .. -DLD80BITS=1 -DNOALIGN=1; make`

*or for x86 Linux*

 `mkdir build; cd build; cmake .. ; make`

*or use ccmake*

Alternatively, you can use the curses-bases ccmake (or any other gui frontend for cmake) to select wich platform to use interactively.

*use ccache if you have it*
Add `-DUSE_CCACHE=1` if you have ccache (it's better if you plan to touch the sources)

*to have a Trace Enabled build*
To have a trace enabled build (warning, it will be slower), add `-DHAVE_TRACE=1` but you will need, at runtime, to have the [Zydis library](https://github.com/zyantific/zydis) library in your `LD_LIBRARY_PATH` or in the system lib folders.

----

Testing
----
A few tests are included.
They can be launched with `ctest`
They are very basic and don't test much for now.

