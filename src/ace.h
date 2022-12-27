#pragma once
#include "aceconfig.h"

typedef struct {
	const char* type;
	const char* id;
	unsigned int size;		// Byte-wise
	unsigned char* data;	// Data array
} EX_ace_entry_c;

typedef struct {
	int size;
	EX_ace_entry_c* buffer;
} EX_ace_buffer_c;

#if defined (__cplusplus)
#define EX_ACE_FUNCTION(x) x

#include <vector>
#include <string>
#include <memory>
#include <filesystem>

struct EX_ace_entry_cpp {
	std::string type;
	std::string id;
	unsigned int size;		// Byte-wise
	unsigned char* data;	// Data array

	EX_ace_entry_cpp() : type(""), id(""), size(0), data(nullptr) {};
	void Dispose() {
		if (data != nullptr) {
			free(data);
		}
	}
};

struct EX_ace_buffer_cpp {
	std::vector<EX_ace_entry_cpp> vector;

	void Dispose() {
		for (int i = vector.size(); i--;) {
			vector[i].Dispose();
		}
	}

	EX_ace_entry_cpp operator[](const int i) {
		return vector[i];
	}
};

typedef EX_ace_entry_cpp ace_entry;
typedef EX_ace_buffer_cpp ace_buffer;

namespace ace {
#else
#define EX_ACE_FUNCTION(x) Ace_##x

#include <stdbool.h>

typedef EX_ace_entry_c ace_entry;
typedef EX_ace_buffer_c ace_buffer;
#endif

#define EX_ACE_ENTRY ace_entry

/* Init():
    Initializes ace static data.
	
	* Default compression level: default compression for zstd; 1 ... 19 are supported
	  as far as this version of zstd goes;
	* Ace_path: path in which to search for a ace file during operations;
	* Ace_name: name of the ace file;
	* Returns: 1 on success, 0 on failure
	* NOTE: Will format path as 'ace_path' / 'ace_name'.ace*.
	*/
int EX_ACE_FUNCTION(Init(int default_compression_level, const char* res_path, const char* ace_path, const char* ace_name, bool scan_changes));

/* Stop():
	Cleans up static data;
	*/
void EX_ACE_FUNCTION(Stop());

/* Generate():
	Generates a new ace file using files inside 'res_path', and outputs it to the
	formatted path " 'output_path' / 'output_name'.ace ";

	* Compression level: compression for zstd; 1 ... 19 are supported as far as this
	  version of zstd goes; Input -1 for default;
	* Res_path: path with all files to be compressed;
	* Output_path: path in which to save the ace file;
	* Output_name: name of the ace file;
	* Returns: 1 on success, 0 on failure
	*/
int EX_ACE_FUNCTION(Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name));

#ifdef __cplusplus
#endif

/* LoadContentBuffer():
	Decompresses the data stored inside the ace file for usage inside the application;

	* Tags: array of tags(ids) to look for inside the ace file;
	* Count: number of elements inside 'tags'.
	*/
ace_buffer EX_ACE_FUNCTION(LoadContentBuffer(const char* tags[], int count));

/* LoadContent():
	Decompresses the data stored inside the ace file for usage inside the application;

	* Tags: a tag(id) to look for inside the ace file.
	*/
EX_ACE_ENTRY EX_ACE_FUNCTION(LoadContent(const char* tag));

#ifdef __cplusplus
// Internal usage
bool CheckFileFormat(const char* fmt, const std::filesystem::path& path, std::string& buffer);
}
#else
void EX_ACE_FUNCTION(FreeEntry(ace_entry entry));
void EX_ACE_FUNCTION(FreeBuffer(ace_buffer buffer));
#endif


