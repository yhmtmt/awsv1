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
#include "sample_generated.h"

/////////////////////////////////////////////////////////// sample implementation of filter class
using namespace Filter::Sample;

class f_sample: public f_base // 1) inherit from f_base
{
private:
  // 2) Define pointers of some flatbuffers tables
  const Sample * sample; // sample table
  
public:
  // 3) constructor should have an object name as a c-string. Then the object name should be passed to the f_base constructor.
  f_sample(const char * fname): f_base(fname), sample(nullptr)
  {
    // 3-1) register table defined in (2). "sample" means the table name refered in the filter. The table content would be automatically updated. 
    register_table("sample", (const void**)&sample);
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
