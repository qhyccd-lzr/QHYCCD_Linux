#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


// we need a termios structure to clear the HUPCL bit
struct termios tio;
const char *device = "/dev/ttyUSB0";
const int mirror_lock_delay = 3; // seconds


int get_device_descriptor( const char *dev )
{
	int fd;

	if( (fd = open( dev, O_RDWR | O_NONBLOCK) ) < 0 )
	{
		return -1;
	}
	return fd;
}


void set_serial_signal( int fd, int status, int dtr, int rts )
{
	if( dtr )         // set the DTR line
		status &= ~TIOCM_DTR;
	else
		status |= TIOCM_DTR;

	if( rts )         // set the RTS line
		status &= ~TIOCM_RTS;
	else
		status |= TIOCM_RTS;

	ioctl( fd, TIOCMSET, &status ); // set the serial port status
}


int main( int argc, char **argv )
{
	int fd = -1, status = 0, arg = 0, use_mirror_lock = 0;

	if( argc < 2 )
	{
		printf( "Usage: %s [0|1] [0|1]\n", argv[0] );
		printf( "\t %s 1\tstart capture\n", argv[0] );
		printf( "\t %s 0\tend capture\n", argv[0] );
		printf( "\t %s 1 1\t start capture with mirror lock", argv[0] );
		return 1;
	}

	if( argc > 2 )
	{
		use_mirror_lock = atoi( argv[2] );
		if( use_mirror_lock != 0 && use_mirror_lock != 1 )
		{
			printf( "Mirror lock wrong value\n", device );
			return EXIT_FAILURE;
		}
	}

	arg = atoi( argv[1] );
	fd = get_device_descriptor( device );
	if( fd < 0 )
	{
		printf( "Couln't open %s\n", device );
		return EXIT_FAILURE;
	}
	
	tcgetattr( fd, &tio );          // get the termio info
	tio.c_cflag &= ~HUPCL;         // clear the HUPCL bit
	tcsetattr( fd, TCSANOW, &tio ); // set the termio info

	ioctl( fd, TIOCMGET, &status ); // get the serial port status

	switch( arg )
	{
	case 0:
		//printf ("000\n");
		set_serial_signal( fd, status, 1, 1 );
		break;
	case 1:
		//printf ("111\n");
		set_serial_signal( fd, status, 0, 0 );
		
		if( use_mirror_lock )
		{
			sleep( mirror_lock_delay );
			set_serial_signal( fd, status, 0, 1 );
			sleep( 1 );
			set_serial_signal( fd, status, 0, 0 );
		}
		break;
	}
	close( fd );

	return EXIT_SUCCESS;
}
