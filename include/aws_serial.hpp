#ifndef AWS_SERIAL_H
#define AWS_SERIAL_H


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

int enc_cbr(int cbr);

#define NULL_SERIAL -1
#define AWS_SERIAL int
AWS_SERIAL open_serial(const char * dname, int cbr, bool nonblk = false);

bool close_serial(AWS_SERIAL h);
int write_serial(AWS_SERIAL, char * buf, int len);
int read_serial(AWS_SERIAL, char * buf, int len);

#endif
