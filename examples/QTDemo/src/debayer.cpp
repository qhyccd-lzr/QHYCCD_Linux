/*
 * debayer.cpp
 *
 *  Created on: 28.02.2012
 *      Author: gm
 */

#include "image.h"
#include "timer.h"
#include "utils.h"
#include "debayer.h"


debayer_base::debayer_base() :
	m_width(0),
	m_height(0),
	m_length(0),
	m_pattern(NULL),
	m_rgb_zone(NULL)
{
}


debayer_base::~debayer_base()
{
	destroy();
}


void debayer_base::destroy()
{
	if( m_pattern )
	{
		delete [] m_pattern;
		m_pattern = NULL;
	}
	if( m_rgb_zone )
	{
		delete [] m_rgb_zone;
		m_rgb_zone = NULL;
	}
	m_length = 0;
}


bool debayer_base::resize( int wd, int ht )
{
	if( wd <= 0 || ht <= 0 )
		return false;

	int new_len = wd*ht;
	if( new_len == m_length )
		return true;

	destroy();

	m_length = new_len;
	m_width  = wd;
	m_height = ht;
	m_pattern  = new char[(m_width+1)*(m_height+1)];
	m_rgb_zone = new real[m_length*3];

	return true;
}


//-----------------------------------------------------------------------------
// bilinear interpolation
template <>
void debayer_bilinear<pixel_mono16::channel_type, int>::process( int wd, int ht, void *data, const std::vector<float> &settings, QImage *out )
{
	if( !resize( wd, ht ) || settings.size() < 5 )
	{
		log_e( "debayer<pixel_type>::process() resize() failed" );
		return;
	}
#ifdef MESURE_TIME
	ctimer total_tm;
	total_tm.start();
#endif

	int x_offset = (int)settings[0];
	int y_offset = (int)settings[1];

	float corr_coef_R = settings[2] / 256.0;
	float corr_coef_G = settings[3] / 256.0;
	float corr_coef_B = settings[4] / 256.0;

	int pat_wd = m_width+1;
	int pat_ht = m_height+1;
	int base = 0;
	int pbase = 0;
	real *R_channel = m_rgb_zone;
	real *G_channel = R_channel + m_length;
	real *B_channel = G_channel + m_length;
	pixel_mono16::channel_type *img = reinterpret_cast<pixel_mono16::channel_type *>(data);

	for( int i=0;i < m_height;i+=2 )
	{
		pbase = i*pat_wd;
		for( int j = 0;j < m_width;j+=2, pbase+=2 )
		{
			m_pattern[ pbase ]          = 1;  //R
			m_pattern[ pbase+1 ]        = 0;  //G
			m_pattern[ pbase+pat_wd ]   = 0;  //G
			m_pattern[ pbase+pat_wd+1 ] = -1; //B
		}
	}

	for( int i = 0;i < m_height;i+=2 ) //im_h
	{
		pbase = i*pat_wd+m_width;
		m_pattern[ pbase ]     = 1;
		m_pattern[ pbase+pat_wd ] = 0;
	}

	for( int j = 0;j < m_width;j+=2 ) //im_h
	{
		pbase = pat_wd*(pat_ht-1)+j;
		m_pattern[ pbase ]   = 1;
		m_pattern[ pbase+1 ] = 0;
	}
	m_pattern[ pat_wd*m_height+m_width ] = 1;

	//-----------
#ifdef MESURE_TIME
	ctimer tm;
	tm.start();
#endif
	int base_up = 0;
	int base_dn = 0;
	// R,G,B channel matrices evaluation
	for( int i = 1;i < m_height-1;i++ )
	{
		base = i*m_width+1;
		pbase = (i+y_offset)*pat_wd + x_offset + 1;
		for( int j = 1;j < m_width-1;j++, base++, pbase++ )
		{
			base_up = base-m_width;
			base_dn = base+m_width;
			if( m_pattern[ pbase ] == 1 ) // Red pixel
			{
				R_channel[ base ] = corr_coef_R*(int)img[ base ];
				G_channel[ base ] = corr_coef_G*(((int)img[ base_up ] + (int)img[ base_dn ] + (int)img[ base-1 ] + (int)img[ base+1 ]) >> 2);
				B_channel[ base ] = corr_coef_B*(((int)img[ base_up-1 ]+(int)img[ base_dn-1 ]+(int)img[ base_up+1 ]+(int)img[ base_dn+1 ]) >> 2);
			}
			else
			if ( m_pattern[ pbase ] == 0 ) // Green pixel
			{
				G_channel[ base ] = corr_coef_G*(int)img[ base ];
				if( m_pattern[ pbase-pat_wd ] == 1 ) // if the upper pixel is red
				{
					R_channel[ base ] = corr_coef_R*(((int)img[ base_up ]+(int)img[ base_dn ]) >> 1);
					B_channel[ base ] = corr_coef_B*(((int)img[ base-1 ]+(int)img[ base+1 ]) >>1);
				}
				else
				if( m_pattern[ pbase-pat_wd ] == -1 ) // if the upper pixel is blue
				{
					R_channel[ base ] = corr_coef_R*(((int)img[ base-1 ]+(int)img[ base+1 ]) >> 1);
					B_channel[ base ] = corr_coef_B*(((int)img[ base_up ]+(int)img[ base_dn ]) >> 1);
				}
			}
			else
			if( m_pattern[ pbase ] == -1 ) // Blue pixel
			{
				R_channel[ base ] = corr_coef_R*(((int)img[ base_up-1 ]+(int)img[ base_dn-1 ]+(int)img[ base_up+1 ]+(int)img[ base_dn+1 ]) >> 2);
				G_channel[ base ] = corr_coef_G*(((int)img[ base_up ] + (int)img[ base_dn ] + (int)img[ base-1 ] + (int)img[ base+1 ]) >> 2);
				B_channel[ base ] = corr_coef_B*(int)img[ base ];
			}

			if( R_channel[ base ] > 255 )
				R_channel[ base ] = 255;
			if( B_channel[ base ] > 255 )
				B_channel[ base ] = 255;
			if( G_channel[ base ] > 255 )
				G_channel[ base ] = 255;

			//printf( "%f\n", B_channel[ idx(i,j) ] );
		}
	}
#ifdef MESURE_TIME
	log_i( "===>RGB_channel = %ld msec", tm.gettime() );
#endif

	u_char *pline = NULL;
	int off = 0;
	for( int j = 0;j < m_height;j++ )
	{
		pline = out->scanLine(j);
		for( int i = 0, i_off = 0;i < m_width;i++, off++, i_off+=4 )
		{
			pline[i_off]   = (pixel_rgb24::channel_type)B_channel[off];      // B
			pline[i_off+1] = (pixel_rgb24::channel_type)G_channel[off];      // G
			pline[i_off+2] = (pixel_rgb24::channel_type)R_channel[off];      // R
		}
	}
#ifdef MESURE_TIME
	log_i( "===> Total time = %ld msec", total_tm.gettime() );
#endif
}
