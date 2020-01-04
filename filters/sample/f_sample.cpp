#include "f_sample.hpp"

// This line generates factory function. 
DEFINE_FILTER(f_sample)

const char * f_sample::str_etype[Bar+1] =
{
  "Foo", "Bar"
};


bool f_sample::proc()
{
  if(is_pause()){
    cout << "Filter is puasing." << endl;
  }

  if(tbl == nullptr){
    spdlog::info("[{}] tbl not found.", get_name());
  }
  
  return true;  
}
