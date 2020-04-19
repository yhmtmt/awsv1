#ifndef AWS_HPP
#define AWS_HPP
// Copyright(c) 2014-2020 Yohei Matsumoto, All right reserved. 

// aws.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Publica License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws.hpp.  If not, see <http://www.gnu.org/licenses/>.
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
using grpc::ServerWriter;
using CommandService::Config;
using CommandService::Command;
using CommandService::RunParam;
using CommandService::StopParam;
using CommandService::QuitParam;
using CommandService::TimeInfo;
using CommandService::Time;
using CommandService::ClockParam;
using CommandService::ClockState;

using CommandService::FltrInfo;
using CommandService::FltrParInfo;
using CommandService::LstFltrsParam;
using CommandService::FltrLst;
using CommandService::FltrIODir;
using CommandService::FltrIOChs;
using CommandService::FltrMsgReq;
using CommandService::FltrMsg;

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

#include "CmdAppBase.hpp"

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
    if(!ptr){
      spdlog::error("{} does not have factory.", m_type_name);
    }
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
  bool run_filter(const string & name);  
  bool stop_filter(const string & name);
  bool quit();  
  bool add_filter(const string & type, const string & name);  
  bool del_filter(const string & name);
  bool set_fltr_par(const FltrInfo * inf);
  bool get_fltr_par(const FltrInfo * inf_req, FltrInfo * inf_rep);  
  bool set_fltr_io_chs(const FltrIOChs * lst);
  bool get_fltr_io_chs(const FltrIOChs * lst_req, FltrIOChs * lst_rep);  
  bool add_channel(const string & type, const string & name);
  bool del_channel(const string & name);  
  bool gen_table(const string & type_name, const string & inst_name);
  bool del_table(const string & inst_name);    
  t_base * get_table(const string & inst_name);
  t_base * get_table(const string & type_name, const string & inst_name);
  f_base * get_filter(const char * name);
  f_base * get_filter(const string & name)
  {
    return get_filter(name.c_str());
  }  
  ch_base * get_channel(const char * name);
  ch_base * get_channel(const string & name)
  {
    return get_channel(name.c_str());
  }  
  void get_fltr_lst(FltrLst * lst);
  void get_ch_lst(ChLst * lst);
  void get_tbl_lst(TblLst * lst);
  
protected:
  mutex m_mtx;

  Config conf;
  char * m_config_file;
  char * m_working_path;
  
  int m_time_rate;

  // start time and end time
  long long m_start_time, m_end_time;
  
  // current time
  long long m_time;

  // time zone in minute
  int m_time_zone_minute;

  map<string, unique_ptr<c_filter_lib>> filter_libs;
  map<string, f_base*> filters;
  map<string, t_base*> tbls;  
  vector<ch_base *> m_channels;

  void clear();
  
  // flag for exiting function run()
  bool m_exit;
      
public:
  c_aws(int argc, char ** argv);
  virtual ~c_aws();

  void lock()
  {
    m_mtx.lock();
  }

  void unlock()
  {
    m_mtx.unlock();
  }

  void set_end_time(long long tend)
  {
    m_end_time = tend;
  }
   
  bool is_exit(){
    return m_exit;
  }

  const string get_log_path(){
    return conf.log_path();
  }

  const string get_data_path(){
    return conf.data_path();
  }
  
  virtual void print_title();
  virtual bool main();
};

#endif
