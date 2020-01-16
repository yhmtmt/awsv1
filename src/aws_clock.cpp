// Copyright(c) 2012-2020 Yohei Matsumoto, All right reserved. 

// aws_clock.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_clock.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_clock.cpp.  If not, see <http://www.gnu.org/licenses/>. 
#include <time.h>

#include <iostream>
#include <algorithm>
#include <cstring>
using namespace std;

#include "aws_clock.hpp"

static const char * strWday[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char * strMonth[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

int mon2i(char * str)
{
  if(str[0] <= '9' && str[0] >= '0')
    return atoi(str);

  for(int i = 0; i < 12; i++){
    if(strcmp(strMonth[i], str) == 0)
      return i + 1;
  }

  return -1;
}


const char * getMonthStr(int month)
{
  if(month < 0 || month >= 12)
    return NULL;
  return strMonth[month];
}

const char * getWeekStr(int wday)
{
  if(wday < 0 || wday >= 7)
    return NULL;
  
  return strWday[wday];
}

void gmtimeex(long long msec, tmex & tm)
{
  time_t sec = (time_t) (msec / 1000);
  struct tm * ptm = gmtime(&sec);
  tm.tm_hour = ptm->tm_hour;
  tm.tm_isdst = ptm->tm_isdst;
  tm.tm_mday = ptm->tm_mday;
  tm.tm_min = ptm->tm_min;
  tm.tm_mon = ptm->tm_mon;
  tm.tm_sec = ptm->tm_sec;
  tm.tm_wday = ptm->tm_wday;
  tm.tm_yday = ptm->tm_yday;
  tm.tm_year = ptm->tm_year;
  tm.tm_msec = msec % 1000;
}


// Original code is from https://codeday.me/jp/qa/20190322/454429.html
const int SecondsPerMinute = 60;
const int SecondsPerHour = 3600;
const int SecondsPerDay = 86400;
const int DaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

bool IsLeapYear(short year)
{
    if (year % 4 != 0) return false;
    if (year % 100 != 0) return true;
    return (year % 400) == 0;
}

long long mkgmtimeex(tmex & tm)
{
    long long secs = 0;
    for (short y = 1970; y < tm.tm_year; ++y)
        secs += (IsLeapYear(y)? 366: 365) * SecondsPerDay;
    for (short m = 0; m < tm.tm_mon; ++m) {
        secs += DaysOfMonth[m - 1] * SecondsPerDay;
        if (m == 2 && IsLeapYear(tm.tm_year)) secs += SecondsPerDay;
    }
    secs += (tm.tm_mday - 1) * SecondsPerDay;
    secs +=  tm.tm_hour * SecondsPerHour;
    secs +=  tm.tm_min * SecondsPerMinute;
    secs +=  tm.tm_sec;
    return secs * 1000 + tm.tm_msec;
}
/*
long long mkgmtimeex(tmex & tm)
{
  long long sec = (long long) mkgmtime((struct tm*) &tm);
  cout << "mkgmtimeex sec " << sec << endl;
  return sec * 1000 + tm.tm_msec;
}
*/

long long mkgmtimeex_tz(tmex & tm /* local time */, 
			int tzm /* time zone in minute */)
{
  return mkgmtimeex(tm) - tzm * 60000;
}

bool decTmStr(char * tmStr, tmex & tm)
{
  // tmStr should begin with "[" and end with "]"
  if(tmStr[0] != '[')
    return false;
  // tmStr[1-3] is week day.
  for(tm.tm_wday = 0; tm.tm_wday < 7;	tm.tm_wday++){
    const char * ptr1 = strWday[tm.tm_wday];
    char * ptr2 = &tmStr[1];
    if(ptr1[0] == ptr2[0] && ptr1[1] == ptr2[1] && ptr1[2] == ptr2[2]){
      break;
    }	
  }
  
  if(tm.tm_wday == 7){
    cerr << "Error in decTmStr. Failed to decode week day." << endl;
    return false;
  }
  
  // tmStr[5-7] is month
  for(tm.tm_mon = 0; tm.tm_mon < 12; tm.tm_mon++){
    const char * ptr1 = strMonth[tm.tm_mon];
    char * ptr2 = &tmStr[5];
    if(ptr1[0] == ptr2[0] && ptr1[1] == ptr2[1] && ptr1[2] == ptr2[2]){
      break;
    }
  }
  
  if(tm.tm_mon == 12){
    cerr << "Error in decTmStr. Failed to decode month." << endl;
    return false;
  }
  
  // tmStr[9-10] is day
  tm.tm_mday = 10 * (tmStr[9] - '0') + tmStr[10] - '0';
  if(tm.tm_mday < 1 || tm.tm_mday >= 31){
    cerr << "Error in decTmStr. Day is out of range." << endl;
    return false;
  }
  
  // tmStr[12-13] is hour
  tm.tm_hour = 10 * (tmStr[12] - '0') + tmStr[13] - '0';
  if(tm.tm_hour < 0 || tm.tm_hour > 23){
    cerr << "Error in decTmStr. Hour is out of range." << endl;
    return false;
  }
  
  // tmStr[15-16] is min
  tm.tm_min = 10 * (tmStr[15] - '0') + tmStr[16] - '0';
  if(tm.tm_min < 0 || tm.tm_min > 59){
    cerr << "Error in decTmStr. Minute is out of range." << endl;
    return false;
  }
  
  // tmStr[18-19] is sec
  tm.tm_sec = 10 * (tmStr[18] - '0') + tmStr[19] - '0';
  if(tm.tm_sec < 0 || tm.tm_sec > 61){
    cerr << "Error in decTmStr. Second is out of range." << endl;
    return false;
  }
  
  // tmStr[21-23] is msec
  tm.tm_msec = 100 * (tmStr[21] - '0') + 10 * (tmStr[22] - '0') + tmStr[23] - '0';
  if(tm.tm_msec < 0 || tm.tm_msec > 999){
    cerr << "Error in decTmStr. Millisecond is out of range." << endl;
    return false;
  }
  
  // tmStr[25-28] is year
  tm.tm_year = 1000 * (tmStr[25] - '0') + 100 * (tmStr[26] - '0')
    + 10 * (tmStr[27] - '0') + tmStr[28] - '0' - 1900;
  if(tm.tm_year < 0){
    cerr << "Error in decTmStr. Year is out of range." << endl;
    return false;
  }
  
  // tmStr
  if(tmStr[29] != ']')
    return false;
  
  tm.tm_isdst = -1;
  return true;
}

c_clock::c_clock(void):
  m_period(166667), m_delta(0), m_delta_adjust(1 * MSEC), m_offset(0), m_bonline(false), m_state(STOP)
{
}

c_clock::~c_clock(void)
{
}

bool c_clock::start(long long offset, bool online)
{
  m_bonline = online;  
  m_offset = offset;
  
  if(m_state == STOP){
    clock_gettime(CLOCK_REALTIME, &m_ts_start);
    if(m_bonline){
      m_tcur = (long long)
	((long long)m_ts_start.tv_sec * SEC + (long long) (m_ts_start.tv_nsec / 100));

      timespec tres;
      clock_getres(CLOCK_REALTIME, &tres);
      m_delta_adjust = (long long)
	((long long)(tres.tv_sec * SEC) + (long long)(tres.tv_nsec / 100));
      m_offset = 0;
    }else{
      m_tcur = 0;
    }   
  }else if(m_state == RUN){
    stop();
    start(offset, online);
  }
  
  m_state = RUN;
  return true;
}

bool c_clock::restart()
{
  if(m_state == PAUSE){
    m_state = RUN;
  }else
    return false;  
  return true;
}

bool c_clock::step(int cycle)
{
  if(m_state == PAUSE){
    m_tcur += (long long) m_period * cycle;
  }else
    return false;
  return true;
}

bool c_clock::step(long long tabs)
{
  if(m_state = PAUSE){
    m_tcur = tabs;
  }else{
    return false;
  }
  return true;
}

// set_time method adjust aws time to the value specified.
// The method only calculates difference as delta, and the value is gradually reduced in multiple call of wait()
// Note that, aws clock never reset system time. The implementation for Linux system uses system clock to reduce drift, 
// that's why the change in system time affects on the aws time. 
void c_clock::set_time(tmex & tm)
{
  long long new_time = mkgmtimeex(tm) * MSEC; // converting to 100ns precision
  m_delta = new_time - m_tcur - m_offset;
}

void c_clock::set_time(long long & t){
  m_delta = t - m_tcur - m_offset;
}

void c_clock::set_time_delta(long long & delta){
  m_delta = delta;
}

long long c_clock::get_time()
{
  if(m_bonline){
      timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      return(long long)
	((long long)ts.tv_sec * SEC + (long long) (ts.tv_nsec / 100));
  }else{
    return m_tcur + m_offset;
  }
}

void c_clock::wait()
{
  long long delta_adjust = 0;
  if(abs(m_delta) < m_delta_adjust) // too small delta is ignored
    delta_adjust = 0;
  else if(abs(m_delta) > (30 * (long long) SEC)) // large delta is adjusted once
    delta_adjust = m_delta;
  else
    delta_adjust = (m_delta < 0 ? -m_delta_adjust : m_delta_adjust);
  
  timespec ts, trem;

  clock_gettime(CLOCK_REALTIME, &ts);	
  
  long long tnew, tdiff, tslp;
  if(m_bonline){
    tnew = (long long)
      ((long long)(ts.tv_sec * (long long)SEC) + (long long) (ts.tv_nsec / 100));

    timespec tsadj;
    if(delta_adjust){
      long long tadj = tnew + delta_adjust;
      tsadj.tv_sec = tadj / SEC;
      tsadj.tv_nsec = (tadj - tsadj.tv_sec * SEC) * 100;
     
      if(clock_settime(CLOCK_REALTIME, &tsadj) == 0){
	m_delta -= delta_adjust;
	tnew = tadj;
      }else{
	cerr << "Failed to adjust time. ts=" << tsadj.tv_sec
	     << "," << tsadj.tv_nsec << " delta=" << delta_adjust << endl;
      }
    }

  }else{
    tnew = (long long)
      ((long long)(ts.tv_sec - m_ts_start.tv_sec) * (long long) SEC)
      + (long long)((ts.tv_nsec - m_ts_start.tv_nsec) / 100);
  }

  tdiff = tnew - m_tcur; // time consumed in this cycle
  tslp = m_period - tdiff; // time to sleep
  if(tslp > 0){
    ts.tv_sec = tslp / SEC;
    ts.tv_nsec = (tslp - ts.tv_sec * SEC) * 100;
    while(nanosleep(&ts, &trem)){
      ts = trem;
    }    
    m_tcur = tnew + tslp;
  }else{
    m_tcur = tnew;
  }
  
  if(!m_bonline){
    if(m_state == PAUSE){
      m_offset -= (tdiff +  max(0LL, tslp));
    }
  }
}

void c_clock::stop()
{
  m_state = STOP;
}

bool c_clock::pause()
{
  if(m_state != RUN)
    return false;
  
  m_state = PAUSE;
  return true;
}
