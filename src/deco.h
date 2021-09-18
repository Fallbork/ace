#pragma once
#include "AtlasComponent.h"
#include <raylib.h>

#include <config.h>

// Always supported!
#define IMG_PNG ".png"
#define SND_WAV_MP3 ".wav,.mp3"	// Always supported!

#pragma region IMAGE_FILEFORMAT_SUPPORT CHECK
#ifdef SUPPORT_FILEFORMAT_BMP
#define IMG_BMP ",.bmp"
#else
#define IMG_BMP ""
#endif

#ifdef SUPPORT_FILEFORMAT_TGA
#define IMG_TGA ",.tga"
#else
#define IMG_TGA ""
#endif

#ifdef SUPPORT_FILEFORMAT_JPG
#define IMG_JPG ",.jpg"
#else
#define IMG_JPG ""
#endif

#ifdef SUPPORT_FILEFORMAT_GIF
#define IMG_GIF ",.gif"
#else
#define IMG_GIF ""
#endif

#ifdef SUPPORT_FILEFORMAT_PIC
#define IMG_PIC ",.pic"
#else
#define IMG_PIC ""
#endif

#ifdef SUPPORT_FILEFORMAT_HDR
#define IMG_HDR ",.hdr"
#else
#define IMG_HDR ""
#endif

#ifdef SUPPORT_FILEFORMAT_PSD
#define IMG_PSD ",.psd"
#else
#define IMG_PSD ""
#endif
#pragma endregion
#define DECO_SUPPORTED_IMG_FILEFORMATS IMG_PNG IMG_BMP IMG_GIF IMG_HDR IMG_JPG IMG_PIC IMG_PSD IMG_TGA

#pragma region SOUND_FILEFORMAT_SUPPORT CHECK
#ifdef SUPPORT_FILEFORMAT_OGG
#define SND_OGG ",.ogg"
#else
#define SND_OGG ""
#endif

#ifdef SUPPORT_FILEFORMAT_XM
#define SND_XM ",.xm"
#else
#define SND_XM ""
#endif

#ifdef SUPPORT_FILEFORMAT_MOD
#define SND_MOD ",.mod"
#else
#define SND_MOD ""
#endif

#ifdef SUPPORT_FILEFORMAT_FLAC
#define SND_FLAC ",.bmp"
#else
#define SND_FLAC ""
#endif
#pragma endregion
#define DECO_SUPPORTED_SND_FILEFORMATS SND_WAV_MP3 SND_FLAC SND_MOD SND_OGG SND_XM

typedef struct {
	const char* type;
	const char* id;
	unsigned int size;		// Byte-wise
	unsigned char* data;	// Data array
} _deco_entry_c;

typedef struct {
	int size;
	_deco_entry_c** buffer;
} _deco_buffer_c;

#ifdef __cplusplus
#define _DECO_FUNCTION(x) x

#include <vector>
#include <string>
#include <memory>
#include <filesystem>

struct _deco_entry_cpp {
	std::string type;
	std::string id;
	unsigned int size;		// Byte-wise
	unsigned char* data;	// Data array

	_deco_entry_cpp() : type(""), id(""), size(0), data(nullptr) {};
	~_deco_entry_cpp() {
		if (data != nullptr) { delete[] data; }
	}
};

typedef std::shared_ptr<_deco_entry_cpp> deco_entry;
typedef std::vector<deco_entry> deco_buffer;

#define _DECO_ENTRY deco_entry

namespace deco {
#else
#define _DECO_FUNCTION(x) Deco_##x

typedef _deco_entry_c deco_entry;
typedef _deco_buffer_c deco_buffer;

#define _DECO_ENTRY deco_entry*
#endif

/* Init():
*   Initializes deco static data.
	
	* Default compression level: default compression for zstd; 1 ... 19 are supported
	  as far as this version of zstd goes;
	* Deco_path: path in which to search for a deco file during operations;
	* Deco_name: name of the deco file;
	* NOTE: Will format path as 'deco_path' / 'deco_name'.deco*.
	*/
void _DECO_FUNCTION(Init(int default_compression_level, const char* deco_path, const char* deco_name));

/* Stop():
	Cleans up static data;
	*/
void _DECO_FUNCTION(Stop());

/* Generate():
	Generates a new deco file using files inside 'res_path', and outputs it to the
	formatted path " 'output_path' / 'output_name'.deco ";

	* Compression level: compression for zstd; 1 ... 19 are supported as far as this
	  version of zstd goes; Input -1 for default;
	* Res_path: path with all files to be compressed;
	* Output_path: path in which to save the deco file;
	* Output_name: name of the deco file;
	* NOTE: Will format path as 'deco_path' / 'deco_name'.deco*.
	*/
void _DECO_FUNCTION(Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name));

#ifdef __cplusplus
#endif

/* LoadContentBuffer():
	Decompresses the data stored inside the deco file for usage inside the application;

	* Tags: array of tags(ids) to look for inside the deco file;
	* Count: number of elements inside 'tags'.
	*/
deco_buffer _DECO_FUNCTION(LoadContentBuffer(const char* tags[], int count));

/* LoadContent():
	Decompresses the data stored inside the deco file for usage inside the application;

	* Tags: a tags(id) to look for inside the deco file.
	*/
_DECO_ENTRY _DECO_FUNCTION(LoadContent(const char* tag));

#ifdef __cplusplus
// Internal usage
bool CheckFileFormat(const char* fmt, const std::filesystem::path& path, std::string* buffer);
}
#else
void _DECO_FUNCTION(FreeEntry(deco_entry* entry));
void _DECO_FUNCTION(FreeBuffer(deco_buffer* buffer));
#endif


