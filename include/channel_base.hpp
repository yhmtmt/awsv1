#ifndef _CH_BASE_H_
#define _CH_BASE_H_
// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_base.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_base.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_base.h.  If not, see <http://www.gnu.org/licenses/>. 

#include <typeinfo>
#include <iostream>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <cfloat>
#include <vector>
#include <map>
#include <list>
#include <queue>
using namespace std;

#include "aws_const.hpp"
#include "aws_coord.hpp"
#include "aws_clock.hpp"
#include "aws_stdlib.hpp"
#include "aws_sock.hpp"
#include "aws_thread.hpp"

class f_base;
class ch_base;

typedef ch_base * (*CreateChannel)(const char * name);
template <class T> ch_base * createChannel(const char * name)
{
  return reinterpret_cast<ch_base*>(new T(name));
}

typedef map<const char *, CreateChannel, cmp> CHMap;

class ch_base
{
protected:
  static CHMap m_chmap;

  // registering filter classes to the factory
  template <class T> static void register_factory(const char * tname)
  {
    m_chmap.insert(CHMap::value_type(tname, createChannel<T>));
  }
  
public:
  // factory function
  static ch_base * create(const char * tname, const char * fname);
  
  // initialize mutex and signal objects. this is called by the c_aws constructor
  static void init();
  
  // uninitialize mutex and signal objects. called by the c_aws destoractor
  static void uninit();

  static void register_factory();
  
protected:
  char * m_name;
  mutex m_mtx;
public:
  ch_base(const char * name):m_name(NULL){
    m_name = new char[strlen(name) + 1];
    strcpy(m_name, name);
  };

  virtual ~ch_base(){
    delete [] m_name;
  }
  
  void lock()
  {
    m_mtx.lock();
  }
  
  void unlock()
  {
    m_mtx.unlock();
  }
	
  const char * get_name(){ return m_name;};
   
  virtual size_t get_dsize()
  { 
    return 0;
  };
  
  virtual size_t write_buf(const char * buf)
  {
    return 0;
  };
  
  virtual size_t read_buf(char * buf)
  {
    return 0;
  }
	
  virtual void print(ostream & out)
  {
  }

  // file writer method
  virtual int write(FILE * pf, long long tcur)
  {
    return 0;
  }

  // file reader method
  virtual int read(FILE * pf, long long tcur)
  {
    return 0;
  }

  virtual bool log2txt(FILE * pbf, FILE * ptf)
  {
    return false;
  }
};

#endif
