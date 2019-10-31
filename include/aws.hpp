#ifndef AWS_H
#define AWS_H
// Copyright(c) 2014 Yohei Matsumoto, Tokyo University of Marine
// Science and Technology, All right reserved. 

// c_aws.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Publica License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// c_aws.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with c_aws.h.  If not, see <http://www.gnu.org/licenses/>.
#include <cstdio>
#include <climits>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <queue>
using namespace std;

#include <signal.h>

#include <dlfcn.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <flatbuffers/flatbuffers.h>

#include "aws_stdlib.hpp"
#include "aws_sock.hpp"
#include "aws_thread.hpp"
#include "aws_clock.hpp"

#include "table_base.hpp"
#include "channel_base.hpp"
#include "filter_base.hpp"
#include "aws_command.hpp"

#include "CmdAppBase.hpp"

class c_rcmd;

class c_filter_lib
{
private:
  void * m_handle;
public:

  c_filter_lib():m_handle(nullptr)
  {
  }
  
  ~c_filter_lib()
  {
    if(m_handle){
      dlclose(m_handle);
      m_handle = nullptr;
    }
  }
  
  bool load(const string & path)
  {
    m_handle = dlopen(path.c_str(), RTLD_NOW);
    if(!m_handle){
      cerr << path << " no such lib." << endl;
      return false;
    }
    return true;
  }

  f_base * create(const string & name)
  {
    if(!m_handle)
      return nullptr;

    void * ptr = dlsym(m_handle, "factory");
    f_base * (*fptr)(const string &);
    fptr = (f_base* (*)(const string &))ptr;
    f_base * filter = fptr(name);

    return filter;
  }
};


//////////////////////////////////////////////////////////// class c_aws
// c_aws is the main class of automatic watch system.
// main() instantiate a c_aws objects and runs c_aws::run(). 
class c_aws: public CmdAppBase
{
protected:
  s_cmd m_cmd;
  mutex m_mtx;
  condition_variable m_cnd_ret, m_cnd_none;

  int m_cmd_port;
  char * m_working_path;
  char * m_log_path;
  char * m_lib_path;

  // for remote command processor
  vector<c_rcmd*> m_rcmds;

  int m_time_rate;

  // start time and end time
  long long m_start_time, m_end_time;
  
  // current time
  long long m_time;

  // time zone in minute
  int m_time_zone_minute;

  // cycle time
  long long m_cycle_time; // in 100ns

  map<string, unique_ptr<c_filter_lib>> filter_libs;
  map<string, f_base*> filters;

  map<string, t_base*> tbls;
  
  vector<ch_base *> m_channels;

  bool m_blk_cmd;
  int skip_space(const char * ptr, int len)
  {
    int len_skip = 0;
    while (*ptr == ' ' || *ptr == '\t') {
      ptr++;
      len_skip++;
      if (len_skip >= len)
	return len;
    }
    return len_skip;
  }

  void proc_command();
  
  void clear();
  
  // flag for exiting function run()
  bool m_exit;
  
  bool m_bonline;
  
  // create and add channel
  bool add_channel(s_cmd & cmd);
  
  // create and add filter
  bool add_filter(s_cmd & cmd);
  
  // check filter graph
  bool check_graph();
  
  // command handlers
  bool handle_chan(s_cmd & cmd);
  bool handle_fltr(s_cmd & cmd);
  bool handle_fcmd(s_cmd & cmd);
  bool handle_fset(s_cmd & cmd);
  bool handle_fget(s_cmd & cmd);
  bool handle_finf(s_cmd & cmd);
  bool handle_fpar(s_cmd & cmd);
  bool handle_chinf(s_cmd & cmd);
  bool handle_quit(s_cmd & cmd);
  bool handle_step(s_cmd & cmd);
  bool handle_cyc(s_cmd & cmd);
  bool handle_pause(s_cmd & cmd);
  bool handle_clear(s_cmd & cmd);
  bool handle_rcmd(s_cmd & cmd);
  bool handle_trat(s_cmd & cmd);
  bool handle_run(s_cmd & cmd);
  bool handle_stop();
  bool handle_frm(s_cmd & cmd);
  bool handle_chrm(s_cmd & cmd);
  
public:
  c_aws(int argc, char ** argv);
  virtual ~c_aws();

  // getting a pointer of a channel object by its name.
  ch_base * get_channel(const char * name);

  // getting a pointer of a filter object by its name
  f_base * get_filter(const char * name);
  
  bool push_command(const char * cmd_str, char * ret_str, bool & ret_stat);
  
  long long  get_cycle_time(){
    return m_cycle_time;
  }
  
  bool is_exit(){
    return m_exit;
  }
  
  virtual void print_title();
  virtual bool main();
};

// class for remote command processor 
// this class is instanciated when the rcmd command is issued.
// aws instantiate at least one.
class c_rcmd
{
private:
  c_aws * m_paws;		// pointer to the system
  SOCKET m_svr_sock;		// socket waiting for commands
  sockaddr_in m_svr_addr;       // server address initiating socket
  sockaddr_in m_client;	        // client address initiating socket
  thread * m_th_rcmd;

  fd_set m_fdread;		// for use select() in recieving command
  fd_set m_fdwrite;		// for use select() in transmitting return value
  fd_set m_fderr;			// for use select() 
  timeval m_to;			// for use select()
  bool m_exit;			// thread termination flag

  char m_buf_recv[CMD_LEN];     // buffer for recieving command
  char m_buf_send[RET_LEN];     // buffer for return value 

  static void thrcmd(c_rcmd * ptr); // command processing thread function
  bool wait_connection(SOCKET & s); 
  bool wait_receive(SOCKET & s, char * buf, int & total);
  bool push_command(const char * cmd_str, char * ret_str, bool ret_stat);
  bool wait_send(SOCKET & s, char * buf);

public:
  c_rcmd(c_aws * paws, unsigned short port);
  ~c_rcmd();

  bool is_exit();
};

extern bool g_kill;

#endif
