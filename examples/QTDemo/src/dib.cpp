#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "dib.h"



u_char *LoadDIBitmap(const char *filename, int *width, int *height, int *bpp )
{
 FILE             *fp;          /* Open file pointer */
 u_char          *bits;        /* Bitmap pixel bits */
 int              bitsize;      /* Size of bitmap */
 int              infosize;     /* Size of header information */
 BITMAPFILEHEADER header;       /* File header */
 BITMAPINFO *info; 		/* Bitmap information */


    /* Try opening the file; use "rb" mode to read this *binary* file. */
    if( (fp = fopen(filename, "rb")) == NULL )
        return (NULL);

    /* Read the file header and any following bitmap information... */
    if( fread(&header, sizeof(BITMAPFILEHEADER), 1, fp) < 1 )
    {
        /* Couldn't read the file header - return NULL... */
    	fclose(fp);
        return NULL;
    }

    if( header.bfType != /*'MB'*/BF_TYPE )	/* Check for BM reversed... */
    {
        /* Not a bitmap file - return NULL... */
        fclose(fp);
        return (NULL);
    }

    infosize = header.bfOffBits - sizeof(BITMAPFILEHEADER);
    if( (info = (BITMAPINFO *)malloc(infosize)) == NULL )
    {
        /* Couldn't allocate memory for bitmap info - return NULL... */
        fclose(fp);
        return NULL;
        }

    if( fread(info, 1, infosize, fp) < (size_t)infosize )
    {
        /* Couldn't read the bitmap header - return NULL... */
        free(info);
        fclose(fp);
        return NULL;
    }

    /* Now that we have all the header info read in, allocate memory for *
     * the bitmap and read *it* in...                                    */
    if( (bitsize = (info)->bmiHeader.biSizeImage) == 0 )
        bitsize = ((info)->bmiHeader.biWidth *
                   (info)->bmiHeader.biBitCount + 7) / 8 *
  	           abs((info)->bmiHeader.biHeight);

    if( (bits = (u_char*)malloc(bitsize)) == NULL )
    {
        /* Couldn't allocate memory - return NULL! */
        free(info);
        fclose(fp);
        return NULL;
    }

    if( fread(bits, 1, bitsize, fp) < (size_t)bitsize )
    {
        /* Couldn't read bitmap - free memory and return NULL! */
        free(info);
        free(bits);
        fclose(fp);
        return NULL;
    }

    /* OK, everything went fine - return the allocated bitmap... */
    *width = info->bmiHeader.biWidth;
    *height = abs(info->bmiHeader.biHeight);
    *bpp = info->bmiHeader.biBitCount;
    
    free(info);
    fclose(fp);
    
 return bits;
}


/*
 * 'SaveDIBitmap()' - Save a DIB/BMP file to disk.
 *
 * Returns 0 on success or -1 on failure...
 */

int SaveDIBitmap(const char *filename, int width, int height, int bpp, u_char *bits)     /* I - Bitmap data */
{
 FILE             *fp;          /* Open file pointer */
 int              size,         /* Size of file */
                  infosize,     /* Size of bitmap info */
                  bitsize;      /* Size of bitmap pixels */
 BITMAPFILEHEADER header;       /* File header */
 BITMAPINFO *info; 		/* Bitmap information */

 	info = (BITMAPINFO *)malloc( sizeof(BITMAPINFO) );
 	if( info == NULL)
 		return -1;
 	
 	// Fill info
 	bzero( info, sizeof(BITMAPINFO) );
 	info->bmiHeader.biSize 		= sizeof(BITMAPINFOHEADER);
 	info->bmiHeader.biWidth 	= width;
 	info->bmiHeader.biHeight 	= height;
 	info->bmiHeader.biPlanes 	= 1;
 	info->bmiHeader.biBitCount 	= bpp;

    /* Try opening the file; use "wb" mode to write this *binary* file. */
    if( (fp = fopen(filename, "wb")) == NULL )
        return -1;

    /* Figure out the bitmap size */
    if( info->bmiHeader.biSizeImage == 0 )
	    bitsize = (info->bmiHeader.biWidth * info->bmiHeader.biBitCount + 7) / 8 * abs(info->bmiHeader.biHeight);
    else
	    bitsize = info->bmiHeader.biSizeImage;
    
    info->bmiHeader.biSizeImage = bitsize;

    /* Figure out the header size */
    infosize = sizeof(BITMAPINFOHEADER);
    
    switch( info->bmiHeader.biCompression )
	{
	 case BI_BITFIELDS:
          infosize += 12; /* Add 3 RGB doubleword masks */
          if( info->bmiHeader.biClrUsed == 0 )
	          break;
	 case BI_RGB:
          if( info->bmiHeader.biBitCount > 8 && info->bmiHeader.biClrUsed == 0 )
	          break;
	 case BI_RLE8:
	 case BI_RLE4:
          if( info->bmiHeader.biClrUsed == 0 )
              infosize += (1 << info->bmiHeader.biBitCount) * 4;
	      else
              infosize += info->bmiHeader.biClrUsed * 4;
	  break;
	}

    size = sizeof(BITMAPFILEHEADER) + infosize + bitsize;

    /* Write the file header, bitmap information, and bitmap pixel data... */
    header.bfType      = BF_TYPE; /* Non-portable... sigh */
    header.bfSize      = size;
    header.bfReserved1 = 0;
    header.bfReserved2 = 0;
    header.bfOffBits   = sizeof(BITMAPFILEHEADER) + infosize;

    if( fwrite(&header, 1, sizeof(BITMAPFILEHEADER), fp) < sizeof(BITMAPFILEHEADER) )
    {
        /* Couldn't write the file header - return... */
    	free( info );
        fclose(fp);
        return -1;
    }

    if( fwrite(info, 1, infosize, fp) < (size_t)infosize )
    {
        /* Couldn't write the bitmap header - return... */
    	free( info );
        fclose(fp);
        return -1;
    }

    if( fwrite(bits, 1, bitsize, fp) < (size_t)bitsize )
    {
        /* Couldn't write the bitmap - return... */
    	free( info );
        fclose(fp);
        return -1;
    }

    /* OK, everything went fine - return... */
    free( info );
    fclose(fp);
    
 return 0;
}


