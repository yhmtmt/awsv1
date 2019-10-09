#include <cstdio>
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <map>
#include <queue>
using namespace std;
#include <dlfcn.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "aws_stdlib.hpp"
#include "aws_sock.hpp"
#include "aws_thread.hpp"

#include "aws_clock.hpp"

#include "channel_base.hpp"
#include "filter_base.hpp"
#include "aws_command.hpp"
#include "aws.hpp"

void c_aws::print_title()
{
  if(name_app)
    cout << name_app;
  else
    cout<< "nanashi";
  
  cout << " Ver." << ver_main << "." << ver_sub;
  cout << " (built " << __DATE__ << " " << __TIME__ << ")" << endl;
  cout << "Copyright (c) " << year_copy << " " << name_coder << " All Rights Reserved" << endl;
  if(contact)
    cout << contact << endl;
}
