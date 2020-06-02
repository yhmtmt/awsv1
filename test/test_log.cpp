#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

#include "gtest/gtest.h"
#include "aws_log.hpp"

class LogTest: public ::testing::Test
{
protected:
  const char * dir_name;
  const char * prefix;
  size_t size_max, size_total;
  string path;
  char buf[512];
  long long time_start, time_end;
  int time_step;
  struct data_inf{
    long long t;
    size_t sz;
    unsigned char * data;
    data_inf():t(0), sz(0), data(nullptr)
    {
    }
    
    data_inf(long long _t, size_t _sz, unsigned char * _data):
      t(_t), sz(_sz), data(_data)
    {
    }    
  };
  
  vector<data_inf> data_list;
  
  LogTest():dir_name("logtest"), prefix("logtest"), size_max(1024),
	    size_total(size_max * 3),
	    time_start(10000), time_end(10000), time_step(100)
  {
  }
  
  c_log olog, ilog;
  virtual void SetUp(){

    // initialize environment
    if(!fs::exists(dir_name)){
      fs::create_directory(dir_name);
      fs::permissions(dir_name, fs::perms::owner_all);
    }

    // we set path as "$current_directory/$dir_name"
    getcwd(buf, sizeof(buf));
    path = buf;
    path += "/";
    path += dir_name;

    // initialize data_list
    srand(0); // we need results reproducible
    int size = 0;
    long long t = time_start;
    while(1){
      size_t sz = rand() % 256;
      size_t sz_record = sizeof(long long) + sizeof(size_t) + sz;
      if(size + sz_record >= size_total)
	break;
      
      data_inf di(t, sz, new unsigned char[sz]);
      for (int i = 0; i < sz; ++i){
	di.data[i] = rand() % 256;
      }
      data_list.push_back(di);
      size += sz_record;
      t += time_step;
    }
    time_end = t;    
    rand();
    
  }
};

TEST_F(LogTest, WriteRead)
{
  olog.init(path, prefix, false, size_max);
  for (int i = 0; i < data_list.size(); i++){
    bool r = olog.write(data_list[i].t, data_list[i].data,  data_list[i].sz);
    ASSERT_TRUE(r);
  }
  olog.destroy();
  
  ilog.init(path, prefix, true, size_max);
  long long t = 9800;
  long long tstep = 25;
  while(t < time_end){
    long long tread = t;
    unsigned int szread;

    if(t < time_start){
      ASSERT_FALSE(ilog.read(tread, (unsigned char*)buf, szread));
      t += tstep;
      continue;
    }else{
      ASSERT_TRUE(ilog.read(tread, (unsigned char*)buf, szread));            
    }

    
    if(t % time_step != 0){
      ASSERT_TRUE(szread == 0);
      t += tstep;
      continue;
    }

    // find corresponding time record
    int irec = 0;
    for(; irec < data_list.size(); irec++){
      if(data_list[irec].t == tread)
	break;
    }
    
    // size check
    ASSERT_TRUE(szread == data_list[irec].sz);
    
    // data check
    bool cmp = true;    
    for(int ibyte = 0; ibyte < szread; ++ibyte){
      if(data_list[irec].data[ibyte] != ((unsigned char*)buf)[ibyte]){
	cmp = false;
	break;
      }
    }
    ASSERT_TRUE(cmp);
    
    t += tstep;
  }
  ilog.destroy();
  
}

