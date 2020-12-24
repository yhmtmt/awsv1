#include <time.h>
#include <stdlib.h>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <iostream>

using namespace std;

#include "aws_clock.hpp"
#include "aws_stdlib.hpp"

int main(int argc, char ** argv)
{
  if(argc < 2){
    cout << "Usage: str2t -Y <year> -M <month> -D <day> -h <hour> -m <minute> -s <second> -ms <millisecond>" << endl;
    return 0;
  }

  tmex tm;
  tm.tm_year = 1970;
  tm.tm_mon = 1;
  tm.tm_mday = 1;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_msec = 0;
  for(int i = 0; i < argc; i++){
    char * str = argv[i];
    
    if(str[0] == '-'){
      switch(str[1]){
      case 'Y':
	tm.tm_year = atoi(argv[i+1]);
	break;
      case 'M':
	tm.tm_mon = mon2i(argv[i+1]);
	if(tm.tm_mon < 1 || tm.tm_mon >= 12){
	  cerr << "Invalid month given." << endl;
	  return 1;
	}
	break;
      case 'D':
	tm.tm_mday = atoi(argv[i+1]);
	if(tm.tm_mday < 1 || tm.tm_mday > 31){
	  cerr << "Day is out of range [1, 31]." << endl;
	  return 1;
	}
	break;
      case 'h':
	tm.tm_hour = atoi(argv[i+1]);
	if(tm.tm_hour < 0 || tm.tm_hour >= 24){
	  cerr << "Hour is out of range [0, 24)" << endl;
	  return 1;
	}
	break;
      case 'm':
	if(str[2] == 's'){
	  tm.tm_msec = atoi(argv[i+1]);
	  if(tm.tm_msec < 0 || tm.tm_msec >= 1000){
	    cerr << "Millisec is out of range [0,1000)." << endl;
	    return 1;
	  }
	}else{
	  tm.tm_min = atoi(argv[i+1]);
	  if(tm.tm_min < 0 || tm.tm_min >= 60){
	    cerr << "Minute is out of range [0, 60)." << endl;
	    return 1;
	  }	 
	}
	break;
      case 's':
	tm.tm_sec = atoi(argv[i+1]);
	if(tm.tm_sec < 0 || tm.tm_sec >= 60){
	  cerr << "Second is out of range [0,60)." << endl;
	  return 1;
	}
	break;
      }
      i++;
    }       
  }
  
  printf("%lld", mkgmtimeex(tm) * MSEC);
  
  return 0;
};
