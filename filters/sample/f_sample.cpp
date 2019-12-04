#include "f_sample.hpp"

DEFINE_FILTER(f_sample)

bool f_sample::proc()
{
  if(is_pause()){
    cout << "Filter is puasing." << endl;
  }

  if(sample != nullptr){
    //    sample = tables["sample"].table->get<Sample>();
    
    spdlog::info("[{}] name:{} num:{} pos:{},{} vel:{},{}", get_name(),  sample->name()->c_str(), sample->num(), sample->pos()->lat(), sample->pos()->lon(), sample->vel()->u(), sample->vel()->v());
  }
  
  // Typical channels has setter/getter with mutual exclusion. 
  if (increment)
    val++;
  
  return true;  
}
