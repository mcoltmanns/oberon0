# A Compiler for the Oberon-0 Programming Language

The [Oberon](https://www.ethoberon.ethz.ch) programming language was proposed in 1987 by 
[Niklaus Wirth](https://people.inf.ethz.ch/wirth/) as a successor to Pascal and Modula-2. 
Due to this lineage, Oberon is an ALGOL-like language with strong (static and dynamic) 
typing discipline. The programming paradigm of Oberon can be classified as imperative, 
structured, modular, and object-oriented.

## About

This project provides the skeleton of a compiler for the Oberon-0 programming language, a 
subset of the full Oberon programming language, as described in Niklaus Wirth's book 
["Compiler Construction"](http://www.ethoberon.ethz.ch/WirthPubl/CBEAll.pdf) (Chapter 6, 
pp. 30-32). The sekelton is written in C++ and serves as the starting point of the 
programming project that accompanies the the MSc course "Compiler Construction" taught at 
the [University of Konstanz](https://uni.kn). The goal of this project is for students to 
design and develop a parser and intermediate representation for the Oberon-0 programming 
language. In order to generate executable code, they will transform their high-level 
intermediate representation into the low-level intermediate representation of the 
[LLVM](http://llvm.org) Compiler Infrastructure. 

## Dependencies and Toolchains

Owing to its origin as a course project, care has been taken that the provided C++ 
sourcecode can be compiled on different operating systems and with different toolchains. 
Currently, the sourcecode only depends on [Boost](https://www.boost.org) and [LLVM](https://llvm.org).
As of February 2022, the following configurations are tested and known to work.

|      | macOS              | Windows              | Linux (Ubuntu) |
|------|--------------------|----------------------|----------------|
|Boost | 1.74.0             | 1.74.0               | 1.74.0         |
|LLVM  | 13.0.0             | 13.0.0               | 13.0.0         |
|CMake | 3.21.1             | 3.19.2               | 3.22.2         |
|IDE   | CLion 2021.3.3     | Visual Studio 17.1.0 | CLion 2021.3.3 |
|CXX   | Apple Clang 13.0.0 | CL 19.31.31104       | GCC 11.2.0     |

## Installing Boost

### macOS

Using [Homebrew](https://brew.sh), Boost can be installed as follows.
```
> brew install boost
```

### Linux

Using the [Advanced Package Tool (APT)](https://wiki.debian.org/Apt), Boost can be installed as follows.
```
> sudo apt install boost
```

### Windows

It is recommended to use Microsoft's [vcpkg](https://github.com/microsoft/vcpkg) C++ Library Manager for Windows in order to install the required Boost components.

1. Install vcpkg by following the steps below.

   ```
   > git clone https://github.com/Microsoft/vcpkg.git
   > cd vcpkg
   > .\bootstrap-vcpkg.bat
   ```

2. Configure automatic, user-wide integration of libraries managed by vcpkg with Visual Studio 2019.

   ```
   > .\vcpkg integrate install
   ```

2. Use vcpkg to install the required Boost libraries (`system`, `convert`, `filesystem`, and `program_options`).

   ```
   > .\vcpkg install boost-system:x64-windows
   > .\vcpkg install boost-convert:x64-windows
   > .\vcpkg install boost-filesystem:x64-windows
   > .\vcpkg install boost-program-options:x64-windows
   ```

## Installing LLVM

### macOS
```
> brew install llvm
```

### Linux
```
> sudo apt install llvm
```

### Windows

Using the Microsoft C++ Library Manager is the simplest way to install LLVM on Windows (x64). There are, however, some drawbacks. First, vcpkg will compile and install both the release and the debug version of LLVM, which requires 70-80 GB of free harddisk space during installation. Second, it typically takes a while before a new release of LLVM is integrated into the vcpkg distribution. Should any of these two factors present a problem, please refer to the instructions given below on how to build LLVM from the sources.

Assuming, you already followed the steps described above to install Boost using vcpkg, LLVM can be compiled and installed with the following command.

   ```
   > .\vcpkg install llvm:x64-windows
   ```
 


