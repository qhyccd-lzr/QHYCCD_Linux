/*
 * debayer.h
 *
 *  Created on: 28.02.2012
 *      Author: gm
 */

#ifndef DEBAYER_H_
#define DEBAYER_H_

#include <Qt>
#include <QImage>
#include <math.h>
#include <vector>

#include "image.h"
#include "timer.h"
#include "utils.h"
#include "debayer.h"


#define MESURE_TIME

class debayer_base
{
public:
	static const int MIN_SETTINGS_NUM = 5;
	debayer_base();
	virtual ~debayer_base();
	virtual void process( int wd, int ht, void *data, const std::vector<float> &settings, QImage *out ) = 0;

protected:
	typedef int real;
	void destroy();
	bool resize( int wd, int ht );
	int    m_width;
	int    m_height;
	int    m_length;
	char  *m_pattern;
	real  *m_rgb_zone;
};


template <typename channel_type, typename buf_type>
class debayer_bilinear : public debayer_base
{
public:
	void process( int wd, int ht, void *data, const std::vector<float> &settings, QImage *out );
};


template <typename channel_type>
class debayer_smooth_hue : public debayer_base
{
public:
	void process( int wd, int ht, void *data, const std::vector<float> &settings, QImage *out );
};


//-----------------------------------------------------------------------------
// bilinear interpolation
template <typename channel_type, typename buf_type>
void debayer_bilinear<channel_type, buf_type>::process( int wd, int ht, void *data, const std::vector<float> &settings, QImage *out )
{
	if( settings.size() < (size_t)MIN_SETTINGS_NUM )
	{
		log_e( "debayer_bilinear<pixel_type>::process(): Got too few settings. < %d", MIN_SETTINGS_NUM );
		return;
	}
	if( !resize( wd, ht ) )
	{
		log_e( "debayer_bilinear<pixel_type>::process() resize() failed" );
		return;
	}
#ifdef MESURE_TIME
	ctimer total_tm;
	total_tm.start();
#endif
	// bilinear
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
	channel_type *img = reinterpret_cast<channel_type *>(data);

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
				R_channel[ base ] = corr_coef_R*(real)img[ base ];
				G_channel[ base ] = corr_coef_G*(((real)img[ base_up ] + (real)img[ base_dn ] + (real)img[ base-1 ] + (real)img[ base+1 ]) * 0.25);
				B_channel[ base ] = corr_coef_B*(((real)img[ base_up-1 ]+(real)img[ base_dn-1 ]+(real)img[ base_up+1 ]+(real)img[ base_dn+1 ]) * 0.25);
			}
			else
			if ( m_pattern[ pbase ] == 0 ) // Green pixel
			{
				G_channel[ base ] = corr_coef_G*(real)img[ base ];
				if( m_pattern[ pbase-pat_wd ] == 1 ) // if the upper pixel is red
				{
					R_channel[ base ] = corr_coef_R*(((real)img[ base_up ]+(real)img[ base_dn ]) * 0.5);
					B_channel[ base ] = corr_coef_B*(((real)img[ base-1 ]+(real)img[ base+1 ]) * 0.5);
				}
				else
				if( m_pattern[ pbase-pat_wd ] == -1 ) // if the upper pixel is blue
				{
					R_channel[ base ] = corr_coef_R*(((real)img[ base-1 ]+(real)img[ base+1 ])* 0.5);
					B_channel[ base ] = corr_coef_B*(((real)img[ base_up ]+(real)img[ base_dn ]) * 0.5);
				}
			}
			else
			if( m_pattern[ pbase ] == -1 ) // Blue pixel
			{
				R_channel[ base ] = corr_coef_R*(((real)img[ base_up-1 ]+(real)img[ base_dn-1 ]+(real)img[ base_up+1 ]+(real)img[ base_dn+1 ])* 0.25);
				G_channel[ base ] = corr_coef_G*(((real)img[ base_up ] + (real)img[ base_dn ] + (real)img[ base-1 ] + (real)img[ base+1 ]) * 0.25);
				B_channel[ base ] = corr_coef_B*(real)img[ base ];
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

template <>
void debayer_bilinear<pixel_mono16::channel_type, int>::process( int wd, int ht, void *data, const std::vector<float> &settings, QImage *out );



// smooth hue interpolation
template <typename channel_type>
void debayer_smooth_hue<channel_type>::process( int wd, int ht, void *data, const std::vector<float> &settings, QImage *out )
{
	if( settings.size() < 5 )
	{
		log_e( "debayer_smooth_hue<pixel_type>::process(): Got too few settings. < %d", MIN_SETTINGS_NUM );
		return;
	}
	if( !resize( wd, ht ) )
	{
		log_e( "debayer_smooth_hue<pixel_type>::process() resize() failed" );
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
	channel_type *img = reinterpret_cast<channel_type *>(data);

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

	// G channel matrix evaluation
#ifdef MESURE_TIME
	ctimer tm;
	tm.start();
#endif
	int base_up = 0;
	int base_dn = 0;
	for( int i = 1;i < m_height-1;i++ )
	{
		base = i*m_width+1;
		pbase = (i+y_offset)*pat_wd + x_offset + 1;
		for( int j = 1;j < m_width-1;j++, base++, pbase++ )
		{
			base_up = base-m_width;
			base_dn = base+m_width;
			if( m_pattern[ pbase ] == 1 ) // Red pixel
				G_channel[ base ] = ((real)img[ base_up ] + (real)img[ base_dn ] + (real)img[ base-1 ] + (real)img[ base+1 ]) * 0.25;

			if( m_pattern[ pbase ] == 0 ) // Green pixel
				G_channel[ base ] = (real)img[ base ];

			if( m_pattern[ pbase ] == -1 ) // Blue pixel
				G_channel[ base ] = ((real)img[ base_up ] + (real)img[ base_dn ] + (real)img[ base-1 ] + (real)img[ base+1 ]) * 0.25;

			G_channel[ base ] *= corr_coef_G;

			if( G_channel[ base ] > 255 )	// 65535
				G_channel[ base ] = 255;	// 65535
		}
	}

	//----------- check zeroes
	for( int i = 0;i < m_length;i++ )
	{
		if( G_channel[ i ] == 0 )
			G_channel[ i ] = 1;
	}

	// R,B channel matrices evaluation
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
				R_channel[ base ] = (real)img[ base ];
				B_channel[ base ] = G_channel[ base ]*0.25*((real)img[ base_up-1 ]/G_channel[ base_up-1 ]+(real)img[ base_dn-1 ]/G_channel[ base_dn-1 ]+(real)img[ base_up+1 ]/G_channel[ base_up+1 ]+(real)img[ base_dn+1 ]/G_channel[ base_dn+1 ]);
			}
			else
			if ( m_pattern[ pbase ] == 0 ) // Green pixel
			{
				if( m_pattern[ pbase-pat_wd ] == 1 ) // if the upper pixel is red
				{
					R_channel[ base ] = G_channel[ base ]*0.5*((real)img[ base_up ]/G_channel[ base_up ]+(real)img[ base_dn ]/G_channel[ base_dn ]);
					B_channel[ base ] = G_channel[ base ]*0.5*((real)img[ base-1 ]/G_channel[ base-1 ]+(real)img[ base+1 ]/G_channel[ base+1 ]);
				}
				else
				if( m_pattern[ pbase-pat_wd ] == -1 ) // if the upper pixel is blue
				{
					R_channel[ base ] = G_channel[ base ]*0.5*((real)img[ base-1 ]/G_channel[ base-1 ]+(real)img[ base+1 ]/G_channel[ base+1 ]);
					B_channel[ base ] = G_channel[ base ]*0.5*((real)img[ base_up ]/G_channel[ base_up ]+(real)img[ base_dn ]/G_channel[ base_dn ]);
				}
			}
			else
			if( m_pattern[ pbase ] == -1 ) // Blue pixel
			{
				R_channel[ base ] = G_channel[ base ]*0.25*((real)img[ base_up-1 ]/G_channel[ base_up-1 ]+(real)img[ base_dn-1 ]/G_channel[ base_dn-1 ]+(real)img[ base_up+1 ]/G_channel[ base_up+1 ]+(real)img[ base_dn+1 ]/G_channel[ base_dn+1 ]);
				B_channel[ base ] = (float)img[ base ];
			}

			R_channel[ base ] *= corr_coef_R;
			B_channel[ base ] *= corr_coef_B;

			if( R_channel[ base ] > 255 )	// 65535
				R_channel[ base ] = 255;
			if( B_channel[ base ] > 255 )
				B_channel[ base ] = 255;

			//printf( "%f\n", G_channel[ base ] );
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


#endif /* DEBAYER_H_ */
