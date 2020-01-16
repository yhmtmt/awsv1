// Copyright(c) 2012 Yohei Matsumoto, All right reserved. 

// f_base is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// f_base is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with f_base.  If not, see <http://www.gnu.org/licenses/>. 

#include "aws.hpp"

// Initialization function. 
// This function is called at the begining of the aws process start. If you
// need to initialize global and static data structure please insert your 
// initialization code here.
void f_base::init(c_aws * paws){
  // initializing basic members of the filter graph.
  m_paws = paws;   
  // Insert your own initialization code
}

// Uninitialization function. 
// This function is called at the end of the aws process. If you add your
// initialization code, correspodning destruction code should be added to
// this function.
void f_base::uninit()
{
}


//////////////////////////////////////////////////// filter parameter
bool f_base::s_fpar::set(const char * valstr)
{
  switch(type){
  case F64:
    *f64 = atof(valstr);break;
  case S64:
    *s64 = atoll(valstr);break;
  case U64:
    *u64 = strtoull(valstr, NULL, 0);break;
  case F32:
    *f32 = (float) atof(valstr); break;
  case S32:
    *s32 = atoi(valstr); break;
  case U32:
    *u32 = strtoul(valstr, NULL, 0);break;
  case S16:
    *s16 = (short) atoi(valstr); break;
  case U16:
    *u16 = (unsigned short) strtoul(valstr, NULL, 0); break;
  case S8:
    *s8 = (char) atoi(valstr); break;
  case U8:
    *u8 = (unsigned char) strtoul(valstr, NULL, 0); break;
  case BIN:
    if(valstr[0] == 'y')
      *bin = true;
    else if (valstr[0] == 'n')
      *bin = false;
    else
      return false;
    break;
  case CSTR:
    if(strlen(valstr)+1 > len)
      return false;
    strncpy(cstr, valstr, len);
    break;
  case ENUM:
    for(*s32 = 0; *s32 < len; (*s32)++){
      if(strcmp(str_enum[*s32], valstr) == 0){
	break;
      }
    }
    break;
  case CH:
    {
      ch_base * pch = m_paws->get_channel(valstr);
      if(pch == NULL){
	cerr << "Channel name " << valstr << " could not be found." << endl;
	return false;
      }
      
      if(strcmp(typeid(*pch).name(), type_name) != 0){
	*ppch = NULL;
	cerr << "Type " << typeid(*pch).name()
	     << " does not match " << type_name << endl;
	cerr << "Channel name " << pch->get_name() << endl;
	return false;
      }
      *ppch = pch;
      break;
    }
  }
  return true;
}

bool f_base::s_fpar::get(string & valstr)
{
  int n;
  ostringstream strm;
  switch(type){
  case F64:
    strm << *f64; break;
  case S64:
    strm << *s64; break;
  case U64:
    strm << *u64; break;
  case F32:
    strm << *f32; break;
  case S32:
    strm << *s32; break;
  case U32:
    strm << *u32; break;
  case S16:
    strm << *s16; break;
  case U16:
    strm << *u16; break;
  case S8:
    strm << (int) *s8; break;
  case U8:
    strm << (unsigned int)*u8; break;
  case BIN:
    strm << ( *bin ? "y" : "n"); break;
  case CSTR:
    strm << cstr; break;
  case ENUM:
    strm << str_enum[*s32]; break;
  case CH:
    if(*ppch)
      strm << (*ppch)->get_name();
    else
      strm << "NULL";
    break;
  default:
    return false;
  }
  valstr = strm.str();
  return true;  
}

bool f_base::s_fpar::get(char * valstr, size_t sz){
  int n;
  switch(type){
  case F64:
    n = snprintf(valstr, sz, "%e", *f64);break;
  case S64:
    n = snprintf(valstr, sz, "%lld", *s64);break;
  case U64:
    n = snprintf(valstr, sz, "%llu", *u64);break;
  case F32:
    n = snprintf(valstr, sz, "%e", *f32);break;
  case S32:
    n = snprintf(valstr, sz, "%d", *s32);break;
  case U32:
    n = snprintf(valstr, sz, "%u", *u32);break;
  case S16:
    n = snprintf(valstr, sz, "%hd", *s16);break;
  case U16:
    n = snprintf(valstr, sz, "%hu", *u16);break;
  case S8:
    n = snprintf(valstr, sz, "%hhd", *s8);break;
  case U8:
    n = snprintf(valstr, sz, "%hhu", *u8);break;
  case BIN:
    n = snprintf(valstr, sz, "%s", (*bin) ? "y" : "n");break;
  case CSTR:
    n = snprintf(valstr, sz, "%s", cstr);break;
  case ENUM:
    n = snprintf(valstr, sz, "%s", str_enum[*s32]);break;
  case CH:
    if(*ppch)
      n = snprintf(valstr, sz, "%s", (*ppch)->get_name());
    else
      n = snprintf(valstr, sz, "NULL");
  }
  return n < sz;
}


void f_base::s_fpar::get_info(string & expstr)
{
  expstr = string(explanation);
}

////////////////////////////////////////////////////// f_base members
mutex f_base::m_mutex;
condition_variable f_base::m_cond;
long long f_base::m_cur_time = 0;
long long f_base::m_count_clock = 0;
int f_base::m_time_zone_minute = 540;
char f_base::m_time_str[32];
tmex f_base::m_tm;
c_clock f_base::m_clk;
c_aws * f_base::m_paws = NULL;

void f_base::set_lib(c_filter_lib * lib)
{
  m_lib = lib;
  m_lib->ref_up();
}

const string & f_base::get_type_name()
{
  return m_lib->get_type_name();  
}

void f_base::fthread()
{
  if(!init_run()){
    spdlog::error("[{}] Initialization failed.", get_name());
    return;
  }
  m_bactive = true;  
  while(m_bactive){
    m_count_pre = m_count_clock;
    
    while(m_cycle < (int) m_intvl){
      clock_wait();
      m_cycle++;
    }
    lock_cmd();
    update_table_objects();
    
    calc_time_diff();
    
    if(!proc()){
      break;
    }
    
    if(m_clk.is_run()){
      m_count_proc++;
      m_max_cycle = max(m_cycle, m_max_cycle);
      m_count_post = m_count_clock;
      m_cycle = (int)(m_count_post - m_count_pre);
      m_cycle -= m_intvl;
    }
    
    unlock_cmd();
  }
  
  m_bactive = false;
  destroy();
}

// Filter thread function
void f_base::sfthread(f_base * filter)
{
  filter->fthread();
}

bool f_base::stop()
{
  if(m_bactive){
    spdlog::info("Stopping {}.", m_name);
    m_bactive = false;
  }
  
  if(m_fthread){
    m_fthread->join();
    delete m_fthread;
    m_fthread = NULL;
    return true;	          
  }
  
  return false;
}

void f_base::clock(long long cur_time)
{
  unique_lock<mutex> lock(m_mutex);
  if(m_clk.is_run())
    m_count_clock++;
  m_cur_time = cur_time;
  gmtimeex(m_cur_time / MSEC  + m_time_zone_minute * 60000, m_tm);
  snprintf(m_time_str, 32, "[%s %s %02d %02d:%02d:%02d.%03d %d] ", 
	   getWeekStr(m_tm.tm_wday), 
	   getMonthStr(m_tm.tm_mon),
	   m_tm.tm_mday,
	   m_tm.tm_hour,
	   m_tm.tm_min,
	   m_tm.tm_sec,
	   m_tm.tm_msec,
	   m_tm.tm_year + 1900);
  lock.unlock();
  m_cond.notify_all();
}

f_base::f_base(const char * name):m_lib(nullptr),
				  m_offset_time(0), m_bactive(false),
				  m_fthread(NULL), m_intvl(1),
				  m_cmd(false), m_mutex_cmd()
{
  m_name = new char[strlen(name) + 1];
  strncpy(m_name, name, strlen(name) + 1);

  register_fpar("TimeShift", &m_offset_time, "Filter time offset relative to global clock. (may be offline mode only)");
  register_fpar("Interval", &m_intvl, "Execution interval in cycle. (default 0)");
  register_fpar("ProcCount", &m_count_proc, "Number of execution."); 
  register_fpar("ClockCount", &m_count_clock, "Number of clock cycles passed." );
  register_fpar("ProcRate", &m_proc_rate, "Processing rate.(Read only)");
  register_fpar("MaxCycle", &m_max_cycle, "Maximum cycles per processing.(Read Only)");
}

f_base::~f_base()
{
  del_table();
  m_chin.clear();
  m_chout.clear();
  delete [] m_name;
  m_lib->ref_down();
}


bool f_base::set_par(const string & par, const string & val)
{
  int ipar = find_par(par.c_str());
  if(ipar < 0){
    spdlog::error("No parameter named {} in filter {}.", par, get_name()); 
    return false;
  }
  
  m_pars[ipar].set(val.c_str());
  return true;
}

bool f_base::get_par(const string & par, string & val)
{
  int ipar = find_par(par.c_str());
  if(ipar < 0){
    spdlog::error("No parameter named {} in filter {}.", par, get_name()); 
    return false;
  }

  return m_pars[ipar].get(val);
}

bool f_base::get_par(const int ipar, string & par, string & val)
{
  if(m_pars.size() > ipar && ipar >= 0){
    par = string(m_pars[ipar].name);
    return m_pars[ipar].get(val);
  }
  return false;
}

bool f_base::get_par(const string & par, string & val, string & exp)
{
  int ipar = find_par(par.c_str());
  if(ipar < 0){
    spdlog::error("No parameter named {} in filter {}.", par, get_name());     
    return false;
  }
  
  m_pars[ipar].get_info(exp);
  return m_pars[ipar].get(val);  
}

bool f_base::get_par(const int ipar, string & par,  string & val, string & exp)
{
  if(m_pars.size() > ipar && ipar >= 0){
    par = string(m_pars[ipar].name);
    m_pars[ipar].get_info(exp);
    return m_pars[ipar].get(val);
  }
  return false;  
}

