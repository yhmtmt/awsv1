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

/////////////////////////////////////////////////////////// sample implementation of filter class
using namespace Filter::Sample;

class f_sample: public f_base // 1) inherit from f_base
{
private:
  // 2) Define pointers of tables, channels and parameters for inter-filter communication 
  const Sample * tbl; // sample table
  ch_sample * ch; // sample channel 
public:
  // 3) constructor should have an instance name as a c-string. In the body, parameters and tables are to be registered for inter-filter communication.
  f_sample(const char * fname): f_base(fname), tbl(nullptr), ch(nullptr)
  {
    // 3-1) register table defined in (2). "sample" means the table name refered in the filter. The table content would be automatically updated. 
    register_table("tbl", (const void**)&tbl);
    // 3-2) register parameters
    register_fpar("ch", (ch_base**)&ch, typeid(ch_sample).name(), "Channel sample.");    
  }
  
  virtual bool init_run(){    
    // 4) override this function if you need to initialized filter class just before invoking fthread.
    //		open file or communication channels, set up and check channels and their aliases.
    return true;
  }

  // override this function if you need to do something in stopping filter thread
  virtual void destroy_run(){
  }

  // 5) implement your filter body. this function is called in the loop of fthread.  
  virtual bool proc();
    
};


#endif
