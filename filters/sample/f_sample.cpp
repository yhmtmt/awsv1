#include "f_sample.hpp"

// This line generates factory function. 
DEFINE_FILTER(f_sample)

bool f_sample::proc()
{
  if(is_pause()){
    cout << "Filter is puasing." << endl;
  }

  if(tbl != nullptr){  
    spdlog::info("[{}] name:{} num:{} pos:{},{} vel:{},{}", get_name(),  tbl->name()->c_str(), tbl->num(), tbl->pos()->lat(), tbl->pos()->lon(), tbl->vel()->u(), tbl->vel()->v());
  }else{
    spdlog::info("[{}] tbl not found.", get_name());
  }
    
  return true;  
}
