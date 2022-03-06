#pragma once
#include "AtlasComponent.h"
#include <raylib.h>

#include <config.h>

/* Ace only reads files whose extensions are known at compile-time,
	so if you wish to use any custom extension that's not available as
	a directive in raylib, feel free to add them inside the following
	directive by following this format:

	".ext1,.ext2, ... ,.extN"
	*/
#define ACE_CUSTOM_FILEFORMATS ""

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
#define ACE_SUPPORTED_IMG_FILEFORMATS IMG_PNG IMG_BMP IMG_GIF IMG_HDR IMG_JPG IMG_PIC IMG_PSD IMG_TGA

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
#define ACE_SUPPORTED_SND_FILEFORMATS SND_WAV_MP3 SND_FLAC SND_MOD SND_OGG SND_XM

typedef struct {
	const char* type;
	const char* id;
	unsigned int size;		// Byte-wise
	unsigned char* data;	// Data array
} EX_ace_entry_c;

typedef struct {
	int size;
	EX_ace_entry_c** buffer;
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
	~EX_ace_entry_cpp() {
		if (data != nullptr) { delete[] data; }
	}
};

typedef std::shared_ptr<EX_ace_entry_cpp> ace_entry;
typedef std::vector<ace_entry> ace_buffer;

#define EX_ACE_ENTRY ace_entry

namespace ace {
#else
#define EX_ACE_FUNCTION(x) Ace_##x

typedef EX_ace_entry_c ace_entry;
typedef EX_ace_buffer_c ace_buffer;

#define EX_ACE_ENTRY ace_entry*
#endif

/* Init():
*   Initializes ace static data.
	
	* Default compression level: default compression for zstd; 1 ... 19 are supported
	  as far as this version of zstd goes;
	* Ace_path: path in which to search for a ace file during operations;
	* Ace_name: name of the ace file;
	* NOTE: Will format path as 'ace_path' / 'ace_name'.ace*.
	*/
void EX_ACE_FUNCTION(Init(int default_compression_level, const char* res_path, const char* ace_path, const char* ace_name, bool scan_changes));

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
	*/
void EX_ACE_FUNCTION(Generate(int compression_level, const char* res_path, const char* output_path, const char* output_name));

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
bool CheckFileFormat(const char* fmt, const std::filesystem::path& path, std::string* buffer);
}
#else
void EX_ACE_FUNCTION(FreeEntry(ace_entry* entry));
void EX_ACE_FUNCTION(FreeBuffer(ace_buffer* buffer));
#endif


