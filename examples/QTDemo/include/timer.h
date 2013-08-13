#ifndef TIMER_H_
#define TIMER_H_

#include <time.h>


class ctimer
{
 public:

	 ctimer()
     {
		 start();
     };
     ~ctimer(){};

     void start( void )
     {
    	 clock_gettime( CLOCK_REALTIME, &tp1 );
     }
     unsigned long gettime( void )
     {
    	 clock_gettime( CLOCK_REALTIME, &tp2 );

    	 //res = diff( &tp1, &tp2 );
    	 //msec = res.tv_sec*1000 + res.tv_nsec*1e-6;
    	 msec = 1000*(tp2.tv_sec - tp1.tv_sec) + (tp2.tv_nsec - tp1.tv_nsec)*(1e-6);
         return msec;
     }

 private:
 	 struct timespec tp1, tp2, res;
 	 unsigned long msec;

 	 inline timespec diff(timespec *start, timespec *end)
 	 {
 	 	timespec temp;
 	 	if( (end->tv_nsec-start->tv_nsec) < 0 )
 	 	{
 	 		temp.tv_sec  = end->tv_sec-start->tv_sec-1;
 	 		temp.tv_nsec = 1000000000+end->tv_nsec-start->tv_nsec;
 	 	} else
 	 	{
 	 		temp.tv_sec  = end->tv_sec-start->tv_sec;
 	 		temp.tv_nsec = end->tv_nsec-start->tv_nsec;
 	 	}
 	 	return temp;
 	 }

};


#endif /*TIMER_H_*/
