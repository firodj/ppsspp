# Disasm (Standalone)

## MacOS:

```
$ cd build-xcode
$ cmake -G Xcode ..
$ cmake --build . --target disasm && ./Debug/disasm --fn start
```

To create distributed Lib:
```
$ cd build-ninja
$ cmake -G Ninja ..
$ cmake --build . --target disasm_combined
```

## Windows MSYS/UCRT64:

Requirement:

```
$ pacman -S mingw-w64-ucrt-x86_64-gcc \
    mingw-w64-ucrt-x86_64-cmake \
    mingw-w64-ucrt-x86_64-ninja
$ # For golang:
$ pacman -S mingw-w64-ucrt-x86_64-go
```

```
> GOROOT=C:\msys64\ucrt64\lib\go
```

```
$ cd build-ucrt
$ cmake -G Ninja ..
$ cmake --build . --target disasm_combined
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

The basicblock (bb) genereated by PPSSPP emulator is missing some part and
some bb is not normalized. Not normalized by meant within a single bb
there is possible another entry point in the middle of bb. Consider
follwing case. At first we only have two bb's, A and B.

```
BB A:

01 x
02 x
03 JMP

BB B:

04 x
05 x
06 JMP 02
```

But BB A, has other entry point onther than 01, which is 02, ref by 06. So
BB A should become:

```
BB A:

01 x

BB C:

02 x
03 JMP
```

