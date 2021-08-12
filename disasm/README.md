# Disasm (Standalone)

MacOS:

```
$ cd build-xcode
$ cmake -G Xcode ..
$ cmake --build . --target disasm && ./Debug/disasm --fn start
```

To Create distributed Lib:
```
$ cd build-ninja
$ cmake -G Ninja ..
$ cmake --build . --target disasm_combined
$ cmake --build . --target disasm2
```

# BBTrace

BBTrace is Basic Block Trace.

* Core/BBTrace.cpp
* Core/BBTrace.h

Each `PSPThread` will have their own `BBTrace`. Started by `__KernelResetThread`.

The bbtrace will record only start of Basic Block's PC, with `RecordPC`.
Currently only Interpret CPU mode in `MIPSInterpret_RunUntil`.

The trace will be `Flush`ed after several traces by `DumpBBTrace` appended
into `*.rec` file.

Forced `Flush` also done by `__KernelDeleteThread` and `__KernelSwitchContext`.

Metadata:

Upon loading executable by `__KernelLoadELFFromPtr`:

* SoraDumpModule: information about module loaded, memory
segments.
* SoraDumpHLE: appended information related HLE function ID, name.
* SoraDumpMemory: PSP RAM Memory consist of loaded executable
  and already mapped.

This way we don't need to parse ELF file again when doing analysis.
