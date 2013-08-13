#ifndef __tiff_h__
#define __tiff_h__

#include "tiffio.h"

unsigned char *read_tiff( const char *fname, int *width, int *height, int *bpp );

#endif
