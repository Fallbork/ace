# deco
'deco' (drifter's embeddable content organizer) files are meant to provide an easy way to distribute resources along with any raylib project. 

'lsqueezer' is a tool to create texture atlases at runtime. It comes as a module of 'deco', with complete support to it.

Both these libraries work with any C/C++ project, as long as the compiler is capable of handling C++17; they use std::filesystem internally to scan and read files in the most native, cross-platform way possible.
Compression is done making use of [zstd](https://github.com/facebook/zstd "Zstandard's GitHub repository")'s dictionary feature, to compress data ranging from a couple bytes up to several MBs.

# Building and usage
To build the examples, simply execute `premake-vs2019.bat`(Windows) or run:
```
cd <clone-directory>
premake5 <flag>
```

Examples are available both in C and C++. To change between them, comment/uncomment the following directive inside `example/Common.h`:
```c
#define _EX_CPP
```

Once the `*.deco` file has been generated, simply ship it along with the executable, inside the relative path specified during `Init()`.

**NOTE**: You need at least *5* different files for this to work. Less than that and zstd will **refuse** to create the dictionary.

# Licensing
'deco' and 'lsqueezer' are both licensed under the [BSD-3-Clause License](https://github.com/Fallbork/deco/blob/main/LICENSE). 'zstd' is dual-licensed under [BSD](https://github.com/facebook/zstd/blob/dev/LICENSE) and [GPLv2](https://github.com/facebook/zstd/blob/dev/COPYING); for this project we chose the BSD license :)
