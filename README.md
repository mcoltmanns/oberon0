# A Compiler for the Oberon-0 Programming Language

The [Oberon](https://www.ethoberon.ethz.ch) programming language was proposed in 1987 by
[Niklaus Wirth](https://people.inf.ethz.ch/wirth/) as a successor to Pascal and Modula-2. Due to this lineage, Oberon is
an ALGOL-like language with strong (static and dynamic) typing discipline. The programming paradigm of Oberon can be 
classified as imperative, structured, modular, and object-oriented.

## About

This project is an implementation of an Oberon-0 compiler in C++ for the [LLVM](http://llvm.org) backend. It strives to implement the language as defined in Wirth's book ["Compiler Construction"](http://www.ethoberon.ethz.ch/WirthPubl/CBEAll.pdf) (Chapter 6, pp. 30-32). It was initially developed as a submission of the programming project that accompanies the M.Sc course "Compiler Construction" at the [University of Konstanz](https://uni.kn), as taught by [Prof. Dr. Michael Grossniklaus](https://dbis.uni-konstanz.de/people/people/grossniklaus/).

## Compatibility

The provided C++ sourcecode can be compiled on different operating systems and with different toolchains. Currently, the
sourcecode only depends on [Boost](https://www.boost.org) and [LLVM](https://llvm.org). As of January 2026, the 
following configurations are tested and known to work.

|       | macOS              | Windows              | Linux (Ubuntu) | Linux (Void) |
|-------|--------------------|----------------------|----------------|--------------|
| Boost | 1.74.0             | 1.74.0               | 1.74.0         | 1.90.0       |
| LLVM  | 13.0.0             | 13.0.0               | 13.0.0         | 21.1.7       |
| CMake | 3.21.1             | 3.19.2               | 3.22.2         | 4.2.2        |
| IDE   | CLion 2021.3.3     | Visual Studio 17.1.0 | CLion 2021.3.3 | NVIM         |
| CXX   | Apple Clang 13.0.0 | CL 19.31.31104       | GCC 11.2.0     | GCC 14.2.1   |

## Prerequisites

In order to build the compiler for the Oberon-0 programming language, Boost and LLVM have to be installed. Step-by-step 
installation instructions for macOS, Linux, and Windows are given below.

### macOS

Using [Homebrew](https://brew.sh), Boost and LLVM can be installed as follows.
```
> brew install boost
> brew install llvm
```

### Linux

Using the [Advanced Package Tool (APT)](https://wiki.debian.org/Apt), Boost and LLVM can be installed as follows.
```
> sudo apt install boost
> sudo apt install llvm
```

### Windows

It is recommended to use Microsoft's [vcpkg](https://github.com/microsoft/vcpkg) C++ Library Manager for Windows in
order to install Boost and LLVM.

1. Install vcpkg by following the steps below.
   ```
   > git clone https://github.com/microsoft/vcpkg.git
   > cd vcpkg
   > .\bootstrap-vcpkg.bat
   ```
2. Configure automatic, user-wide integration of libraries managed by vcpkg with Visual Studio.
   ```
   > .\vcpkg integrate install
   ```
3. Use vcpkg to install the following Boost libraries: `system`, `convert`, `filesystem`, and `program_options`.
   ```
   > .\vcpkg install boost-system:x64-windows
   > .\vcpkg install boost-convert:x64-windows
   > .\vcpkg install boost-filesystem:x64-windows
   > .\vcpkg install boost-program-options:x64-windows
   ```
4. Using the Microsoft C++ Library Manager is also the simplest way to install LLVM on Windows (x64). Assuming that you 
   already followed the steps described above to install Boost using vcpkg, LLVM can be compiled and installed with the 
   following command.
   ```
   > .\vcpkg install llvm:x64-windows
   ```
   There are, however, some drawbacks to installing LLVM using the Microsoft C++ Library Manager. First, vcpkg will 
   compile and install both the release and the debug version of LLVM, which requires 70-80 GB of free hard-disk space 
   during installation. Second, it typically takes a while before a new release of LLVM is integrated into the vcpkg 
   distribution. Should any of these two factors present a problem, please refer to the instructions on how to build 
   LLVM from the sources in the [LLVM Demo](https://gitlab.inf.uni-konstanz.de/dbis/education/llvm-demo) project.
 
## Cloning the Project

In order to clone the repository, use the following command.
```
> git clone https://github.com/mcoltmanns/oberon0.git
```
This will create a new directory `oberon0` in the current directory. This new directory is referred to as the *root 
directory* of the project.  

## Building the Project

For macOS and Linux, [CLion](https://www.jetbrains.com/clion/) is recommended as a development environment. For Windows, 
the recommended development environment is [Visual Studio](https://visualstudio.microsoft.com/vs/). Alternatively, 
[CMake](https://cmake.org) can be used directly on the command line to build the project.

### CLion

In order to build the compiler for the Oberon-0 programming language using CLion, follow the steps below.

1. Start CLion and select `Open` in the welcome dialog.
2. Navigate to the root directory of the project, e.g., `oberon0c`, to which you cloned the project repository and 
   click `Open`.
4. Once the project is open, build it using the `Build` → `Build Project` menu.

**Note** Under macOS on Apple Silicon (e.g., M1), it may be required to pass the command line argument 
`-DCMAKE_OSX_ARCHITECTURES=arm64` to CMake to ensure that an ARM64 binary is built.

Once the build successfully terminates, the `oberon0c` executable can be found in the `oberon0c/cmake-build-debug` or 
`oberon0c/cmake-build-release` subdirectory.

### Visual Studio

In order to build the compiler for the Oberon-0 programming language using Visual Studio, follow the steps below.

1. Start Visual Studio and select `File` → `Open` → `Folder...` from the menu or press `Ctrl`+`Shift`+`Alt`+`O`.
2. Navigate to the root directory of the project, e.g., `oberon0c`, to which you cloned the project repository and 
   click `Select Folder`.
4. Visual Studio will now run CMake to set up the project. Make sure that the build configuration in the toolbar of 
   Visual Studio matches the build type specified earlier. For example, if LLVM was compiled in `Release` mode, Visual 
   Studio's build configuration has to be `x64-Release`.
5. Start the build using the `Build` → `Build All` menu or press `Ctrl`+`Shift`+`B`.

Once the build successfully terminates, the `oberon0c.exe` executable can be found in the 
`oberon0c\out\build\x64-Debug` or `oberon0c\out\build\x64-Release` directory.

### CMake

In order to build the compiler for the Oberon-0 programming language using CMake, follow the steps below.

1. On the command line, navigate to the root directory of the project, e.g., `oberon0`. The root directory of the 
   project is the directory that was created when you cloned the GitLab repository.
2. Create a build directory.
   ```
   > mkdir build
   ```
3. Navigate to the build directory and run CMake to configure the project and generate a native build system.
   ```
   > cd build
   > cmake ..
   ```
4. Call the build system to compile and link the project.
   ```
   > cmake --build .
   ```

Once the build successfully terminates, the executable of the compiler for the Oberon-0 programming language can be 
found in the `build` directory. 

### Use
Currently, compilation to LLVM IR and object files is supported.

To compile a program to LLVM IR:
```
> oberon0c Program.Mod ll
```

To compile a program to an object file:
```
> oberon0c Program.Mod o
```

On some linux platforms, compilation to an object file causes problems with position-independent code. To circumvent this bug,
it is recommended to first compile to LLVM IR and then use clang to produce the final executable:
```
> oberon0c Program.Mod ll
> clang -fPIE Program.ll
```
