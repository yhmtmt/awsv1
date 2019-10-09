#ifndef AWS_SOCK_H
#define AWS_SOCK_H
// Copyright(c) 2014 Yohei Matsumoto, Tokyo University of Marine
// Science and Technology, All right reserved. 

// aws_sock.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_sock.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_sock.hpp.  If not, see <http://www.gnu.org/licenses/>. 

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define SOCKET int

#ifndef SOCKET_ERROR 
#define SOCKET_ERROR (-1)
#endif

#ifndef closesocket
inline int closesocket(SOCKET s)
{
  return ::close(s);
}
#endif

#define SD_RECEIVE 0
#define SD_SEND 1
#define SD_BOTH 2


inline int set_sock_nb(SOCKET s)
{
  int val = 1;
  // if succeeded, zero is returned
  return ioctl(s, FIONBIO, &val);
}

inline int get_socket_error()
{
  return errno;
}

inline bool ewouldblock(int er){
  return er == EWOULDBLOCK;
}

inline bool econnreset(int er){
  return er == ECONNRESET;
}

int dump_socket_error();

inline void set_sockaddr_addr(sockaddr_in & addr, const char * str_addr = NULL)
{
  if(str_addr == NULL){
    addr.sin_addr.s_addr = INADDR_ANY;
  }else{
    addr.sin_addr.s_addr = inet_addr(str_addr);
  }
};

#endif
