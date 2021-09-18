# deco
'deco' (drifter's embeddable content organizer) files are meant to provide an easy way to distribute resources along with any raylib project. 

'lsqueezer' is a tool to create texture atlases at runtime. It comes as a module of 'deco', with complete support to it.

Both these libraries work with any C/C++ project, as long as the compiler is capable of handling C++17; they use std::filesystem internally to scan and read files in the most native, cross-platform way possible.
Compression is done making use of [zstd](https://github.com/facebook/zstd "Zstandard's GitHub repository")'s dictionary feature, to compress data ranging from a couple bytes up to several MBs.

# Usage (deco)
```cpp
/* (1) Create a '.deco' file
   Recommended: Check if a '.deco' file doesn't exist already. This is an expensive function.*/
	// C++
	deco::Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name);
	// C
	Deco_Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name);

// (2) Initialize the library
	// C++
	deco::Init(int default_compression_level, const char* deco_path, const char* deco_name);
	// C
	Deco_Init(int default_compression_level, const char* deco_path, const char* deco_name);

// (3) Load content
	// C++
	deco::LoadContentBuffer(const char* tags[], int count);
	deco::LoadContent(const char* tag);
	// C
	Deco_LoadContentBuffer(const char* tags[], int count);
	Deco_LoadContent(const char* tag);

// (4) C ONLY - Free buffers and entries
	Deco_FreeBuffer(deco_buffer* buffer);
	Deco_FreeEntry(deco_entry* entry);

// (5) Stop the library
	// C++
	deco::Stop();
	// C
	Deco_Stop();

```

Once the `*.deco` file has been generated, simply ship it along with the executable, inside the relative path specified during `Init()`. To specify custom file formats to use with deco, simply follow the instructions inside `deco.h`.

**NOTE**: You need at least *5* different files for this to work. Less than that and zstd will **refuse** to create the dictionary.

# Usage (lsqueezer)
```cpp
// (1) Start lsqueezer
	// C++
	auto ls = lsqueezer(const size_t w, const size_t h, const bool verbose = false);
	/* C
		Note: just like malloc/free, to every LS_Init you MUST have an equivalent call to LS_Stop, or else you'll leak memory.*/
	LS_Init(const size_t w, const size_t h, const bool verbose);

// (2) Create the atlas image
	// C++
	ls.Run(deco_buffer& entries);
	ls.RunTags(const char** tags, int size);
	ls.RunDirectory(const char** tags, int size, const char* folder_path);
	// C
	LS_RunTags(const char** tags, int size);
	LS_RunDirectory(const char** tags, int size, const char* folder_path);

// (3) Retrieve positions in atlas(AtlasComponents)
	// C++
	ls[std::string& tag];
	ls[const char* tag];
	// C
	LS_GetComponent(const char* tag);

// (4) C ONLY - Free lsqueezer
	LS_Stop();

```

# Examples
To build the examples, simply execute `premake-vs2019.bat`(Windows) or run:
```
cd <clone-directory>
premake5 <flag>
```

Examples are available both in C and C++. To change between them, comment/uncomment the following directive inside `example/Common.h`:
```c
#define _EX_CPP
```

# Licensing
'deco' and 'lsqueezer' are both licensed under the [BSD-3-Clause License](https://github.com/Fallbork/deco/blob/main/LICENSE). 'zstd' is dual-licensed under [BSD](https://github.com/facebook/zstd/blob/dev/LICENSE) and [GPLv2](https://github.com/facebook/zstd/blob/dev/COPYING); for this project we chose the BSD license :)
