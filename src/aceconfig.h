#pragma once

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
