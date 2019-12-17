#ifndef CH_SCALAR_HPP
#define CH_SCALAR_HPP
// Copyright(c) 2012 Yohei Matsumoto, All right reserved. 

// ch_scalar.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_scalar.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_scalar.h.  If not, see <http://www.gnu.org/licenses/>. 
#include "channel_base.hpp"

class ch_sample: public ch_base
{
 protected:
  long long t, tf;
  int val, valf;
  char * buf;
public:
  ch_sample(const char * name):ch_base(name),val(0), buf(NULL)
  {
    buf = new char[get_dsize()];
  }
  
  virtual ~ch_sample(){
    delete[] buf;
  }
  
  void set(const long long _t, const int _val)
  {
    lock();
    t = _t;
    val = _val;
    unlock();
  }
  
  void get(long long & _t, int & _val)
  {
    lock();
    _t = t;
    _val = val;
    unlock();
  }
  
  virtual size_t get_dsize(){
    return sizeof(t) + sizeof(val);
  }

  virtual size_t write_buf(const char * buf){
    t = *((long long*) buf);
    val = *((int*) (((long long*) buf) + 1));
    return sizeof(t) + sizeof(val);
  }

   virtual size_t write_buf_f(const char * buf){
    tf = *((long long*) buf);
    valf = *((int*) (((long long*) buf) + 1));
    return sizeof(t) + sizeof(val);
  }
 
  virtual size_t read_buf(char * buf){
    *((long long*)buf) = t;
    *((int*)(((long long*)buf) + 1)) = val;
    return sizeof(t) + sizeof(val);
  }

  virtual void print(ostream & out){
    out << "channel " << m_name << " " << val << endl;
  }

  virtual int write(FILE * pf, long long tcur)
  {
    if (!pf)
      return 0;

    lock();
    read_buf(buf);
    unlock();
    
    fwrite((void*)buf, get_dsize(), 1, pf);
    
    return get_dsize();
  }
  
  virtual int read(FILE * pf, long long tcur)
  {
    if(!pf)
      return 0;

    if(tf <= tcur)
      set(tf, valf);    
    else
      return 0;
    int res = fread((void*)buf, get_dsize(), 1, pf);

    if(!res)
      return 0;

    lock();
    write_buf_f(buf);
    unlock();
    return res;
  }

  virtual bool log2txt(FILE * pbf, FILE * ptf)
  {
    if(!pbf || !ptf)
      {
	cerr << "In ch_sample::log2txt(), File is not opened." << endl;
      }

    fprintf(ptf, "T, val\n"); 
    while(!feof(pbf)){
      int res = fread((void*)buf, get_dsize(), 1, pbf);
      if(!res){
	break;
      }

      write_buf(buf);
      fprintf(ptf, "%lld, %d\n", t, val);
    }
    return true;
  }
  
};
#endif
