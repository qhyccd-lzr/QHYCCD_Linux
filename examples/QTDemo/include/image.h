//---------------------------------------------------------------------------

#ifndef ImageH
#define ImageH
//---------------------------------------------------------------------------

#include <stdint.h>
#include <Qt>
#include <QImage>
#include <vector>

#include "utils.h"


class pixel_base
{
public:
	typedef enum
	{
		BPP_MONO_8  = 8,
		BPP_MONO_12 = 12,
		BPP_MONO_16 = 16,
		BPP_RGB_24  = 24,
		BPP_RGB_48  = 48
	}pixel_format;
	enum channel_order
	{
		R_CH = 0,
		G_CH,
		B_CH
	};

	virtual ~pixel_base() {}

	virtual int bytes() const  = 0;
	virtual pixel_format bits() const  = 0;
	virtual int channels() const  = 0;
	virtual int bytes_per_channel() const = 0;

	static int bytes_per_pixel( int bits ) { return (bits + 7) / 8; };
};


class pixel_mono8 : public pixel_base
{
public:
	static const int channel_no = 1;
	virtual int bytes() const { return 1; }
	virtual pixel_format bits() const { return BPP_MONO_8; }
	virtual int channels() const { return channel_no; }
	virtual int bytes_per_channel() const { return (bits()+7)/8/channels(); }

	typedef unsigned char channel_type;
};


class pixel_mono12 : public pixel_base
{
public:
	static const int channel_no = 1;
	virtual int bytes() const { return 2; }
	virtual pixel_format bits() const { return BPP_MONO_12; }
	virtual int channels() const { return channel_no; }
	virtual int bytes_per_channel() const { return (bits()+7)/8/channels(); }

	typedef unsigned short channel_type;
};


class pixel_mono16 : public pixel_base
{
public:
	static const int channel_no = 1;
	virtual int bytes() const { return 2; }
	virtual pixel_format bits() const { return BPP_MONO_16; }
	virtual int channels() const { return channel_no; }
	virtual int bytes_per_channel() const { return (bits()+7)/8/channels(); }

	typedef unsigned short channel_type;
};


class pixel_rgb24 : public pixel_base
{
public:
	static const int channel_no = 3;
	virtual int bytes() const { return 3; }
	virtual pixel_format bits() const { return BPP_RGB_24; }
	virtual int channels() const { return channel_no; }
	virtual int bytes_per_channel() const { return (bits()+7)/8/channels(); }

	typedef unsigned char channel_type;
};


class pixel_rgb48 : public pixel_base
{
public:
	static const int channel_no = 3;
	virtual int bytes() const { return 6; }
	virtual pixel_format bits() const { return BPP_RGB_48; }
	virtual int channels() const { return channel_no; }
	virtual int bytes_per_channel() const { return (bits()+7)/8/channels(); }

	typedef unsigned short channel_type;
};


class debayer_base;

class cimage_base
{
public:
	virtual ~cimage_base() {};
	virtual bool load( const char *fName ) = 0;
	virtual bool save( const char *fName ) const = 0;
	virtual bool load_bmp( const char *fname ) = 0;
	virtual bool save_bmp( const char *fname ) const = 0;
	virtual void fill( int ch ) = 0;
	virtual void clear( void ) = 0;
	virtual bool assign( const cimage_base *psrc ) = 0;
	virtual bool assign_raw_data( const char *pdata, int len, int wd, int ht,int bpp ) = 0;
	virtual bool resize( int wd, int ht ) = 0;
	virtual bool create_preview( void ) = 0;
	virtual int  bpp( void ) const = 0;
	virtual pixel_base::pixel_format format( void ) const = 0;
	virtual const QImage *get_preview_ptr( void ) const = 0;
	virtual void set_preview_ptr( const QImage *ptr ) = 0;
	virtual void set_preview_debayer_settings( const std::vector<float> &settings ) = 0;
	virtual int channel_count() const = 0;
	virtual int width( void ) const = 0;
	virtual int height( void ) const = 0;
	virtual bool create_histogram_preview( int Low, int Up, bool LogHist, bool Auto, bool /*Apply*/ ) = 0;

	virtual const double *hist_channel( int ch ) = 0;
	virtual const double *hist_line() = 0;

	static void get_compatible_image( cimage_base **dst, int wd, int ht, int bpp );
};


template <class pixel_type>
class cimage : public cimage_base
{
public:
	cimage();
	virtual ~cimage();
	virtual bool load( const char *fName );
	virtual bool save( const char *fName ) const;
	virtual bool load_bmp( const char *fname );
	virtual bool save_bmp( const char *fname ) const;
	virtual void fill( int ch );
	virtual void clear( void );
	virtual bool assign( const cimage_base *psrc );
	virtual bool assign_raw_data( const char *pdata, int size, int wd, int ht,int bpp );
	virtual bool resize( int wd, int ht );
	virtual bool create_preview( void );
	virtual int  bpp( void ) const;
	virtual int channel_count() const;
	virtual pixel_base::pixel_format format( void ) const;
	virtual const QImage *get_preview_ptr( void ) const;
	virtual void set_preview_ptr( const QImage *ptr );
	virtual void set_preview_debayer_settings( const std::vector<float> &settings );
	virtual int width( void ) const
	{
		return m_width;
	}
	virtual int height( void ) const
	{
		return m_height;
	}
	virtual bool create_histogram_preview( int Low, int Up, bool LogHist, bool Auto, bool /*Apply*/ );

	virtual const double *hist_channel( int ch )
	{
		if( ch >= 0 && ch < m_channel_count )
			return m_hist_channel_lut[ch];
		return NULL;
	}
	virtual const double *hist_line() { return m_hist_line; };

private:
	int m_width;
	int m_height;
	int m_length;
	pixel_type m_format;
	int m_bpp;
	int m_channel_count;
	typename pixel_type::channel_type *m_data;
	typename pixel_type::channel_type *m_hist_data;
	int m_hist_data_length;
	QImage *m_preview;
	std::vector<float> m_preview_debayer_settings;

	double m_hist_line[ 1<<16 ];
	double m_hist_channel_lut[pixel_type::channel_no][ 1<<16 ];
	uint32_t m_hist_lut[ 1<<16 ];
	uint32_t m_hist_buf[ 1<<16 ];

	debayer_base *m_debayer;

	//virtual bool assign_data( const typename pixel_type::channel_type *pdata, int wd, int ht );
	typename pixel_type::channel_type data( int offset, int channel = 0 ) const;
	uint32_t* histogram( const typename pixel_type::channel_type *channel_data, int low, int up, uint32_t* norm );

};

template <>
bool cimage<pixel_mono8>::create_preview( void );

template <>
bool cimage<pixel_mono12>::create_preview( void );

template <>
bool cimage<pixel_mono16>::create_preview( void );

template <>
bool cimage<pixel_rgb24>::create_preview( void );

template <>
bool cimage<pixel_rgb48>::create_preview( void );

#endif
