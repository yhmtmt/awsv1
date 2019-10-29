#include <iostream>
#include <cmath>
#include <cstring>
#include <vector>
#include <map>
#include <list>
using namespace std;
#include <dlfcn.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <flatbuffers/flatbuffers.h>

#include "aws_const.hpp"
#include "aws_clock.hpp"
#include "f_sample.hpp"

DEFINE_FILTER(f_sample)

bool f_sample::proc()
{
  if(is_pause()){
    cout << "Filter is puasing." << endl;
  }

  spdlog::info("[{}] f64par {}, s64par {}, u64par {}", get_name(),  m_f64par, m_s64par, m_u64par);
  
  // Typical channels has setter/getter with mutual exclusion. 
  if (increment)
    val++;
  
  return true;  
}
