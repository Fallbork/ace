#include "deco.h";
#include "dictionary/dib.h";
#include "lsqueezer/lesser_squeezer.h"

#include <map>
#include <limits>
#include <locale>
#include <sstream>
#include <fstream>
#include <zstd.h>

#define _DECO_MAX_ALLOC_CAPACITY 256 KB
#define _DECO_STREAMSIZE_MAX LLONG_MAX
#define _DECO_DELIM ','
#define _DECO_EXTERN extern "C"

static int s_default_level = 0;
static const char* s_default_path = NULL;

struct deco_pointer {
	std::string id;
	std::streampos pos;
};

class deco_iterator {
	std::vector<deco_pointer> visited_;
	std::fstream& stream_;
	std::streampos pos_;
	ZSTD_DCtx* dctx_;
	ZSTD_DDict* ddict_;
	bool is_valid_;

	void seek_pos(std::streampos pos) {
		stream_.clear();
		stream_.seekg(pos);
		pos_ = std::move(pos);
	}

	deco_pointer query_pointer() {
		std::streampos init_pos = pos_;
		deco_pointer vi{ std::move(parse_value()), std::move(init_pos) };
		seek_pos(init_pos);	// return to initial pos
		return vi;
	}

	void store_pointer(deco_pointer& ptr) {
		if (std::find_if(visited_.begin(), visited_.end(),
			[&](deco_pointer& val) {
				return ptr.id == val.id;
			}
		) == visited_.end()) {
			visited_.push_back(std::move(ptr));
		}
	}

	void skip_value(size_t count = NULL) {
		if (count == NULL) { stream_.ignore(_DECO_STREAMSIZE_MAX, _DECO_DELIM); }
		else {
			for (size_t i = 0; i < count; i++) {
				stream_.ignore();
			}
			if (stream_.peek() == _DECO_DELIM) {
				stream_.ignore();	// skip delim
			}
		}
		pos_ = std::move(stream_.tellg());
	}

	void skip_entry() {
		auto pointer = query_pointer();
		store_pointer(pointer);
		skip_value();	// name
		skip_value();	// ext
		skip_value();	// uncompressed size
		skip_value(std::stoll(parse_value()));	// compressed size && data
	}

	std::string parse_value() {
		std::string buffer;
		stream_ >> buffer;
		if (stream_.peek() == _DECO_DELIM) {
			stream_.ignore();	// skip delim
		}
		if (*(buffer.begin()) == '"') {
			if (*(buffer.rbegin()) == '"') { buffer = buffer.substr(1, buffer.length() - 2); }
			else { buffer = buffer.substr(1, buffer.length() - 1); }
		}
		else if (*(buffer.rbegin()) == '"') { buffer = buffer.substr(0, buffer.length() - 1); }
		pos_ = std::move(stream_.tellg());
		return buffer;
	}

	unsigned char* parse_compressed(unsigned int bytes, unsigned int out_bytes) {
		char* read_buf = new char[bytes];
		stream_.read(read_buf, bytes);
		if (stream_.peek() == _DECO_DELIM) {
			stream_.ignore();	// skip delim
		}
		pos_ = std::move(stream_.tellg());
		unsigned char* out_buf = new unsigned char[out_bytes];
		ZSTD_decompress_usingDDict(dctx_, out_buf, out_bytes, read_buf, bytes, ddict_);
		delete[] read_buf;
		return out_buf;
	}

	deco_entry parse_entry() {
		if (!is_valid_) { puts("ERROR AT " __FUNCTION__ ": Not a .deco file! Seeking failed."); return {}; }
		deco_entry elem = std::make_shared<_deco_entry_cpp>();
		store_pointer(query_pointer());
		elem->id = std::move(parse_value());
		elem->type = std::move(parse_value());
		elem->size = (unsigned int)std::stoi(std::move(parse_value()));
		const int compressed_size = std::stoi(std::move(parse_value()));
		elem->data = parse_compressed(compressed_size, elem->size);
		return std::move(elem);
	}

public:
	deco_iterator(std::fstream& stream) : stream_(stream), is_valid_(false), pos_(0), dctx_(nullptr), ddict_(nullptr) {
		stream_.clear();
		stream_.seekg(0, std::ios::beg);	// Seek the beggining just in case
		if (stream.is_open()) {
			if (std::stoi(std::move(parse_value())) == 0xDEC0) { is_valid_ = true; }
			else return;
			const size_t dict_size = std::stoi(std::move(parse_value()));
			char* dict = new char[dict_size];
			stream_.read((char*)dict, dict_size);
			if (stream_.peek() == _DECO_DELIM) {
				stream_.ignore();	// skip delim
			}
			pos_ = std::move(stream_.tellg());
			dctx_ = ZSTD_createDCtx();
			ddict_ = ZSTD_createDDict(dict, dict_size);
			delete[] dict;
		}
	};

	~deco_iterator() {
		if (stream_.is_open()) { stream_.close(); }
		ZSTD_freeDCtx(dctx_);
		ZSTD_freeDDict(ddict_);
	};

	deco_entry operator[](const char* entry_id) {
		if (!is_valid_) { puts("ERROR AT " __FUNCTION__ ": Not a .deco file! Seeking failed."); return nullptr; }
		std::streampos init_pos = pos_;
		stream_.clear();
		auto visited_it = std::find_if(visited_.begin(), visited_.end(),
			[&](deco_pointer& ref) { return ref.id == entry_id; });
		if (visited_it != visited_.end()) {	// element has been visited previously!
			seek_pos(visited_it->pos);
			return parse_entry();
		}

		deco_pointer ptr = query_pointer();
		if (ptr.id == entry_id) {
			return parse_entry();
		}
		while (ptr.id != entry_id && !stream_.eof()) {	// read until found
			skip_entry();
			ptr = query_pointer();
			if (ptr.id == entry_id) {
				return parse_entry();
			}
		}
		// return to the beggining of search if not found
		puts("ERROR AT " __FUNCTION__ ": Could not find entry.");
		seek_pos(init_pos);
		return nullptr;
	}
};

class deco_facet : public std::ctype<char> {
	mask delim_table[table_size];

public:
	deco_facet(size_t refs = 0) : std::ctype<char>(delim_table, false, refs) {
		std::copy_n(classic_table(), table_size, delim_table);
		delim_table[','] = (mask)space;
	}
};

namespace deco {
	void Init(int default_compression_level, const char* deco_path, const char* deco_name) {
		if (s_default_path) { free((void*)s_default_path); }
		s_default_level = default_compression_level;
		std::string fmt_path = std::string(deco_path) + '/' + deco_name + ".deco";
		s_default_path = (const char*)malloc(fmt_path.size() + 1);
		if (s_default_path) { memcpy((void*)s_default_path, fmt_path.c_str(), fmt_path.size() + 1); }
	}

	void Stop() {
		free((void*)s_default_path);
	}

	void Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name) {
		namespace fs = std::filesystem;
		std::string ext;
		std::ofstream out;
		std::ifstream in;
		std::string fmt_path = std::string(output_path) + '/' + output_name + ".deco";
		out.open(std::move(fmt_path), std::ios::out | std::ios::trunc | std::ios::binary);
		if (!out) {
			puts("ERROR AT " __FUNCTION__ ": Could not create deco file!");
			return;
		}
		out << 0xDEC0 << _DECO_DELIM;

		// Create Dictionary
		std::vector<std::string> paths;
		float total_bytes = 0.0f;
		for (auto& entry : fs::directory_iterator(res_path)) {
			if (entry.is_regular_file() && entry.path().has_extension() &&
				CheckFileFormat(DECO_SUPPORTED_IMG_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(DECO_SUPPORTED_SND_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(DECO_CUSTOM_FILEFORMATS, entry.path(), &ext))
				paths.push_back(std::move(entry.path().string()));
				total_bytes += entry.file_size();
			}
		}

		char** c_paths = new char* [paths.size()];
		for (size_t i = 0; i < paths.size(); i++) {
			const size_t length = paths[i].size();
			c_paths[i] = new char[length + 1]; // yeehaw to null terminators
			memcpy((void*)c_paths[i], paths[i].c_str(), paths[i].size() + 1);
		}
		ZDICT_cover_params_t params;
		memset(&params, 0, sizeof(params));
		params.d = 8;
		params.k = 256;
		params.steps = 4;
		params.splitPoint = 1.0;
		params.shrinkDict = 0;
		params.shrinkDictMaxRegression = 1;
		params.zParams = ZDICT_params_t{ (compression_level < 0 ? 10 : compression_level), 0, 0 };

		auto dict = DECO_DIB_TrainFromFiles(total_bytes / 8.0f, (const char**)c_paths, paths.size(), NULL, NULL, &params, NULL, NULL);

		ZSTD_CDict* cdict = ZSTD_createCDict(dict.data, dict.size, params.zParams.compressionLevel);
		(out << dict.size << _DECO_DELIM).write((const char*)dict.data, dict.size) << _DECO_DELIM; // SKETCHY AF
		free(dict.data);
		for (size_t i = 0; i < paths.size(); i++) {
			delete[] c_paths[i];
		}
		delete[] c_paths;

		// Read and compress.
		ZSTD_CCtx* cctx = ZSTD_createCCtx();
		for (auto& entry : fs::directory_iterator(res_path)) {
			if (entry.is_regular_file() && entry.path().has_extension() &&
				CheckFileFormat(DECO_SUPPORTED_IMG_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(DECO_SUPPORTED_SND_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(DECO_CUSTOM_FILEFORMATS, entry.path(), &ext)) {
				in.open(entry.path(), std::ios::in | std::ios::binary);
				if (!in) {
					std::string path = entry.path().string();
					printf("ERROR AT " __FUNCTION__ ": Could not open file! (\"%s\")\n", path.c_str());
					return;
				}
				std::filebuf* buf = in.rdbuf();							   // PAIN
				const size_t src_size = buf->pubseekoff(0, in.end, in.in); // PAIN
				buf->pubseekpos(0, in.in);								   // PAIN

				char* dst_buf = new char[_DECO_MAX_ALLOC_CAPACITY];
				int dst_size = 0;
				{
					char* src_buf = (char*)malloc(src_size);
					in.read(src_buf, src_size);
					dst_size = ZSTD_compress_usingCDict(cctx, dst_buf, _DECO_MAX_ALLOC_CAPACITY, src_buf, src_size, cdict);
					free(src_buf);
				}
				out << entry.path().stem() << _DECO_DELIM << ext << _DECO_DELIM << src_size << _DECO_DELIM;
				out << dst_size << _DECO_DELIM;
				out.write(dst_buf, dst_size) << _DECO_DELIM;
				delete[] dst_buf;
				in.close();
			}
	}
		ZSTD_freeCDict(cdict);
		ZSTD_freeCCtx(cctx);
		out.close();
	}

	bool CheckFileFormat(const char* fmt, const std::filesystem::path& path, std::string* buffer) {
		std::stringstream ss(fmt);
		std::string* ext = (buffer == nullptr) ? new std::string() : buffer;
		while (std::getline(ss, *ext, ',')) {
			if (path.extension().string() == *ext) { return true; }
		}
		if (buffer == nullptr) { delete ext; }
		return false;
	}

	deco_buffer LoadContentBuffer(const char* tags[], int count) {
		namespace fs = std::filesystem;
		std::fstream in;
		in.open(s_default_path, std::ios::in | std::ios::binary);
		if (!in) {
			puts("ERROR AT " __FUNCTION__ ": Could not open deco file!");
			return {};
		}
		std::locale x(std::locale::classic(), new deco_facet);	// does this leak?
		in.imbue(x);

		deco_buffer elements;
		deco_iterator it(in);
		for (size_t i = 0; i < count; i++) {
			deco_entry e = it[tags[i]];
			if (e != nullptr) { elements.push_back(e); }
		}
		return elements;
	}

	deco_entry LoadContent(const char* tag) {
		namespace fs = std::filesystem;
		std::fstream in;
		in.open(s_default_path, std::ios::in | std::ios::binary);
		if (!in) {
			puts("ERROR AT " __FUNCTION__ ": Could not open deco file!");
			return {};
		}
		std::locale x(std::locale::classic(), new deco_facet);	// does this leak?
		in.imbue(x);

		deco_iterator it(in);
		deco_entry e = it[tag];
		if (e == nullptr) { return nullptr; }
		return e;
	}
}

extern "C" {
	void Deco_Init(int default_compression_level, const char* deco_path, const char* deco_name) {
		deco::Init(default_compression_level, deco_path, deco_name);
	}

	void Deco_Stop() {
		deco::Stop();
	}

	void Deco_Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name) {
		deco::Generate(compression_level, res_path, output_path, output_name);
	}

	_deco_buffer_c Deco_LoadContentBuffer(const char* tags[], int count) {
		deco_buffer ret = deco::LoadContentBuffer(tags, count);
		_deco_buffer_c c_buf;
		c_buf.size = ret.size();
		c_buf.buffer = (_deco_entry_c**)malloc(c_buf.size * sizeof(_deco_entry_c*));
		for (size_t i = 0; i < ret.size(); i++) {
			deco_entry& entry = ret[i];
			_deco_entry_c* c_entry = (_deco_entry_c*)malloc(sizeof(_deco_entry_c));
			if (c_entry) {
				c_entry->id = (const char*)malloc(entry->id.size() + 1);
				memcpy((void*)c_entry->id, entry->id.c_str(), entry->id.size() + 1);
				c_entry->type = (const char*)malloc(entry->type.size() + 1);
				memcpy((void*)c_entry->type, entry->type.c_str(), entry->type.size() + 1);
				c_entry->data = (unsigned char*)malloc(entry->size);
				memcpy((void*)c_entry->data, entry->data, entry->size);
				c_entry->size = entry->size;
				c_buf.buffer[i] = c_entry;
			}
		}
		return c_buf;
	}

	_deco_entry_c* Deco_LoadContent(const char* tag) {
		deco_entry entry = deco::LoadContent(tag);
		_deco_entry_c* c_entry = (_deco_entry_c*)malloc(sizeof(_deco_entry_c));
		if (c_entry) {
			c_entry->id = (const char*)malloc(entry->id.size() + 1);
			memcpy((void*)c_entry->id, entry->id.c_str(), entry->id.size() + 1);
			c_entry->type = (const char*)malloc(entry->type.size() + 1);
			memcpy((void*)c_entry->type, entry->type.c_str(), entry->type.size() + 1);
			c_entry->data = (unsigned char*)malloc(entry->size);
			memcpy((void*)c_entry->data, entry->data, entry->size);
			c_entry->size = entry->size;
		}
		return c_entry;
	}

	void Deco_FreeEntry(_deco_entry_c* entry) {
		free((void*)entry->data);
		free((void*)entry->id);
		free((void*)entry->type);
		free((void*)entry);
	}

	void Deco_FreeBuffer(_deco_buffer_c* buffer) {
		for (int i = 0; i < buffer->size; i++) {
			Deco_FreeEntry(buffer->buffer[i]);
		}
		free((void*)buffer);
	}
}
