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
  string msg_str;
  msg_str = "Sample filter's message at ";
  msg_str += get_time_str();
  auto sample_message = msg_builder.CreateString(msg_str);
  auto msg_loc = CreateSampleMsg(msg_builder, sample_message, f64, f32, u32, s32, u16, s16, u8, s8);
  FinishSampleMsgBuffer(msg_builder, msg_loc);
  return true;  
}
