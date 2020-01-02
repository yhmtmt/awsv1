#ifndef AWS_H
#define AWS_H
// Copyright(c) 2014,2019 Yohei Matsumoto, All right reserved. 

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
#include <cstdlib>
#include <climits>
#include <cmath>
#include <cstring>
#include <iostream>
#include <memory>
#include <fstream>
#include <sstream>
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

#include <grpcpp/grpcpp.h>

#include <google/protobuf/util/json_util.h>
#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>
#include <flatbuffers/util.h>

#include "command.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using CommandService::Config;
using CommandService::Command;
using CommandService::RunParam;
using CommandService::StopParam;
using CommandService::QuitParam;

using CommandService::FltrInfo;
using CommandService::FltrParInfo;
using CommandService::LstFltrsParam;
using CommandService::FltrLst;

using CommandService::ChInfo;
using CommandService::LstChsParam;
using CommandService::ChLst;

using CommandService::TblRef;
using CommandService::TblInfo;
using CommandService::TblData;
using CommandService::LstTblsParam;
using CommandService::TblLst;

using CommandService::Result;

#include "aws_const.hpp"
#include "aws_stdlib.hpp"
#include "aws_sock.hpp"
#include "aws_coord.hpp"
#include "aws_thread.hpp"
#include "aws_clock.hpp"
#include "aws_map.hpp"

#include "table_base.hpp"
#include "channel_base.hpp"
#include "filter_base.hpp"
#include "aws_command.hpp"

#include "CmdAppBase.hpp"

class c_rcmd;



class c_filter_lib
{
private:
  string m_type_name;
  void * m_handle;
  int m_ref;
public:

  c_filter_lib():m_handle(nullptr), m_ref(0)
  {
  }
  
  ~c_filter_lib()
  {
    if(m_handle){
      dlclose(m_handle);
      m_handle = nullptr;
    }
  }
  
  bool load(const string & lib_path, const string & type_name)
  {
    m_type_name = type_name;
    string path;
    path = lib_path + string("/lib") + type_name + string(".so");    
    m_handle = dlopen(path.c_str(), RTLD_NOW);
    if(!m_handle){
      spdlog::error("Failed to open {}.", path);
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

    filter->set_lib(this);
    return filter;
  }

  const string & get_type_name() const
  {
    return m_type_name;
  }

  void ref_up()
  {
    m_ref++;
  }

  void ref_down()
  {
    m_ref--;
  }
  
  const int ref() const
  {
    return m_ref;
  }
};


//////////////////////////////////////////////////////////// class c_aws
// c_aws is the main class of automatic watch system.
// main() instantiate a c_aws objects and runs c_aws::run(). 
class c_aws: public CmdAppBase
{
public:
  bool run_filter(const string & name)
  {
    auto itr = filters.find(name);
    if(itr == filters.end()){
      spdlog::error("Failed to run filter {}.", name);
      return false;
    }

    f_base * filter = itr->second;
    
    if(!filter->run()){
      spdlog::error("Failed to run filter {}.", name);      
      return false;
    }
    
    return true;
  }
  
  bool stop_filter(const string & name)
  {
    auto itr = filters.find(name);
    if(itr == filters.end()){
      spdlog::error("No filter {} found, cannot be stopped.", name);
      return false;
    }

    f_base * filter = itr->second;
    while(!filter->stop());
    
    filter->destroy();
    filter->runstat();
    
    return true;
  }

  void quit()
  {
    spdlog::info("Quit aws");
    m_exit = true;
  }
  
  bool add_filter(const string & type, const string & name);  
  bool del_filter(const string & name);
  bool set_fltr_par(const FltrInfo * inf)
  {
    f_base * f = get_filter(inf->inst_name());
    int num_pars = inf->pars_size();
    bool suc = true;
    for(int ipar = 0; ipar < num_pars; ipar++){
      const FltrParInfo & par_inf = inf->pars(ipar);
      if(!f->set_par(par_inf.name(), par_inf.val())){
	spdlog::error("Failed to set parameter {} in {}.",
		      par_inf.name(), f->get_name());
	suc = false;
      }
    }
    return suc;
  }
  
  bool get_fltr_par(const FltrInfo * inf_req, FltrInfo * inf_rep)
  {
    f_base * f = get_filter(inf_req->inst_name());
    int num_pars = inf_req->pars_size();

    bool suc = true;
    if(num_pars == 0){ // all parameters
      for (int ipar = 0; ipar < f->get_num_pars(); ipar++){
	auto par = inf_rep->add_pars();
	string name, val, exp;
	if(!f->get_par(ipar, name, val, exp)){
	  suc = false;
	}else{
	  par->set_name(name);
	  par->set_val(val);
	  par->set_exp(exp);
	}
      }
    }else{
      for(int ipar = 0; ipar < num_pars; ipar++){
	auto par_req = inf_req->pars(ipar);
	auto par = inf_rep->add_pars();
	string val;
	if(!f->get_par(par_req.name(), val)){
	  spdlog::error("Failed to get parameter {} in {}.",
			par_req.name(), f->get_name());
	  suc = false;
	}else{
	  par->set_val(val);
	}
      }
    }
    
    return suc;
  }
  
  bool add_channel(const string & type, const string & name);
  bool del_channel(const string & name);
  
  bool gen_table(const string & type_name, const string & inst_name)
  {
    auto tbl = get_table(inst_name);
    if(tbl){ // the instance name has already been used      
      spdlog::error("Table {} has already been instantiated.", inst_name);
      return false;
    }

    tbl = new t_base(type_name, inst_name);
    if(tbl)
      tbls[inst_name] = tbl;
    else{
      spdlog::error("Table {} of {} cannot be instantiated because of the memory allocation error.", inst_name, type_name);
      return false;
    }

    spdlog::info("Table {} of {} created.", inst_name, type_name);
    return true;
  }

  bool del_table(const string & inst_name)
  {
    auto tbl = get_table(inst_name);
    if(!tbl){ // No such table      
      spdlog::error("In del_table(), table {} not found.", inst_name);
      return false;
    }
    delete tbl;
    tbls.erase(inst_name);
    spdlog::info("Table {} deleted.", inst_name);
    return true;
  }
  
  t_base * get_table(const string & inst_name)
  {
    auto itr = tbls.find(inst_name);
    if(itr == tbls.end())
      return nullptr;
    
    return itr->second;
  }

  t_base * get_table(const string & type_name, const string & inst_name)
  {
    auto tbl = get_table(inst_name);
    if(tbl->is_type(type_name))
      return tbl;
    spdlog::error("Talbe {} found, but type is not {}.", inst_name, type_name);
    return nullptr;
  }

  f_base * get_filter(const string & name)
  {
    return get_filter(name.c_str());
  }

  void get_fltr_lst(FltrLst * lst)
  {
    for(auto itr = filters.begin(); itr != filters.end(); itr++){
      f_base * f = itr->second;
      FltrInfo * info = lst->add_fltrs();
      info->set_inst_name(f->get_name());
      info->set_type_name(f->get_type_name());
      info->set_is_active(f->is_active());      
    }
  }
  
  ch_base * get_channel(const string & name)
  {
    return get_channel(name.c_str());
  }

  void get_ch_lst(ChLst * lst)
  {
    for(auto itr = m_channels.begin() ;itr != m_channels.end(); itr++){
      ch_base * ch = *itr;
      ChInfo * info = lst->add_chs();
      info->set_inst_name(ch->get_name());
      info->set_type_name(typeid(*ch).name());
    }
  }

  void get_tbl_lst(TblLst * lst)
  {
    for(auto itr = tbls.begin(); itr != tbls.end(); itr++){
      t_base * t = itr->second;
      TblInfo * info = lst->add_tbls();
      info->set_inst_name(t->get_name());
      info->set_type_name(t->get_type());
    }
  }
  
protected:
  s_cmd m_cmd;
  mutex m_mtx;
  condition_variable m_cnd_ret, m_cnd_none;

  Config conf;
  int m_cmd_port;
  char * m_config_file;
  char * m_working_path;
  
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
  bool handle_frm(s_cmd & cmd);
  bool handle_chrm(s_cmd & cmd);
  bool handle_stop();
  
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
