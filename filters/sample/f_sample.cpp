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

  msg_builder.Clear();
  auto sample_message = msg_builder.CreateString("Sample filter's message.");
  CreateSampleMsg(msg_builder, sample_message, f64, f32, u32, s32, u16, s16, u8, s8);
  
  return true;  
}
