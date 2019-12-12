#include "f_sample.hpp"

// This line generates factory function. 
DEFINE_FILTER(f_sample)

bool f_sample::proc()
{
  if(is_pause()){
    cout << "Filter is puasing." << endl;
  }

  if(sample != nullptr){  
    spdlog::info("[{}] name:{} num:{} pos:{},{} vel:{},{}", get_name(),  sample->name()->c_str(), sample->num(), sample->pos()->lat(), sample->pos()->lon(), sample->vel()->u(), sample->vel()->v());
  }else{
    spdlog::info("[{}] sample table not found.");
  }
    
  return true;  
}
