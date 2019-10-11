// Copyright(c) 2012 Yohei Matsumoto, Tokyo University of Marine
// Science and Technology, All right reserved. 

// c_aws is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// c_aws is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with c_aws.  If not, see <http://www.gnu.org/licenses/>.

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

#include "aws_stdlib.hpp"
#include "aws_sock.hpp"
#include "aws_thread.hpp"
#include "aws_clock.hpp"

#include "channel_base.hpp"
#include "filter_base.hpp"
#include "aws_command.hpp"
#include "aws.hpp"

// Command explanation
// * channel <type> <name>
// Creating an channel instance of <type> with <name> 
// * filter <type> <name> -i <input list> -o <output list>
// Creating an filter instance of <type> with <name>. In <input list> and <output list>, users can specify names of the channel instances.
// * fcmd <filter name> <filter command>
// Executing filter commands defined in each filter.  
// * fset <filter name> <parameter and value list>
// Setting the values to their specified parameters. If only a parameter name is specified as <parameter and value list>, an explanation to the parameter is returned.
// * fget <filter name> <parameter list>
// Getting the values of the parameters specified.
// * finf | finf <filter name> | finf n <filter id>
// This will be hidden command. The first case, the number of filters are returned.
// The second and third case, filter name, filter id, number of parameters, number of input channels, number of output channels, are returned  
// * fpar <filter name> <parameter id>
// An explanatio of the parameter of the filter is returned.
// * chinf | chinf <channel name> | chinf n <channel id>
// This will be hidden command. The first case, the number of channels are returned. 
// The second and third cases, channel name and id are returned.
// * go | go <start time> | go <start time> <end time> | go to <end time>
// start the filter graph's threads from <start time> to <end time> if specified. Filter graph moves the state "stop" to "run".
// Start and end times are not allowed for online mode. In the online mode, the filter graph's start and end time should be controled in script.
// * stop 
// stop the filter graph's threads.
// * quit
// shutdown the aws process
// * step | step <time> | step c <cycle>
// In pause state, first usage proceeds one cycle, second one proceeds to the specified <time>, and third one proceeds specified cycles.
// * cyc <sec>
// Setting cycle time 
// * online {yes | no}
// Setting execution mode. In the offline mode, pause and step is allowed. 
// * pause
// The filter graph moves the sate "run" to "pause".
// * clear
// All filters and channels are discarded.
// * rcmd <port number>
// Invoking reciever thread of rcmd with <port number>
// * trat <int >= 1
// Setting trat. trat enables the faster time clocking. For example, the time goes twice as fast as usual with trat of 2. 
// Not that, trat is only allowed for offline mode.


c_aws::c_aws(int argc, char ** argv):CmdAppBase(argc, argv),
				     m_cmd_port(20000),
				     m_working_path(nullptr),
				     m_lib_path(nullptr), m_log_path(nullptr),
				     m_bonline(true), m_exit(false),
				     m_cycle_time(166667), m_time(0), m_time_zone_minute(540), m_time_rate(1)
{
  set_name_app("aws");
  set_version(0, 20);
  set_year_copy(2018);
  set_name_coder("Yohei Matsumoto");
  set_contact("matumoto (at) kaiyodai.ac.jp");
  
  add_arg("-port", "Port number the command processor uses.");
  add_val(&m_cmd_port, "int");
  
  add_arg("-wpath", "Path to the working directory.");
  add_val(&m_working_path, "string");

  add_arg("-logpath", "Path to the log files");
  add_val(&m_log_path, "string");
  
  add_arg("-libpath", "Path to the filter library.");
  add_val(&m_lib_path, "string");
    
  add_arg("-tzm", "Time Zone in minutes.");
  add_val(&m_time_zone_minute, "int");
  
  // Initializing filter globals
  f_base::init(this);
  
  // Initializing channel globals
  ch_base::init();
  
  srand((unsigned int) time(NULL));
}

c_aws::~c_aws()
{
  f_base::uninit();
  ch_base::uninit();
  clear();
}


void c_aws::clear()
{
  for(auto itr = filters.begin(); 
      itr != filters.end(); itr++)
    delete itr->second;
  filters.clear();
  filter_libs.clear();

  for(vector<ch_base*>::iterator itr = m_channels.begin();
      itr != m_channels.end(); itr++)
    delete (*itr);
  
  m_channels.clear();
}

bool c_aws::push_command(const char * cmd_str, char * ret_str,
			 bool & ret_stat) {
  unique_lock<mutex> lock(m_mtx);
  s_cmd & cmd = m_cmd;
  memset(m_cmd.ret, 0, RET_LEN);
  
  // split token
  int itok = 0;
  int total_len = 0;
  const char * ptr0 = cmd_str;
  char * ptr1 = m_cmd.mem;
  
  while (itok < CMD_ARGS) {
    // Skip space and tab
    int len_skip = skip_space(ptr0, CMD_LEN - total_len);
    total_len += len_skip;
    if (total_len >= CMD_LEN) {
      ret_stat = false;
      return false;
    }
    ptr0 = ptr0 + len_skip;
    
    // extract token (continuous string or [] bounded strings)
    m_cmd.args[itok] = ptr1;
    int len = 0; // token length
    int bq = 0;  // non-zero value in []
    while (bq || *ptr0 != ' ' && *ptr0 != '\t' && *ptr0 != '\0') {
      if (*ptr0 == '[') {
	bq++;
      }
      else if (*ptr0 == ']') {
	bq--;
      }
      
      *ptr1 = *ptr0;
      ptr0++;
      ptr1++;
      total_len++;
      len++;
      
      if (total_len == CMD_LEN) {
	ret_stat = false;
	return false;
      }
    }

    // store token 
    if (len != 0) {
      *ptr1 = '\0'; // terminating the token
      ptr1++;
      itok++;
    }

    // the command should terminate with null character
    if (*ptr0 == '\0') 
      break;
  }

  if (itok == 0) { // no token.
    ret_stat = true;
    return true;
  }
  cmd.num_args = itok;
  
  // decoding command type
  cmd.type = cmd_str_to_id(m_cmd.args[0]);
  if (cmd.type == CMD_UNKNOWN) {
    spdlog::error("Unknown command {}",  m_cmd.args[0]);
    return false;
  }

  // waiting for the command processed
  m_cmd.stat = CS_SET;
  {
    e_cmd_stat & stat = m_cmd.stat;
    m_cnd_ret.wait(lock, [&stat] {return stat == CS_RET || stat == CS_ERR; });
  }
  
  memcpy(ret_str, m_cmd.ret, RET_LEN);
  
  if(m_cmd.stat == CS_ERR)
    ret_stat = false;
  else
    ret_stat = true;
  
  m_cmd.stat = CS_NONE;
  
  return true;
}

///////////////////////// command handler
// handle_stop stops all the filters in the graph
bool c_aws::handle_stop()
{
  bool stopped = false;
  f_base::m_clk.stop();
  spdlog::info("Stopping all filters.");
  while(!stopped){
    stopped = true;
    for(auto fitr = filters.begin(); 
	fitr != filters.end(); fitr++){
      stopped = stopped && fitr->second->stop(); 
    }
    f_base::send_clock_signal();
  }

  for(auto fitr = filters.begin();
      fitr != filters.end(); fitr++){
    fitr->second->destroy();
    fitr->second->runstat();    
  }
  spdlog::info("All filters successfully stopped.");
  
  return true;
}

// handle_run runs all the filters in the graph.
bool c_aws::handle_chan(s_cmd & cmd)
{
  bool result;
  if(cmd.num_args < 3){
    spdlog::error("Too few argument for command {}", cmd.args[0]);
    return false;
  }
  
  if(!(result = add_channel(cmd))){
    string message("Failed to create channel ");
    message += cmd.args[2];
    message += " of ";
    message += cmd.args[1];
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
  }
  return result;
}

bool c_aws::handle_fltr(s_cmd & cmd)
{
  bool result;
  if(cmd.num_args < 3){
    string message("Too few argument for command ");
    message += cmd.args[0];
    message += ".";
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN,  message.c_str());
    return false;
  }
  
  if(!(result = add_filter(cmd))){
    string message ("Failed to create filter ");
    message += cmd.args[2];
    message += " of ";
    message += cmd.args[1];
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN,  message.c_str());
  }
  return result;
}

bool c_aws::handle_fset(s_cmd & cmd)
{
  bool result;
  
  if(cmd.num_args <= 2){
    string message("Too few argument for command ");
    message += cmd.args[0];
    message += ".";
    spdlog::error(message);
    result = false;
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    return result;
  }

  auto fitr = filters.find(string(cmd.args[1]));
  
  if(fitr == filters.end()){
    string message("In command ");
    message += cmd.args[0];
    message += ", filter ";
    message += cmd.args[1];
    message += " not found.";
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    if(cmd.num_args == 3){ // no value present
      result = fitr->second->get_par_info_by_fset(cmd);
    }else{
      fitr->second->lock_cmd(true);
      if(!fitr->second->set_par(cmd)){
	result = false;
      }else{
	result = true;
      }
      fitr->second->unlock_cmd(true);
    }
  }
  
  return result;
}

bool c_aws::handle_fget(s_cmd & cmd)
{
  bool result;
  if(cmd.num_args <= 2){
    string message("Too few argument for command ");
    message += cmd.args[0];
    message += ".";
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN,  message.c_str());
    result = false;
    return result;
  }

  f_base * pfilter = get_filter(cmd.args[1]);
  auto fitr = filters.find(string(cmd.args[1]));
  
  if(fitr == filters.end()){
    string message("In command ");
    message += cmd.args[0];
    message += ", filter ";
    message += cmd.args[1];
    message += " was not found.";    
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    fitr->second->lock_cmd(true);
    if(!fitr->second->get_par(cmd)){
      result = false;
    }else{
      result = true;
    }
    fitr->second->unlock_cmd(true);
  }
  
  return result;
}

bool c_aws::handle_finf(s_cmd & cmd)
{
  bool result;
  // This command retrieves filter information
  f_base * pfilter = NULL;
  int ifilter;
  string message("In command ");
  message += cmd.args[0];
  
  if(cmd.num_args == 2){ // The second argument is filter name
    pfilter = get_filter(cmd.args[1]);
    if(pfilter == NULL){
      message = string(", filter ") + cmd.args[1] + string(" was not found.");
      spdlog::error(message);
      snprintf(cmd.get_ret_str(), RET_LEN,  message.c_str());
    }else{
      ifilter = 0;
      for(auto iftr = filters.begin(); iftr != filters.end(); iftr++){
	if(pfilter == iftr->second)
	  break;
	ifilter++;
      }
    }
  }else if(cmd.num_args == 3 && cmd.args[1][0] == 'n'){ 
    // if n is specified as the second argument, the filter id is used as 
    // the argument.
    ifilter = atoi(cmd.args[2]);
    if(ifilter < filters.size()){
      auto itr = filters.begin();
      for (int i = 0; i != ifilter; i++, itr++);
      pfilter = itr->second;       
    }else{
      message += " , filter id = " + to_string(ifilter) + " not exist.";
      spdlog::error(message);
      snprintf(cmd.get_ret_str(), RET_LEN,  message.c_str());
    }
  }else{ // if there is no argument, the number of filters is returned
    snprintf(cmd.get_ret_str(), RET_LEN, "%d", (int) filters.size());
    result = true;
    return result;
  }
  
  if(pfilter == NULL){
    result = false;
  }else{
    pfilter->get_info(cmd, ifilter);
    result = true;
  }
  
  return result;
}

bool c_aws::handle_fpar(s_cmd & cmd)
{
  bool result;
  f_base * pfilter = get_filter(cmd.args[1]);
  if(pfilter == NULL){
    string message("In command ");
    message += cmd.args[0];
    message += string(", filter ") + cmd.args[1] + " was not found.";
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    if(!pfilter->get_par_info(cmd)){
      result = false;
    }else{
      result = true;
    }
  }
  return result;
}

bool c_aws::handle_chinf(s_cmd & cmd)
{
  bool result;
  
  // This command retrieves channel information the codes below are almost
  // same as FINF
  ch_base * pch = NULL;
  int ich;
  if(cmd.num_args == 2){
    pch = get_channel(cmd.args[1]);
    if(pch == NULL){
      snprintf(cmd.get_ret_str(), RET_LEN, "Channel %s was not found.", cmd.args[1]);
    }else{
      for(ich = 0; ich < m_channels.size(); ich++){
	if(pch == m_channels[ich])
	  break;
      }
    }
  }else if(cmd.num_args == 3 && cmd.args[1][0] == 'n'){
    ich = atoi(cmd.args[2]);
    if(ich >= m_channels.size()){
      snprintf(cmd.get_ret_str(), RET_LEN, "Channel id=%d does not exist.", ich);
    }else{
      pch = m_channels[ich];
    }
  }else{
    snprintf(cmd.get_ret_str(), RET_LEN, "%d", (int)m_channels.size());
    result = true;
    return result;
  }
  
  if(pch == NULL){
    result = false;
  }else{
    pch->get_info(cmd, ich);
    result = true;
  }
	
  return result;
}

bool c_aws::handle_quit(s_cmd & cmd)
{
  bool result = true;
  spdlog::info("Quit aws");
  m_exit = true;
  return result;
}

bool c_aws::handle_step(s_cmd & cmd)
{
  bool result;
  if(!f_base::m_clk.is_pause()){
    string message("Step command can only be used in pause state.");
    spdlog::info(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    // parsing argument 
    int cycle;
    if(cmd.num_args == 1){
      cycle = 1;
      f_base::m_clk.step(cycle);
      result = true;
    }else if(cmd.num_args == 2){
      // step to absolute time
      long long tabs;
      tmex tm;
      if(decTmStr(cmd.args[1], tm)){
	tabs = mkgmtimeex_tz(tm, f_base::get_tz()) * MSEC;
      }else{
	tabs = (long long) atol(cmd.args[1]) * (long long) SEC;
      }
      f_base::m_clk.step(tabs);
      result = true;
    }else if(cmd.num_args == 3){
      if(cmd.args[1][0] != 'c'){
	result = false;
      }else{
	cycle = atoi(cmd.args[2]);
	f_base::m_clk.step(cycle);
	result = true;
      }
    }
  }
  
  return result;
}

bool c_aws::handle_cyc(s_cmd & cmd)
{
  bool result;
  if(!f_base::m_clk.is_stop()){
    string message("Cycle time cannot be changed during execution");
    spdlog::error(message);   
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    m_cycle_time = (long long) (atof(cmd.args[1]) * SEC);
    spdlog::info("Cycle time changed to {%lld}", m_cycle_time);
    result = true;
  }
  return result;
}

bool c_aws::handle_pause(s_cmd & cmd)
{
  bool result;
  if(!f_base::m_clk.is_run() || m_bonline){
    string message("Pause command should be used in run state under online mode.");
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    if(!f_base::m_clk.pause()){
      result = false;
    }else{
      result = true;
    }
  }
  return result;
}

bool c_aws::handle_clear(s_cmd & cmd)
{
  bool result;
  if(f_base::m_clk.is_run()){
    string message("Graph cannot be cleared during execution.");
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    clear();
    result = true;
  }
  return result;
}

bool c_aws::handle_rcmd(s_cmd & cmd)
{
  bool result;
  if(cmd.num_args == 2){
    c_rcmd * pcmd = new c_rcmd(this, atoi(cmd.args[1]));
    if(!pcmd->is_exit()){
      spdlog::info("New remote command port {} opened.", cmd.args[1]);
      m_rcmds.push_back(pcmd);
      result = true;
    }else{
      delete pcmd;
    }
  }
  return result;
}

bool c_aws::handle_trat(s_cmd & cmd)
{
  bool result;
  if(!f_base::m_clk.is_stop()){
    string message("Trat cannot be changed during execution");
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    result = false;
  }else{
    m_time_rate = (int) atoi(cmd.args[1]);
    result = true;
  }
  return result;
}

bool c_aws::handle_run(s_cmd & cmd)
{
  f_base::set_tz(m_time_zone_minute);
  
  if(f_base::m_clk.is_run()){
    string message("Filter graph is running.");
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());    
    return false;
  }
  
  if(f_base::m_clk.is_pause()){ // pause to run transition happens
    // in pause mode only 
    if(cmd.num_args == 3){
      if(cmd.args[1][0] != 't' || cmd.args[1][1] != 'o'){
	string message ("In pause state, \"go [to <later absolute time>]\"");
	spdlog::error(message);
	snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
	return false;
      }
      tmex tm;
      if(decTmStr(cmd.args[2], tm))
	m_end_time = mkgmtimeex_tz(tm, f_base::get_tz()) * MSEC;
      else
	m_end_time = (long long) atol(cmd.args[1]) * (long long) SEC;
    }else{
      m_end_time = LLONG_MAX;
    }
    
    if(m_end_time <= f_base::m_clk.get_time()){
      string message("Time should be later than current time.");
      spdlog::error(message);
      snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
      return false;
    }
    
    return f_base::m_clk.restart();
  }
  
  if(!check_graph()){
    string message("Failed to run filter graph.");
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    return false;
  }
  
  // if the starting time is given the time is decoded and stored into m_start_time 
  if(cmd.num_args >= 2){
    m_bonline = false;
    tmex tm;
    if(decTmStr(cmd.args[1], tm))
      m_start_time = mkgmtimeex_tz(tm, f_base::get_tz()) * MSEC;
    else
      m_start_time = (long long) atol(cmd.args[1]) * (long long) SEC;
    
  }else{
    // if no time specified, current time is used as the start time.
    m_bonline = true;
    m_start_time = (long long) time(NULL) * SEC; 
  }
  
  // if the end time is given, decoded and stored into m_end_time.
  if(cmd.num_args >= 3){
    tmex tm;
    if(decTmStr(cmd.args[2], tm))
      m_end_time = mkgmtimeex_tz(tm, f_base::get_tz()) * MSEC;
    else
      m_end_time = (long long) atol(cmd.args[2]) * (long long) SEC;
  }else{
    m_end_time = LLONG_MAX;
  }
  
  f_base::m_clk.start((unsigned) m_cycle_time, 
		      (unsigned) m_cycle_time, m_start_time, 
		      m_bonline, m_time_rate);
  f_base::m_clk.pause();
  
  m_time = f_base::m_clk.get_time();
  f_base::clock(m_start_time);  
  f_base::init_run_all();
  
  // check filter's status. 
  for(auto fitr = filters.begin();
      fitr != filters.end(); fitr++){
    spdlog::info("Starting filter {}.", fitr->second->get_name());
    
    if(!fitr->second->run(m_start_time, m_end_time)){
      string message ("Failed to start filetr");
      message += fitr->second->get_name();      
      snprintf(cmd.get_ret_str(),  RET_LEN, message.c_str());
      f_base::m_clk.stop();
      return false;
    }
  }

  f_base::m_clk.restart();
  
  return true;
}

bool c_aws::handle_frm(s_cmd & cmd)
{
  if(cmd.num_args != 2){
    string message("Too few argument for ");
    message += cmd.args[0];
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    return false;
  }
  auto itr = filters.find(cmd.args[1]);
  if(itr != filters.end()){
    filters.erase(itr);  
    spdlog::info("Filter {} removed.", cmd.args[1]);
  }else{
    string message("In command ");
    message += cmd.args[0];
    message += ", filter ";
    message += cmd.args[1];
    message += " not found.";
    spdlog::error(message);
    snprintf(cmd.get_ret_str(), RET_LEN, message.c_str());
    return false;
  }
  
  return true;
}

bool c_aws::handle_chrm(s_cmd & cmd)
{
  if(cmd.num_args != 2){
    return false;
  }
  
  for(vector<ch_base*>::iterator itr = m_channels.begin();
      itr != m_channels.end(); itr++){
    if(strcmp(cmd.args[1], (*itr)->get_name()) == 0){
      m_channels.erase(itr);
      break;
    }
  }
  
  return true;
}

void c_aws::proc_command()
{
  
  if (m_cmd.stat != CS_SET){
    // no command
    m_cnd_ret.notify_all();
    return;
  }
  
  {
    unique_lock<mutex> lock(m_mtx);
    
    s_cmd & cmd = m_cmd;
    bool result = false;
    switch(cmd.type){
    case CMD_CHAN:
      result = handle_chan(cmd);
      break;
    case CMD_FLTR:
      result = handle_fltr(cmd);
      break;
    case CMD_FSET:
      result = handle_fset(cmd);
      break;
    case CMD_FGET:
      result = handle_fget(cmd);
      break;
    case CMD_FINF:
      result = handle_finf(cmd);
      break;
    case CMD_FPAR:
      result = handle_fpar(cmd);
      break;
    case CMD_CHINF:
      result = handle_chinf(cmd);
      break;
    case CMD_GO:
      handle_run(cmd);
      result = true;
      break;
    case CMD_STOP:
      handle_stop();
      result = true;
      break;
    case CMD_QUIT:
      m_exit = true;
      result = true;
      break;
    case CMD_STEP:
      result = handle_step(cmd);
      break;
    case CMD_CYC:
      result = handle_cyc(cmd);
      break;
    case CMD_PAUSE:
      result = handle_pause(cmd);
      break;
    case CMD_CLEAR:
      result = handle_clear(cmd);
      break;
    case CMD_RCMD:
      result = handle_rcmd(cmd);
      break;
    case CMD_TRAT:
      result = handle_trat(cmd);
      break;
    case CMD_CHRM:
      result = handle_chrm(cmd);
      break;
    case CMD_FRM:
      result = handle_frm(cmd);
      break;
    case CMD_CD:
      if(cmd.num_args != 2)
	result = false;
      else{
	if(chdir(cmd.args[1]) == -1)
	  result = false;
	else
	  result = true;
      }
      break;
    case CMD_TIME:
      if(cmd.num_args == 1){
	snprintf(cmd.get_ret_str(), RET_LEN, "%s", f_base::get_time_str());
	result = true;
      }else if(cmd.num_args == 2 && cmd.args[1][0] == 'n'){
	snprintf(cmd.get_ret_str(), RET_LEN, "%lld", f_base::m_clk.get_time());
	result = true;
      }else if(cmd.num_args == 2 && cmd.args[1][0] == 's'){
	f_base::set_sys_time();
	result = true;
      }else
	result = false;
    }
    
    if(!result){
      m_cmd.stat = CS_ERR;
    }else{
      m_cmd.stat = CS_RET;
    }
    
    m_cmd.set_ret_stat(result);
    
    lock.unlock();
    m_cnd_ret.notify_all();
  }
}

ch_base * c_aws::get_channel(const char * name)
{
  for(vector<ch_base*>::iterator itr = m_channels.begin();
      itr != m_channels.end(); itr++){
    const char * cname = (*itr)->get_name();
    if(strcmp(cname, name) == 0)
      return *itr;
  }
  return NULL;
}

f_base * c_aws::get_filter(const char * name)
{
  auto filter = filters.find(string(name));
  if(filter == filters.end())
    return nullptr;
  return filter->second;    
}


bool c_aws::add_channel(s_cmd & cmd)
{
  char ** tok = cmd.args;
  int itok = 1;
  if(get_channel(tok[itok+1]) != NULL){
    cerr << "Cannot register channels with same name " << tok[itok+1] << "." << endl;
    return false;
  }
  
  ch_base * pchan = ch_base::create(tok[itok], tok[itok+1]);
  if(pchan == NULL)
    return false;
  m_channels.push_back(pchan);
  return true;
}

bool c_aws::add_filter(s_cmd & cmd)
{
  char ** tok = cmd.args;
  int itok = 1;
  
  if(get_filter(tok[itok+1]) != nullptr){  
    spdlog::error("In command {0}, filter {1} has already been instantiated.", cmd.args[0], cmd.args[itok+1]);    
    return false;
  }

  string type_str(tok[itok]);
  auto filter_lib = filter_libs.find(type_str);
  if(filter_lib == filter_libs.end()){
    string path;
    if(m_lib_path)
      path =  string(m_lib_path);
    else
      path = string("lib");    
    path += string("/lib") + type_str + string(".so");
    
    unique_ptr<c_filter_lib> lib(new c_filter_lib);
    if(!lib->load(path)){
      spdlog::error("Failed to load shared object {}.", path);
      return false;
    }

    spdlog::info("{} successfully loaded.", path);
    filter_libs.insert(
		       make_pair(type_str,
				 std::unique_ptr<c_filter_lib>(move(lib)))
		       );
    
    filter_lib = filter_libs.find(type_str);
  }

  string filter_str(tok[itok+1]);
 f_base * pfilter = filter_lib->second->create(filter_str);
  if(pfilter == nullptr){
    spdlog::error("Failed to instantiate filter {0} of {1}", filter_str, type_str);
    return false;
  }
  
  try{
    bool is_input = true;
    for(itok = 3; itok < cmd.num_args; itok++){
      if(tok[itok][0] == '-'){
	if(tok[itok][1] == 'i'){
	  is_input = true;
	}else if(tok[itok][1] == 'o'){
	  is_input = false;
	}
      }else{
	ch_base * pchan = get_channel(tok[itok]);
	if(pchan == NULL){

	  spdlog::error("In command {0}, channel {1} to be connected with filter {2} not found.", cmd.args[0], tok[2], tok[itok]);
	  throw cmd.ret;
	}
	if(is_input)
	  pfilter->set_ichan(pchan);     
	else
	  pfilter->set_ochan(pchan);
      }
    }
  }catch(char *){
    delete pfilter;
    return false;
  }
  
  filters.insert(make_pair(filter_str, pfilter));  

  spdlog::info("Filter {} created.", filter_str);
  return true;
}

bool c_aws::check_graph()
{
  bool healthy = true;
  
  for(auto itr = filters.begin(); 
      itr != filters.end(); itr++){
    if(!itr->second->check()){
      healthy = false;
    }
  }
  
  return healthy;
}

bool c_aws::main()
{
  string logpath;
  if(m_log_path){
    logpath = string(m_log_path);    
  }else{
    logpath = string("logs");
  }
  logpath += "/aws.log";
  spdlog::flush_every(chrono::seconds(5));
  try{
    auto console_sink = make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = make_shared<spdlog::sinks::basic_file_sink_mt>(logpath);
    spdlog::set_default_logger(make_shared<spdlog::logger>("aws", spdlog::sinks_init_list({console_sink, file_sink})));    
  }
  catch (const spdlog::spdlog_ex & ex)
    {
      cout << "Log Initialization failed: "<< ex.what() << endl;
      return false;
    }
  spdlog::set_pattern("[%c][%L][%t] %v");
  
  spdlog::info("Logging started on {}", logpath);
  
  c_rcmd * prcmd = new c_rcmd(this, m_cmd_port);
  if(!prcmd->is_exit()){
    spdlog::info("Starting command server with port {}", m_cmd_port);
    m_rcmds.push_back(prcmd);
  }else{
    delete prcmd;
    spdlog::error("Failed to start command server.");
    return false;
  }
  
  if(m_working_path){
    if(chdir(m_working_path) != 0)
      spdlog::error("Failed to change working path to {}.", m_working_path);
  }

  if(m_lib_path){
    spdlog::info("Filter path is configured as {}.", m_lib_path);
  }else{
    spdlog::info("Filter path is configuread as ./lib");
  }
  
  m_exit = false;
  
  while(!m_exit){
    proc_command();
    if(!f_base::m_clk.is_stop()){
      // wait the time specified in cyc command.
      f_base::m_clk.wait();
      
      // getting current time
      m_time = f_base::m_clk.get_time();
      
      // sending clock signal to each filters. The time string for current time is generated simultaneously
      f_base::clock(m_time);
      
      // checking activity of filters. 
      for(auto itr = filters.begin(); 
	  itr != filters.end(); itr++){
	if(itr->second->is_main_thread())
	  itr->second->fthread();
	if(!itr->second->is_active()){
	  cout << itr->second->get_name() << " stopped." << endl;
	  f_base::m_clk.stop();
	  break;
	}
      }
      
      // Time is exceeded over m_end_time, automatically pause.
      if(!m_bonline && m_time >= m_end_time){
	f_base::m_clk.pause();
      }
      
      if(f_base::m_clk.is_stop()){
	// stop all the filters
	handle_stop();
	m_exit = true;
      }
    }
  }

  handle_stop();
  
  for(int i = 0; i < m_rcmds.size() ;i++){
    delete m_rcmds[i];
  }
  m_rcmds.clear();
  return true;
}

//////////////////////////////////////////////////////// class c_rcmd member
c_rcmd::c_rcmd(c_aws * paws, unsigned short port):m_paws(paws), m_th_rcmd(NULL)
{

  signal(SIGPIPE, SIG_IGN);
  
  m_to.tv_sec = 5;
  m_to.tv_usec = 0;
  m_exit = true;
  m_svr_sock = socket(AF_INET, SOCK_STREAM, 0);
  if(m_svr_sock == SOCKET_ERROR){
    spdlog::error("During command server start-up, socket() failed with SOCKET_ERROR");
    m_svr_sock = -1;
    return;
  }
  
  m_svr_addr.sin_family = AF_INET;
  m_svr_addr.sin_port = htons(port);
  m_svr_addr.sin_addr.s_addr = INADDR_ANY;
  
  int ret;
  int val = 1;
  ret = setsockopt(m_svr_sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
  ret = ::bind(m_svr_sock, (sockaddr*)&m_svr_addr, sizeof(m_svr_addr));
  
  if(ret != 0){
    spdlog::error("Durign command server start-up, bind() failed with SOCKET_ERROR");
    closesocket(m_svr_sock);
    m_svr_sock = -1;
    return;
  }
  
  ret = listen(m_svr_sock, 5);
  if(ret == SOCKET_ERROR){
    spdlog::error("During command server start-up, listen() failed with SOCKET_ERROR");
    closesocket(m_svr_sock);
    m_svr_sock = -1;
    return;
  }
  
  m_exit = false;
  
  m_th_rcmd = new thread(thrcmd, this);
}

c_rcmd::~c_rcmd(){
  if(!m_exit){
    m_exit = true;
    m_th_rcmd->join();
    delete m_th_rcmd;
    m_th_rcmd = NULL;
  }
  if(m_svr_sock != SOCKET_ERROR && m_svr_sock != -1)
    closesocket(m_svr_sock);
}

// wait_connection waits connection to socket s. The function don't return
// until session opened or timeout.
bool c_rcmd::wait_connection(SOCKET & s){
  m_to.tv_sec = 3;
  m_to.tv_usec = 0;
  FD_ZERO(&m_fdread);
  FD_ZERO(&m_fderr);
  FD_SET(m_svr_sock, &m_fdread);
  FD_SET(m_svr_sock, &m_fderr);
  int n = select((int)(m_svr_sock)+1, &m_fdread, NULL, &m_fderr, &m_to);
  
  if(n > 0){
    if(FD_ISSET(m_svr_sock, &m_fdread)){
      int len = sizeof(m_svr_addr);
#ifdef _WIN32
      s = accept(m_svr_sock, (sockaddr*)&m_svr_addr, &len);
#else
      s = accept(m_svr_sock, (sockaddr*)&m_svr_addr, (socklen_t*) &len);
#endif
      return true;
    }else if(FD_ISSET(m_svr_sock, &m_fderr)){
      spdlog::error("In command server, wait_connection encountered socket error.");
      m_exit = true;
      return false;
    }
  }
  return false;
}

// wait_receive receives data to buf from socket s. Data length is returned
// to the argument "total". This function should be called after session opened.
bool c_rcmd::wait_receive(SOCKET & s, char * buf, int & total){
  m_to.tv_sec = 3;
  m_to.tv_usec = 0;
  FD_ZERO(&m_fdread);
  FD_ZERO(&m_fderr);
  FD_SET(s, &m_fdread);
  FD_SET(s, &m_fderr);
  
  int n = select((int)s+1, &m_fdread, NULL, &m_fderr, &m_to);
  if(n > 0){
    if(FD_ISSET(s, &m_fdread)){
      total += recv(s, buf + total, CMD_LEN - total, 0);
      return total == CMD_LEN;
    }else if(FD_ISSET(s, & m_fderr)){
      spdlog::error("In command server, wait_receive encountered socket error.");
       return false;
    }
  }	
  
  return false;
}

// push_command send command string (cmd_str) to the aws object.
// this function blocks until the command processed in the aws object.
// return string is stored in ret_str. if the command failed, ret_stat is
// set as false.
bool c_rcmd::push_command(const char * cmd_str, char * ret_str, bool ret_stat){
  bool stat = false;
  if(!m_paws->push_command(cmd_str, ret_str, ret_stat)){
    spdlog::error("Unknown command {} ", cmd_str);
    return false;
  }
  return true;
}

// wait_send send is used to send return string to the sender of the command.
// This function returns after sending data or timeout
bool c_rcmd::wait_send(SOCKET & s, char * buf)
{
  m_to.tv_sec = 3;
  m_to.tv_usec = 0;
  FD_ZERO(&m_fdwrite);
  FD_ZERO(&m_fderr);
  FD_SET(s, &m_fdwrite);
  FD_SET(s, &m_fderr);
  int n = select((int)s+1, NULL, &m_fdwrite, &m_fderr, &m_to);
  
  if(n > 0){
    if(FD_ISSET(s, &m_fdwrite)){
      send(s, buf, CMD_LEN, 0);
      return true;
    }else if(FD_ISSET(s, &m_fderr)){
      spdlog::error("In command server, wait_send encountered socket error.");
      return false;
    }
  }
  return false;
}

// command processing thread repeates 1. waiting for connection from command 
// sender, 2. receiving the command, 3. processing the command in aws, 4. 
// returning the resulting return value. This thread is invoked in the constructor,
// and terminated in the destructor.
void c_rcmd::thrcmd(c_rcmd * prcmd)
{
  while(!prcmd->is_exit()){
    SOCKET s;
    if(!prcmd->wait_connection(s)){
      continue;
    }
    
    while(1){
      int total = 0;
      if (!prcmd->wait_receive(s, prcmd->m_buf_recv, total))
	break;
      if(strcmp("eoc", prcmd->m_buf_recv) == 0){
	break;
      }
      
      // push command 
      bool stat = false;
      if(total == CMD_LEN){
	prcmd->m_paws->push_command(prcmd->m_buf_recv, 
				    prcmd->m_buf_send, stat);	  
      }
      else {
	break;
      }
      
      // return
      while(!prcmd->wait_send(s, prcmd->m_buf_send));
    }
    closesocket(s);
  }
}

bool c_rcmd::is_exit(){
  return m_paws->is_exit() || m_exit;
}


int main(int argc, char ** argv)
{
  c_aws aws(argc, argv);
  
  aws.run();
  return 0;
}
