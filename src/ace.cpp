#include "ace.h";
#include "dictionary/dib.h";

#include <map>
#include <limits>
#include <locale>
#include <sstream>
#include <fstream>
#include <zstd.h>
#include <md5.h>

#define EX_ACE_MAX_ALLOC_CAPACITY 256 KB
#define EX_ACE_STREAMSIZE_MAX LLONG_MAX
#define EX_ACE_DELIM ','

static int s_default_level = 0;
static const char* s_default_path = NULL;

struct ace_pointer {
	std::string id;
	std::streampos pos;
};

class ace_facet : public std::ctype<char> {
	mask delim_table[table_size];

public:
	ace_facet(size_t refs = 0) : std::ctype<char>(delim_table, false, refs) {
		std::copy_n(classic_table(), table_size, delim_table);
		delim_table[','] = (mask)space;
	}
};

// Singleton
class ace_iterator {
	std::vector<ace_pointer> visited_;
	std::fstream stream_;
	std::streampos pos_;
	ZSTD_DCtx* dctx_;
	ZSTD_DDict* ddict_;
	bool is_valid_;

	void seek_pos(std::streampos pos) {
		stream_.clear();
		stream_.seekg(pos);
		pos_ = std::move(pos);
	}

	ace_pointer query_pointer() {
		std::streampos init_pos = pos_;
		ace_pointer vi{ std::move(parse_value()), std::move(init_pos) };
		seek_pos(init_pos);	// return to initial pos
		return vi;
	}

	void store_pointer(ace_pointer& ptr) {
		if (std::find_if(visited_.begin(), visited_.end(),
			[&](ace_pointer& val) {
				return ptr.id == val.id;
			}
		) == visited_.end()) {
			visited_.push_back(std::move(ptr));
		}
	}

	void skip_value(size_t count = NULL) {
		if (count == NULL) { stream_.ignore(EX_ACE_STREAMSIZE_MAX, EX_ACE_DELIM); }
		else {
			for (size_t i = 0; i < count; i++) {
				stream_.ignore();
			}
			if (stream_.peek() == EX_ACE_DELIM) {
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
		std::string buffer = {};
		stream_ >> buffer;
		if (buffer.size() != 0) {
			if (stream_.peek() == EX_ACE_DELIM) {
				stream_.ignore();	// skip delim
			}
			if (*(buffer.begin()) == '"') {
				if (*(buffer.rbegin()) == '"') { buffer = buffer.substr(1, buffer.length() - 2); }
				else { buffer = buffer.substr(1, buffer.length() - 1); }
			}
			else if (*(buffer.rbegin()) == '"') { buffer = buffer.substr(0, buffer.length() - 1); }
			pos_ = std::move(stream_.tellg());
		}
		return buffer;
	}

	unsigned char* parse_compressed(unsigned int bytes, unsigned int out_bytes) {
		char* read_buf = new char[bytes];
		stream_.read(read_buf, bytes);
		if (stream_.peek() == EX_ACE_DELIM) {
			stream_.ignore();	// skip delim
		}
		pos_ = std::move(stream_.tellg());
		unsigned char* out_buf = new unsigned char[out_bytes];
		ZSTD_decompress_usingDDict(dctx_, out_buf, out_bytes, read_buf, bytes, ddict_);
		delete[] read_buf;
		return out_buf;
	}

	ace_entry parse_entry() {
		ace_entry entry = {};

		if (!is_valid_) { 
			puts("ERROR AT " __FUNCTION__ ": Not a .ace file! Seeking failed.");
			return entry;
		}

		store_pointer(query_pointer());
		entry.id = std::move(parse_value());
		entry.type = std::move(parse_value());
		entry.size = (unsigned int)std::stoi(std::move(parse_value()));
		const int compressed_size = std::stoi(std::move(parse_value()));
		entry.data = parse_compressed(compressed_size, entry.size);
		return std::move(entry);
	}

	ace_iterator() : is_valid_(false), pos_(0), dctx_(NULL), ddict_(NULL) {};

public:
	ace_iterator(ace_iterator const&) = delete;             // Copy construct
	ace_iterator(ace_iterator&&) = delete;                  // Move construct
	ace_iterator& operator=(ace_iterator const&) = delete;  // Copy assign
	ace_iterator& operator=(ace_iterator&&) = delete;

	static ace_iterator& Get() {
		static ace_iterator it;
		return it;
	}

	ace_iterator* Prime(const char* path) {
		if (stream_.is_open()) {
			stream_.close();
		}
		stream_.open(s_default_path, std::ios::in | std::ios::binary);
		if (!stream_) {
			puts("ERROR AT " __FUNCTION__ ": Could not open ace file!");
			return NULL;
		}
		std::locale x(std::locale::classic(), new ace_facet);	// does this leak?
		stream_.imbue(x);
		stream_.clear();
		stream_.seekg(0, std::ios::beg);	// Seek the beggining just in case
		if (std::stoi(std::move(parse_value())) == 0xACE) {
			is_valid_ = true;
		}
		else return NULL;
		skip_value(); // MD5 Hash
		const size_t dict_size = std::stoi(std::move(parse_value()));
		char* dict = new char[dict_size];
		stream_.read((char*)dict, dict_size);
		if (stream_.peek() == EX_ACE_DELIM) {
			stream_.ignore();	// skip delim
		}
		pos_ = std::move(stream_.tellg());
		dctx_ = ZSTD_createDCtx();
		ddict_ = ZSTD_createDDict(dict, dict_size);
		delete[] dict;

		return this;
	}

	ace_entry operator[](const char* entry_id) {
		if (!is_valid_) {
			puts("ERROR AT " __FUNCTION__ ": Not a .ace file! Seeking failed.");
			return {};
		}

		std::streampos init_pos = pos_;
		stream_.clear();
		auto visited_it = std::find_if(visited_.begin(), visited_.end(),
			[&](ace_pointer& ref) { return ref.id == entry_id; });
		if (visited_it != visited_.end()) {	// element has been visited previously!
			seek_pos(visited_it->pos);
			return parse_entry();
		}

		ace_pointer ptr = query_pointer();
		if (ptr.id == entry_id) {
			return parse_entry();
		}
		while (ptr.id != entry_id && !stream_.eof() && ptr.id.size() != 0) {	// read until found
			skip_entry();
			ptr = query_pointer();
			if (ptr.id.size() == 0) {
				break;
			}
			if (ptr.id == entry_id) {
				return parse_entry();
			}
		}
		// return to the beggining of search if not found
		puts("ERROR AT " __FUNCTION__ ": Could not find entry.");
		seek_pos(init_pos);
		return {};
	}

	~ace_iterator() {
		if (stream_.is_open()) { stream_.close(); }
		ZSTD_freeDCtx(dctx_);
		ZSTD_freeDDict(ddict_);
	};
};

static unsigned char* CheckDirectoryMD5(const char* path) {
	namespace fs = std::filesystem;
	std::string md5_buffer;
	MD5_CTX ctx = { 0 };
	MD5_Init(&ctx);
	// This is not portable at ALL!!!!!!
	for (auto& entry : fs::directory_iterator(path)) {
		if (entry.is_regular_file()) {
			std::string entry_path = entry.path().string();
			std::replace(entry_path.begin(), entry_path.end(), '\\', '/');
			std::string entry_info = entry.path().filename().string();
			struct stat info;
			if (stat(entry_path.c_str(), &info) == 0) {
				entry_info.append(std::to_string(info.st_mtime));
			}
			md5_buffer.append(entry_info);
		}
	}
	MD5_Update(&ctx, md5_buffer.c_str(), md5_buffer.size());
	unsigned char* ret = new unsigned char[16];
	MD5_Final(ret, &ctx);
	return ret;
}

namespace ace {
	int Init(int default_compression_level, const char* res_path, const char* ace_path, const char* ace_name, bool scan_changes) {
		if (s_default_path) { free((void*)s_default_path); }
		s_default_level = default_compression_level;
		std::string fmt_path = std::string(ace_path) + '/' + ace_name + ".ace";
		s_default_path = (const char*)malloc(fmt_path.size() + 1);
		if (s_default_path) { memcpy((void*)s_default_path, fmt_path.c_str(), fmt_path.size() + 1); }
		if (scan_changes) {
			unsigned char* md5_dir = CheckDirectoryMD5(res_path);
			unsigned char* md5_ace = new unsigned char[16];
			memset(md5_ace, 0, 16);
			{
				std::fstream ace;
				ace.open(fmt_path);
				if (!ace) {
					puts("ERROR AT " __FUNCTION__ ": Could not open ace file; Generating...");
				}
				else {
					std::locale x(std::locale::classic(), new ace_facet);
					ace.imbue(x);
					int OxACE = 0;
					ace >> OxACE;	// 0xACE
					if (OxACE != 0xACE) {
						puts("ERROR AT " __FUNCTION__ ": Not a valid .ace file! Generating...");
					}
					else {
						if (ace.peek() == EX_ACE_DELIM) {
							ace.ignore();	// skip delim
						}
						ace.read((char*)md5_ace, 16); // MD5 Hash
					}
					ace.close();
				}
			}
			bool valid = true;
			for (int i = 0; i < 16; i++) {
				if (md5_ace[i] != md5_dir[i]) {
					valid = false;
					break;
				}
			}
			if (!valid) { 
				int success = Generate(default_compression_level, res_path, ace_path, ace_name);
				if (!success) {
					return 0;
				}
			}
			delete[] md5_ace;
			delete[] md5_dir;
		}
		ace_iterator::Get().Prime(fmt_path.c_str());
		return 1;
	}

	void Stop() {
		free((void*)s_default_path);
	}

	int Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name) {
		namespace fs = std::filesystem;
		std::string ext;
		std::ofstream out;
		std::ifstream in;
		std::string fmt_path = std::string(output_path) + '/' + output_name + ".ace";
		out.open(std::move(fmt_path), std::ios::out | std::ios::trunc | std::ios::binary);
		if (!out) {
			puts("ERROR AT " __FUNCTION__ ": Could not create ace file!");
			return 0;
		}
		unsigned char* md5_dir = CheckDirectoryMD5(res_path);
		(out << 0xACE << EX_ACE_DELIM).write((const char*)md5_dir, 16) << EX_ACE_DELIM;
		delete[] md5_dir;

		// Create Dictionary
		std::vector<std::string> paths;
		float total_bytes = 0.0f;
		for (auto& entry : fs::directory_iterator(res_path)) {
			if (entry.is_regular_file() && entry.path().has_extension() &&
				CheckFileFormat(ACE_SUPPORTED_IMG_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(ACE_SUPPORTED_SND_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(ACE_CUSTOM_FILEFORMATS, entry.path(), &ext)) {
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

		auto dict = ACE_DIB_TrainFromFiles(total_bytes / 8.0f, (const char**)c_paths, paths.size(), NULL, NULL, &params, NULL, NULL);

		ZSTD_CDict* cdict = ZSTD_createCDict(dict.data, dict.size, params.zParams.compressionLevel);
		(out << dict.size << EX_ACE_DELIM).write((const char*)dict.data, dict.size) << EX_ACE_DELIM; // SKETCHY AF
		free(dict.data);
		for (size_t i = 0; i < paths.size(); i++) {
			delete[] c_paths[i];
		}
		delete[] c_paths;

		// Read and compress.
		ZSTD_CCtx* cctx = ZSTD_createCCtx();
		for (auto& entry : fs::directory_iterator(res_path)) {
			if (entry.is_regular_file() && entry.path().has_extension() &&
				CheckFileFormat(ACE_SUPPORTED_IMG_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(ACE_SUPPORTED_SND_FILEFORMATS, entry.path(), &ext) ||
				CheckFileFormat(ACE_CUSTOM_FILEFORMATS, entry.path(), &ext)) {
				in.open(entry.path(), std::ios::in | std::ios::binary);
				if (!in) {
					std::string path = entry.path().string();
					printf("ERROR AT " __FUNCTION__ ": Could not open file! (\"%s\")\n", path.c_str());
					return 0;
				}
				std::filebuf* buf = in.rdbuf();							   // PAIN
				const size_t src_size = buf->pubseekoff(0, in.end, in.in); // PAIN
				buf->pubseekpos(0, in.in);								   // PAIN

				char* dst_buf = new char[EX_ACE_MAX_ALLOC_CAPACITY];
				int dst_size = 0;
				{
					char* src_buf = (char*)malloc(src_size);
					in.read(src_buf, src_size);
					dst_size = ZSTD_compress_usingCDict(cctx, dst_buf, EX_ACE_MAX_ALLOC_CAPACITY, src_buf, src_size, cdict);
					free(src_buf);
				}
				out << entry.path().stem() << EX_ACE_DELIM << ext << EX_ACE_DELIM << src_size << EX_ACE_DELIM;
				out << dst_size << EX_ACE_DELIM;
				out.write(dst_buf, dst_size) << EX_ACE_DELIM;
				delete[] dst_buf;
				in.close();
			}
		}
		ZSTD_freeCDict(cdict);
		ZSTD_freeCCtx(cctx);
		out.close();
		return 1;
	}

	bool CheckFileFormat(const char* fmt, const std::filesystem::path& path, std::string* buffer) {
		std::stringstream ss(fmt);
		std::string* ext = (buffer == NULL) ? new std::string() : buffer;
		while (std::getline(ss, *ext, ',')) {
			if (path.extension().string() == *ext) { return true; }
		}
		if (buffer == NULL) { delete ext; }
		return false;
	}

	ace_buffer LoadContentBuffer(const char* tags[], int count) {
		ace_buffer elements;
		for (size_t i = 0; i < count; i++) {
			ace_entry e = ace_iterator::Get()[tags[i]];
			if (e.id != "") { elements.push_back(e); }
		}
		return elements;
	}

	ace_entry LoadContent(const char* tag) {
		ace_entry e = ace_iterator::Get()[tag];
		return e;
	}
}

extern "C" {
	int Ace_Init(int default_compression_level, const char* res_path, const char* ace_path, const char* ace_name, bool scan_changes) {
		return ace::Init(default_compression_level, res_path, ace_path, ace_name, scan_changes);
	}

	void Ace_Stop() {
		ace::Stop();
	}

	int Ace_Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name) {
		return ace::Generate(compression_level, res_path, output_path, output_name);
	}

	EX_ace_buffer_c Ace_LoadContentBuffer(const char* tags[], int count) {
		ace_buffer ret = ace::LoadContentBuffer(tags, count);
		EX_ace_buffer_c c_buf;
		c_buf.buffer = (EX_ace_entry_c*)malloc(ret.size() * sizeof(EX_ace_entry_c));
		c_buf.size = ret.size();
		for (size_t i = 0; i < ret.size(); i++) {
			ace_entry& entry = ret[i];
			EX_ace_entry_c c_entry = {};
			c_entry.id = (const char*)malloc((entry.id.size() + 1) * sizeof(char));
			memcpy((void*)c_entry.id, entry.id.c_str(), (entry.id.size() + 1) * sizeof(char));
			c_entry.type = (const char*)malloc((entry.type.size() + 1) * sizeof(char));
			memcpy((void*)c_entry.type, entry.type.c_str(), (entry.type.size() + 1) * sizeof(char));
			c_entry.data = (unsigned char*)malloc(entry.size);
			memcpy((void*)c_entry.data, entry.data, entry.size);
			c_entry.size = entry.size;
			c_buf.buffer[i] = c_entry;
		}
		return c_buf;
	}

	EX_ace_entry_c Ace_LoadContent(const char* tag) {
		ace_entry entry = ace::LoadContent(tag);
		EX_ace_entry_c c_entry = {};
		if (entry.id != "") {
			c_entry.id = (const char*)malloc((entry.id.size() + 1) * sizeof(char));
			memcpy((void*)c_entry.id, entry.id.c_str(), (entry.id.size() + 1) * sizeof(char));
			c_entry.type = (const char*)malloc((entry.type.size() + 1) * sizeof(char));
			memcpy((void*)c_entry.type, entry.type.c_str(), (entry.type.size() + 1) * sizeof(char));
			c_entry.data = (unsigned char*)malloc(entry.size);
			memcpy((void*)c_entry.data, entry.data, entry.size);
			c_entry.size = entry.size;
		}
		return c_entry;
	}

	void Ace_FreeEntry(EX_ace_entry_c entry) {
		free((void*)entry.data);
		free((void*)entry.id);
		free((void*)entry.type);
	}

	void Ace_FreeBuffer(EX_ace_buffer_c buffer) {
		for (int i = 0; i < buffer.size; i++) {
			Ace_FreeEntry(buffer.buffer[i]);
		}
		free(buffer.buffer);
	}
}
