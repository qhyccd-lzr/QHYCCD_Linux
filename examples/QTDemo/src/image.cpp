//---------------------------------------------------------------------------
#include <Qt>
#include <QImage>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "image.h"
#include "debayer.h"
#include "utils.h"
#include "tiff.h"
#include "dib.h"
#include "tiffio.h"

//---------------------------------------------------------------------------


double RGB_coeffs[3] = { 0.299, 0.587, 0.114 };

void cimage_base :: get_compatible_image( cimage_base **dst, int wd, int ht, int bpp )
{
	if( !dst )
		return;

	cimage_base *image = *dst;

	if( image && image->width() == wd && image->height() == ht && image->bpp() == bpp )
		return;

	// destroy old if needed and build new
	if( image && image->bpp() != bpp )
	{
		delete image;
		image = NULL;
	}

	if( !image )
	{
		switch( (pixel_base::pixel_format)bpp )
		{
		case pixel_base::BPP_MONO_8:
			image = new cimage<pixel_mono8>();
			break;
		case pixel_base::BPP_MONO_12:
			image = new cimage<pixel_mono12>();
			break;
		case pixel_base::BPP_MONO_16:
			image = new cimage<pixel_mono16>();
			break;
		case pixel_base::BPP_RGB_24:
			image = new cimage<pixel_rgb24>();
			break;
		case pixel_base::BPP_RGB_48:
			image = new cimage<pixel_rgb48>();
			break;
		default:
		{
			log_e( "cimage_base :: get_compatible_image(): Unsupported BPP = %d", bpp );
			return;
		}
		}
		*dst = image;
	}
	image->resize( wd, ht );
}

template <class pixel_type>
cimage<pixel_type> :: cimage()
{

    m_width   = -1;
    m_height  = -1;
    m_length  = 0;
    m_hist_data_length = 0;
    m_bpp	  = m_format.bits();
    m_channel_count = m_format.channels();
    m_data    = NULL;
    m_hist_data = NULL;
    m_preview = NULL;
    m_debayer = new debayer_bilinear<typename pixel_type::channel_type, int>();
}


template <class pixel_type>
cimage<pixel_type> :: ~cimage()
{
    if( m_data )
        delete [] m_data;
    if( m_hist_data )
    	delete [] m_hist_data;
    if( m_preview )
    	delete m_preview;
    if( m_debayer )
    	delete m_debayer;
}


template <class pixel_type>
bool cimage<pixel_type>::load_bmp( const char *fname )
{
 u_char *BitmapBits; /* Bitmap data */
 u_char *pline;
 int wd, ht, bpp;
 int i, j, off = 0;

	 // Read ONLY 24 bit BMP
    BitmapBits = LoadDIBitmap( fname, &wd, &ht, &bpp );
	if( !BitmapBits || bpp != 24 )
		return false;

	if( !m_preview || m_preview->width() != wd || m_preview->height() != ht )
	{
		if( m_preview )
			delete m_preview;
		m_preview = new QImage( wd, ht, QImage::Format_RGB32 );
	}

	resize( wd, ht );

	for( j = m_height-1;j >= 0;j-- )
	{
	     pline = m_preview->scanLine(j);
		 for( i = 0;i < m_width;i++, off++ )
		 {
			 pline[i*4]   = BitmapBits[off*3];
			 pline[i*4+1] = BitmapBits[off*3+1];
			 pline[i*4+2] = BitmapBits[off*3+2];
		 }
	}

	free(BitmapBits);

 return true;
}


template <class pixel_type>
bool cimage<pixel_type>::save_bmp( const char *fname ) const
{
 u_char *BitmapBits; /* Bitmap data */
 u_char *pline;
 int i, j, off = 0;


 	// Write ONLY 24 bit BMP
	BitmapBits = (u_char *)malloc( 3*m_width*m_height );
	if( !BitmapBits )
	{
		return false;
	}

	for( j = m_height-1;j >= 0;j-- )
	{
	     pline = m_preview->scanLine(j);
	     for( i = 0;i < m_width;i++, off++ )
	     {
	      	  BitmapBits[off*3] = pline[i*4];
	       	  BitmapBits[off*3+1] = pline[i*4+1];
	       	  BitmapBits[off*3+2] = pline[i*4+2];
	     }
	}

	SaveDIBitmap(fname, m_width, m_height, 24, BitmapBits);

	free(BitmapBits);

 return true;
}


template <class pixel_type>
bool cimage<pixel_type> :: load( const char *fName )
{
 int wd, ht, bpp;
 unsigned char *arr;

     arr = read_tiff( fName, &wd, &ht, &bpp );

     if( arr && bpp == pixel_base::bytes_per_pixel(bpp)*8 )
     {
    	 resize( wd, ht );

    	 assign_raw_data( (const char*)arr, wd*ht*pixel_base::bytes_per_pixel(bpp), wd, ht, bpp );
     }
     else
     {
    	 log_e("cimage<pixel_type> :: load(): incompatible image");
     }

     free( arr );

 return true;
}


template <class pixel_type>
bool cimage<pixel_type> :: save( const char *fname ) const
{
     if( m_width <= 0 || m_height <= 0 )
     {
         u_msg( "CImage :: Save: Width or Height < 0" );
         return false;
     }

    //tiff_io<typename pixel_type::channel_type, pixel_type::channel_no>::write_tiff( fname, m_data, m_width, m_height, m_format.bytes_per_channel()*8 );

     TIFF *tif = NULL;
     unsigned short spp, photo;
     time_t now;
     char time_str[20] = {0};
     struct tm *tmp;


     now = time( NULL );
     tmp = localtime( &now );
     strftime( time_str, sizeof(time_str), "%Y:%m:%d %H:%M:%S", tmp );

     tif = TIFFOpen( fname, "w" );

     if( !tif )
    	 return false;

     spp = m_channel_count; // Samples per pixel
     photo = m_channel_count == 1 ? PHOTOMETRIC_MINISBLACK : PHOTOMETRIC_RGB;
     TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, m_width);
     TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, m_format.bytes_per_channel()*8 );
     TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, spp);
     TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
     TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photo);
     TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
     TIFFSetField(tif, TIFFTAG_DATETIME, time_str);

     if( m_channel_count == 1 )
     {
    	 for( int i = 0; i < m_height; i++ )
    		 TIFFWriteScanline( tif, (tdata_t)(m_data + i*m_width), i, 0 );
     }
     else
     {

    	 typename pixel_type::channel_type *pdst = new typename pixel_type::channel_type[m_width*m_channel_count];
    	 typename pixel_type::channel_type * r_data = m_data;
    	 typename pixel_type::channel_type * g_data = r_data + m_length;
    	 typename pixel_type::channel_type * b_data = g_data + m_length;

    	 for( int i = 0, off_src = 0; i < m_height; i++ )
    	 {
    		 for( int j = 0, off_dst = 0;j < m_width; ++j, ++off_src )
    		 {
    			 pdst[off_dst++] = r_data[off_src];
    			 pdst[off_dst++] = g_data[off_src];
    			 pdst[off_dst++] = b_data[off_src];
    		 }
    		 TIFFWriteScanline(tif, (tdata_t)pdst, i, 0);
    	 }
    	 delete [] pdst;
     }

     TIFFClose( tif );

 return true;
}


template <class pixel_type>
void cimage<pixel_type> :: clear( void )
{
     m_width  = -1;
     m_height = -1;

     if( m_data )
     {
         delete [] m_data;
         m_data = NULL;
     }

}


template <class pixel_type>
bool cimage<pixel_type> :: assign( const cimage_base *psrc )
{
     if( psrc == this )
         return true;

     if( psrc->bpp() != bpp() )
    	 return false;
     if( !resize( psrc->width(), psrc->height() ) )
         return false;
     const cimage<pixel_type> *src_image = static_cast< const cimage<pixel_type>* >(psrc);
     if( !src_image )
    	 return false;

     int channel_len = m_length*sizeof( typename pixel_type::channel_type );
     for( int i = 0;i < m_channel_count;i++ )
     {
    	 memmove( m_data + i*channel_len,
    			 src_image->m_data + i*channel_len,
    			 channel_len );
     }

 return true;
}


template <class pixel_type>
bool cimage<pixel_type> :: assign_raw_data( const char *pdata, int size, int wd, int ht, int bits )
{
	if( bits != bpp() || (wd*ht*pixel_base::bytes_per_pixel(bits) != size) )
		return false;

	if( !resize( wd, ht ) )
	    return false;

	// splitting raw RGB into RGB channels
	for( int i = 0;i < m_channel_count;i++ )
	{
		typename pixel_type::channel_type *pdst = m_data + i*m_length;
		const typename pixel_type::channel_type *psrc = reinterpret_cast<const typename pixel_type::channel_type*>(pdata)+i;

		for( int j = 0, off = 0;j < m_length;j++, off+=m_channel_count )
			pdst[j] = psrc[off];
	}

	return true;
}


template <class pixel_type>
void cimage<pixel_type> :: fill( int ch )
{
     if( !m_data )
         return;

     for( int j = 0;j < m_channel_count;j++ )
     {
    	 for( int i = 0;i < m_length;i++ )
    		 m_data[j*m_length+i] = ch;
     }
}


template <class pixel_type>
bool cimage<pixel_type> :: resize( int wd, int ht )
{
     if( wd <= 0 || ht <= 0 )
         return false;

     if( wd == m_width && ht == m_height )
         return true;

     if( m_data )
         delete [] m_data;

     m_width  = wd;
     m_height = ht;
     m_length = m_width * m_height;

     m_data  = new typename pixel_type::channel_type[ m_channel_count*m_length ];

     fill( 0 );

 return true;
}


//template <class pixel_type>
//bool cimage<pixel_type> :: create_preview( void )
//{
//	log_i( "cimage<pixel_type> :: create_preview(): Pixel format not supperted" );
//	return true;
//}


template <class pixel_type>
const QImage *cimage<pixel_type> :: get_preview_ptr( void ) const
{
 return m_preview;
}


template <class pixel_type>
void cimage<pixel_type>::set_preview_ptr( const QImage *ptr )
{
	if( ptr == m_preview )
		return;
	if( m_preview )
		delete m_preview;

	m_preview = const_cast<QImage *>(ptr);
}

template <class pixel_type>
void cimage<pixel_type>::set_preview_debayer_settings( const std::vector<float> &settings )
{
	if( pixel_type::channel_no > 1 )
		m_preview_debayer_settings.clear();
	else
		m_preview_debayer_settings = settings;
}


template <class pixel_type>
int  cimage<pixel_type> :: bpp( void ) const
{
 return m_bpp;
}


template <class pixel_type>
int  cimage<pixel_type> :: channel_count() const
{
	return m_channel_count;
}


template <class pixel_type>
pixel_base::pixel_format cimage<pixel_type> :: format( void ) const
{
	return (pixel_base::pixel_format)m_format.bits();
}

template <class pixel_type>
typename pixel_type::channel_type cimage<pixel_type>::data( int offset, int channel ) const
{
	if( offset < 0 || offset >= m_length || channel >= m_channel_count )
		return 0;

	return m_data[ m_length*channel + offset ];
}


template <class pixel_type>
uint32_t* cimage<pixel_type>::histogram( const typename pixel_type::channel_type *channel_data, int low, int up, uint32_t* norm )
{
	int i, j, len = (1 << (m_bpp/pixel_type::channel_no));
	const typename pixel_type::channel_type *p;
	uint32_t *buf;

	p = channel_data;

	//buf = new uint32_t[ len ];
	buf = m_hist_buf;

	if( buf )
	{
		memset( buf, 0, len*sizeof(uint32_t) );
		*norm = 0;
		for( i = 0;i < m_height;i++ )
			for( j = 0;j < m_width;j++, p++ )
				if( *p >= low && *p <= up )
				{
					buf[*p]++;
					(*norm)++;
				}
	}
	else
	{
		log_e("cimage<pixel_type>::histogram(): no memory");
		abort();
	}

	return buf;
}


template <class pixel_type>
bool cimage<pixel_type>::create_histogram_preview( int Low, int Up, bool LogHist, bool Auto, bool /*Apply*/ )
{
 int i, j, off;
 typename pixel_type::channel_type pixmax;
 typename pixel_type::channel_type *pI = NULL;
 typename pixel_type::channel_type *channel_data = NULL;
 u_char *pline = NULL;
 uint32_t *hist = NULL;
 double k = 0, h_max, view_cor = 1.0;
 uint32_t norm, c;
 QImage *Dst = NULL;
 bool allocate = true;
 bool do_debayer = !m_preview_debayer_settings.empty();


     if( m_width <= 0 || m_height <= 0 )
         return false;

     pixmax = (1 << (m_bpp/pixel_type::channel_no)) - 1;

     h_max = 0;

     // try to prepare debayer
     if( (!do_debayer) || (do_debayer && m_hist_data_length != m_length) )
     {
    	 if( m_hist_data )
    	 {
    		 delete [] m_hist_data;
    		 m_hist_data = NULL;
    		 m_hist_data_length = 0;
    	 }
     }
     if( do_debayer )
     {
    	 bool alloc_hist_image = true;
    	 if( m_hist_data )
    		 alloc_hist_image = false;
    	 if( alloc_hist_image )
    	 {
    		 m_hist_data_length = m_length;
    		 m_hist_data = new typename pixel_type::channel_type[ m_hist_data_length ];
    	 }
     }

     // through all channels
     for( int ch = 0;ch < m_channel_count;ch++ )
     {
    	 channel_data = m_data + ch*m_length;

    	 // Crop dynamic range by max bit rate
    	 for( i = 0;i < m_length;i++ )
    		 channel_data[i] = MIN( channel_data[i], pixmax );

    	 hist = histogram( channel_data, Low, Up, &norm );

    	 // Prepare view factor
    	 if( norm )
    		 k = (double)(pixmax+1) / norm;
    	 c = 0;

    	 for( i = 0;i <= pixmax;i++ )
    		 if( (double)hist[i] > h_max )
    			 h_max = (double)hist[i];

    	 double	s = pixmax+.99, d = pixmax / log(s+1.0), q = s / (double)(Up-Low);

    	 for( i = 0;i <= pixmax;i++ )
    	 {
    		 if( Auto )
    		 {
    			 m_hist_lut[i] = (norm) ? (typename pixel_type::channel_type)(k*(c += hist[i])) : ((i <= Low) ? 0 : pixmax);
    		 }
    		 else
    		 {
    			 if( i < Low )
    				 m_hist_lut[i] = 0;
    			 else
				 if( i > Up)
					 m_hist_lut[i] = pixmax;
				 else
				 {
					 s = q * (i - Low);
					 m_hist_lut[i] = (typename pixel_type::channel_type)(LogHist ? log(1.+s)*d : s);
				 }
    		 }

    		 m_hist_channel_lut[ch][i] = (double)hist[i];
    		 m_hist_line[i] = (double)(m_hist_lut[i]*h_max/(double)pixmax) ;

    	 }

    	 view_cor = 256.0 / (double)pixmax;

    	 // Fill preview image
    	 Dst = const_cast<QImage *>( get_preview_ptr() );
    	 if( Dst )
    	 {
    		 if( Dst->width() == m_width && Dst->height() == m_height )
    			 allocate = false;
    		 else
    			 delete Dst;
    	 }
    	 if( allocate )
    	 {
    		 Dst = new QImage ( m_width, m_height, QImage::Format_RGB32 );
    		 set_preview_ptr( Dst );
    	 }

    	 if( !Dst )
    	 {
    		 log_e("cimage<pixel_type>::create_histogram_preview(): Dst image = NULL. Abort!");
    		 abort();
    	 }

    	 // render result
    	 if( do_debayer )
    	 {
    		 for( i = 0;i < m_length;i++ )
    			 m_hist_data[i] = m_hist_lut[ MIN(pixmax, channel_data[i]) ];

    		 m_debayer->process( m_width, m_height, (void*)m_hist_data, m_preview_debayer_settings, m_preview );
    	 }
    	 else
    	 {
    		 for( j = 0;j < m_height;j++ )
    		 {
    			 pline = (u_char *)Dst->scanLine(j);
    			 pI = channel_data + j*m_width;
    			 for( i = 0;i < m_width;i++, pI++ )
    			 {
    				 off = i<<2;
    				 pline[off + m_channel_count-1-ch] = (u_char)(m_hist_lut[ MIN(pixmax, *pI) ] * view_cor);
    			 }
    		 }
    	 }
     }

     if( !do_debayer )
     {
    	 if( m_channel_count == 1 )	//fill unfilled channels
    	 {
    		 for( j = 0;j < m_height;j++ )
    		 {
    			 pline = (u_char *)Dst->scanLine(j);
    			 pI = channel_data + j*m_width;
    			 for( i = 0;i < m_width;i++, pI++ )
    			 {
    				 off = i<<2;
    				 pline[off + 1] =
    				 pline[off + 2] = (u_char)(m_hist_lut[ MIN(pixmax, *pI) ] * view_cor);
    			 }
    		 }
    	 }
     }

 return true;
}


template <>
bool cimage<pixel_mono8>::create_preview( void )
{
	u_char *pline;
	int off = 0;
	bool allocate = true;


	if( m_preview  )
	{
		if( m_preview->width() == m_width && m_preview->height() == m_height )
			allocate = false;
		else
			delete m_preview;
	}

	if( allocate )
		m_preview = new QImage( m_width, m_height, QImage::Format_RGB32 );

	// Fill preview image
	if( m_preview_debayer_settings.empty() )
	{
		pixel_mono8::channel_type *psrc = m_data;
		for( int j = 0;j < m_height;j++ )
		{
			pline = m_preview->scanLine(j);
			for( int i = 0, i_off = 0;i < m_width;i++, off++, i_off+=4 )
			{
				u_char idata = psrc[off];

				pline[i_off]   = idata;      // B
				pline[i_off+1] = idata;      // G
				pline[i_off+2] = idata;      // R
			}
		}
	}
	else
	{
		m_debayer->process( m_width, m_height, (void*)m_data, m_preview_debayer_settings, m_preview );
	}

	return true;
}


template <>
bool cimage<pixel_mono12>::create_preview( void )
{
	u_char *pData, idata;
	u_char *pline;
	int off = 0;
	bool allocate = true;


	if( m_preview  )
	{
		if( m_preview->width() == m_width && m_preview->height() == m_height )
			allocate = false;
		else
			delete m_preview;
	}
	if( allocate )
		m_preview = new QImage( m_width, m_height, QImage::Format_RGB32 );

	// Fill preview image
	pData = (u_char *)m_data;

	if( m_preview_debayer_settings.empty() )
	{
		for( int j = 0;j < m_height;j++ )
		{
			pline = m_preview->scanLine(j);
			for( int i = 0, i_off = 0;i < m_width;i++, off+=2, i_off+=4 )
			{
				idata = ( (((pixel_mono16::channel_type)pData[off+1] << 8) | ((pixel_mono16::channel_type)pData[off])) >> 4);

				pline[i_off]   = idata;      // B
				pline[i_off+1] = idata;      // G
				pline[i_off+2] = idata;      // R
			}
		}
	}
	else
	{
		m_debayer->process( m_width, m_height, (void*)m_data, m_preview_debayer_settings, m_preview );
	}

	return true;
}


template <>
bool cimage<pixel_mono16>::create_preview( void )
{
	u_char *pline;
	int off = 0;
	bool allocate = true;


	if( m_preview  )
	{
		if( m_preview->width() == m_width && m_preview->height() == m_height )
			allocate = false;
		else
			delete m_preview;
	}

	if( allocate )
		m_preview = new QImage( m_width, m_height, QImage::Format_RGB32 );

	// Fill preview image
	if( m_preview_debayer_settings.empty() )
	{
		pixel_mono16::channel_type *psrc = m_data;
		for( int j = 0;j < m_height;j++ )
		{
			pline = m_preview->scanLine(j);
			for( int i = 0, i_off = 0;i < m_width;i++, off++, i_off+=4 )
			{
				u_char idata = (psrc[off] >> 8);

				pline[i_off]   = idata;      // B
				pline[i_off+1] = idata;      // G
				pline[i_off+2] = idata;      // R
			}
		}
	}
	else
	{
		m_debayer->process( m_width, m_height, (void*)m_data, m_preview_debayer_settings, m_preview );
	}

	return true;
}


template <>
bool cimage<pixel_rgb24>::create_preview( void )
{
	u_char *pline;
	int off = 0;
	bool allocate = true;


	if( m_preview  )
	{
		if( m_preview->width() == m_width && m_preview->height() == m_height )
			allocate = false;
		else
			delete m_preview;
	}

	if( allocate )
		m_preview = new QImage( m_width, m_height, QImage::Format_RGB32 );

	pixel_rgb24::channel_type *r_data = m_data;
	pixel_rgb24::channel_type *g_data = r_data + m_length;
	pixel_rgb24::channel_type *b_data = g_data + m_length;

	// Fill preview image
	for( int j = 0;j < m_height;j++ )
	{
		pline = m_preview->scanLine(j);
		for( int i = 0, i_off = 0;i < m_width;i++, off++, i_off+=4 )
		{
			pline[i_off]   = b_data[off];      // B
			pline[i_off+1] = g_data[off];      // G
			pline[i_off+2] = r_data[off];      // R
		}
	}

	return true;
}


template <>
bool cimage<pixel_rgb48>::create_preview( void )
{
	u_char *pline;
	int off = 0;
	bool allocate = true;


	if( m_preview  )
	{
		if( m_preview->width() == m_width && m_preview->height() == m_height )
			allocate = false;
		else
			delete m_preview;
	}

	if( allocate )
		m_preview = new QImage( m_width, m_height, QImage::Format_RGB32 );

	pixel_rgb48::channel_type *r_data = m_data;
	pixel_rgb48::channel_type *g_data = r_data + m_length;
	pixel_rgb48::channel_type *b_data = g_data + m_length;

	// Fill preview image
	for( int j = 0;j < m_height;j++ )
	{
		pline = m_preview->scanLine(j);
		for( int i = 0, i_off = 0;i < m_width;i++, off++, i_off+=4 )
		{
			pline[i_off]   = (b_data[off] >> 8);      // B
			pline[i_off+1] = (g_data[off] >> 8);      // G
			pline[i_off+2] = (r_data[off] >> 8);      // R
		}
	}

	return true;
}

