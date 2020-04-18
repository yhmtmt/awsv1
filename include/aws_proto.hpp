// Copyright(c) 2020 Yohei Matsumoto, All right reserved. 

// aws_proto.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_proto.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_proto.hpp.  If not, see <http://www.gnu.org/licenses/>. 

#include <google/protobuf/util/json_util.h>

template <class T> inline bool save_proto_object(const string & filepath, T & object)
{
  ofstream file;
  file.open(filepath, ios_base::binary);
  if(!file.is_open()){
    return false;
  }
  
  if(filepath.substr(filepath.find_last_of(".") + 1) == "json"){
    string json_string;
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    if(!google::protobuf::util::MessageToJsonString(object, &json_string, options).ok()){
      return false;
    }
    file << json_string;
  }else{
    if(!object.SerializeToOstream(&file)){
      return false;
    }
  }
  return true;
}


template <class T> inline bool load_proto_object(const string filepath, T & object)
{
  ifstream file;
  file.open(filepath, ios_base::binary);
  if(!file.is_open()){
    return false;
  }
    
  if(filepath.substr(filepath.find_last_of(".") + 1) == "json"){
    // for json file.
    stringstream strstream;
    strstream << file.rdbuf();
    string json_string(strstream.str());
    google::protobuf::util::Status st =
      google::protobuf::util::JsonStringToMessage(json_string, &object);
    if(!st.ok()){
      return false;
    }
  }else{ // for binary file
    if(!object.ParseFromIstream(&file)){
      return false;
    }
  }
  return true;
}
