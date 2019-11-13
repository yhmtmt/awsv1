
inline bool load_config(const std::string & config_file, Config & conf)
{
  std::ifstream file_in(config_file);
  if(!file_in.is_open())
    return false;

  std::stringstream strstream;
  strstream << file_in.rdbuf();

  std::string json_string(strstream.str());

  if(!google::protobuf::util::JsonStringToMessage(json_string, &conf).ok())
    return false;

  return true;
}

inline bool save_config(const std::string & config_file, Config & conf)
{
  std::string json_string;
  google::protobuf::util::JsonPrintOptions options;
  options.add_whitespace = true;
  options.always_print_primitive_fields = true;
  options.preserve_proto_field_names = true;
  if(!google::protobuf::util::MessageToJsonString(conf, &json_string, options).ok())
    return false;

  std::ofstream file_out(config_file);
  if(!file_out.is_open())
    return false;

  file_out << json_string;
  return true;
}



inline bool save_tbl_as_json(std::ostream & out,
			     const std::string & schema_file_name,
			     uint8_t * buffer)
{
  std::string schema_file;
  bool ok = flatbuffers::LoadFile(schema_file_name.c_str(), true, &schema_file);
  if(!ok){
    std::cerr << "Failed to load schema file sample.bfbs" << std::endl;
    return false;
  }
  
  //  auto &schema = *reflection::GetSchema(schema_file.c_str());
  std::string json_file;
  flatbuffers::Parser parser;
  parser.Deserialize((uint8_t*)schema_file.c_str(), schema_file.length());
  GenerateText(parser, buffer, & json_file);
  out.write(json_file.c_str(), json_file.length());

  return true;
}

inline bool load_tbl_json_parser(const std::string & schema_file_name,
				 flatbuffers::Parser & parser)
{
    std::string schema_file;
    bool ok = flatbuffers::LoadFile(schema_file_name.c_str(), true, &schema_file);
    if(!ok){
      std::cerr << "Error: Failed to load schema file "
		<< schema_file_name << std::endl;
      return false;
    }

    std::string json_file;
    ok = parser.Deserialize((const uint8_t*)schema_file.c_str(),
		       schema_file.length());
    if(!ok){
      std::cerr << "Error: Failed to deserialize schema file "
		<< schema_file_name << std::endl;
      return false;
    }
    return true;
}

inline bool load_tbl_from_json_string(const std::string & json_file,
				      const std::string & schema_file_name,
				      flatbuffers::FlatBufferBuilder & builder)
{
  std::string schema_file;
  bool ok = flatbuffers::LoadFile(schema_file_name.c_str(), true, &schema_file);
  if(!ok){
    std::cerr << "Failed to load schema file " << schema_file_name << std::endl;
    return false;
  }

  flatbuffers::Parser parser;
  ok = parser.Deserialize((uint8_t*)schema_file.c_str(), schema_file.length())
    && parser.Parse(json_file.c_str());
  if(!ok){
    std::cerr << "Failed to parse : " << parser.error_ << std::endl;
    return false;
  }

  builder.Swap(parser.builder_);
  return true;
}

inline bool load_tbl_from_json_file(const std::string & json_file_name,
				    const std::string & schema_file_name,
				    flatbuffers::FlatBufferBuilder & builder)
{
  std::string json_file;
  bool ok = flatbuffers::LoadFile(json_file_name.c_str(), true, &json_file);
  if(!ok){
    std::cerr << "Failed to load json file " << json_file_name << std::endl;
    return false;
  }

  return load_tbl_from_json_string(json_file, schema_file_name, builder);
}

