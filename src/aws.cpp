// Copyright(c) 2012-2019 Yohei Matsumoto, All right reserved. 

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

#include "aws.hpp"

#include "table_util.hpp"

class CommandServiceImpl final : public Command::Service
{
private:
  c_aws * paws;
public:
  CommandServiceImpl(c_aws * paws_):paws(paws_){};

  Status Run(ServerContext * context, const RunParam * par,
	     Result * res) override
  {
    paws->lock();
    if(paws->run_filter(par->fltr_name())){
      res->set_is_ok(true);
    }else{
      string msg ("Failed to run filter ");
      msg += par->fltr_name() + ".";
      res->set_is_ok(false);
      res->set_message(msg);
      spdlog::error(msg);
    }
    paws->unlock();
    return Status::OK;
  }

  Status Stop(ServerContext * context, const StopParam * par,
	      Result * res) override
  {
    paws->lock();
    if(paws->stop_filter(par->fltr_name())){
      res->set_is_ok(true);
    }else{
      string msg("Failed to stop filter ");
      msg += par->fltr_name() + ".";
      res->set_is_ok(false);
      res->set_message(msg);
      spdlog::error(msg);
    }
    paws->unlock();
    return Status::OK;
  }

  Status Quit(ServerContext * context, const QuitParam * par,
	      Result * res) override
  {
    paws->lock();
    paws->quit();
    res->set_is_ok(true);
    paws->unlock();
    return Status::OK;
  }
  
  Status GenFltr(ServerContext * context, const FltrInfo * inf,
		 Result * res) override
  {
    paws->lock();
    if(paws->add_filter(inf->type_name(), inf->inst_name()))
      res->set_is_ok(true);
    else{
      string msg("Failed to generate filter ");
      msg += inf->inst_name() + " of " + inf->type_name();
      res->set_is_ok(false);
      res->set_message(msg);
      spdlog::error(msg);
    }
    paws->unlock();
    return Status::OK;
  }

  Status DelFltr(ServerContext * context, const FltrInfo * inf,
		 Result * res) override
  {
    paws->lock();
    if(paws->del_filter(inf->inst_name())){
      res->set_is_ok(true);
    }else{
      string msg("Failed to remove filter ");
      msg += inf->inst_name() + ".";
      res->set_message(msg);
      res->set_is_ok(false);
    }
    paws->unlock();
    return Status::OK;
  }

  Status SetFltrPar(ServerContext * context, const FltrInfo * inf,
		    Result * res) override
  {
    paws->lock();
    if(paws->set_fltr_par(inf)){
      res->set_is_ok(true);
    }else{
      string msg("Failed to set filter parameters.");
      res->set_message(msg);
      res->set_is_ok(false);
    }
    paws->unlock();
    return Status::OK;
  }

  Status GetFltrPar(ServerContext * context, const FltrInfo * inf_req,
		    FltrInfo * inf_rep) override
  {
    paws->lock();
    paws->get_fltr_par(inf_req, inf_rep);
    paws->unlock();
    return Status::OK;
  }
  
  Status LstFltrs(ServerContext * context, const LstFltrsParam * par,
		  FltrLst * lst) override
  {
    paws->lock();
    paws->get_fltr_lst(lst);
    paws->unlock();
    return Status::OK;
  }

  Status SetFltrIOChs(ServerContext * context, const FltrIOChs * lst, Result * res)
  {
    paws->lock();
    if(!paws->set_fltr_io_chs(lst)){
      string message("Failed to set ");
      if(lst->dir() == FltrIODir::IN){
	message += "input channels of ";
      }else{
	message += "output channels of ";
      }
      message += lst->inst_name() + ".";
      res->set_message(message);
      res->set_is_ok(false);
      spdlog::error(message);
    }else{
      res->set_is_ok(true);
    }
       
    paws->unlock();
    return Status::OK;
  }  

  Status GetFltrIOChs(ServerContext * context, const FltrIOChs * lst_req, FltrIOChs * lst_rep)
  {
    paws->lock();
    if(!paws->get_fltr_io_chs(lst_req, lst_rep)){
      string message("Failed to get ");
      if(lst_req->dir() == FltrIODir::IN){
	message += "input channels of ";
      }else{
	message += "output channels of ";
      }
      message += lst_req->inst_name() + ".";
      spdlog::error(message);
    }
       
    paws->unlock();
    return Status::OK;
  }
  
    
  
  Status GenCh(ServerContext * context, const ChInfo * inf,
	       Result * res) override
  {
    paws->lock();
    if(paws->add_channel(inf->type_name(), inf->inst_name()))
      res->set_is_ok(true);
    else{
      string msg("Failed to generate channel ");
      msg += inf->inst_name() + " of " + inf->type_name() + ".";
      res->set_is_ok(false);
      res->set_message(msg);
      spdlog::error(msg);
    }
    paws->unlock();
    return Status::OK;
  }

  Status DelCh(ServerContext * context, const ChInfo * inf,
	       Result * res) override
  {
    paws->lock();
    if(paws->del_channel(inf->inst_name())){
      res->set_is_ok(true);
    }else{
      string msg("Failed to remove channel ");
      msg += inf->inst_name() + ".";
      res->set_message(msg);
      res->set_is_ok(false);
    }
    paws->unlock();
    return Status::OK;
  }

  Status LstChs(ServerContext * context, const LstChsParam * par,
		ChLst * lst) override
  {
    paws->lock();
    paws->get_ch_lst(lst);
    paws->unlock();
    return Status::OK;
  }
  
  Status GenTbl(ServerContext * context, const TblInfo * inf,
		Result * res) override
  {
    paws->lock();
    if(paws->gen_table(inf->type_name(), inf->inst_name()))
      res->set_is_ok(true);
    else{
      string msg("Failed to generate table ");
      msg += inf->inst_name() + " of " + inf->type_name();
      res->set_is_ok(false);
      res->set_message(msg);
      spdlog::error(msg);
    }
    paws->unlock();
    return Status::OK;
  }

  Status GetTbl(ServerContext * context, const TblInfo * inf,
		TblData * data) override
  {
    paws->lock();
    t_base * tbl;
    if(inf->type_name().length()){
      tbl = paws->get_table(inf->type_name(), inf->inst_name());
      data->set_inst_name(inf->inst_name());
      data->set_type_name(inf->type_name());
    }else{
      tbl = paws->get_table(inf->inst_name());
      data->set_inst_name(inf->inst_name());
      data->set_type_name(tbl->get_type());
    }
    
    if(tbl){
      string tbl_data((const char*)tbl->get_data().get(), tbl->get_data_size());
      data->set_tbl(tbl_data);
    }else{
      data->set_tbl(string());
    }
    paws->unlock();
    return Status::OK;    
  }

  Status SetTbl(ServerContext * context, const TblData * data,
		Result * res) override
  {
    paws->lock();
    auto tbl = paws->get_table(data->type_name(), data->inst_name());
    if(tbl){
      tbl->set(data->tbl());
      res->set_is_ok(true);
    }else{
      string msg("Failed to set table on ");
      msg += data->inst_name() + " of " + data->type_name();
      res->set_is_ok(false);
      res->set_message(msg);
      spdlog::error(msg);
    }
    paws->unlock();
    return Status::OK;    
  }

  Status SetTblRef(ServerContext * context, const TblRef * ref,
		   Result * res) override
  {
    paws->lock();
    auto tbl = paws->get_table(ref->tbl_name());
    auto flt = paws->get_filter(ref->flt_name());
    if(tbl && flt && tbl->set_flt_ref(ref->flt_tbl_name(), flt)){
      res->set_is_ok(true);
    }else{
      string msg("Failed to set table ");
      msg += ref->tbl_name() + " to " + ref->flt_name() + "." + ref->flt_tbl_name();
      res->set_is_ok(false);
      res->set_message(msg);
      spdlog::error(msg);      
    }
    paws->unlock();
    return Status::OK;    
  }

  Status DelTbl(ServerContext * context, const TblInfo * inf,
		Result * res) override
  {
    paws->lock();
    if(paws->del_table(inf->inst_name())){
      res->set_is_ok(true);
    }else{
      res->set_is_ok(false);
      string msg("No table named ");
      msg += inf->inst_name();
      msg += ".";
      res->set_message(msg);
    }
    paws->unlock();
    return Status::OK;
  }

  Status LstTbls(ServerContext * context, const LstTblsParam * par,
		 TblLst * lst)
  {
    paws->lock();
    paws->get_tbl_lst(lst);
    paws->unlock();
    return Status::OK;
  }
};

c_aws::c_aws(int argc, char ** argv):CmdAppBase(argc, argv),
				     m_working_path(nullptr),
				     m_config_file(nullptr),
				     m_exit(false),
				     m_cycle_time(166667),
				     m_time(0), m_time_zone_minute(540),
				     m_time_rate(1)
{
  set_name_app("aws");
  set_version(1, 00);
  set_year_copy(2019);
  set_name_coder("Yohei Matsumoto");
  set_contact("yhmtmt (at) gmail.com");
    
  add_arg("-wpath", "Path to the working directory.");
  add_val(&m_working_path, "string");

  add_arg("-config", "Configuration file");
  add_val(&m_config_file, "string");
     
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


bool c_aws::add_filter(const string & type, const string & name)
{
  if(get_filter(name) != nullptr){
    spdlog::error("Filter {} of {} has already been instantiated.", name, type);
    return false;
  }
  auto filter_lib = filter_libs.find(type);
  if(filter_lib == filter_libs.end()){   
    unique_ptr<c_filter_lib> lib(new c_filter_lib);
    if(!lib->load(conf.lib_path(), type)){
      spdlog::error("Failed to load filter library {}.", type);
      return false;
    }

    spdlog::info("Filter library {} successfully loaded.", type);
    filter_libs.insert(
		       make_pair(type,
				 std::unique_ptr<c_filter_lib>(move(lib)))
		       );
    
    filter_lib = filter_libs.find(type);
  }

  f_base * pfilter = filter_lib->second->create(name);
  if(pfilter == nullptr){
    spdlog::error("Failed to instantiate filter {0} of {1}", name, type);
    return false;
  }

  filters.insert(make_pair(name, pfilter));  
  
  spdlog::info("Filter {} created.", name);

  return true;
}

bool c_aws::del_filter(const string & name)
{
  auto itr = filters.find(name);  
  if(itr != filters.end()){
    if(itr->second->is_active()){
      spdlog::error("Filter {} is active.", name);      
      return false;
    }
    
    f_base * f = itr->second;
    const c_filter_lib * lib = f->get_lib();
    
    filters.erase(itr);
    delete f;    
    spdlog::info("Filter {} removed.", name);

    // remove library if the ref-count is zero.
    if(lib->ref() == 0){
      string type_name = lib->get_type_name();
      auto filter_lib = filter_libs.find(type_name);
      filter_libs.erase(filter_lib);
      spdlog::info("Filter lib {} is unloaded.", type_name);
    }
  }else{
    string message("Filter named ");
    message += name + " cannot be found.";
    spdlog::error(message);
    return false;
  }
  return true;
}

bool c_aws::add_channel(const string & type, const string & name)
{
  if(get_channel(name) != NULL){
    spdlog::error("Channel {} of {} has already been instantiated.", name, type);
    return false;
  }
  ch_base * pchan = ch_base::create(type.c_str(), name.c_str());
  if(pchan == NULL)
    return false;

  m_channels.push_back(pchan);
  return true;
}

bool c_aws::del_channel(const string & name)
{
  for(auto itr = m_channels.begin();
      itr != m_channels.end(); itr++){
    if(strcmp(name.c_str(), (*itr)->get_name()) == 0){
      m_channels.erase(itr);
      break;
    }
  }
  return true;
}



bool c_aws::main()
{
  if(m_config_file){
    if(!load_config(m_config_file, conf)){
      return false;
    }
  }else if(!load_config("aws.conf", conf)){
    return false;
  }
  
  string logpath = conf.log_path() + "/aws.log";
  
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
  spdlog::set_pattern("%^[%c][%L][%t]%$ %v");
  
  spdlog::info("Logging started on {}", logpath);
  
  if(m_working_path){
    if(chdir(m_working_path) != 0)
      spdlog::error("Failed to change working path to {}.", m_working_path);
  }

  spdlog::info("Filter path is configured as {}.", conf.lib_path());
  
  m_exit = false;

  string server_address = conf.address() + ":" + conf.port();
  CommandServiceImpl service(this);
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);
  unique_ptr<Server> server(builder.BuildAndStart());

  thread server_thread([&](){server->Wait();});
  spdlog::info("Command service started on {}.", server_address);

  
  f_base::set_tz(m_time_zone_minute);
  f_base::init_run_all();
  m_start_time = (long long) time(NULL) * SEC; 
  m_end_time = LLONG_MAX;
  m_cycle_time = conf.cycle_time();
  
  f_base::m_clk.start((unsigned) m_cycle_time, 
		      (unsigned) m_cycle_time, m_start_time, 
		      true, m_time_rate);
    
  while(!m_exit){
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
	auto f = itr->second;
	f->lock_cmd();
	if(f->is_main_thread() && f->is_active())
	  f->fthread();
	f->unlock_cmd();
      }      
    }
  }

  server->Shutdown();
  server_thread.join();
  
  return true;
}

int main(int argc, char ** argv)
{
  c_aws aws(argc, argv);
  
  aws.run();
  return 0;
}
