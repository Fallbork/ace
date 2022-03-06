# ace
'ace' (automatic content embedder) files are meant to provide an easy way to distribute resources along with any raylib project. 

'lsqueezer' is a tool to create texture atlases at runtime. It comes as a module of 'ace', with complete support to it.

Both these libraries work with any C/C++ project, as long as the compiler is capable of handling C++17; they use std::filesystem internally to scan and read files in the most native, cross-platform way possible.
Compression is done making use of [zstd](https://github.com/facebook/zstd "Zstandard's GitHub repository")'s dictionary feature, to compress data ranging from a couple bytes up to several MBs.

# Usage (ace)
```cpp
// (1) Initialize the library - This function can automatically scan for changes inside the resource folder.
	// C++
	ace::Init(int default_compression_level, const char* res_path, const char* ace_path, const char* ace_name, bool scan_changes);
	// C
	Ace_Init(int default_compression_level, const char* res_path, const char* ace_path, const char* ace_name, bool scan_changes);

// (1.1) Generate the file again if needed
	//Recommended: Check if a '.ace' file doesn't exist already. This is an expensive function.
	// C++
	ace::Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name);
	// C
	Ace_Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name);
	
// (2) Load content
	// C++
	ace_buffer ace::LoadContentBuffer(const char* tags[], int count);
	ace_entry ace::LoadContent(const char* tag);
	// C
	ace_buffer* Ace_LoadContentBuffer(const char* tags[], int count);
	ace_entry* Ace_LoadContent(const char* tag);

// (2.1) C ONLY - Free buffers and entries
	Ace_FreeBuffer(ace_buffer* buffer);
	Ace_FreeEntry(ace_entry* entry);

// (3) Stop the library
	// C++
	ace::Stop();
	// C
	Ace_Stop();
```

Once the `*.ace` file has been generated, simply ship it along with the executable, inside the relative path specified during `Init()`. To specify custom file formats to use with ace, simply follow the instructions inside `ace.h`.

**NOTE**: You need at least *5* different files for this to work. Less than that and zstd will **refuse** to create the dictionary.

# Usage (lsqueezer)
```cpp
// (1) Start lsqueezer
	// C++
	auto ls = lsqueezer(const size_t w, const size_t h, const bool verbose = false);
	/* C
	    Note: just like malloc/free, to every LS_Init you MUST have an equivalent call to LS_Stop, or else you'll leak memory.
	    */
	LS_Init(const size_t w, const size_t h, const bool verbose);

// (2) Create the atlas image
	// C++
	Image ls.Run(ace_buffer& entries);
	Image ls.RunTags(const char** tags, int size);
	Image ls.RunDirectory(const char** tags, int size, const char* folder_path);
	// C
	Image LS_RunTags(const char** tags, int size);
	Image LS_RunDirectory(const char** tags, int size, const char* folder_path);

// (3) Retrieve positions in atlas(AtlasComponents)
	// C++
	AtlasComponent ls[std::string& tag];
	AtlasComponent ls[const char* tag];
	// C
	AtlasComponent LS_GetComponent(const char* tag);

// (3.1) C ONLY - Free lsqueezer
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
'ace' and 'lsqueezer' are both licensed under the [BSD-3-Clause License](https://github.com/Fallbork/ace/blob/main/LICENSE). 'zstd' is dual-licensed under [BSD](https://github.com/facebook/zstd/blob/dev/LICENSE) and [GPLv2](https://github.com/facebook/zstd/blob/dev/COPYING); for this project we chose the BSD license :)
