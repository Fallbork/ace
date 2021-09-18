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

#include "lesser_squeezer.h"
#include "Rect.h"
#include "MaxRectsBinPack.h"
#include <fstream>
#include <filesystem>
#include <assert.h>

static lsqueezer* s_lsqueezer = nullptr;

Image lsqueezer::CreateBinFromEntries(deco_buffer& entries) {
	if (verbose_) { puts("LSQUEEZER: Generating & populating buffers"); }
	Image bin_image = GenImageColor(bin_width_, bin_height_, BLANK);

	std::vector<rbp::RectSize> dimensions;
	for (size_t i = 0; i < entries.size(); i++) {
		// TODO: ideally store size in deco...
		auto& elem = entries[i];
		Image img;
		img = LoadImageFromMemory(elem->type.c_str(), elem->data, elem->size);
		rbp::RectSize rs = { img.width, img.height };
		dimensions.push_back(rs);
		UnloadImage(img);
	}

	float best_occupancy = 0.0f;
	std::vector<rbp::Rect> rects;
	for (char i = 0; i < 5; i++) {
		// TODO: Move to Guilloutine Packer.
		rbp::MaxRectsBinPack pack(bin_width_, bin_height_, false);
		std::vector<rbp::RectSize> temp_dimensions(dimensions);
		std::vector<rbp::Rect> out;

		double occupancy = 0;
		if (verbose_) { printf("LSQUEEZER: Calculating occupancy using method #%d\n", i); }
		pack.Insert(temp_dimensions, out, (rbp::MaxRectsBinPack::FreeRectChoiceHeuristic)i);

		if (temp_dimensions.size() != 0) {
			puts("ERROR AT " __FUNCTION__ ": Atlas size too small. Aborting.");
			return { 0 };
		}

		occupancy = pack.Occupancy();
		if (verbose_) { printf("LSQUEEZER: Occupancy of method #%d: %.02f\n", i, occupancy); }
		if (occupancy > best_occupancy) {
			best_occupancy = occupancy;
			rects.swap(out);
		}
	}

	if (best_occupancy <= 0) {
		puts("ERROR AT " __FUNCTION__ ": 'best_occupancy' was less than/is 0. Aborting.");
		return {0};
	}

	if (verbose_) { puts("LSQUEEZER: Populating bin"); }
	for (size_t i = 0; i < entries.size(); i++) {
		auto& elem = entries[i];
		Image img = LoadImageFromMemory(elem->type.c_str(), elem->data, elem->size);
		rbp::Rect& r = rects[i];
		assert(r.width == img.width);
		assert(r.height == img.height);
		if (verbose_) { printf("LSQUEEZER: Copying image (%s) to bin\n", elem->id.c_str()); }
		for (size_t y_src = 0; y_src < img.height; y_src++) {
			void* dest = (char*)bin_image.data + ((r.y + y_src) * bin_image.width + r.x) * 4;
			void* src = (char*)img.data + y_src * img.width * 4;
			memcpy(dest, src, img.width * 4);
		}
		map_[elem->id] = { r.width, r.height, r.x, r.y, 0, 0 };
		UnloadImage(img);
	}
	return bin_image;
}

Image lsqueezer::Run(deco_buffer& entries) {
	if (verbose_) { puts("LSQUEEZER: Preparing to run l[esser]squeezer!"); }
	return CreateBinFromEntries(entries);
}

Image lsqueezer::RunTags(const char** tags, int size) {
	if (verbose_) { puts("LSQUEEZER: Preparing to run l[esser]squeezer!"); }
	if (verbose_) { puts("LSQUEEZER: Fetching requested content from deco"); }
	auto entries = deco::LoadContentBuffer(tags, size);
	return CreateBinFromEntries(entries);
}

Image lsqueezer::RunDirectory(const char** tags, int size, const char* folder_path) {
	namespace fs = std::filesystem;
	if (verbose_) { puts("LSQUEEZER: Preparing to run l[esser]squeezer!"); }

	if (verbose_) { printf("LSQUEEZER: Fetching files from directory (\"%s\")", folder_path); }
	deco_buffer entries;
	std::string ext;
	for (auto& entry : fs::directory_iterator(folder_path)) {
		for (int i = 0; i < size; i++) {
			if (entry.is_regular_file() && entry.path().has_extension() &&
				deco::CheckFileFormat(DECO_SUPPORTED_IMG_FILEFORMATS, entry.path(), &ext)) {
				if (entry.path().filename().string() == tags[i]) {
					std::fstream in;
					in.open(entry.path(), std::ios::in | std::ios::binary);
					if (!in) {
						printf("ERROR AT " __FUNCTION__ ": Could not open file! (\"%s\")\n", entry.path().c_str());
						return {0};
					}
					deco_entry e = std::make_shared<_deco_entry_cpp>();
					e->id = entry.path().filename().string();
					e->type = ext;

					std::filebuf* buf = in.rdbuf();
					e->size = buf->pubseekoff(0, in.end, in.in);
					buf->pubseekpos(0, in.in);
					e->data = new unsigned char[e->size];
					in.read((char*)e->data, e->size);
					in.close();
					entries.push_back(std::move(e));
				}
				else {
					auto e = deco::LoadContent(tags[i]);
					if (e != nullptr) { entries.push_back(e); }
				}
			}
		}
	}

	if (verbose_) { puts("LSQUEEZER: Generating & populating buffers"); }
	Image bin_image = GenImageColor(bin_width_, bin_height_, BLANK);

	std::vector<rbp::RectSize> dimensions;
	for (size_t i = 0; i < entries.size(); i++) {
		// TODO: ideally store size in deco...
		auto& elem = entries[i];
		Image img;
		img = LoadImageFromMemory(elem->type.c_str(), elem->data, elem->size);
		rbp::RectSize rs = { img.width, img.height };
		dimensions.push_back(rs);
		UnloadImage(img);
	}

	float best_occupancy = 0.0f;
	std::vector<rbp::Rect> rects;
	for (char i = 0; i < 5; i++) {
		// TODO: Move to Guilloutine Packer.
		rbp::MaxRectsBinPack pack(bin_width_, bin_height_, false);
		std::vector<rbp::RectSize> temp_dimensions(dimensions);
		std::vector<rbp::Rect> out;

		double occupancy = 0;
		if (verbose_) { printf("LSQUEEZER: Calculating occupancy using method #%d\n", i); }
		pack.Insert(temp_dimensions, out, (rbp::MaxRectsBinPack::FreeRectChoiceHeuristic)i);

		occupancy = pack.Occupancy();
		if (verbose_) { printf("LSQUEEZER: Occupancy of method #%d: %.02f\n", i, occupancy); }
		if (occupancy > best_occupancy) {
			best_occupancy = occupancy;
			rects.swap(out);
		}
	}

	if (best_occupancy <= 0) {
		puts("ERROR AT " __FUNCTION__ ": 'best_occupancy' was less than/is 0. Aborting.");
		return {0};
	}

	if (verbose_) { puts("LSQUEEZER: Populating bin"); }
	for (size_t i = 0; i < entries.size(); i++) {
		auto& elem = entries[i];
		Image img = LoadImageFromMemory(elem->type.c_str(), elem->data, elem->size);
		rbp::Rect& r = rects[i];
		assert(r.width == img.width);
		assert(r.height == img.height);
		if (verbose_) { printf("LSQUEEZER: Copying image (%s) to bin\n", elem->id.c_str()); }
		for (size_t y_src = 0; y_src < img.height; y_src++) {
			void* dest = (char*)bin_image.data + ((r.y + y_src) * bin_image.width + r.x) * 4;
			void* src = (char*)img.data + y_src * img.width * 4;
			memcpy(dest, src, img.width * 4);
		}
		map_[elem->id] = { r.width, r.height, r.x, r.y, 0, 0 };
		UnloadImage(img);
	}
	return bin_image;
}

extern "C" {
	void LS_Init(const int w, const int h, const bool verbose) {
		s_lsqueezer = new lsqueezer(w, h, verbose);
	}
	
	AtlasComponent LS_GetComponent(const char* name) {
		if (!s_lsqueezer) { puts("ERROR AT " __FUNCTION__ ": lsqueezer hasn't been initialized!"); }
		else return (*s_lsqueezer)[name];
	}
	
	Image LS_RunTags(const char** tags, int size) {
		return s_lsqueezer->RunTags(tags, size);
	}
	Image LS_RunDirectory(const char** tags, int size, const char* folder_path) {
		return s_lsqueezer->RunDirectory(tags, size, folder_path);
	}
	
	void LS_Stop() {
		if (s_lsqueezer) delete s_lsqueezer;
	}
}