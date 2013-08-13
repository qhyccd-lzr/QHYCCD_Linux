#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils.h"


void u_msg( const char *fmt, ...)
{
        va_list     argptr;
        QString     text;

        va_start (argptr,fmt);
        text.vsprintf(fmt, argptr);
        va_end (argptr);

        QMessageBox::information( NULL, "Info...", text, QMessageBox::Ok, QMessageBox::Ok );

        //QMessageBox::information( this, "Info...", QString().sprintf("test = %d", 13), QMessageBox::Ok, QMessageBox::Ok );
}


bool u_yes( const QString &question )
{
	if( QMessageBox::question( NULL, "Question", question, QMessageBox::Yes | QMessageBox::No ) == QMessageBox::Yes )
	    return true;

 return false;
}


double u_pow( double a, int n )
{
 int i;
 double res = 1;

       if( n <= 0 )
           return 1;

       for( i = 0;i < n;i++ )
            res *= a;

 return res;
}



void log_msg( FILE *stm, const char *fmt, ... )
{
 va_list	argptr;
 //char		text[];

 	va_start( argptr, fmt );
	vfprintf( stm, fmt, argptr);
	va_end (argptr);

	fflush( stm );

}


int u_mkdir(const char *path, mode_t mode)
{
    struct stat     st;
    int             status = 0;

    if( stat(path, &st) != 0 )
    {
        /* Directory does not exist */
        if( mkdir(path, mode) != 0 )
            status = -1;
    }
    else
    if( !S_ISDIR(st.st_mode) )
    {
        status = -1;
    }

    return status;
}


int u_mkpath(const char *path, mode_t mode)
{
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = strdup(path);

    status = 0;
    pp = (char *)path;
    while( status == 0 && (sp = strchr(pp, '/')) != 0 )
    {
        if( sp != pp )
        {
            // Neither root nor double slash in path
            *sp = '\0';
            status = u_mkdir(path, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if( status == 0 )
        status = u_mkdir(path, mode);

    free(copypath);
    return (status);
}

