/*************************************************************************
 *
 * Copyright (C) 2014-2015 Barbara Geller & Ansel Sermersheim 
 * Copyright (C) 1997-2014 by Dimitri van Heesch.
 * All rights reserved.    
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License version 2
 * is hereby granted. No representations are made about the suitability of
 * this software for any purpose. It is provided "as is" without express or
 * implied warranty. See the GNU General Public License for more details.
 *
 * Documents produced by DoxyPress are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
*************************************************************************/

/*
 * LodePNG version 20080927
 * Copyright (c) 2005-2008 Lode Vandevenne
*/

#ifndef LODEPNG_H
#define LODEPNG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*The following defines can be commented disable code sections. Gives potential faster compile and smaller binary.*/

#define LODEPNG_COMPILE_ZLIB                /*deflate&zlib encoder and deflate&zlib decoder*/
#define LODEPNG_COMPILE_PNG                 /*png encoder and png decoder*/

//#define LODEPNG_COMPILE_DECODER           /*deflate&zlib decoder and png decoder*/

#define LODEPNG_COMPILE_ENCODER             /*deflate&zlib encoder and png encoder*/
#define LODEPNG_COMPILE_DISK                /*the optional built in harddisk file loading and saving functions*/

//#define LODEPNG_COMPILE_ANCILLARY_CHUNKS  /*any code or struct datamember related to chunks other than IHDR, IDAT, PLTE, tRNS, IEND*/
//#define LODEPNG_COMPILE_UNKNOWN_CHUNKS    /*handling of unknown chunks*/


#ifdef LODEPNG_COMPILE_DECODER
typedef struct LodeZlib_DecompressSettings {
   unsigned ignoreAdler32;
} LodeZlib_DecompressSettings;

extern const LodeZlib_DecompressSettings LodeZlib_defaultDecompressSettings;
void LodeZlib_DecompressSettings_init(LodeZlib_DecompressSettings *settings);
#endif 

#ifdef LODEPNG_COMPILE_ENCODER
typedef struct LodeZlib_DeflateSettings { /*deflate = compress*/
   /*LZ77 related settings*/
   unsigned btype; /*the block type for LZ*/
   unsigned useLZ77; /*whether or not to use LZ77*/
   unsigned windowSize; /*the maximum is 32768*/
} LodeZlib_DeflateSettings;

extern const LodeZlib_DeflateSettings LodeZlib_defaultDeflateSettings;
void LodeZlib_DeflateSettings_init(LodeZlib_DeflateSettings *settings);
#endif 

// **
#ifdef LODEPNG_COMPILE_ZLIB

#ifdef LODEPNG_COMPILE_DECODER
/*This function reallocates the out buffer and appends the data.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid buffer and *outsize its size in bytes.*/
unsigned LodeZlib_decompress(unsigned char **out, size_t *outsize, const unsigned char *in, size_t insize,
                             const LodeZlib_DecompressSettings *settings);
#endif

#ifdef LODEPNG_COMPILE_ENCODER
/*This function reallocates the out buffer and appends the data.
Either, *out must be NULL and *outsize must be 0, or, *out must be a valid buffer and *outsize its size in bytes.*/
unsigned LodeZlib_compress(unsigned char **out, size_t *outsize, const unsigned char *in, size_t insize, 
                           const LodeZlib_DeflateSettings *settings);
#endif

#endif

// **
#ifdef LODEPNG_COMPILE_PNG

/*LodePNG_chunk functions: These functions need as input a large enough amount of allocated memory.*/

unsigned LodePNG_chunk_length(const unsigned char *chunk); /*get the length of the data of the chunk. Total chunk length has 12 bytes more.*/

void LodePNG_chunk_type(char type[5], const unsigned char *chunk); /*puts the 4-byte type in null terminated string*/
unsigned char LodePNG_chunk_type_equals(const unsigned char *chunk, const char *type); /*check if the type is the given type*/

/*properties of PNG chunks gotten from capitalization of chunk type name, as defined by the standard*/
unsigned char LodePNG_chunk_critical(const unsigned char *chunk); /*0: ancillary chunk, 1: it's one of the critical chunk types*/
unsigned char LodePNG_chunk_private(const unsigned char *chunk); /*0: public, 1: private*/
unsigned char LodePNG_chunk_safetocopy(const unsigned char *chunk); /*0: the chunk is unsafe to copy, 1: the chunk is safe to copy*/


/*get pointer to the data of the chunk*/
unsigned char *LodePNG_chunk_data(unsigned char *chunk); 

/*get pointer to the data of the chunk*/
const unsigned char *LodePNG_chunk_data_const(const unsigned char *chunk); 

/*returns 0 if the crc is correct, 1 if it's incorrect*/
unsigned LodePNG_chunk_check_crc(const unsigned char *chunk); 

/*generates the correct CRC from the data and puts it in the last 4 bytes of the chunk*/
void LodePNG_chunk_generate_crc(unsigned char *chunk); 

/*iterate to next chunks.*/
unsigned char *LodePNG_chunk_next(unsigned char *chunk);
const unsigned char *LodePNG_chunk_next_const(const unsigned char *chunk);

/*add chunks to out buffer. It reallocs the buffer to append the data. returns error code*/
unsigned LodePNG_append_chunk(unsigned char **out, size_t *outlength,
                              const unsigned char *chunk); 

/*appends chunk that was already created, to the data. Returns pointer to start of appended chunk, or NULL if error happened*/

unsigned LodePNG_create_chunk(unsigned char **out, size_t *outlength, unsigned length, const char *type,
                              const unsigned char *data);

/*appends new chunk to out. Returns pointer to start of appended chunk, or NULL if error happened; may change memory address of out buffer*/

typedef struct LodePNG_InfoColor { /*info about the color type of an image*/
   /*header (IHDR)*/
   unsigned colorType;       /*color type*/
   unsigned bitDepth;        /*bits per sample*/

   /*palette (PLTE)*/
   unsigned char *palette;   /*palette in RGBARGBA... order*/
   size_t palettesize;       /*palette size in number of colors (amount of bytes is 4 * palettesize)*/

   /*transparent color key (tRNS)*/
   unsigned key_defined;     /*is a transparent color key given?*/
   unsigned key_r;           /*red component of color key*/
   unsigned key_g;           /*green component of color key*/
   unsigned key_b;           /*blue component of color key*/
} LodePNG_InfoColor;

void LodePNG_InfoColor_init(LodePNG_InfoColor *info);
void LodePNG_InfoColor_cleanup(LodePNG_InfoColor *info);
unsigned LodePNG_InfoColor_copy(LodePNG_InfoColor *dest, const LodePNG_InfoColor *source);

/*Use these functions instead of allocating palette manually*/
void LodePNG_InfoColor_clearPalette(LodePNG_InfoColor *info);
unsigned LodePNG_InfoColor_addPalette(LodePNG_InfoColor *info, unsigned char r, unsigned char g, unsigned char b,
                                      unsigned char a); /*add 1 color to the palette*/

/*additional color info*/
unsigned LodePNG_InfoColor_getBpp(const LodePNG_InfoColor *info);           /*bits per pixel*/
unsigned LodePNG_InfoColor_getChannels(const LodePNG_InfoColor *info);      /*amount of channels*/
unsigned LodePNG_InfoColor_isGreyscaleType(const LodePNG_InfoColor *info);  /*is it a greyscale type? (colorType 0 or 4)*/
unsigned LodePNG_InfoColor_isAlphaType(const LodePNG_InfoColor *info);      /*has it an alpha channel? (colorType 2 or 6)*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

typedef struct LodePNG_Time  { 
   /*LodePNG's encoder does not generate the current time. To make it add a time chunk the correct time has to be provided*/
   unsigned      year;    /*2 bytes*/
   unsigned char month;   /*1-12*/
   unsigned char day;     /*1-31*/
   unsigned char hour;    /*0-23*/
   unsigned char minute;  /*0-59*/
   unsigned char second;  /*0-60 (to allow for leap seconds)*/
} LodePNG_Time;

typedef struct LodePNG_Text { 
   /*non-international text*/

   size_t num;
   char **keys;     /*the keyword of a text chunk (e.g. "Comment")*/
   char **strings;  /*the actual text*/
} LodePNG_Text;

void LodePNG_Text_init(LodePNG_Text *text);
void LodePNG_Text_cleanup(LodePNG_Text *text);
unsigned LodePNG_Text_copy(LodePNG_Text *dest, const LodePNG_Text *source);

/*Use these functions instead of allocating the char**s manually*/
void LodePNG_Text_clear(LodePNG_Text *text);
unsigned LodePNG_Text_add(LodePNG_Text *text, const char *key, const char *str); /*push back both texts at once*/


typedef struct LodePNG_IText {
   /*international text*/

   size_t num;
   char **keys;       /*the English keyword of the text chunk (e.g. "Comment")*/
   char **langtags;   /*the language tag for this text's international language, ISO/IEC 646 string, e.g. ISO 639 language tag*/
   char **transkeys;  /*keyword translated to the international language - UTF-8 string*/
   char **strings;    /*the actual international text - UTF-8 string*/
} LodePNG_IText;

void LodePNG_IText_init(LodePNG_IText *text);
void LodePNG_IText_cleanup(LodePNG_IText *text);
unsigned LodePNG_IText_copy(LodePNG_IText *dest, const LodePNG_IText *source);

/*Use these functions instead of allocating the char**s manually*/
void LodePNG_IText_clear(LodePNG_IText *text);
unsigned LodePNG_IText_add(LodePNG_IText *text, const char *key, const char *langtag, const char *transkey,
                           const char *str); /*push back the 4 texts of 1 chunk at once*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
typedef struct LodePNG_UnknownChunks { /*unknown chunks read from the PNG, or extra chunks the user wants to have added in the encoded PNG*/
   /*There are 3 buffers, one for each position in the PNG where unknown chunks can appear
     each buffer contains all unknown chunks for that position consecutively
     The 3 buffers are the unknown chunks between certain critical chunks:
     0: IHDR-PLTE, 1: PLTE-IDAT, 2: IDAT-IEND*/
   unsigned char *data[3];
   size_t datasize[3]; /*size in bytes of the unknown chunks, given for protection*/

} LodePNG_UnknownChunks;

void LodePNG_UnknownChunks_init(LodePNG_UnknownChunks *chunks);
void LodePNG_UnknownChunks_cleanup(LodePNG_UnknownChunks *chunks);
unsigned LodePNG_UnknownChunks_copy(LodePNG_UnknownChunks *dest, const LodePNG_UnknownChunks *src);
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/

typedef struct LodePNG_InfoPng { /*information about the PNG image, except pixels and sometimes except width and height*/
   /*header (IHDR), palette (PLTE) and transparency (tRNS)*/
   unsigned width;             /*width of the image in pixels (ignored by encoder, but filled in by decoder)*/
   unsigned height;            /*height of the image in pixels (ignored by encoder, but filled in by decoder)*/
   unsigned compressionMethod; /*compression method of the original file*/
   unsigned filterMethod;      /*filter method of the original file*/
   unsigned interlaceMethod;   /*interlace method of the original file*/
   LodePNG_InfoColor color;    /*color type and bits, palette, transparency*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

   /*suggested background color (bKGD)*/
   unsigned background_defined; /*is a suggested background color given?*/
   unsigned background_r;       /*red component of suggested background color*/
   unsigned background_g;       /*green component of suggested background color*/
   unsigned background_b;       /*blue component of suggested background color*/

   /*non-international text chunks (tEXt and zTXt)*/
   LodePNG_Text text;

   /*international text chunks (iTXt)*/
   LodePNG_IText itext;

   /*time chunk (tIME)*/
   unsigned char time_defined; /*if 0, no tIME chunk was or will be generated in the PNG image*/
   LodePNG_Time time;

   /*phys chunk (pHYs)*/
   unsigned      phys_defined; /*is pHYs chunk defined?*/
   unsigned      phys_x;
   unsigned      phys_y;
   unsigned char phys_unit; /*may be 0 (unknown unit) or 1 (metre)*/

#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
   /*unknown chunks*/
   LodePNG_UnknownChunks unknown_chunks;
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/

} LodePNG_InfoPng;

void LodePNG_InfoPng_init(LodePNG_InfoPng *info);
void LodePNG_InfoPng_cleanup(LodePNG_InfoPng *info);
unsigned LodePNG_InfoPng_copy(LodePNG_InfoPng *dest, const LodePNG_InfoPng *source);

typedef struct LodePNG_InfoRaw { /*contains user-chosen information about the raw image data, which is independent of the PNG image*/
   LodePNG_InfoColor color;
} LodePNG_InfoRaw;

void LodePNG_InfoRaw_init(LodePNG_InfoRaw *info);
void LodePNG_InfoRaw_cleanup(LodePNG_InfoRaw *info);
unsigned LodePNG_InfoRaw_copy(LodePNG_InfoRaw *dest, const LodePNG_InfoRaw *source);

/*
LodePNG_convert: Converts from any color type to 24-bit or 32-bit (later maybe more supported). return value = LodePNG error code
The out buffer must have (w * h * bpp + 7) / 8, where bpp is the bits per pixel of the output color type (LodePNG_InfoColor_getBpp)
*/
unsigned LodePNG_convert(unsigned char *out, const unsigned char *in, LodePNG_InfoColor *infoOut, LodePNG_InfoColor *infoIn, unsigned w, unsigned h);

#ifdef LODEPNG_COMPILE_DECODER

typedef struct LodePNG_DecodeSettings {
   LodeZlib_DecompressSettings zlibsettings; /*in here is the setting to ignore Adler32 checksums*/

   unsigned ignoreCrc; /*ignore CRC checksums*/
   unsigned color_convert; /*whether to convert the PNG to the color type you want. Default: yes*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
   unsigned readTextChunks; /*if false but rememberUnknownChunks is true, they're stored in the unknown chunks*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
   unsigned rememberUnknownChunks; /*store all bytes from unknown chunks in the InfoPng (off by default, useful for a png editor)*/
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
} LodePNG_DecodeSettings;

void LodePNG_DecodeSettings_init(LodePNG_DecodeSettings *settings);

typedef struct LodePNG_Decoder {
   LodePNG_DecodeSettings settings;
   LodePNG_InfoRaw infoRaw;
   LodePNG_InfoPng infoPng; /*info of the PNG image obtained after decoding*/
   unsigned error;
} LodePNG_Decoder;

void LodePNG_Decoder_init(LodePNG_Decoder *decoder);
void LodePNG_Decoder_cleanup(LodePNG_Decoder *decoder);
void LodePNG_Decoder_copy(LodePNG_Decoder *dest, const LodePNG_Decoder *source);

/*decoding functions*/
/*This function allocates the out buffer and stores the size in *outsize.*/
void LodePNG_decode(LodePNG_Decoder *decoder, unsigned char **out, size_t *outsize, const unsigned char *in, size_t insize);
unsigned LodePNG_decode32(unsigned char **out, unsigned *w, unsigned *h, const unsigned char *in, size_t insize); /*return value is error*/

#ifdef LODEPNG_COMPILE_DISK
unsigned LodePNG_decode32f(unsigned char **out, unsigned *w, unsigned *h, const char *filename);
#endif /*LODEPNG_COMPILE_DISK*/
void LodePNG_inspect(LodePNG_Decoder *decoder, const unsigned char *in, size_t size); /*read the png header*/

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

typedef struct LodePNG_EncodeSettings {
   LodeZlib_DeflateSettings zlibsettings; /*settings for the zlib encoder, such as window size, ...*/

   unsigned autoLeaveOutAlphaChannel; /*automatically use color type without alpha instead of given one, if given image is opaque*/
   unsigned force_palette; /*force creating a PLTE chunk if colortype is 2 or 6 (= a suggested palette). If colortype is 3, PLTE is _always_ created.*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
   unsigned add_id; /*add LodePNG version as text chunk*/
   unsigned text_compression; /*encode text chunks as zTXt chunks instead of tEXt chunks, and use compression in iTXt chunks*/
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
} LodePNG_EncodeSettings;

void LodePNG_EncodeSettings_init(LodePNG_EncodeSettings *settings);

typedef struct LodePNG_Encoder {
   LodePNG_EncodeSettings settings;
   LodePNG_InfoPng
   infoPng; /*the info specified by the user may not be changed by the encoder. The encoder will try to generate a PNG close to the given info.*/
   LodePNG_InfoRaw infoRaw; /*put the properties of the input raw image in here*/
   unsigned error;
} LodePNG_Encoder;

void LodePNG_Encoder_init(LodePNG_Encoder *encoder);
void LodePNG_Encoder_cleanup(LodePNG_Encoder *encoder);
void LodePNG_Encoder_copy(LodePNG_Encoder *dest, const LodePNG_Encoder *source);

/*This function allocates the out buffer and stores the size in *outsize.*/
void LodePNG_encode(LodePNG_Encoder *encoder, unsigned char **out, size_t *outsize, const unsigned char *image, unsigned w, unsigned h);
unsigned LodePNG_encode32(unsigned char **out, size_t *outsize, const unsigned char *image, unsigned w, unsigned h); /*return value is error*/
#ifdef LODEPNG_COMPILE_DISK
unsigned LodePNG_encode32f(const char *filename, const unsigned char *image, unsigned w, unsigned h);
#endif /*LODEPNG_COMPILE_DISK*/
#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_DISK
/*free functions allowing to load and save a file from/to harddisk*/
/*This function allocates the out buffer and stores the size in *outsize.*/
unsigned LodePNG_loadFile(unsigned char **out, size_t *outsize, const char *filename);
unsigned LodePNG_saveFile(const unsigned char *buffer, size_t buffersize, const char *filename);
#endif


#endif


