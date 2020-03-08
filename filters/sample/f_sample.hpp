// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// f_sample.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Publica License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// f_sample.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with f_sample.cpp.  If not, see <http://www.gnu.org/licenses/>. 

#ifndef F_SAMPLE_H
#define F_SAMPLE_H
#include "filter_base.hpp"
#include "ch_sample.hpp"
#include "sample_generated.h"
#include "sample_msg_generated.h"

/////////////////////////////////////////////////////////// sample implementation of filter class
using namespace Filter::Sample;
using namespace Filter::SampleMsg;

class f_sample: public f_base // 1) inherit from f_base
{
private:
  // 2) Define pointers of tables, channels and parameters for inter-filter communication 
  const Sample * tbl; // sample table (see fbs/sample.fbs in the project root)
  ch_sample * ch;     // sample channel (see channels/ch_sample.hpp/cpp in the project root)
  double f64;
  unsigned long long u64;
  long long s64;
  float f32;
  unsigned int u32;
  int s32;
  short s16;
  unsigned short u16;
  char s8;
  unsigned char u8;
  bool b;
  char cstr[128];
  bool init_force_fail;
  enum etype{
    Foo = 0, Bar
  };
  static const char * str_etype[Bar+1];
  etype e;

  flatbuffers::FlatBufferBuilder msg_builder;
  char * msg;
public:
  // 3) constructor should have an instance name as a c-string. In the body, parameters and tables are to be registered for inter-filter communication.
  f_sample(const char * fname): f_base(fname), tbl(nullptr), ch(nullptr), f64(0.0), u64(0), s64(0), f32(0.0f), u32(0), s32(0), s16(0), u16(0),
				s8(0), u8(0), b(false), e(Foo), init_force_fail(false), msg_builder(1024), msg(NULL)
  {
    cstr[0] = '\0';
    
    // 3-1) register table defined in (2). "tbl" means the table name refered in the filter. The table content would be automatically updated. 
    register_table("tbl", (const void**)&tbl);
    // 3-2) register parameters
    register_fpar("ch", (ch_base**)&ch,
		  typeid(ch_sample).name(), "Channel sample."); // channel registration example
    register_fpar("f64", &f64, "Double parameter example.");
    register_fpar("u64", &u64, "Unsigned long long parameter example.");
    register_fpar("s64", &s64, "Long long parameter example.");
    register_fpar("f32", &f32, "Float parameter example.");
    register_fpar("u32", &u32, "Unsigned int parameter example.");
    register_fpar("s32", &s32, "Int parameter example.");
    register_fpar("s16", &s16, "Short parameter example.");
    register_fpar("u16", &u16, "Unsigned short parameter example.");
    register_fpar("s8", &s8, "Char parameter example.");
    register_fpar("u8", &u8, "Unsigned char paramete example.");
    register_fpar("b", &b, "Bool parameter example.");
    register_fpar("str", cstr, sizeof(cstr), "String parameter example.");
    register_fpar("e", (int*)&e, (int)(Bar + 1), str_etype, "Enum type example.");
    register_fpar("init_force_fail", &init_force_fail, "Force init_run failed to test fail-case at run command.");
  }
  
  virtual bool init_run(){    
    // 4) override this function if you need to initialized filter class just before invoking fthread.
    //		open file or communication channels, set up and check channels and their aliases.

    if(init_force_fail)
      return false;
    
    return true;
  }

  // override this function if you need to do something in stopping filter thread
  virtual void destroy_run(){
  }

  // 5) implement your filter body. this function is called in the loop of fthread.  
  virtual bool proc();

  // 6) message dispatcher
  virtual const char * get_msg_type_name()
  {
    return "sample_msg";
  }

  virtual const char * get_msg()
  {
    return (char*) msg_builder.GetBufferPointer();
  }

  virtual const size_t get_msg_size()
  {
    return (size_t) msg_builder.GetSize();
  }
};


#endif
