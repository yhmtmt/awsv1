#include <iostream>
using namespace std;

#include "aws_serial.hpp"

int enc_cbr(int cbr)
{
	switch(cbr){
	case 110:
		return B110;     //  baud rate
	case 300:
		return B300;     //  baud rate
	case 600:
		return B600;     //  baud rate
	case 1200:
		return B1200;     //  baud rate
	case 2400:
		return B2400;     //  baud rate
	case 4800:
		return B4800;     //  baud rate
	case 9600:
		return B9600;     //  baud rate
	case 19200:
		return B19200;     //  baud rate
	case 38400:
		return B38400;     //  baud rate
	case 57600:
		return B57600;     //  baud rate
	case 115200:
		return B115200;     //  baud rate
	}

	return -1;
}

AWS_SERIAL open_serial(const char * dname, int cbr, bool nonblk)
{
	AWS_SERIAL h = NULL_SERIAL;
	termios copt;
	if(enc_cbr(cbr) < 0)
		return h;

	h = ::open(dname, O_RDWR | O_NOCTTY /*| (nonblk ? O_NDELAY : 0)| O_NDELAY*/);
	if(h == NULL_SERIAL){
		return h;
	}

	tcgetattr(h, &copt);

	copt.c_cflag = enc_cbr(cbr) | CS8 | CLOCAL | CREAD;	
	copt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	copt.c_iflag &= ~(IGNCR | ICRNL | INLCR);
	copt.c_oflag &= ~(OPOST);

	copt.c_iflag |= IGNPAR;
	copt.c_cc[VMIN] = 1;
	copt.c_cc[VTIME] = 0;
	
	/*
	cfsetispeed(&copt, enc_cbr(cbr));
	cfsetospeed(&copt, enc_cbr(cbr));
	copt.c_cflag &= ~PARENB;
	copt.c_cflag &= ~CSTOPB;
	copt.c_cflag &= ~CSIZE;
	copt.c_cflag |= CS8;
	copt.c_cflag |= (CLOCAL | CREAD);
	*/
	cfmakeraw(&copt);

	tcflush(h, TCIFLUSH);

	if(tcsetattr(h, TCSANOW, &copt) != 0){
		::close(h);
		return NULL_SERIAL;
	}

	return h;
}


bool close_serial(AWS_SERIAL h)
{
	if(::close(h) == 0)
		return true;
	return false;
}

int write_serial(AWS_SERIAL h, char * buf, int len)
{
  if(h == NULL_SERIAL)
    return 0;
  int len_sent;
  len_sent = write(h, buf, len);
  /*
    cout << "Write serial: " << len << " bytes." << endl;
    cout.write(buf, len);
    cout << "(";
    for(int i = 0; i < len; i++)
    printf("%02x ", (int) buf[i]);
    cout << ")";
  */
  
  return len_sent;
}

int read_serial(AWS_SERIAL h, char * buf, int len)
{
  int len_rcvd;
  
  fd_set rd, er;
  timeval tout;
  tout.tv_sec = 0;
  tout.tv_usec = 0;
  FD_ZERO(&rd);
  FD_ZERO(&er);
  FD_SET(h, &rd);
  FD_SET(h, &er);
  
  int res = select(h+1, &rd, NULL, &er, &tout);
  len_rcvd = 0;
  if(res > 0){
    if(FD_ISSET(h, &rd)){
      len_rcvd = read(h, buf, len);
    }else if(FD_ISSET(h, &er)){
      cerr << "Error in read_serial." << endl;
      return -1;
    }
  }else if(res < 0){
    cerr << "Error in read_serial." << endl;
    return -1;
  }else{
    return 0;
  }  
  return len_rcvd;
}
