#include <stdio.h>
#include <time.h>

#include "utils.h"
#include "tiff.h"



//********************************************************************
unsigned char *read_tiff( const char *fname, int *width, int *height, int *bpp )
{
 TIFF *tif = NULL;
 unsigned char *data = NULL;
 int is_tile = 0, ret;
 bool err = false;
 int bps = 0;
 uint32 img_wd = 0, img_ht = 0;
 tsize_t scanline_sz;
 int bytes_per_point = 1;
 int i;


	tif = TIFFOpen( fname, "r" );

	if( !tif )
	{
		return NULL;
	}

	is_tile = TIFFIsTiled( tif );
	if( is_tile )
	{
		err = true;
		goto on_ex;
	}

	TIFFGetField( tif, TIFFTAG_BITSPERSAMPLE, &bps );
	TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &img_wd );
	TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &img_ht );

	scanline_sz = TIFFScanlineSize( tif );

	bytes_per_point = (bps >> 3);

	data = (unsigned char *)malloc( img_wd*img_ht*bytes_per_point );
	if( !data )
	{
		err = true;
		goto on_ex;
	}
	for( i = 0;i < (int)img_ht;i++ )
	{
		ret = TIFFReadScanline( tif, (tdata_t)(data+scanline_sz*i), i );
		if( ret == -1 )
		{
			err = true;
			goto on_ex;
		}
	}

	*width	= img_wd;
	*height = img_ht;
	*bpp	= bps;

on_ex:
	TIFFClose( tif );

	if( err && data )
	{
		free( data );
		data = NULL;
	}

 return data;
}

