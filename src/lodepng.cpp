/************************************************************************
*
* Copyright (C) 2014-2019 Barbara Geller & Ansel Sermersheim
* Copyright (C) 1997-2014 by Dimitri van Heesch
*
* DoxyPress is free software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License version 2
* as published by the Free Software Foundation.
*
* DoxyPress is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* Documents produced by DoxyPress are derivative works derived from the
* input used in their production; they are not affected by this license.
*
*************************************************************************/

void LodePNG_IText_init(LodePNG_IText *text)
{
   text->num = 0;
   text->keys = NULL;
   text->langtags = NULL;
   text->transkeys = NULL;
   text->strings = NULL;
}

void LodePNG_IText_cleanup(LodePNG_IText *text)
{
   LodePNG_IText_clear(text);
}

unsigned LodePNG_IText_copy(LodePNG_IText *dest, const LodePNG_IText *source)
{
   size_t i = 0;
   dest->keys = 0;
   dest->langtags = 0;
   dest->transkeys = 0;
   dest->strings = 0;
   dest->num = 0;
   for (i = 0; i < source->num; i++) {
      unsigned error = LodePNG_IText_add(dest, source->keys[i], source->langtags[i], source->transkeys[i], source->strings[i]);
      if (error) {
         return error;
      }
   }
   return 0;
}

void LodePNG_IText_clear(LodePNG_IText *text)
{
   size_t i;
   for (i = 0; i < text->num; i++) {
      string_cleanup(&text->keys[i]);
      string_cleanup(&text->langtags[i]);
      string_cleanup(&text->transkeys[i]);
      string_cleanup(&text->strings[i]);
   }
   free(text->keys);
   free(text->langtags);
   free(text->transkeys);
   free(text->strings);
}

unsigned LodePNG_IText_add(LodePNG_IText *text, const char *key, const char *langtag, const char *transkey, const char *str)
{
   char **new_keys = (char **)(realloc(text->keys, sizeof(char *) * (text->num + 1)));
   char **new_langtags = (char **)(realloc(text->langtags, sizeof(char *) * (text->num + 1)));
   char **new_transkeys = (char **)(realloc(text->transkeys, sizeof(char *) * (text->num + 1)));
   char **new_strings = (char **)(realloc(text->strings, sizeof(char *) * (text->num + 1)));
   if (!new_keys || !new_langtags || !new_transkeys || !new_strings) {
      free(new_keys);
      free(new_langtags);
      free(new_transkeys);
      free(new_strings);
      return 9934;
   }

   text->num++;
   text->keys = new_keys;
   text->langtags = new_langtags;
   text->transkeys = new_transkeys;
   text->strings = new_strings;

   string_init(&text->keys[text->num - 1]);
   string_set(&text->keys[text->num - 1], key);

   string_init(&text->langtags[text->num - 1]);
   string_set(&text->langtags[text->num - 1], langtag);

   string_init(&text->transkeys[text->num - 1]);
   string_set(&text->transkeys[text->num - 1], transkey);

   string_init(&text->strings[text->num - 1]);
   string_set(&text->strings[text->num - 1], str);

   return 0;
}

#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

void LodePNG_InfoPng_init(LodePNG_InfoPng *info)
{
   info->width = info->height = 0;
   LodePNG_InfoColor_init(&info->color);
   info->interlaceMethod = 0;
   info->compressionMethod = 0;
   info->filterMethod = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
   info->background_defined = 0;
   info->background_r = info->background_g = info->background_b = 0;

   LodePNG_Text_init(&info->text);
   LodePNG_IText_init(&info->itext);

   info->time_defined = 0;
   info->phys_defined = 0;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
   LodePNG_UnknownChunks_init(&info->unknown_chunks);
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
}

void LodePNG_InfoPng_cleanup(LodePNG_InfoPng *info)
{
   LodePNG_InfoColor_cleanup(&info->color);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
   LodePNG_Text_cleanup(&info->text);
   LodePNG_IText_cleanup(&info->itext);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
   LodePNG_UnknownChunks_cleanup(&info->unknown_chunks);
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
}

unsigned LodePNG_InfoPng_copy(LodePNG_InfoPng *dest, const LodePNG_InfoPng *source)
{
   unsigned error = 0;
   LodePNG_InfoPng_cleanup(dest);
   *dest = *source;
   LodePNG_InfoColor_init(&dest->color);
   error = LodePNG_InfoColor_copy(&dest->color, &source->color);
   if (error) {
      return error;
   }

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
   error = LodePNG_Text_copy(&dest->text, &source->text);
   if (error) {
      return error;
   }
   error = LodePNG_IText_copy(&dest->itext, &source->itext);
   if (error) {
      return error;
   }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
   LodePNG_UnknownChunks_init(&dest->unknown_chunks);
   error = LodePNG_UnknownChunks_copy(&dest->unknown_chunks, &source->unknown_chunks);
   if (error) {
      return error;
   }
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
   return error;
}

void LodePNG_InfoPng_swap(LodePNG_InfoPng *a, LodePNG_InfoPng *b)
{
   LodePNG_InfoPng temp = *a;
   *a = *b;
   *b = temp;
}

unsigned LodePNG_InfoColor_copy(LodePNG_InfoColor *dest, const LodePNG_InfoColor *source)
{
   size_t i;
   LodePNG_InfoColor_cleanup(dest);
   *dest = *source;
   dest->palette = (unsigned char *)malloc(source->palettesize * 4);
   if (!dest->palette && source->palettesize) {
      return 9935;
   }
   for (i = 0; i < source->palettesize * 4; i++) {
      dest->palette[i] = source->palette[i];
   }
   return 0;
}

void LodePNG_InfoRaw_init(LodePNG_InfoRaw *info)
{
   LodePNG_InfoColor_init(&info->color);
}

void LodePNG_InfoRaw_cleanup(LodePNG_InfoRaw *info)
{
   LodePNG_InfoColor_cleanup(&info->color);
}

unsigned LodePNG_InfoRaw_copy(LodePNG_InfoRaw *dest, const LodePNG_InfoRaw *source)
{
   unsigned error = 0;
   LodePNG_InfoRaw_cleanup(dest);

   *dest = *source;
   LodePNG_InfoColor_init(&dest->color);
   error = LodePNG_InfoColor_copy(&dest->color, &source->color);

   return error;
}


/*
converts from any color type to 24-bit or 32-bit (later maybe more supported). return value = LodePNG error code
the out buffer must have (w * h * bpp + 7) / 8 bytes, where bpp is the bits per pixel of the output color type (LodePNG_InfoColor_getBpp)
for < 8 bpp images, there may _not_ be padding bits at the end of scanlines.
*/
unsigned LodePNG_convert(unsigned char *out, const unsigned char *in, LodePNG_InfoColor *infoOut, LodePNG_InfoColor *infoIn, unsigned w, unsigned h)
{
   const size_t numpixels = w * h; /*amount of pixels*/
   const unsigned OUT_BYTES = LodePNG_InfoColor_getBpp(infoOut) / 8; /*bytes per pixel in the output image*/
   const unsigned OUT_ALPHA = LodePNG_InfoColor_isAlphaType(infoOut); /*use 8-bit alpha channel*/
   size_t i, c, bp = 0; /*bitpointer, used by less-than-8-bit color types*/

   /*cases where in and out already have the same format*/
   if (LodePNG_InfoColor_equal(infoIn, infoOut)) {
      size_t i, size = (w * h * LodePNG_InfoColor_getBpp(infoIn) + 7) / 8;
      for (i = 0; i < size; i++) {
         out[i] = in[i];
      }
      return 0;
   }

   if ((infoOut->colorType == 2 || infoOut->colorType == 6) && infoOut->bitDepth == 8) {
      if (infoIn->bitDepth == 8) {
         switch (infoIn->colorType) {
            case 0: /*greyscale color*/
               for (i = 0; i < numpixels; i++) {
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = 255;
                  }
                  out[OUT_BYTES * i + 0] = out[OUT_BYTES * i + 1] = out[OUT_BYTES * i + 2] = in[i];
                  if (OUT_ALPHA && infoIn->key_defined && in[i] == infoIn->key_r) {
                     out[OUT_BYTES * i + 3] = 0;
                  }
               }
               break;
            case 2: /*RGB color*/
               for (i = 0; i < numpixels; i++) {
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = 255;
                  }
                  for (c = 0; c < 3; c++) {
                     out[OUT_BYTES * i + c] = in[3 * i + c];
                  }
                  if (OUT_ALPHA && infoIn->key_defined == 1 && in[3 * i + 0] == infoIn->key_r && in[3 * i + 1] == infoIn->key_g && in[3 * i + 2] == infoIn->key_b) {
                     out[OUT_BYTES * i + 3] = 0;
                  }
               }
               break;
            case 3: /*indexed color (palette)*/
               for (i = 0; i < numpixels; i++) {
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = 255;
                  }
                  if (in[i] >= infoIn->palettesize) {
                     return 46;
                  }
                  for (c = 0; c < OUT_BYTES; c++) {
                     out[OUT_BYTES * i + c] = infoIn->palette[4 * in[i] + c];   /*get rgb colors from the palette*/
                  }
               }
               break;
            case 4: /*greyscale with alpha*/
               for (i = 0; i < numpixels; i++) {
                  out[OUT_BYTES * i + 0] = out[OUT_BYTES * i + 1] = out[OUT_BYTES * i + 2] = in[2 * i + 0];
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = in[2 * i + 1];
                  }
               }
               break;
            case 6: /*RGB with alpha*/
               for (i = 0; i < numpixels; i++) {
                  for (c = 0; c < OUT_BYTES; c++) {
                     out[OUT_BYTES * i + c] = in[4 * i + c];
                  }
               }
               break;
            default:
               break;
         }
      } else if (infoIn->bitDepth == 16) {
         switch (infoIn->colorType) {
            case 0: /*greyscale color*/
               for (i = 0; i < numpixels; i++) {
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = 255;
                  }
                  out[OUT_BYTES * i + 0] = out[OUT_BYTES * i + 1] = out[OUT_BYTES * i + 2] = in[2 * i];
                  if (OUT_ALPHA && infoIn->key_defined && 256U * in[i] + in[i + 1] == infoIn->key_r) {
                     out[OUT_BYTES * i + 3] = 0;
                  }
               }
               break;
            case 2: /*RGB color*/
               for (i = 0; i < numpixels; i++) {
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = 255;
                  }
                  for (c = 0; c < 3; c++) {
                     out[OUT_BYTES * i + c] = in[6 * i + 2 * c];
                  }
                  if (OUT_ALPHA && infoIn->key_defined && 256U * in[6 * i + 0] + in[6 * i + 1] == infoIn->key_r &&
                        256U * in[6 * i + 2] + in[6 * i + 3] == infoIn->key_g && 256U * in[6 * i + 4] + in[6 * i + 5] == infoIn->key_b) {
                     out[OUT_BYTES * i + 3] = 0;
                  }
               }
               break;
            case 4: /*greyscale with alpha*/
               for (i = 0; i < numpixels; i++) {
                  out[OUT_BYTES * i + 0] = out[OUT_BYTES * i + 1] = out[OUT_BYTES * i + 2] = in[4 * i]; /*most significant byte*/
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = in[4 * i + 2];
                  }
               }
               break;
            case 6: /*RGB with alpha*/
               for (i = 0; i < numpixels; i++) {
                  for (c = 0; c < OUT_BYTES; c++) {
                     out[OUT_BYTES * i + c] = in[8 * i + 2 * c];
                  }
               }
               break;
            default:
               break;
         }
      } else { /*infoIn->bitDepth is less than 8 bit per channel*/
         switch (infoIn->colorType) {
            case 0: /*greyscale color*/
               for (i = 0; i < numpixels; i++) {
                  unsigned value = readBitsFromReversedStream(&bp, in, infoIn->bitDepth);
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = 255;
                  }
                  if (OUT_ALPHA && infoIn->key_defined && value && ((1U << infoIn->bitDepth) - 1U) == infoIn->key_r && ((1U << infoIn->bitDepth) - 1U)) {
                     out[OUT_BYTES * i + 3] = 0;
                  }
                  value = (value * 255) / ((1 << infoIn->bitDepth) - 1); /*scale value from 0 to 255*/
                  out[OUT_BYTES * i + 0] = out[OUT_BYTES * i + 1] = out[OUT_BYTES * i + 2] = (unsigned char)(value);
               }
               break;
            case 3: /*indexed color (palette)*/
               for (i = 0; i < numpixels; i++) {
                  unsigned value = readBitsFromReversedStream(&bp, in, infoIn->bitDepth);
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 3] = 255;
                  }
                  if (value >= infoIn->palettesize) {
                     return 47;
                  }
                  for (c = 0; c < OUT_BYTES; c++) {
                     out[OUT_BYTES * i + c] = infoIn->palette[4 * value + c];   /*get rgb colors from the palette*/
                  }
               }
               break;
            default:
               break;
         }
      }

   } else if (LodePNG_InfoColor_isGreyscaleType(infoOut) && infoOut->bitDepth == 8) {
      // conversion from grayscale to grayscale

      if (! LodePNG_InfoColor_isGreyscaleType(infoIn)) {
         return 62;
      }

      if (infoIn->bitDepth == 8) {
         switch (infoIn->colorType) {
            case 0: /*greyscale color*/
               for (i = 0; i < numpixels; i++) {
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 1] = 255;
                  }
                  out[OUT_BYTES * i] = in[i];
                  if (OUT_ALPHA && infoIn->key_defined && in[i] == infoIn->key_r) {
                     out[OUT_BYTES * i + 1] = 0;
                  }
               }
               break;
            case 4: /*greyscale with alpha*/
               for (i = 0; i < numpixels; i++) {
                  out[OUT_BYTES * i + 0] = in[2 * i + 0];
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 1] = in[2 * i + 1];
                  }
               }
               break;
            default:
               return 31;
         }
      } else if (infoIn->bitDepth == 16) {
         switch (infoIn->colorType) {
            case 0: /*greyscale color*/
               for (i = 0; i < numpixels; i++) {
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 1] = 255;
                  }
                  out[OUT_BYTES * i] = in[2 * i];
                  if (OUT_ALPHA && infoIn->key_defined && 256U * in[i] + in[i + 1] == infoIn->key_r) {
                     out[OUT_BYTES * i + 1] = 0;
                  }
               }
               break;
            case 4: /*greyscale with alpha*/
               for (i = 0; i < numpixels; i++) {
                  out[OUT_BYTES * i] = in[4 * i]; /*most significant byte*/
                  if (OUT_ALPHA) {
                     out[OUT_BYTES * i + 1] = in[4 * i + 2];   /*most significant byte*/
                  }
               }
               break;
            default:
               return 31;
         }
      } else { /*infoIn->bitDepth is less than 8 bit per channel*/
         if (infoIn->colorType != 0) {
            return 31;   /*colorType 0 is the only grayscale type with < 8 bits per channel*/
         }
         for (i = 0; i < numpixels; i++) {
            unsigned value = readBitsFromReversedStream(&bp, in, infoIn->bitDepth);
            if (OUT_ALPHA) {
               out[OUT_BYTES * i + 1] = 255;
            }
            if (OUT_ALPHA && infoIn->key_defined && value && ((1U << infoIn->bitDepth) - 1U) == infoIn->key_r && ((1U << infoIn->bitDepth) - 1U)) {
               out[OUT_BYTES * i + 1] = 0;
            }
            value = (value * 255) / ((1 << infoIn->bitDepth) - 1); /*scale value from 0 to 255*/
            out[OUT_BYTES * i] = (unsigned char)(value);
         }
      }
   } else {
      return 59;
   }

   return 0;
}

/*Paeth predicter, used by PNG filter type 4*/
static int paethPredictor(int a, int b, int c)
{
   int p = a + b - c;
   int pa = p > a ? p - a : a - p;
   int pb = p > b ? p - b : b - p;
   int pc = p > c ? p - c : c - p;

   if (pa <= pb && pa <= pc) {
      return a;
   } else if (pb <= pc) {
      return b;
   } else {
      return c;
   }
}

/*shared values used by multiple Adam7 related functions*/

static const unsigned ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 }; /*x start values*/
static const unsigned ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 }; /*y start values*/
static const unsigned ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 }; /*x delta values*/
static const unsigned ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 }; /*y delta values*/

static void Adam7_getpassvalues(unsigned passw[7], unsigned passh[7], size_t filter_passstart[8], size_t padded_passstart[8], size_t passstart[8],
                                unsigned w, unsigned h, unsigned bpp)
{
   /*the passstart values have 8 values: the 8th one actually indicates the byte after the end of the 7th (= last) pass*/
   unsigned i;

   /*calculate width and height in pixels of each pass*/
   for (i = 0; i < 7; i++) {
      passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
      passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
      if (passw[i] == 0) {
         passh[i] = 0;
      }
      if (passh[i] == 0) {
         passw[i] = 0;
      }
   }

   filter_passstart[0] = padded_passstart[0] = passstart[0] = 0;
   for (i = 0; i < 7; i++) {
      filter_passstart[i + 1] = filter_passstart[i] + ((passw[i] &&
                                passh[i]) ? passh[i] * (1 + (passw[i] * bpp + 7) / 8) : 0); /*if passw[i] is 0, it's 0 bytes, not 1 (no filtertype-byte)*/
      padded_passstart[i + 1] = padded_passstart[i] + passh[i] * ((passw[i] * bpp + 7) /
                                8); /*bits padded if needed to fill full byte at end of each scanline*/
      passstart[i + 1] = passstart[i] + (passh[i] * passw[i] * bpp + 7) / 8; /*only padded at end of reduced image*/
   }
}

#ifdef LODEPNG_COMPILE_DECODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Decoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*read the information from the header and store it in the LodePNG_Info. return value is error*/
void LodePNG_inspect(LodePNG_Decoder *decoder, const unsigned char *in, size_t inlength)
{
   if (inlength == 0 || in == 0) {
      decoder->error = 48;   /*the given data is empty*/
      return;
   }
   if (inlength < 29) {
      decoder->error = 27;   /*error: the data length is smaller than the length of the header*/
      return;
   }

   /*when decoding a new PNG image, make sure all parameters created after previous decoding are reset*/
   LodePNG_InfoPng_cleanup(&decoder->infoPng);
   LodePNG_InfoPng_init(&decoder->infoPng);
   decoder->error = 0;

   if (in[0] != 137 || in[1] != 80 || in[2] != 78 || in[3] != 71 || in[4] != 13 || in[5] != 10 || in[6] != 26 || in[7] != 10) {
      decoder->error = 28;   /*error: the first 8 bytes are not the correct PNG signature*/
      return;
   }
   if (in[12] != 'I' || in[13] != 'H' || in[14] != 'D' || in[15] != 'R') {
      decoder->error = 29;   /*error: it doesn't start with a IHDR chunk!*/
      return;
   }

   /*read the values given in the header*/
   decoder->infoPng.width = LodePNG_read32bitInt(&in[16]);
   decoder->infoPng.height = LodePNG_read32bitInt(&in[20]);
   decoder->infoPng.color.bitDepth = in[24];
   decoder->infoPng.color.colorType = in[25];
   decoder->infoPng.compressionMethod = in[26];
   decoder->infoPng.filterMethod = in[27];
   decoder->infoPng.interlaceMethod = in[28];

   if (!decoder->settings.ignoreCrc) {
      unsigned CRC = LodePNG_read32bitInt(&in[29]);
      unsigned checksum = Crc32_crc(&in[12], 17);
      if (CRC != checksum) {
         decoder->error = 57;
         return;
      }
   }

   if (decoder->infoPng.compressionMethod != 0) {
      decoder->error = 32;   /*error: only compression method 0 is allowed in the specification*/
      return;
   }
   if (decoder->infoPng.filterMethod != 0)      {
      decoder->error = 33;   /*error: only filter method 0 is allowed in the specification*/
      return;
   }
   if (decoder->infoPng.interlaceMethod > 1)    {
      decoder->error = 34;   /*error: only interlace methods 0 and 1 exist in the specification*/
      return;
   }

   decoder->error = checkColorValidity(decoder->infoPng.color.colorType, decoder->infoPng.color.bitDepth);
}

static unsigned unfilterScanline(unsigned char *recon, const unsigned char *scanline, const unsigned char *precon, size_t bytewidth,
                                 unsigned char filterType, size_t length)
{
   /*
   For PNG filter method 0
   unfilter a PNG image scanline by scanline. when the pixels are smaller than 1 byte, the filter works byte per byte (bytewidth = 1)
   precon is the previous unfiltered scanline, recon the result, scanline the current one
   the incoming scanlines do NOT include the filtertype byte, that one is given in the parameter filterType instead
   recon and scanline MAY be the same memory address! precon must be disjoint.
   */

   size_t i;
   switch (filterType) {
      case 0:
         for (i = 0; i < length; i++) {
            recon[i] = scanline[i];
         }
         break;
      case 1:
         for (i =         0; i < bytewidth; i++) {
            recon[i] = scanline[i];
         }
         for (i = bytewidth; i <    length; i++) {
            recon[i] = scanline[i] + recon[i - bytewidth];
         }
         break;
      case 2:
         if (precon) for (i = 0; i < length; i++) {
               recon[i] = scanline[i] + precon[i];
            }
         else       for (i = 0; i < length; i++) {
               recon[i] = scanline[i];
            }
         break;
      case 3:
         if (precon) {
            for (i =         0; i < bytewidth; i++) {
               recon[i] = scanline[i] + precon[i] / 2;
            }
            for (i = bytewidth; i <    length; i++) {
               recon[i] = scanline[i] + ((recon[i - bytewidth] + precon[i]) / 2);
            }
         } else {
            for (i =         0; i < bytewidth; i++) {
               recon[i] = scanline[i];
            }
            for (i = bytewidth; i <    length; i++) {
               recon[i] = scanline[i] + recon[i - bytewidth] / 2;
            }
         }
         break;
      case 4:
         if (precon) {
            for (i =         0; i < bytewidth; i++) {
               recon[i] = (unsigned char)(scanline[i] + paethPredictor(0, precon[i], 0));
            }
            for (i = bytewidth; i <    length; i++) {
               recon[i] = (unsigned char)(scanline[i] + paethPredictor(recon[i - bytewidth], precon[i], precon[i - bytewidth]));
            }
         } else {
            for (i =         0; i < bytewidth; i++) {
               recon[i] = scanline[i];
            }
            for (i = bytewidth; i <    length; i++) {
               recon[i] = (unsigned char)(scanline[i] + paethPredictor(recon[i - bytewidth], 0, 0));
            }
         }
         break;
      default:
         return 36; /*error: unexisting filter type given*/
   }
   return 0;
}

static unsigned unfilter(unsigned char *out, const unsigned char *in, unsigned w, unsigned h, unsigned bpp)
{
   /*
   For PNG filter method 0
   this function unfilters a single image (e.g. without interlacing this is called once, with Adam7 it's called 7 times)
   out must have enough bytes allocated already, in must have the scanlines + 1 filtertype byte per scanline
   w and h are image dimensions or dimensions of reduced image, bpp is bits per pixel
   in and out are allowed to be the same memory address!
   */

   unsigned y;
   unsigned char *prevline = 0;

   size_t bytewidth = (bpp + 7) / 8; /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
   size_t linebytes = (w * bpp + 7) / 8;

   for (y = 0; y < h; y++) {
      size_t outindex = linebytes * y;
      size_t inindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
      unsigned char filterType = in[inindex];

      unsigned error = unfilterScanline(&out[outindex], &in[inindex + 1], prevline, bytewidth, filterType, linebytes);
      if (error) {
         return error;
      }

      prevline = &out[outindex];
   }

   return 0;
}

static void Adam7_deinterlace(unsigned char *out, const unsigned char *in, unsigned w, unsigned h, unsigned bpp)
{
   /*Note: this function works on image buffers WITHOUT padding bits at end of scanlines with non-multiple-of-8 bit amounts, only between reduced images is padding
   out must be big enough AND must be 0 everywhere if bpp < 8 in the current implementation (because that's likely a little bit faster)*/
   unsigned passw[7], passh[7];
   size_t filter_passstart[8], padded_passstart[8], passstart[8];
   unsigned i;

   Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

   if (bpp >= 8) {
      for (i = 0; i < 7; i++) {
         unsigned x, y, b;
         size_t bytewidth = bpp / 8;
         for (y = 0; y < passh[i]; y++)
            for (x = 0; x < passw[i]; x++) {
               size_t pixelinstart = passstart[i] + (y * passw[i] + x) * bytewidth;
               size_t pixeloutstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
               for (b = 0; b < bytewidth; b++) {
                  out[pixeloutstart + b] = in[pixelinstart + b];
               }
            }
      }
   } else { /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
      for (i = 0; i < 7; i++) {
         unsigned x, y, b;
         unsigned ilinebits = bpp * passw[i];
         unsigned olinebits = bpp * w;
         size_t obp, ibp; /*bit pointers (for out and in buffer)*/
         for (y = 0; y < passh[i]; y++)
            for (x = 0; x < passw[i]; x++) {
               ibp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
               obp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
               for (b = 0; b < bpp; b++) {
                  unsigned char bit = readBitFromReversedStream(&ibp, in);
                  setBitOfReversedStream0(&obp, out, bit); /*note that this function assumes the out buffer is completely 0, use setBitOfReversedStream otherwise*/
               }
            }
      }
   }
}

static void removePaddingBits(unsigned char *out, const unsigned char *in, size_t olinebits, size_t ilinebits, unsigned h)
{
   /*
   After filtering there are still padding bits if scanlines have non multiple of 8 bit amounts. They need to be removed (except at last scanline of (Adam7-reduced) image) before working with pure image buffers for the Adam7 code, the color convert code and the output to the user.
   in and out are allowed to be the same buffer, in may also be higher but still overlapping; in must have >= ilinebits*h bits, out must have >= olinebits*h bits, olinebits must be <= ilinebits
   also used to move bits after earlier such operations happened, e.g. in a sequence of reduced images from Adam7
   only useful if (ilinebits - olinebits) is a value in the range 1..7
   */
   unsigned y;
   size_t diff = ilinebits - olinebits;
   size_t obp = 0, ibp = 0; /*bit pointers*/
   for (y = 0; y < h; y++) {
      size_t x;
      for (x = 0; x < olinebits; x++) {
         unsigned char bit = readBitFromReversedStream(&ibp, in);
         setBitOfReversedStream(&obp, out, bit);
      }
      ibp += diff;
   }
}

/*out must be buffer big enough to contain full image, and in must contain the full decompressed data from the IDAT chunks*/
static unsigned postProcessScanlines(unsigned char *out, unsigned char *in, const LodePNG_InfoPng *infoPng) /*return value is error*/
{
   /*
   This function converts the filtered-padded-interlaced data into pure 2D image buffer with the PNG's colortype. Steps:
   *) if no Adam7: 1) unfilter 2) remove padding bits (= posible extra bits per scanline if bpp < 8)
   *) if adam7: 1) 7x unfilter 2) 7x remove padding bits 3) Adam7_deinterlace
   NOTE: the in buffer will be overwritten with intermediate data!
   */
   unsigned bpp = LodePNG_InfoColor_getBpp(&infoPng->color);
   unsigned w = infoPng->width;
   unsigned h = infoPng->height;
   unsigned error = 0;
   if (bpp == 0) {
      return 31;   /*error: invalid colortype*/
   }

   if (infoPng->interlaceMethod == 0) {
      if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) {
         error = unfilter(in, in, w, h, bpp);
         if (error) {
            return error;
         }
         removePaddingBits(out, in, w * bpp, ((w * bpp + 7) / 8) * 8, h);
      } else {
         error = unfilter(out, in, w, h, bpp);   /*we can immediatly filter into the out buffer, no other steps needed*/
      }
   } else { /*interlaceMethod is 1 (Adam7)*/
      unsigned passw[7], passh[7];
      size_t filter_passstart[8], padded_passstart[8], passstart[8];
      unsigned i;

      Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

      for (i = 0; i < 7; i++) {
         error = unfilter(&in[padded_passstart[i]], &in[filter_passstart[i]], passw[i], passh[i], bpp);
         if (error) {
            return error;
         }
         if (bpp < 8) { /*TODO: possible efficiency improvement: if in this reduced image the bits fit nicely in 1 scanline, move bytes instead of bits or move not at all*/
            /*remove padding bits in scanlines; after this there still may be padding bits between the different reduced images: each reduced image still starts nicely at a byte*/
            removePaddingBits(&in[passstart[i]], &in[padded_passstart[i]], passw[i] * bpp, ((passw[i] * bpp + 7) / 8) * 8, passh[i]);
         }
      }

      Adam7_deinterlace(out, in, w, h, bpp);
   }

   return error;
}

/*read a PNG, the result will be in the same color type as the PNG (hence "generic")*/
static void decodeGeneric(LodePNG_Decoder *decoder, unsigned char **out, size_t *outsize, const unsigned char *in, size_t size)
{
   unsigned char IEND = 0;
   const unsigned char *chunk;
   size_t i;
   ucvector idat; /*the data from idat chunks*/

   /*for unknown chunk order*/
   unsigned unknown = 0;
   unsigned critical_pos = 1; /*1 = after IHDR, 2 = after PLTE, 3 = after IDAT*/

   /*provide some proper output values if error will happen*/
   *out = 0;
   *outsize = 0;

   if (size == 0 || in == 0) {
      decoder->error = 48;   /*the given data is empty*/
      return;
   }

   LodePNG_inspect(decoder, in, size); /*reads header and resets other parameters in decoder->infoPng*/
   if (decoder->error) {
      return;
   }

   ucvector_init(&idat);

   chunk = &in[33]; /*first byte of the first chunk after the header*/

   while (!IEND) { /*loop through the chunks, ignoring unknown chunks and stopping at IEND chunk. IDAT data is put at the start of the in buffer*/
      unsigned chunkLength;
      const unsigned char *data; /*the data in the chunk*/

      if ((size_t)((chunk - in) + 12) > size || chunk < in) {
         decoder->error = 30;   /*error: size of the in buffer too small to contain next chunk*/
         break;
      }
      chunkLength = LodePNG_chunk_length(chunk); /*length of the data of the chunk, excluding the length bytes, chunk type and CRC bytes*/
      if (chunkLength > 2147483647) {
         decoder->error = 63;
         break;
      }
      if ((size_t)((chunk - in) + chunkLength + 12) > size || (chunk + chunkLength + 12) < in) {
         decoder->error = 35;   /*error: size of the in buffer too small to contain next chunk*/
         break;
      }
      data = LodePNG_chunk_data_const(chunk);

      /*IDAT chunk, containing compressed image data*/
      if (LodePNG_chunk_type_equals(chunk, "IDAT")) {
         size_t oldsize = idat.size;
         if (!ucvector_resize(&idat, oldsize + chunkLength)) {
            decoder->error = 9936;
            break;
         }
         for (i = 0; i < chunkLength; i++) {
            idat.data[oldsize + i] = data[i];
         }
         critical_pos = 3;
      }
      /*IEND chunk*/
      else if (LodePNG_chunk_type_equals(chunk, "IEND")) {
         IEND = 1;
      }
      /*palette chunk (PLTE)*/
      else if (LodePNG_chunk_type_equals(chunk, "PLTE")) {
         unsigned pos = 0;
         if (decoder->infoPng.color.palette) {
            free(decoder->infoPng.color.palette);
         }
         decoder->infoPng.color.palettesize = chunkLength / 3;
         decoder->infoPng.color.palette = (unsigned char *)malloc(4 * decoder->infoPng.color.palettesize);
         if (!decoder->infoPng.color.palette && decoder->infoPng.color.palettesize) {
            decoder->error = 9937;
            break;
         }
         if (!decoder->infoPng.color.palette) {
            decoder->infoPng.color.palettesize = 0;   /*malloc failed...*/
         }
         if (decoder->infoPng.color.palettesize > 256) {
            decoder->error = 38;   /*error: palette too big*/
            break;
         }
         for (i = 0; i < decoder->infoPng.color.palettesize; i++) {
            decoder->infoPng.color.palette[4 * i + 0] = data[pos++]; /*R*/
            decoder->infoPng.color.palette[4 * i + 1] = data[pos++]; /*G*/
            decoder->infoPng.color.palette[4 * i + 2] = data[pos++]; /*B*/
            decoder->infoPng.color.palette[4 * i + 3] = 255; /*alpha*/
         }
         critical_pos = 2;
      }
      /*palette transparency chunk (tRNS)*/
      else if (LodePNG_chunk_type_equals(chunk, "tRNS")) {
         if (decoder->infoPng.color.colorType == 3) {
            if (chunkLength > decoder->infoPng.color.palettesize) {
               decoder->error = 39;   /*error: more alpha values given than there are palette entries*/
               break;
            }
            for (i = 0; i < chunkLength; i++) {
               decoder->infoPng.color.palette[4 * i + 3] = data[i];
            }
         } else if (decoder->infoPng.color.colorType == 0) {
            if (chunkLength != 2) {
               decoder->error = 40;   /*error: this chunk must be 2 bytes for grayscale image*/
               break;
            }
            decoder->infoPng.color.key_defined = 1;
            decoder->infoPng.color.key_r = decoder->infoPng.color.key_g = decoder->infoPng.color.key_b = 256 * data[0] + data[1];
         } else if (decoder->infoPng.color.colorType == 2) {
            if (chunkLength != 6) {
               decoder->error = 41;   /*error: this chunk must be 6 bytes for RGB image*/
               break;
            }
            decoder->infoPng.color.key_defined = 1;
            decoder->infoPng.color.key_r = 256 * data[0] + data[1];
            decoder->infoPng.color.key_g = 256 * data[2] + data[3];
            decoder->infoPng.color.key_b = 256 * data[4] + data[5];
         } else {
            decoder->error = 42;   /*error: tRNS chunk not allowed for other color models*/
            break;
         }
      }
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
      /*background color chunk (bKGD)*/
      else if (LodePNG_chunk_type_equals(chunk, "bKGD")) {
         if (decoder->infoPng.color.colorType == 3) {
            if (chunkLength != 1) {
               decoder->error = 43;   /*error: this chunk must be 1 byte for indexed color image*/
               break;
            }
            decoder->infoPng.background_defined = 1;
            decoder->infoPng.background_r = decoder->infoPng.background_g = decoder->infoPng.background_g = data[0];
         } else if (decoder->infoPng.color.colorType == 0 || decoder->infoPng.color.colorType == 4) {
            if (chunkLength != 2) {
               decoder->error = 44;   /*error: this chunk must be 2 bytes for grayscale image*/
               break;
            }
            decoder->infoPng.background_defined = 1;
            decoder->infoPng.background_r = decoder->infoPng.background_g = decoder->infoPng.background_b = 256 * data[0] + data[1];
         } else if (decoder->infoPng.color.colorType == 2 || decoder->infoPng.color.colorType == 6) {
            if (chunkLength != 6) {
               decoder->error = 45;   /*error: this chunk must be 6 bytes for grayscale image*/
               break;
            }
            decoder->infoPng.background_defined = 1;
            decoder->infoPng.background_r = 256 * data[0] + data[1];
            decoder->infoPng.background_g = 256 * data[2] + data[3];
            decoder->infoPng.background_b = 256 * data[4] + data[5];
         }
      }
      /*text chunk (tEXt)*/
      else if (LodePNG_chunk_type_equals(chunk, "tEXt")) {
         if (decoder->settings.readTextChunks) {
            char *key = 0, *str = 0;

            while (!decoder->error) { /*not really a while loop, only used to break on error*/
               unsigned length, string2_begin;

               for (length = 0; length < chunkLength && data[length] != 0; length++) ;
               if (length + 1 >= chunkLength) {
                  decoder->error = 75;
                  break;
               }
               key = (char *)malloc(length + 1);
               if (!key) {
                  decoder->error = 9938;
                  break;
               }
               key[length] = 0;
               for (i = 0; i < length; i++) {
                  key[i] = data[i];
               }

               string2_begin = length + 1;
               if (string2_begin > chunkLength)  {
                  decoder->error = 75;
                  break;
               }
               length = chunkLength - string2_begin;
               str = (char *)malloc(length + 1);
               if (!str) {
                  decoder->error = 9939;
                  break;
               }
               str[length] = 0;
               for (i = 0; i < length; i++) {
                  str[i] = data[string2_begin + i];
               }

               decoder->error = LodePNG_Text_add(&decoder->infoPng.text, key, str);

               break;
            }

            free(key);
            free(str);
         }
      }
      /*compressed text chunk (zTXt)*/
      else if (LodePNG_chunk_type_equals(chunk, "zTXt")) {
         if (decoder->settings.readTextChunks) {
            unsigned length, string2_begin;
            char *key = 0;
            ucvector decoded;

            ucvector_init(&decoded);

            while (!decoder->error) { /*not really a while loop, only used to break on error*/
               for (length = 0; length < chunkLength && data[length] != 0; length++) ;
               if (length + 2 >= chunkLength) {
                  decoder->error = 75;
                  break;
               }
               key = (char *)malloc(length + 1);
               if (!key) {
                  decoder->error = 9940;
                  break;
               }
               key[length] = 0;
               for (i = 0; i < length; i++) {
                  key[i] = data[i];
               }

               if (data[length + 1] != 0) {
                  decoder->error = 72;   /*the 0 byte indicating compression must be 0*/
                  break;
               }

               string2_begin = length + 2;
               if (string2_begin > chunkLength)  {
                  decoder->error = 75;
                  break;
               }
               length = chunkLength - string2_begin;
               decoder->error = LodePNG_decompress(&decoded.data, &decoded.size, (unsigned char *)(&data[string2_begin]), length, &decoder->settings.zlibsettings);
               if (decoder->error) {
                  break;
               }
               ucvector_push_back(&decoded, 0);

               decoder->error = LodePNG_Text_add(&decoder->infoPng.text, key, (char *)decoded.data);

               break;
            }

            free(key);
            ucvector_cleanup(&decoded);
            if (decoder->error) {
               break;
            }
         }
      }
      /*international text chunk (iTXt)*/
      else if (LodePNG_chunk_type_equals(chunk, "iTXt")) {
         if (decoder->settings.readTextChunks) {
            unsigned length, begin, compressed;
            char *key = 0, *langtag = 0, *transkey = 0;
            ucvector decoded;
            ucvector_init(&decoded);

            while (!decoder->error) { /*not really a while loop, only used to break on error*/
               if (chunkLength < 5) {
                  decoder->error = 76;
                  break;
               }
               for (length = 0; length < chunkLength && data[length] != 0; length++) ;
               if (length + 2 >= chunkLength) {
                  decoder->error = 75;
                  break;
               }
               key = (char *)malloc(length + 1);
               if (!key) {
                  decoder->error = 9941;
                  break;
               }
               key[length] = 0;
               for (i = 0; i < length; i++) {
                  key[i] = data[i];
               }

               compressed = data[length + 1];
               if (data[length + 2] != 0) {
                  decoder->error = 72;   /*the 0 byte indicating compression must be 0*/
                  break;
               }

               begin = length + 3;
               length = 0;
               for (i = begin; i < chunkLength && data[i] != 0; i++) {
                  length++;
               }
               if (begin + length + 1 >= chunkLength) {
                  decoder->error = 75;
                  break;
               }
               langtag = (char *)malloc(length + 1);
               if (!langtag) {
                  decoder->error = 9942;
                  break;
               }
               langtag[length] = 0;
               for (i = 0; i < length; i++) {
                  langtag[i] = data[begin + i];
               }

               begin += length + 1;
               length = 0;
               for (i = begin; i < chunkLength && data[i] != 0; i++) {
                  length++;
               }
               if (begin + length + 1 >= chunkLength) {
                  decoder->error = 75;
                  break;
               }
               transkey = (char *)malloc(length + 1);
               if (!transkey) {
                  decoder->error = 9943;
                  break;
               }
               transkey[length] = 0;
               for (i = 0; i < length; i++) {
                  transkey[i] = data[begin + i];
               }

               begin += length + 1;
               if (begin > chunkLength)  {
                  decoder->error = 75;
                  break;
               }
               length = chunkLength - begin;

               if (compressed) {
                  decoder->error = LodePNG_decompress(&decoded.data, &decoded.size, (unsigned char *)(&data[begin]), length, &decoder->settings.zlibsettings);
                  if (decoder->error) {
                     break;
                  }
                  ucvector_push_back(&decoded, 0);
               } else {
                  if (!ucvector_resize(&decoded, length + 1)) {
                     decoder->error = 9944;
                     break;
                  }
                  decoded.data[length] = 0;
                  for (i = 0; i < length; i++) {
                     decoded.data[i] = data[begin + i];
                  }
               }

               decoder->error = LodePNG_IText_add(&decoder->infoPng.itext, key, langtag, transkey, (char *)decoded.data);

               break;
            }

            free(key);
            free(langtag);
            free(transkey);
            ucvector_cleanup(&decoded);
            if (decoder->error) {
               break;
            }
         }
      } else if (LodePNG_chunk_type_equals(chunk, "tIME")) {
         if (chunkLength != 7) {
            decoder->error = 73;
            break;
         }
         decoder->infoPng.time_defined = 1;
         decoder->infoPng.time.year = 256 * data[0] + data[+ 1];
         decoder->infoPng.time.month = data[2];
         decoder->infoPng.time.day = data[3];
         decoder->infoPng.time.hour = data[4];
         decoder->infoPng.time.minute = data[5];
         decoder->infoPng.time.second = data[6];
      } else if (LodePNG_chunk_type_equals(chunk, "pHYs")) {
         if (chunkLength != 9) {
            decoder->error = 74;
            break;
         }
         decoder->infoPng.phys_defined = 1;
         decoder->infoPng.phys_x = 16777216 * data[0] + 65536 * data[1] + 256 * data[2] + data[3];
         decoder->infoPng.phys_y = 16777216 * data[4] + 65536 * data[5] + 256 * data[6] + data[7];
         decoder->infoPng.phys_unit = data[8];
      }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
      else { /*it's not an implemented chunk type, so ignore it: skip over the data*/
         if (LodePNG_chunk_critical(chunk)) {
            decoder->error = 69;   /*error: unknown critical chunk (5th bit of first byte of chunk type is 0)*/
            break;
         }
         unknown = 1;
#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
         if (decoder->settings.rememberUnknownChunks) {
            LodePNG_UnknownChunks *unknown = &decoder->infoPng.unknown_chunks;
            decoder->error = LodePNG_append_chunk(&unknown->data[critical_pos - 1], &unknown->datasize[critical_pos - 1], chunk);
            if (decoder->error) {
               break;
            }
         }
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
      }

      if (!decoder->settings.ignoreCrc && !unknown) { /*check CRC if wanted, only on known chunk types*/
         if (LodePNG_chunk_check_crc(chunk)) {
            decoder->error = 57;
            break;
         }
      }

      if (!IEND) {
         chunk = LodePNG_chunk_next_const(chunk);
      }
   }

   if (!decoder->error) {
      ucvector scanlines;
      ucvector_init(&scanlines);
      if (!ucvector_resize(&scanlines, ((decoder->infoPng.width * (decoder->infoPng.height * LodePNG_InfoColor_getBpp(
                                            &decoder->infoPng.color) + 7)) / 8) + decoder->infoPng.height)) {
         decoder->error = 9945;   /*maximum final image length is already reserved in the vector's length - this is not really necessary*/
      }
      if (!decoder->error) {
         decoder->error = LodePNG_decompress(&scanlines.data, &scanlines.size, idat.data, idat.size,
                                             &decoder->settings.zlibsettings);   /*decompress with the Zlib decompressor*/
      }

      if (!decoder->error) {
         ucvector outv;
         ucvector_init(&outv);
         if (!ucvector_resizev(&outv, (decoder->infoPng.height * decoder->infoPng.width * LodePNG_InfoColor_getBpp(&decoder->infoPng.color) + 7) / 8, 0)) {
            decoder->error = 9946;
         }
         if (!decoder->error) {
            decoder->error = postProcessScanlines(outv.data, scanlines.data, &decoder->infoPng);
         }
         *out = outv.data;
         *outsize = outv.size;
      }
      ucvector_cleanup(&scanlines);
   }

   ucvector_cleanup(&idat);
}

void LodePNG_decode(LodePNG_Decoder *decoder, unsigned char **out, size_t *outsize, const unsigned char *in, size_t insize)
{
   *out = 0;
   *outsize = 0;
   decodeGeneric(decoder, out, outsize, in, insize);
   if (decoder->error) {
      return;
   }
   if (!decoder->settings.color_convert || LodePNG_InfoColor_equal(&decoder->infoRaw.color, &decoder->infoPng.color)) {
      /*same color type, no copying or converting of data needed*/
      /*store the infoPng color settings on the infoRaw so that the infoRaw still reflects what colorType
      the raw image has to the end user*/
      if (!decoder->settings.color_convert) {
         decoder->error = LodePNG_InfoColor_copy(&decoder->infoRaw.color, &decoder->infoPng.color);
         if (decoder->error) {
            return;
         }
      }
   } else {
      /*color conversion needed; sort of copy of the data*/
      unsigned char *data = *out;

      // TODO: check if this works according to the statement in the documentation:
      // "The converter can convert from grayscale input color type, to 8-bit grayscale or grayscale with alpha"

      if (!(decoder->infoRaw.color.colorType == 2 || decoder->infoRaw.color.colorType == 6) && !(decoder->infoRaw.color.bitDepth == 8)) {
         decoder->error = 56;
         return;
      }

      *outsize = (decoder->infoPng.width * decoder->infoPng.height * LodePNG_InfoColor_getBpp(&decoder->infoRaw.color) + 7) / 8;
      *out = (unsigned char *)malloc(*outsize);
      if (!(*out)) {
         decoder->error = 9947;
         *outsize = 0;
      } else {
         decoder->error = LodePNG_convert(*out, data, &decoder->infoRaw.color, &decoder->infoPng.color, decoder->infoPng.width, decoder->infoPng.height);
      }
      free(data);
   }
}

unsigned LodePNG_decode32(unsigned char **out, unsigned *w, unsigned *h, const unsigned char *in, size_t insize)
{
   unsigned error;
   size_t dummy_size;
   LodePNG_Decoder decoder;
   LodePNG_Decoder_init(&decoder);
   LodePNG_decode(&decoder, out, &dummy_size, in, insize);
   error = decoder.error;
   *w = decoder.infoPng.width;
   *h = decoder.infoPng.height;
   LodePNG_Decoder_cleanup(&decoder);
   return error;
}

#ifdef LODEPNG_COMPILE_DISK
unsigned LodePNG_decode32f(unsigned char **out, unsigned *w, unsigned *h, const char *filename)
{
   unsigned char *buffer;
   size_t buffersize;
   unsigned error;
   error = LodePNG_loadFile(&buffer, &buffersize, filename);
   if (!error) {
      error = LodePNG_decode32(out, w, h, buffer, buffersize);
   }
   free(buffer);
   return error;
}
#endif /*LODEPNG_COMPILE_DISK*/

void LodePNG_DecodeSettings_init(LodePNG_DecodeSettings *settings)
{
   settings->color_convert = 1;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
   settings->readTextChunks = 1;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
   settings->ignoreCrc = 0;
#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
   settings->rememberUnknownChunks = 0;
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
   LodeZlib_DecompressSettings_init(&settings->zlibsettings);
}

void LodePNG_Decoder_init(LodePNG_Decoder *decoder)
{
   LodePNG_DecodeSettings_init(&decoder->settings);
   LodePNG_InfoRaw_init(&decoder->infoRaw);
   LodePNG_InfoPng_init(&decoder->infoPng);
   decoder->error = 1;
}

void LodePNG_Decoder_cleanup(LodePNG_Decoder *decoder)
{
   LodePNG_InfoRaw_cleanup(&decoder->infoRaw);
   LodePNG_InfoPng_cleanup(&decoder->infoPng);
}

void LodePNG_Decoder_copy(LodePNG_Decoder *dest, const LodePNG_Decoder *source)
{
   LodePNG_Decoder_cleanup(dest);
   *dest = *source;
   LodePNG_InfoRaw_init(&dest->infoRaw);
   LodePNG_InfoPng_init(&dest->infoPng);
   dest->error = LodePNG_InfoRaw_copy(&dest->infoRaw, &source->infoRaw);
   if (dest->error) {
      return;
   }
   dest->error = LodePNG_InfoPng_copy(&dest->infoPng, &source->infoPng);
   if (dest->error) {
      return;
   }
}

#endif /*LODEPNG_COMPILE_DECODER*/

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Encoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*chunkName must be string of 4 characters*/
static unsigned addChunk(ucvector *out, const char *chunkName, const unsigned char *data, size_t length)
{
   unsigned error = LodePNG_create_chunk(&out->data, &out->size, (unsigned)length, chunkName, data);
   if (error) {
      return error;
   }
   out->allocsize = out->size; /*fix the allocsize again*/
   return 0;
}

static void writeSignature(ucvector *out)
{
   /*8 bytes PNG signature*/
   ucvector_push_back(out, 137);
   ucvector_push_back(out, 80);
   ucvector_push_back(out, 78);
   ucvector_push_back(out, 71);
   ucvector_push_back(out, 13);
   ucvector_push_back(out, 10);
   ucvector_push_back(out, 26);
   ucvector_push_back(out, 10);
}

static unsigned addChunk_IHDR(ucvector *out, unsigned w, unsigned h, unsigned bitDepth, unsigned colorType, unsigned interlaceMethod)
{
   unsigned error = 0;
   ucvector header;
   ucvector_init(&header);

   LodePNG_add32bitInt(&header, w); /*width*/
   LodePNG_add32bitInt(&header, h); /*height*/
   ucvector_push_back(&header, (unsigned char)bitDepth); /*bit depth*/
   ucvector_push_back(&header, (unsigned char)colorType); /*color type*/
   ucvector_push_back(&header, 0); /*compression method*/
   ucvector_push_back(&header, 0); /*filter method*/
   ucvector_push_back(&header, interlaceMethod); /*interlace method*/

   error = addChunk(out, "IHDR", header.data, header.size);
   ucvector_cleanup(&header);

   return error;
}

static unsigned addChunk_PLTE(ucvector *out, const LodePNG_InfoColor *info)
{
   unsigned error = 0;
   size_t i;
   ucvector PLTE;
   ucvector_init(&PLTE);
   for (i = 0; i < info->palettesize * 4; i++) if (i % 4 != 3) {
         ucvector_push_back(&PLTE, info->palette[i]);   /*add all channels except alpha channel*/
      }
   error = addChunk(out, "PLTE", PLTE.data, PLTE.size);
   ucvector_cleanup(&PLTE);

   return error;
}

static unsigned addChunk_tRNS(ucvector *out, const LodePNG_InfoColor *info)
{
   unsigned error = 0;
   size_t i;
   ucvector tRNS;
   ucvector_init(&tRNS);
   if (info->colorType == 3) {
      for (i = 0; i < info->palettesize; i++) {
         ucvector_push_back(&tRNS, info->palette[4 * i + 3]);   /*add only alpha channel*/
      }
   } else if (info->colorType == 0) {
      if (info->key_defined) {
         ucvector_push_back(&tRNS, (unsigned char)(info->key_r / 256));
         ucvector_push_back(&tRNS, (unsigned char)(info->key_r % 256));
      }
   } else if (info->colorType == 2) {
      if (info->key_defined) {
         ucvector_push_back(&tRNS, (unsigned char)(info->key_r / 256));
         ucvector_push_back(&tRNS, (unsigned char)(info->key_r % 256));
         ucvector_push_back(&tRNS, (unsigned char)(info->key_g / 256));
         ucvector_push_back(&tRNS, (unsigned char)(info->key_g % 256));
         ucvector_push_back(&tRNS, (unsigned char)(info->key_b / 256));
         ucvector_push_back(&tRNS, (unsigned char)(info->key_b % 256));
      }
   }

   error = addChunk(out, "tRNS", tRNS.data, tRNS.size);
   ucvector_cleanup(&tRNS);

   return error;
}

static unsigned addChunk_IDAT(ucvector *out, const unsigned char *data, size_t datasize, LodeZlib_DeflateSettings *zlibsettings)
{
   ucvector zlibdata;
   unsigned error = 0;

   /*compress with the Zlib compressor*/
   ucvector_init(&zlibdata);
   error = LodePNG_compress(&zlibdata.data, &zlibdata.size, data, datasize, zlibsettings);
   if (!error) {
      error = addChunk(out, "IDAT", zlibdata.data, zlibdata.size);
   }
   ucvector_cleanup(&zlibdata);

   return error;
}

static unsigned addChunk_IEND(ucvector *out)
{
   unsigned error = 0;
   error = addChunk(out, "IEND", 0, 0);
   return error;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static unsigned addChunk_tEXt(ucvector *out, const char *keyword, const char *textstring) /*add text chunk*/
{
   unsigned error = 0;
   size_t i;
   ucvector text;
   ucvector_init(&text);
   for (i = 0; keyword[i] != 0; i++) {
      ucvector_push_back(&text, (unsigned char)keyword[i]);
   }
   ucvector_push_back(&text, 0);
   for (i = 0; textstring[i] != 0; i++) {
      ucvector_push_back(&text, (unsigned char)textstring[i]);
   }
   error = addChunk(out, "tEXt", text.data, text.size);
   ucvector_cleanup(&text);

   return error;
}

static unsigned addChunk_zTXt(ucvector *out, const char *keyword, const char *textstring, LodeZlib_DeflateSettings *zlibsettings)
{
   unsigned error = 0;
   ucvector data, compressed;
   size_t i, textsize = strlen(textstring);

   ucvector_init(&data);
   ucvector_init(&compressed);
   for (i = 0; keyword[i] != 0; i++) {
      ucvector_push_back(&data, (unsigned char)keyword[i]);
   }
   ucvector_push_back(&data, 0); /* 0 termination char*/
   ucvector_push_back(&data, 0); /*compression method: 0*/

   error = LodePNG_compress(&compressed.data, &compressed.size, (unsigned char *)textstring, textsize, zlibsettings);
   if (!error) {
      for (i = 0; i < compressed.size; i++) {
         ucvector_push_back(&data, compressed.data[i]);
      }
      error = addChunk(out, "zTXt", data.data, data.size);
   }

   ucvector_cleanup(&compressed);
   ucvector_cleanup(&data);
   return error;
}

static unsigned addChunk_iTXt(ucvector *out, unsigned compressed, const char *keyword, const char *langtag, const char *transkey,
                              const char *textstring, LodeZlib_DeflateSettings *zlibsettings)
{
   unsigned error = 0;
   ucvector data, compressed_data;
   size_t i, textsize = strlen(textstring);

   ucvector_init(&data);

   for (i = 0; keyword[i] != 0; i++) {
      ucvector_push_back(&data, (unsigned char)keyword[i]);
   }
   ucvector_push_back(&data, 0); /*null termination char*/
   ucvector_push_back(&data, compressed ? 1 : 0); /*compression flag*/
   ucvector_push_back(&data, 0); /*compression method*/
   for (i = 0; langtag[i] != 0; i++) {
      ucvector_push_back(&data, (unsigned char)langtag[i]);
   }
   ucvector_push_back(&data, 0); /*null termination char*/
   for (i = 0; transkey[i] != 0; i++) {
      ucvector_push_back(&data, (unsigned char)transkey[i]);
   }
   ucvector_push_back(&data, 0); /*null termination char*/

   if (compressed) {
      ucvector_init(&compressed_data);
      error = LodePNG_compress(&compressed_data.data, &compressed_data.size, (unsigned char *)textstring, textsize, zlibsettings);
      if (!error) {
         for (i = 0; i < compressed_data.size; i++) {
            ucvector_push_back(&data, compressed_data.data[i]);
         }
         for (i = 0; textstring[i] != 0; i++) {
            ucvector_push_back(&data, (unsigned char)textstring[i]);
         }
      }
   } else { /*not compressed*/
      for (i = 0; textstring[i] != 0; i++) {
         ucvector_push_back(&data, (unsigned char)textstring[i]);
      }
   }

   if (!error) {
      error = addChunk(out, "iTXt", data.data, data.size);
   }
   ucvector_cleanup(&data);
   return error;
}

static unsigned addChunk_bKGD(ucvector *out, const LodePNG_InfoPng *info)
{
   unsigned error = 0;
   ucvector bKGD;
   ucvector_init(&bKGD);
   if (info->color.colorType == 0 || info->color.colorType == 4) {
      ucvector_push_back(&bKGD, (unsigned char)(info->background_r / 256));
      ucvector_push_back(&bKGD, (unsigned char)(info->background_r % 256));
   } else if (info->color.colorType == 2 || info->color.colorType == 6) {
      ucvector_push_back(&bKGD, (unsigned char)(info->background_r / 256));
      ucvector_push_back(&bKGD, (unsigned char)(info->background_r % 256));
      ucvector_push_back(&bKGD, (unsigned char)(info->background_g / 256));
      ucvector_push_back(&bKGD, (unsigned char)(info->background_g % 256));
      ucvector_push_back(&bKGD, (unsigned char)(info->background_b / 256));
      ucvector_push_back(&bKGD, (unsigned char)(info->background_b % 256));
   } else if (info->color.colorType == 3) {
      ucvector_push_back(&bKGD, (unsigned char)(info->background_r % 256)); /*palette index*/
   }

   error = addChunk(out, "bKGD", bKGD.data, bKGD.size);
   ucvector_cleanup(&bKGD);

   return error;
}

static unsigned addChunk_tIME(ucvector *out, const LodePNG_Time *time)
{
   unsigned error = 0;
   unsigned char *data = (unsigned char *)malloc(7);
   if (!data) {
      return 9948;
   }
   data[0] = (unsigned char)(time->year / 256);
   data[1] = (unsigned char)(time->year % 256);
   data[2] = time->month;
   data[3] = time->day;
   data[4] = time->hour;
   data[5] = time->minute;
   data[6] = time->second;
   error = addChunk(out, "tIME", data, 7);
   free(data);
   return error;
}

static unsigned addChunk_pHYs(ucvector *out, const LodePNG_InfoPng *info)
{
   unsigned error = 0;
   ucvector data;
   ucvector_init(&data);

   LodePNG_add32bitInt(&data, info->phys_x);
   LodePNG_add32bitInt(&data, info->phys_y);
   ucvector_push_back(&data, info->phys_unit);

   error = addChunk(out, "pHYs", data.data, data.size);
   ucvector_cleanup(&data);

   return error;
}

#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

static void filterScanline(unsigned char *out, const unsigned char *scanline, const unsigned char *prevline, size_t length, size_t bytewidth,
                           unsigned char filterType)
{
   size_t i;
   switch (filterType) {
      case 0:
         for (i = 0; i < length; i++) {
            out[i] = scanline[i];
         }
         break;
      case 1:
         for (i =         0; i < bytewidth; i++) {
            out[i] = scanline[i];
         }
         for (i = bytewidth; i <    length; i++) {
            out[i] = scanline[i] - scanline[i - bytewidth];
         }
         break;
      case 2:
         if (prevline) for (i = 0; i < length; i++) {
               out[i] = scanline[i] - prevline[i];
            }
         else         for (i = 0; i < length; i++) {
               out[i] = scanline[i];
            }
         break;
      case 3:
         if (prevline) {
            for (i =         0; i < bytewidth; i++) {
               out[i] = scanline[i] - prevline[i] / 2;
            }
            for (i = bytewidth; i <    length; i++) {
               out[i] = scanline[i] - ((scanline[i - bytewidth] + prevline[i]) / 2);
            }
         } else {
            for (i =         0; i < length; i++) {
               out[i] = scanline[i];
            }
            for (i = bytewidth; i < length; i++) {
               out[i] = scanline[i] - scanline[i - bytewidth] / 2;
            }
         }
         break;
      case 4:
         if (prevline) {
            for (i =         0; i < bytewidth; i++) {
               out[i] = (unsigned char)(scanline[i] - paethPredictor(0, prevline[i], 0));
            }
            for (i = bytewidth; i <    length; i++) {
               out[i] = (unsigned char)(scanline[i] - paethPredictor(scanline[i - bytewidth], prevline[i], prevline[i - bytewidth]));
            }
         } else {
            for (i =         0; i < bytewidth; i++) {
               out[i] = scanline[i];
            }
            for (i = bytewidth; i <    length; i++) {
               out[i] = (unsigned char)(scanline[i] - paethPredictor(scanline[i - bytewidth], 0, 0));
            }
         }
         break;
      default:
         return; /*unexisting filter type given*/
   }
}

static unsigned filter(unsigned char *out, const unsigned char *in, unsigned w, unsigned h, const LodePNG_InfoColor *info)
{
   /*
   For PNG filter method 0
   out must be a buffer with as size: h + (w * h * bpp + 7) / 8, because there are the scanlines with 1 extra byte per scanline

   There is a nice heuristic described here: http://www.cs.toronto.edu/~cosmin/pngtech/optipng.html. It says:
    *  If the image type is Palette, or the bit depth is smaller than 8, then do not filter the image (i.e. use fixed filtering, with the filter None).
    * (The other case) If the image type is Grayscale or RGB (with or without Alpha), and the bit depth is not smaller than 8, then use adaptive filtering heuristic as follows: independently for each row, apply all five filters and select the filter that produces the smallest sum of absolute values per row.

   Here the above method is used mostly. Note though that it appears to be better to use the adaptive filtering on the plasma 8-bit palette example, but that image isn't the best reference for palette images in general.
   */

   unsigned bpp = LodePNG_InfoColor_getBpp(info);
   size_t linebytes = (w * bpp + 7) / 8; /*the width of a scanline in bytes, not including the filter type*/
   size_t bytewidth = (bpp + 7) / 8; /*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
   const unsigned char *prevline = 0;
   unsigned x, y;
   unsigned heuristic;
   unsigned error = 0;

   if (bpp == 0) {
      return 31;   /*invalid color type*/
   }

   /*choose heuristic as described above*/
   if (info->colorType == 3 || info->bitDepth < 8) {
      heuristic = 0;
   } else {
      heuristic = 1;
   }

   if (heuristic == 0) { /*None filtertype for everything*/
      for (y = 0; y < h; y++) {
         size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
         size_t inindex = linebytes * y;
         const unsigned TYPE = 0;
         out[outindex] = TYPE; /*filter type byte*/
         filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, TYPE);
         prevline = &in[inindex];
      }
   } else if (heuristic == 1) { /*adaptive filtering*/
      size_t sum[5];
      ucvector attempt[5]; /*five filtering attempts, one for each filter type*/
      size_t smallest = 0;
      unsigned type, bestType = 0;

      for (type = 0; type < 5; type++) {
         ucvector_init(&attempt[type]);
      }
      for (type = 0; type < 5; type++) {
         if (!ucvector_resize(&attempt[type], linebytes)) {
            error = 9949;
            break;
         }
      }

      if (!error) {
         for (y = 0; y < h; y++) {
            /*try the 5 filter types*/
            for (type = 0; type < 5; type++) {
               filterScanline(attempt[type].data, &in[y * linebytes], prevline, linebytes, bytewidth, type);

               /*calculate the sum of the result*/
               sum[type] = 0;
               for (x = 0; x < attempt[type].size; x += 3) {
                  sum[type] += attempt[type].data[x];   /*note that not all pixels are checked to speed this up while still having probably the best choice*/
               }

               /*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
               if (type == 0 || sum[type] < smallest) {
                  bestType = type;
                  smallest = sum[type];
               }
            }

            prevline = &in[y * linebytes];

            /*now fill the out values*/
            out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
            for (x = 0; x < linebytes; x++) {
               out[y * (linebytes + 1) + 1 + x] = attempt[bestType].data[x];
            }
         }
      }

      for (type = 0; type < 5; type++) {
         ucvector_cleanup(&attempt[type]);
      }
   }
#if 0 /*deflate the scanline with a fixed tree after every filter attempt to see which one deflates best. This is slow, and _does not work as expected_: the heuristic gives smaller result!*/
   else if (heuristic == 2) { /*adaptive filtering by using deflate*/
      size_t size[5];
      ucvector attempt[5]; /*five filtering attempts, one for each filter type*/
      size_t smallest;
      unsigned type = 0, bestType = 0;
      unsigned char *dummy;
      LodeZlib_DeflateSettings deflatesettings = LodeZlib_defaultDeflateSettings;
      deflatesettings.btype =
         1; /*use fixed tree on the attempts so that the tree is not adapted to the filtertype on purpose, to simulate the true case where the tree is the same for the whole image*/
      for (type = 0; type < 5; type++) {
         ucvector_init(&attempt[type]);
         ucvector_resize(&attempt[type], linebytes);
      }
      for (y = 0; y < h; y++) { /*try the 5 filter types*/
         for (type = 0; type < 5; type++) {
            filterScanline(attempt[type].data, &in[y * linebytes], prevline, linebytes, bytewidth, type);
            size[type] = 0;
            dummy = 0;
            LodePNG_compress(&dummy, &size[type], attempt[type].data, attempt[type].size, &deflatesettings);
            free(dummy);
            /*check if this is smallest size (or if type == 0 it's the first case so always store the values)*/
            if (type == 0 || size[type] < smallest) {
               bestType = type;
               smallest = size[type];
            }
         }
         prevline = &in[y * linebytes];
         out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
         for (x = 0; x < linebytes; x++) {
            out[y * (linebytes + 1) + 1 + x] = attempt[bestType].data[x];
         }
      }
      for (type = 0; type < 5; type++) {
         ucvector_cleanup(&attempt[type]);
      }
   }
#endif

   return error;
}

static void addPaddingBits(unsigned char *out, const unsigned char *in, size_t olinebits, size_t ilinebits, unsigned h)
{
   /*The opposite of the removePaddingBits function
   olinebits must be >= ilinebits*/
   unsigned y;
   size_t diff = olinebits - ilinebits;
   size_t obp = 0, ibp = 0; /*bit pointers*/
   for (y = 0; y < h; y++) {
      size_t x;
      for (x = 0; x < ilinebits; x++) {
         unsigned char bit = readBitFromReversedStream(&ibp, in);
         setBitOfReversedStream(&obp, out, bit);
      }
      /*obp += diff; --> no, fill in some value in the padding bits too, to avoid "Use of uninitialised value of size ###" warning from valgrind*/
      for (x = 0; x < diff; x++) {
         setBitOfReversedStream(&obp, out, 0);
      }
   }
}

static void Adam7_interlace(unsigned char *out, const unsigned char *in, unsigned w, unsigned h, unsigned bpp)
{
   /*Note: this function works on image buffers WITHOUT padding bits at end of scanlines with non-multiple-of-8 bit amounts, only between reduced images is padding*/
   unsigned passw[7], passh[7];
   size_t filter_passstart[8], padded_passstart[8], passstart[8];
   unsigned i;

   Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

   if (bpp >= 8) {
      for (i = 0; i < 7; i++) {
         unsigned x, y, b;
         size_t bytewidth = bpp / 8;
         for (y = 0; y < passh[i]; y++)
            for (x = 0; x < passw[i]; x++) {
               size_t pixelinstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
               size_t pixeloutstart = passstart[i] + (y * passw[i] + x) * bytewidth;
               for (b = 0; b < bytewidth; b++) {
                  out[pixeloutstart + b] = in[pixelinstart + b];
               }
            }
      }
   } else { /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/
      for (i = 0; i < 7; i++) {
         unsigned x, y, b;
         unsigned ilinebits = bpp * passw[i];
         unsigned olinebits = bpp * w;
         size_t obp, ibp; /*bit pointers (for out and in buffer)*/
         for (y = 0; y < passh[i]; y++)
            for (x = 0; x < passw[i]; x++) {
               ibp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
               obp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
               for (b = 0; b < bpp; b++) {
                  unsigned char bit = readBitFromReversedStream(&ibp, in);
                  setBitOfReversedStream(&obp, out, bit);
               }
            }
      }
   }
}

/*out must be buffer big enough to contain uncompressed IDAT chunk data, and in must contain the full image*/
static unsigned preProcessScanlines(unsigned char **out, size_t *outsize, const unsigned char *in,
                                    const LodePNG_InfoPng *infoPng) /*return value is error*/
{
   /*
   This function converts the pure 2D image with the PNG's colortype, into filtered-padded-interlaced data. Steps:
   *) if no Adam7: 1) add padding bits (= posible extra bits per scanline if bpp < 8) 2) filter
   *) if adam7: 1) Adam7_interlace 2) 7x add padding bits 3) 7x filter
   */
   unsigned bpp = LodePNG_InfoColor_getBpp(&infoPng->color);
   unsigned w = infoPng->width;
   unsigned h = infoPng->height;
   unsigned error = 0;

   if (infoPng->interlaceMethod == 0) {
      *outsize = h + (h * ((w * bpp + 7) / 8)); /*image size plus an extra byte per scanline + possible padding bits*/
      *out = (unsigned char *)malloc(*outsize);
      if (!(*out) && (*outsize)) {
         error = 9950;
      }

      if (!error) {
         if (bpp < 8 && w * bpp != ((w * bpp + 7) / 8) * 8) { /*non multiple of 8 bits per scanline, padding bits needed per scanline*/
            ucvector padded;
            ucvector_init(&padded);
            if (!ucvector_resize(&padded, h * ((w * bpp + 7) / 8))) {
               error = 9951;
            }
            if (!error) {
               addPaddingBits(padded.data, in, ((w * bpp + 7) / 8) * 8, w * bpp, h);
               error = filter(*out, padded.data, w, h, &infoPng->color);
            }
            ucvector_cleanup(&padded);
         } else {
            error = filter(*out, in, w, h, &infoPng->color);   /*we can immediatly filter into the out buffer, no other steps needed*/
         }
      }
   } else { /*interlaceMethod is 1 (Adam7)*/
      unsigned char *adam7 = (unsigned char *)malloc((h * w * bpp + 7) / 8);
      if (!adam7 && ((h * w * bpp + 7) / 8)) {
         error = 9952;   /*malloc failed*/
      }

      while (!error) { /*not a real while loop, used to break out to cleanup to avoid a goto*/
         unsigned passw[7], passh[7];
         size_t filter_passstart[8], padded_passstart[8], passstart[8];
         unsigned i;

         Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

         *outsize = filter_passstart[7]; /*image size plus an extra byte per scanline + possible padding bits*/
         *out = (unsigned char *)malloc(*outsize);
         if (!(*out) && (*outsize)) {
            error = 9953;
            break;
         }

         Adam7_interlace(adam7, in, w, h, bpp);

         for (i = 0; i < 7; i++) {
            if (bpp < 8) {
               ucvector padded;
               ucvector_init(&padded);
               if (!ucvector_resize(&padded, h * ((w * bpp + 7) / 8))) {
                  error = 9954;
               }
               if (!error) {
                  addPaddingBits(&padded.data[padded_passstart[i]], &adam7[passstart[i]], ((passw[i] * bpp + 7) / 8) * 8, passw[i] * bpp, passh[i]);
                  error = filter(&(*out)[filter_passstart[i]], &padded.data[padded_passstart[i]], passw[i], passh[i], &infoPng->color);
               }

               ucvector_cleanup(&padded);
            } else {
               error = filter(&(*out)[filter_passstart[i]], &adam7[padded_passstart[i]], passw[i], passh[i], &infoPng->color);
            }
         }

         break;
      }

      free(adam7);
   }

   return error;
}

/*palette must have 4 * palettesize bytes allocated*/
static unsigned isPaletteFullyOpaque(const unsigned char *palette, size_t palettesize) /*palette given in format RGBARGBARGBARGBA...*/
{
   size_t i;
   for (i = 0; i < palettesize; i++) {
      if (palette[4 * i + 3] != 255) {
         return 0;
      }
   }
   return 1;
}

/*this function checks if the input image given by the user has no transparent pixels*/
static unsigned isFullyOpaque(const unsigned char *image, unsigned w, unsigned h, const LodePNG_InfoColor *info)
{
   /*TODO: When the user specified a color key for the input image, then this function must also check for pixels that are the same as the color key and treat those as transparent.*/

   unsigned i, numpixels = w * h;
   if (info->colorType == 6) {
      if (info->bitDepth == 8) {
         for (i = 0; i < numpixels; i++) if (image[i * 4 + 3] != 255) {
               return 0;
            }
      } else {
         for (i = 0; i < numpixels; i++) if (image[i * 8 + 6] != 255 || image[i * 8 + 7] != 255) {
               return 0;
            }
      }
      return 1; /*no single pixel with alpha channel other than 255 found*/
   } else if (info->colorType == 4) {
      if (info->bitDepth == 8) {
         for (i = 0; i < numpixels; i++) if (image[i * 2 + 1] != 255) {
               return 0;
            }
      } else {
         for (i = 0; i < numpixels; i++) if (image[i * 4 + 2] != 255 || image[i * 4 + 3] != 255) {
               return 0;
            }
      }
      return 1; /*no single pixel with alpha channel other than 255 found*/
   } else if (info->colorType == 3) {
      /*when there's a palette, we could check every pixel for translucency, but much quicker is to just check the palette*/
      return (isPaletteFullyOpaque(info->palette, info->palettesize));
   }

   return 0; /*color type that isn't supported by this function yet, so assume there is transparency to be safe*/
}

#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
static unsigned addUnknownChunks(ucvector *out, unsigned char *data, size_t datasize)
{
   unsigned char *inchunk = data;
   while ((size_t)(inchunk - data) < datasize) {
      unsigned error = LodePNG_append_chunk(&out->data, &out->size, inchunk);
      if (error) {
         return error;   /*error: not enough memory*/
      }
      out->allocsize = out->size; /*fix the allocsize again*/
      inchunk = LodePNG_chunk_next(inchunk);
   }
   return 0;
}
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/

void LodePNG_encode(LodePNG_Encoder *encoder, unsigned char **out, size_t *outsize, const unsigned char *image, unsigned w, unsigned h)
{
   LodePNG_InfoPng info;
   ucvector outv;
   unsigned char *data = 0; /*uncompressed version of the IDAT chunk data*/
   size_t datasize = 0;

   // provide some proper output values if error will happen
   *out     = 0;
   *outsize = 0;
   encoder->error = 0;

   // UNSAFE copy to avoid having to cleanup, but we will only change primitive parameters, and not invoke the cleanup
   //function nor touch the palette's buffer so we use it safely
   info = encoder->infoPng;

   info.width = w;
   info.height = h;

   if (encoder->settings.autoLeaveOutAlphaChannel && isFullyOpaque(image, w, h, &encoder->infoRaw.color)) {
      /*go to a color type without alpha channel*/
      if (info.color.colorType == 6) {
         info.color.colorType = 2;
      } else if (info.color.colorType == 4) {
         info.color.colorType = 0;
      }
   }

   if (encoder->settings.zlibsettings.windowSize > 32768) {
      encoder->error = 60;   /*error: windowsize larger than allowed*/
      return;
   }

   if (encoder->settings.zlibsettings.btype > 2) {
      encoder->error = 61;   /*error: unexisting btype*/
      return;
   }

   if (encoder->infoPng.interlaceMethod > 1) {
      encoder->error = 71;   /*error: unexisting interlace mode*/
      return;
   }
   if ((encoder->error = checkColorValidity(info.color.colorType, info.color.bitDepth))) {
      return;   /*error: unexisting color type given*/
   }
   if ((encoder->error = checkColorValidity(encoder->infoRaw.color.colorType, encoder->infoRaw.color.bitDepth))) {
      return;   /*error: unexisting color type given*/
   }

   if (!LodePNG_InfoColor_equal(&encoder->infoRaw.color, &info.color)) {
      unsigned char *converted;
      size_t size = (w * h * LodePNG_InfoColor_getBpp(&info.color) + 7) / 8;

      if ((info.color.colorType != 6 && info.color.colorType != 2) || (info.color.bitDepth != 8)) {
         encoder->error = 59;   /*for the output image, only these types are supported*/
         return;
      }
      converted = (unsigned char *)malloc(size);
      if (!converted && size) {
         encoder->error = 9955;   /*error: malloc failed*/
      }
      if (!encoder->error) {
         encoder->error = LodePNG_convert(converted, image, &info.color, &encoder->infoRaw.color, w, h);
      }
      if (!encoder->error) {
         preProcessScanlines(&data, &datasize, converted, &info);
         /*filter(data.data, converted.data, w, h, LodePNG_InfoColor_getBpp(&info.color));*/
      }

      free(converted);

   } else {
      preProcessScanlines(&data, &datasize, image, &info);
      /*filter(data.data, image, w, h, LodePNG_InfoColor_getBpp(&info.color));*/
   }

   ucvector_init(&outv);
   while (!encoder->error) {
      /*not really a while loop, this is only used to break out if an error happens to avoid goto's to do the ucvector cleanup*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
      size_t i;
#endif

      /*write signature and chunks*/
      writeSignature(&outv);

      /*IHDR*/
      addChunk_IHDR(&outv, w, h, info.color.bitDepth, info.color.colorType, info.interlaceMethod);

#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
      /*unknown chunks between IHDR and PLTE*/
      if (info.unknown_chunks.data[0]) {
         encoder->error = addUnknownChunks(&outv, info.unknown_chunks.data[0], info.unknown_chunks.datasize[0]);
         if (encoder->error) {
            break;
         }
      }
#endif

      /*PLTE*/
      if (info.color.colorType == 3) {
         if (info.color.palettesize == 0 || info.color.palettesize > 256) {
            encoder->error = 68;
            break;
         }
         addChunk_PLTE(&outv, &info.color);
      }

      if (encoder->settings.force_palette && (info.color.colorType == 2 || info.color.colorType == 6)) {
         if (info.color.palettesize == 0 || info.color.palettesize > 256) {
            encoder->error = 68;
            break;
         }
         addChunk_PLTE(&outv, &info.color);
      }

      /*tRNS*/
      if (info.color.colorType == 3 && !isPaletteFullyOpaque(info.color.palette, info.color.palettesize)) {
         addChunk_tRNS(&outv, &info.color);
      }
      if ((info.color.colorType == 0 || info.color.colorType == 2) && info.color.key_defined) {
         addChunk_tRNS(&outv, &info.color);
      }
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
      /*bKGD (must come between PLTE and the IDAt chunks*/
      if (info.background_defined) {
         addChunk_bKGD(&outv, &info);
      }
      /*pHYs (must come before the IDAT chunks)*/
      if (info.phys_defined) {
         addChunk_pHYs(&outv, &info);
      }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
      /*unknown chunks between PLTE and IDAT*/
      if (info.unknown_chunks.data[1]) {
         encoder->error = addUnknownChunks(&outv, info.unknown_chunks.data[1], info.unknown_chunks.datasize[1]);
         if (encoder->error) {
            break;
         }
      }
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
      /*IDAT (multiple IDAT chunks must be consecutive)*/
      encoder->error = addChunk_IDAT(&outv, data, datasize, &encoder->settings.zlibsettings);
      if (encoder->error) {
         break;
      }
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
      /*tIME*/
      if (info.time_defined) {
         addChunk_tIME(&outv, &info.time);
      }
      /*tEXt and/or zTXt*/
      for (i = 0; i < info.text.num; i++) {
         if (strlen(info.text.keys[i]) > 79) {
            encoder->error = 66;
            break;
         }
         if (strlen(info.text.keys[i]) < 1) {
            encoder->error = 67;
            break;
         }
         if (encoder->settings.text_compression) {
            addChunk_zTXt(&outv, info.text.keys[i], info.text.strings[i], &encoder->settings.zlibsettings);
         } else {
            addChunk_tEXt(&outv, info.text.keys[i], info.text.strings[i]);
         }
      }
      /*LodePNG version id in text chunk*/
      if (encoder->settings.add_id) {
         unsigned alread_added_id_text = 0;
         for (i = 0; i < info.text.num; i++)
            if (!strcmp(info.text.keys[i], "LodePNG")) {
               alread_added_id_text = 1;
               break;
            }
         if (alread_added_id_text == 0) {
            addChunk_tEXt(&outv, "LodePNG", VERSION_STRING);   /*it's shorter as tEXt than as zTXt chunk*/
         }
      }
      /*iTXt*/
      for (i = 0; i < info.itext.num; i++) {
         if (strlen(info.itext.keys[i]) > 79) {
            encoder->error = 66;
            break;
         }
         if (strlen(info.itext.keys[i]) < 1) {
            encoder->error = 67;
            break;
         }
         addChunk_iTXt(&outv, encoder->settings.text_compression,
                       info.itext.keys[i], info.itext.langtags[i], info.itext.transkeys[i], info.itext.strings[i],
                       &encoder->settings.zlibsettings);
      }
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#ifdef LODEPNG_COMPILE_UNKNOWN_CHUNKS
      /*unknown chunks between IDAT and IEND*/
      if (info.unknown_chunks.data[2]) {
         encoder->error = addUnknownChunks(&outv, info.unknown_chunks.data[2], info.unknown_chunks.datasize[2]);
         if (encoder->error) {
            break;
         }
      }
#endif /*LODEPNG_COMPILE_UNKNOWN_CHUNKS*/
      /*IEND*/
      addChunk_IEND(&outv);

      break; /*this isn't really a while loop; no error happened so break out now!*/
   }

   free(data);
   /*instead of cleaning the vector up, give it to the output*/
   *out = outv.data;
   *outsize = outv.size;
}

unsigned LodePNG_encode32(unsigned char **out, size_t *outsize, const unsigned char *image, unsigned w, unsigned h)
{
   unsigned error;
   LodePNG_Encoder encoder;
   LodePNG_Encoder_init(&encoder);
   LodePNG_encode(&encoder, out, outsize, image, w, h);
   error = encoder.error;
   LodePNG_Encoder_cleanup(&encoder);
   return error;
}

#ifdef LODEPNG_COMPILE_DISK
unsigned LodePNG_encode32f(const char *filename, const unsigned char *image, unsigned w, unsigned h)
{
   unsigned char *buffer;
   size_t buffersize;
   unsigned error = LodePNG_encode32(&buffer, &buffersize, image, w, h);
   LodePNG_saveFile(buffer, buffersize, filename);
   free(buffer);
   return error;
}
#endif /*LODEPNG_COMPILE_DISK*/

void LodePNG_EncodeSettings_init(LodePNG_EncodeSettings *settings)
{
   LodeZlib_DeflateSettings_init(&settings->zlibsettings);
   settings->autoLeaveOutAlphaChannel = 1;
   settings->force_palette = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
   settings->add_id = 1;
   settings->text_compression = 0;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

void LodePNG_Encoder_init(LodePNG_Encoder *encoder)
{
   LodePNG_EncodeSettings_init(&encoder->settings);
   LodePNG_InfoPng_init(&encoder->infoPng);
   LodePNG_InfoRaw_init(&encoder->infoRaw);
   encoder->error = 1;
}

void LodePNG_Encoder_cleanup(LodePNG_Encoder *encoder)
{
   LodePNG_InfoPng_cleanup(&encoder->infoPng);
   LodePNG_InfoRaw_cleanup(&encoder->infoRaw);
}

void LodePNG_Encoder_copy(LodePNG_Encoder *dest, const LodePNG_Encoder *source)
{
   LodePNG_Encoder_cleanup(dest);
   *dest = *source;
   LodePNG_InfoPng_init(&dest->infoPng);
   LodePNG_InfoRaw_init(&dest->infoRaw);
   dest->error = LodePNG_InfoPng_copy(&dest->infoPng, &source->infoPng);
   if (dest->error) {
      return;
   }
   dest->error = LodePNG_InfoRaw_copy(&dest->infoRaw, &source->infoRaw);
   if (dest->error) {
      return;
   }
}

#endif /*LODEPNG_COMPILE_ENCODER*/

#endif /*LODEPNG_COMPILE_PNG*/


#ifdef LODEPNG_COMPILE_DISK

unsigned LodePNG_loadFile(unsigned char **out, size_t *outsize,const char *filename)
{
   /*designed for loading files from hard disk in a dynamically allocated buffer*/

   FILE *file;
   long size;

   /*provide some proper output values if error will happen*/
   *out = 0;
   *outsize = 0;

   file = fopen(filename, "rb");
   if (! file) {
      return 78;
   }

   /*get filesize:*/
   fseek(file , 0 , SEEK_END);
   size = ftell(file);
   rewind(file);

   /*read contents of the file into the vector*/
   if (size > 0) {
      *outsize = 0;
      *out = (unsigned char *)malloc((size_t)size);
      if (size && (*out)) {
         (*outsize) = fread(*out, 1, (size_t)size, file);
      }
   }

   fclose(file);
   if (!(*out) && size) {
      return 80;   /*the above malloc failed*/
   }

   return 0;
}

/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
unsigned LodePNG_saveFile(const unsigned char *buffer, size_t buffersize, const char *filename)
{
   FILE *file;
   file = fopen(filename, "wb" );

   if (! file) {
      return 79;
   }

   fwrite((char *)buffer , 1 , buffersize, file);
   fclose(file);

   return 0;
}

#endif /*LODEPNG_COMPILE_DISK*/

