/* This version of lesser_squeezer.* is a direct modification of the files in the original 'squeezer' repo.
/* 
/* "squeezer" - MIT License.
 * Copyright (c) huxingyi@msn.com All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once
#include "../ace.h"
#include "../AtlasComponent.h"	// I couldn't forward declare it for some weird reason????
#include <raylib.h>

#ifdef __cplusplus
#include <map>
#include <string>
#include <vector>

#define EX_LS_FUNCTION(x) x

typedef std::map<std::string, AtlasComponent> AtlasMap;

class lsqueezer {
	const bool verbose_;
	const size_t bin_width_;
	const size_t bin_height_;
	AtlasMap map_;

	Image inline CreateBinFromEntries(std::vector<ace_entry>& entries);

public:

	lsqueezer(const size_t w, const size_t h, const bool verbose = false) : bin_height_(h), bin_width_(w), verbose_(verbose) {
		puts("LSQUEEZER: Context created");
	};
	AtlasComponent& operator[](std::string& arg) { 
		if (map_.count(arg) != 0) { return map_[arg]; }
		return AtlasComponent{ 0 };
	}
	AtlasComponent& operator[](const char* arg) {
		if (map_.count(arg) != 0) { return map_[arg]; }
		return AtlasComponent{ 0 };
	}

	/* Run():
		Create atlas from ace entries.

		* Entries: a vector of previously loaded ace entries.
		*/
	Image Run(ace_buffer& entries);
#else
#define EX_LS_FUNCTION(x) LS_##x

	/* Init():
		Initializes static data.
	
		* W: width of bin;
		* H: height of bin;
		* Verbose: output information to console.
		*/
	void LS_Init(const int w, const int h, const bool verbose);

	/* GetComponent():
		Get a component's data.

		* Name: the component's name; same as the file name/ace entry id.
		* Size: size of the tags array;
		* NOTE: if no match to an id is found, it returns a NULL struct.
		*/
	AtlasComponent LS_GetComponent(const char* name);

	/* Stop():
		Cleans up static data.
		*/
	void LS_Stop();
#endif
	/* RunTags():
		Create atlas from ace tags.
	
		* Tags: an array of entry ids(names); lsqueezer will automatically pull them from ace;
		* Size: size of the tags array;
		* NOTE: if no match to an id is found, it is skipped; the function will fail if the bin
		  is not large enough to fit all the contents.
		*/
	Image EX_LS_FUNCTION(RunTags(const char** tags, int size));

	/* RunDirectory():
		Create atlas from files inside a directory.
	
		* Tags: an array of entry ids(names);
		* Size: size of the tags array;
		* Folder_path: a directory from which to load images from;
		* NOTE: in case the requested entry id is not found in the folder, the function will
		  attempt to pull it from ace. If no match to an id is found, it is skipped; the 
		  function will fail if the bin is not large enough to fit all the contents.
		*/
	Image EX_LS_FUNCTION(RunDirectory(const char** tags, int size, const char* folder_path));

#ifdef __cplusplus
};
#endif



