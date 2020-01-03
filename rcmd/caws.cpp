#include <iostream>
#include <fstream>

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/idl.h>
#include <flatbuffers/util.h>

#include <grpcpp/grpcpp.h>
#include <google/protobuf/util/json_util.h>
#include "command.grpc.pb.h"
using grpc::Channel;
using grpc::ClientContext;
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

#include "table_util.hpp"

enum cmd_id{
  RUN=0, STOP, QUIT,
  GEN_FLTR, DEL_FLTR, LST_FLTRS, SET_FLTR_PAR, GET_FLTR_PAR,
  GEN_CH, DEL_CH, LST_CHS,
  GEN_TBL, GET_TBL, SET_TBL, SET_TBL_REF, DEL_TBL, LST_TBLS,
  JSON, UNKNOWN
};

const char * str_cmd[UNKNOWN] = {
  "run", "stop", "quit",
  "genfltr", "delfltr", "lstfltrs", "setfltrpar", "getfltrpar",
  "gench", "delch", "lstchs",
  "gentbl", "gettbl", "settbl", "settblref", "deltbl", "lsttbl",
  ".json"
};

const char * str_cmd_usage[UNKNOWN] =
{
  "<filter inst name>", // RUN
  "<filter inst name>", // STOP
  "", // QUIT
  "<type name> <inst name>", // GEN_FLTR
  "<inst name>", // DEL_FLTR
  "", // LST_FLTRS
  "<inst name> [<par name> <val> ...]", // SET_FLTR_PAR
  "<inst name> [<par name> ...]", // GET_FLTR_PAR
  "<type name> <inst name>", // GEN_CH
  "<inst name>", // DEL_CH
  "", // LST_CHS
  "<type name> <inst name>", // GEN_TBL
  "<inst name>", // GET_TBL
  "<name> <type> [ -f <json file> | -s <json string> ]", // SET_TBL
  "<table inst name> <filter inst name> <filter table name>", // SET_TBL_REF
  "<inst name>", // DEL_TBL
  "", // LST_TBL
  "<json file>"
};


const cmd_id get_cmd_id(const char * str)
{
  for(int id = 0; id < (int)JSON; id++){
    int c = 0;
    for(; str_cmd[id][c] && str[c] && str_cmd[id][c] == str[c];c++);
    
    if(!str_cmd[id][c] && !str[c])
      return (cmd_id)id;	  
  }

  int c = strlen(str);
  if(c <= strlen(str_cmd[JSON]))
    return UNKNOWN;
  
  const char * ext = str_cmd[JSON] + strlen(str_cmd[JSON]);
    
  for(; *ext != '.' && *ext == str[c]; ext--, c--);
  if(*ext == str[c])
    return JSON;

  return UNKNOWN;    
}

class CommandHandler
{
private:
  std::string lib_path_;
  std::unique_ptr<Command::Stub> stub_;
public:
  CommandHandler(std::shared_ptr<Channel> channel,
		 const std::string & lib_path)
    : stub_(CommandService::Command::NewStub(channel)),
      lib_path_(lib_path)
  {
  }

  bool Run(const std::string & name)
  {
    RunParam par;
    par.set_fltr_name(name);
    Result res;
    ClientContext context;
    Status status = stub_->Run(&context, par, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }

    return true;
  }

  bool Stop(const std::string & name)
  {
    StopParam par;
    par.set_fltr_name(name);
    Result res;
    ClientContext context;
    Status status = stub_->Stop(&context, par, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }

    return true;
  }

  bool Quit()
  {
    QuitParam par;
    Result res;
    ClientContext context;
    Status status = stub_->Quit(&context, par, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }

    return true;    
  }

  bool GenFltr(const std::string & inst_name, const std::string & type_name)
  {
    FltrInfo info;
    info.set_type_name(type_name);
    info.set_inst_name(inst_name);
    Result res;
    ClientContext context;
    Status status = stub_->GenFltr(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }

    return true;
  }

  bool DelFltr(const std::string & name)
  {
    FltrInfo info;    
    info.set_inst_name(name);
    Result res;    
    ClientContext context;
    Status status = stub_->DelFltr(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }

    return true;
  }

  bool LstFltrs()
  {
    LstFltrsParam par;
    FltrLst lst;
    ClientContext context;
    Status status = stub_->LstFltrs(&context, par, &lst);
    if(!status.ok()){
      std::cout << "Error in LstFltrs" << std::endl;
      return false;
    }

    for(int ifltr = 0; ifltr < lst.fltrs_size(); ifltr++){
      const FltrInfo & info = lst.fltrs(ifltr);
      std::cout << info.inst_name() << "\t" << info.type_name() << "\t";
      if(info.is_active())
	std::cout << "active" << std::endl;
      else
	std::cout << "inactive" << std::endl;
    }
    
    return true;
  }

  bool SetFltrPar(const std::string & fltr_name,
		  const std::vector<std::string> & pars,
		  const std::vector<std::string> & vals)
  {
    if(pars.size() != vals.size()){
      return false;
    }
    
    FltrInfo info;
    info.set_inst_name(fltr_name);
    for(int ipar = 0; ipar < pars.size(); ipar++){
      FltrParInfo & par = *info.add_pars();
      par.set_name(pars[ipar]);
      par.set_val(vals[ipar]);
    }

    Result res;
    ClientContext context;
    Status status = stub_->SetFltrPar(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error: " << res.message() << std::endl;
      return false;
    }

    return true;
  }

  bool GetFltrPar(const std::string & fltr_name,
		  const std::vector<std::string> & pars)
  {
    FltrInfo info_req, info_rep;
    info_req.set_inst_name(fltr_name);
    for (int ipar = 0; ipar < pars.size(); ipar++){
      FltrParInfo & par = *info_req.add_pars();
      par.set_name(pars[ipar]);
    }

    Result res;
    ClientContext context;
    Status status = stub_->GetFltrPar(&context, info_req, &info_rep);
    if(!status.ok()){
      std::cout << "Error in GetFltrPar." << std::endl;
      return false;
    }

    for (int ipar = 0; ipar < pars.size(); ipar++){
      const FltrParInfo & par = info_rep.pars(ipar);
      std::cout << par.name() << " " << par.val() << std::endl;
    }
    return true;
  }
  
  bool GenCh(const std::string & inst_name, const std::string & type_name)
  {
    ChInfo info;
    info.set_inst_name(inst_name);
    info.set_type_name(type_name);
    Result res;
    ClientContext context;
    Status status = stub_->GenCh(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }

    return true;
  }

  bool DelCh(const std::string & name)
  {
    ChInfo info;
    info.set_inst_name(name);
    Result res;
    ClientContext context;
    Status status = stub_->DelCh(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }

    return true;
  }

  bool LstChs()
  {
    LstChsParam par;
    ChLst lst;
    ClientContext context;
    Status status = stub_->LstChs(&context, par, &lst);
    if(!status.ok()){
      std::cout << "Error in LstChs" << std::endl;
      return false;
    }

    for(int ich = 0; ich < lst.chs_size(); ich++){
      const ChInfo & info = lst.chs(ich);
      std::cout << info.inst_name() << "\t" << info.type_name() << "\t" << std::endl;
    }
    
    return true;
  }

  
  bool GenTbl(const std::string & inst_name, const std::string & type_name)
  {
    TblInfo info;
    info.set_type_name(type_name);
    info.set_inst_name(inst_name);

    Result res;
    ClientContext context;
    Status status = stub_->GenTbl(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }
    
    return true;
  }

  bool GetTbl(const std::string & name)
  {
    TblInfo info;
    info.set_inst_name(name);

    TblData data;
    ClientContext context;
    Status status = stub_->GetTbl(&context, info, &data);
    if(!status.ok()){
      std::cout << "Error in GetTbl()." << std::endl;
      return false;
    }
    
    std::string schema_file_name;
    schema_file_name = lib_path_ + "/" +  data.type_name() + ".bfbs";
    flatbuffers::Parser parser;
    
    if(!load_tbl_json_parser(schema_file_name, parser))
      return false;
 
    std::string json_file;
    bool ok = GenerateText(parser, (const uint8_t*)data.tbl().c_str(), &json_file);
    if(!ok){
      std::cerr << "Error: Failed to convert table " << name << 
	" to json string." << std::endl;
      return false;
    }
    
    std::cout << json_file << std::endl;
    
    return true;
  }

  bool SetTbl(const std::string & name, const std::string & type,
	      const std::string & opt, const std::string & opt_str)
  {
    TblData data;
    data.set_inst_name(name);
    data.set_type_name(type);

    std::string schema_file_name;
    schema_file_name = lib_path_ + "/" + type + ".bfbs";
    flatbuffers::Parser parser;
    if(!load_tbl_json_parser(schema_file_name, parser))
      return false;

    std::string json_str;
    if(opt == "-s"){ // opt_str is json string
      json_str = opt_str;
    }else if(opt == "-f"){ // opt_str is json file
      bool ok = flatbuffers::LoadFile(opt_str.c_str(), true, &json_str);
      
      if(!ok){
	std::cerr << "Error: Failed to load " << opt_str << "." << std::endl;
	return false;
      }
    }
    
    bool ok = parser.Parse(json_str.c_str());
    if(!ok){
      std::cerr << "Error: " << parser.error_ << std::endl;
      return false;
    }

    data.set_tbl((const void*)parser.builder_.GetBufferPointer(),
		 parser.builder_.GetSize());

    Result res;
    ClientContext context;
    Status status = stub_->SetTbl(&context, data, &res);

    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }
    
    return true;
  }

  bool SetTblRef(const std::string & tbl_name, const std::string & flt_name,
		 const std::string & flt_tbl_name)
  {
    TblRef ref;
    ref.set_tbl_name(tbl_name);
    ref.set_flt_name(flt_name);
    ref.set_flt_tbl_name(flt_tbl_name);

    Result res;
    ClientContext context;
    Status status = stub_->SetTblRef(&context, ref, &res);

    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }
   
    return true;
  }
  
  bool DelTbl(const std::string & name)
  {
    TblInfo info;
    info.set_inst_name(name);

    Result res;
    ClientContext context;
    Status status = stub_->DelTbl(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }
      
    return true;
  }

  bool LstTbls()
  {
    LstTblsParam par;
    TblLst lst;
    ClientContext context;
    Status status = stub_->LstTbls(&context, par, &lst);
    if(!status.ok()){
      std::cout << "Error in LstTbls" << std::endl;
      return false;
    }

    for(int itbl = 0; itbl < lst.tbls_size(); itbl++){
      const TblInfo & info = lst.tbls(itbl);
      std::cout << info.inst_name() << "\t" << info.type_name() << "\t" << std::endl;
    }
    
    return true;
  } 
};

void dump_usage(const cmd_id & id = UNKNOWN)
{
  std::cerr << "caws [<command>|<command json file>]" << std::endl;
  if(id == UNKNOWN){
    for(int i = 0; i < (int)UNKNOWN; i++){
      std::cerr << "   " << str_cmd[i] << " " << str_cmd_usage[i] << std::endl;
    }
  }else{
    std::cerr << "   " << str_cmd[id] << " " << str_cmd_usage[id] << std::endl;
  }
}

bool ParseAndProcessCommandArguments(int argc, char ** argv)
{
  Config conf;

  if(!load_config("aws.conf", conf)){
    return false;
  }
  
// argv[1] : command string
// run <filter inst name>
// stop <filter inst name>
// quit
// genfltr <type name> <inst name>
// delfltr <name>
// gench <type name> <inst name>
// delch <inst name> 
// gentbl <type name> <inst name> 
// gettbl <inst name> [<type name>]
// settbl <type name> <inst name> [-f <jsonfile> | -s <jsonstring>]
// settblref <table inst name> <filter inst name> <filter table name>
// deltbl <inst name> 
// <jsonfile>
  std::string server_address = conf.address() + ":" + conf.port();

  CommandHandler handler(grpc::CreateChannel(server_address,
					     grpc::InsecureChannelCredentials()),
			 conf.lib_path());
  
  if(argc < 2){
    dump_usage();
    return false;
  }

  cmd_id id = get_cmd_id(argv[1]);

  if(id == JSON){
    std::cout << "Json file " << argv[1]
	      << " is given. But still the parser is under construction. "
	      << std::endl;
    return true;
  }

  switch(id){
  case RUN:
    if(argc != 3){
      dump_usage(id);
      return false;
    }
    return handler.Run(argv[2]);
  case STOP:
    if(argc != 3){
      dump_usage(id);
      return false;
    }
    return handler.Stop(argv[2]);
  case QUIT:
    if(argc != 2){
      dump_usage(id);
      return false;
    }
    return handler.Quit();
  case GEN_FLTR:
    if(argc != 4){
      dump_usage(id);
      return false;
    }
    return handler.GenFltr(argv[3], argv[2]);
  case DEL_FLTR:
    if(argc != 3){
      dump_usage(id);
      return false;
    }
    return handler.DelFltr(argv[2]);
  case LST_FLTRS:
    if(argc != 2){
      dump_usage(id);
      return false;
    }
    return handler.LstFltrs();
  case SET_FLTR_PAR:
    if(argc < 3 || (argc % 2 == 0)){
      dump_usage(id);
      return false;      
    }else{      
      std::string fltr_name(argv[2]);
      std::vector<std::string> pars, vals;
      for(int iarg = 3; iarg < argc; iarg += 2){
	pars.push_back(std::string(argv[iarg]));
	vals.push_back(std::string(argv[iarg+1]));
      }
      return handler.SetFltrPar(fltr_name, pars, vals);
    }    
  case GET_FLTR_PAR:
    if(argc < 3){
      dump_usage(id);
      return false;
    }else{
      std::string fltr_name(argv[2]);
      std::vector<std::string> pars;
      for(int iarg = 3; iarg < argc; iarg++){
	pars.push_back(std::string(argv[iarg]));
      }
      return handler.GetFltrPar(fltr_name, pars);
    }
  case GEN_CH:
    if(argc != 4){
      dump_usage(id);
      return false;
    }
    return handler.GenCh(argv[3], argv[2]);
  case DEL_CH:
    if(argc != 3){
      dump_usage(id);
      return false;
    }
    return handler.DelCh(argv[2]);
  case LST_CHS:
    if(argc != 2){
      dump_usage(id);
      return false;
    }
    return handler.LstChs();
  case GEN_TBL:
    if(argc != 4){
      dump_usage(id);
      return false;
    }
    return handler.GenTbl(argv[3], argv[2]);
  case GET_TBL:
    if(argc != 3){
      dump_usage(id);
      return false;
    }
    return handler.GetTbl(argv[2]);
  case SET_TBL:
    if(argc != 6){
      dump_usage(id);
      return false;
    }
    return handler.SetTbl(argv[3], argv[2], argv[4], argv[5]);
  case SET_TBL_REF:
    if(argc != 5){
      dump_usage(id);
      return false;
    }
    return handler.SetTblRef(argv[2], argv[3], argv[4]);
  case DEL_TBL:
    if(argc != 3){
      dump_usage(id);
      return false;
    }
    return handler.DelTbl(argv[2]);
  case LST_TBLS:
    if(argc != 2){
      dump_usage(id);
      return false;
    }
    return handler.LstTbls();	       
  default:
    return false;
  }
  
  return true;
}

int main(int argc, char ** argv)
{

  if(!ParseAndProcessCommandArguments(argc, argv))
    return 1;
  
  return 0;
}

