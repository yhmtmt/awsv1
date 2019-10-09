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

#include "aws_const.hpp"
#include "aws_clock.hpp"
#include "f_sample.hpp"

extern "C" f_base * factory(const std::string & name)
{
  return new f_sample(name.c_str());
}
