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


using CommandService::Command;
using CommandService::TblRef;
using CommandService::TblInfo;
using CommandService::TblData;
using CommandService::Result;

enum cmd_id{
  GEN_TBL = 0, GET_TBL, SET_TBL, SET_TBL_REF, JSON, UNKNOWN
};

const char * str_cmd[UNKNOWN] = {
  "gentbl", "gettbl", "settbl", "settblref", ".json"
};

const cmd_id get_cmd_id(const char * str)
{
  for(int id = 0; id < (int)JSON; id++){
    int c = 0;
    for(; str_cmd[id][c] && str[c];c++);
    
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

  bool GenTbl(const std::string & name, const std::string & type)
  {
    TblInfo info;
    info.set_type_name(type);
    info.set_inst_name(name);

    Result res;
    ClientContext context;
    Status status = stub_->GenTbl(&context, info, &res);
    if(!status.ok()){
      std::cout << "Error:" << res.message() << std::endl;
      return false;
    }
    return true;
  }

  bool GetTbl(const std::string & name, const std::string & type)
  {
    TblInfo info;
    info.set_type_name(type);
    info.set_inst_name(name);

    TblData data;
    ClientContext context;
    Status status = stub_->GetTbl(&context, info, &data);
    if(!status.ok()){
      std::cout << "Error in GetTbl()." << std::endl;
      return false;
    }
    
    std::string schema_file_name;
    std::string schema_file;
    schema_file_name = lib_path_ + "/" +  data.type_name() + ".bfbs";
    bool ok flatbuffers::LoadFile(schema_file_name.c_str(), true, schema_file);
    if(!ok){
      std::cerr << "Error: Failed to load schema file "
		<< schema_file_name << std::endl;
      return false;
    }

    std::string json_file;
    flatbuffers::Parser parser;
    ok = parser.Deserialize((const uint8_t*)schema_file.c_str(),
		       schema_file.length());
    if(!ok){
      std::cerr << "Error: Failed to deserialize schema file "
		<< schema_file_name << std::endl;
      return false;
    }
    
    ok = GenerateText(parser, (const uint8_t*)data.tbl().c_str(), &json_file);
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
    return true;
  }

  bool SetTblRef(const std::string & tbl_name, const std::string & flt_name,
		 const std::string & flt_tbl_name)
  {
    return true;
  }
  
};


void dump_usage()
{
  std::cerr << "caws [<command>|<command json file>]" << std::endl;
  std::cerr << "   " << str_cmd[GEN_TBL] << " <name> <type>" << std::endl;
  std::cerr << "   " << str_cmd[GET_TBL] << " <name> [<type>]" << std::endl;
  std::cerr << "   " << str_cmd[SET_TBL] << " <name> <type> [-f <json file> | -s <json string>]" << std::endl;
  std::cerr << "   " << str_cmd[SET_TBL_REF] << " <table name> <filter name> <filter table name>" << std::endl;
}

bool ParseAndProcessCommandArguments(int argc, char ** argv)
{
// argv[1] : command string
// gentbl <name> <type> 
// gettbl <name> [<type>]
// settbl <name> <type> [-f <jsonfile> | -s <jsonstring>]
// settblref <table_name> <filter_name> <filter_table_name>
// <jsonfile>

  if(argc < 2){
    dump_usage();
  }

  cmd_id id = get_cmd_id(argv[1]);

  if(id == JSON){
    std::cout << "Json file " << argv[1] << " is given. But still the parser is under construction. " << std::endl;
    return true;
  }

  switch(id){
  case GEN_TBL:
    if(argc != 4){
      dump_usage();
      return false;
    }
    break;		    
  case GET_TBL:
    if(argc == 3 || argc == 4){
      dump_usage();
      return false;
    }
    break;
  case SET_TBL:
    if(argc == 6){
      dump_usage();
      return false;
    }
    break;
  case SET_TBL_REF:
    if(argc == 5){
      dump_usage();
      return false;
    }
    break;
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

