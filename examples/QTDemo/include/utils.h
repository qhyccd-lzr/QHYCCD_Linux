#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <Qt>
#include <QMessageBox>

#include <sys/stat.h>
#include <sys/types.h>

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

bool u_yes( const QString &question );
void u_msg( const char *fmt, ...);

double u_pow( double a, int n );

void log_msg( FILE *stm, const char *fmt, ... );
#define log_i( fmt, ... )	log_msg( stdout, fmt"\n", ##__VA_ARGS__ )
#define log_e( fmt, ... )	log_msg( stderr, fmt"\n", ##__VA_ARGS__ )

int u_mkdir(const char *path, mode_t mode);
int u_mkpath(const char *path, mode_t mode);

#endif /*UTILS_H_*/
