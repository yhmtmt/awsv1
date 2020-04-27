// Copyright(c) 2016-2020 Yohei Matsumoto, Tokyo University of Marine
// Science and Technology, All right reserved. 

// ch_nmea.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_nmea.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_nmea.h.  If not, see <http://www.gnu.org/licenses/>. 


#ifndef CH_NMEA_HPP
#define CH_NMEA_HPP
#include "channel_base.hpp"

template<size_t buffer_size = 64, size_t data_size = 256, const char * table_definition_file=nullptr>
class ch_binary_data: public ch_base
{
protected:
  size_t m_max_buf;
  unsigned char data_queue[buffer_size][data_size];
  unsigned char data_len[buffer_size];
  
  int m_head, m_tail, m_num;
public:  
  ch_binary_data(const char * name): ch_base(name), 
				  m_head(0), m_tail(0), m_num(0)
  {    
  }
  
  virtual ~ch_binary_data()
  {
  }

  void push(const unsigned char * data, size_t len = 256)
  {
    lock();
    data_len[m_tail] = max(len, buffer_size);
    memcpy(data_queue[m_tail], data, data_len[m_tail]);

    m_tail = (m_tail + 1) % data_size;
    if(m_num == buffer_size){
      m_head = (m_head + 1) % data_size;
    }else{
      m_num++;
    }
    unlock();
  }
  
  void pop(unsigned char * data, size_t & len)
  {
    lock();
    if(m_num){
      len = data_len[m_head];      
      memcpy(data, data_queue[m_head], data_len[m_head]);
      
      m_head = (m_head + 1) % data_size;
      m_num--;
    }else{
      len = 0;
    }
    unlock();
  }

  int get_num_dat(){
    lock();
    int num = m_num;
    unlock();
    return num;
  }
  
  virtual size_t get_dsize()
  {
    
    return buffer_size * data_size * sizeof(unsigned char)
      + sizeof(unsigned char);
  }
  
  virtual size_t write_buf(const char *buf)
  {
    // buffer layout
    // | <number of data> | <data length 1> | <data 1> 
    // | <data length 2> | <data 2> | ... | <data length n> | <data n> |
    unsigned char num = buf[0];
    size_t len = sizeof(num);
    lock();
    for(int i = 0; i < num; i++, m_tail = (m_tail + 1) % buffer_size){
      const unsigned char * p = (const unsigned char*)(buf + len);
      memcpy(data_queue[m_tail], p + 1, *p);
      len += sizeof(*buf) + *buf;
      if(m_num == buffer_size){
	m_head = (m_head + 1) % data_size;
      }else{
	m_num++;
      }      
    }
    unlock();
    return len;
  }
  
  virtual size_t read_buf(char * buf)
  {
    unsigned char & num = *((unsigned char *)buf);
    size_t len = sizeof(unsigned char);
    lock();
    for(;m_head != m_tail; m_head = (m_head + 1) % buffer_size){
      unsigned char * p = (unsigned char*)(buf + len);
      *p = data_len[m_head];
      memcpy(p+1, data_queue[m_head], *p);
      len += sizeof(unsigned char) + *p;
    }
    num = m_num;
    m_tail = m_head = m_num = 0;
    unlock();
    return len;
  }
  
  virtual void print(ostream & out)
  {
  }
  
  virtual int write(FILE * pf, long long tcur)
  {
    return 0;
  }
  
  virtual int read(FILE * pf, long long tcur)
  {
    return 0;
  }
  
  virtual bool log2txt(FILE * pbf, FILE * ptf)
  {
    return 0;
  }  
};

typedef ch_binary_data<64, 256, nullptr> ch_nmea_data;
typedef ch_binary_data<64, 64, nullptr> ch_n2k_data;
typedef ch_binary_data<64, 64, nullptr> ch_ctrl_data;

class ch_nmea: public ch_base
{
protected:
  int m_max_buf;
  int m_head;
  int m_tail;
  
  
  char ** m_buf;
  unsigned int m_new_nmeas;
  
  bool alloc(int size){
    m_buf = new char * [size];
    if(!m_buf)
      return false;
    
    char * p = new char [84 * size];
    if(!p){
      delete[] m_buf;
      m_buf = NULL;
      return false;
    }
    
    for(int i = 0; i < size; i++, p+=84){
      m_buf[i] = p;
    }
    
    m_new_nmeas = 0;
    return true;
  }
  
  void release(){
    if(m_buf){
      if(m_buf[0])
	delete[] m_buf[0];
      delete[] m_buf;
    }
    m_buf = NULL;
  }
  
public:
  ch_nmea(const char * name): ch_base(name), m_max_buf(128), 
			      m_head(0), m_tail(0), m_new_nmeas(0)
  {
    alloc(m_max_buf);
  }
  
  virtual ~ch_nmea()
  {
    release();
  }
  
  bool pop(char * buf)
  {
    lock();
    if(m_head == m_tail){
      unlock();
      return false;
      
    }
    char * p = m_buf[m_head];
    for(;*p != '\0'; p++, buf++){
      *buf = *p;
    }
    
    *buf = *p;
    m_head = (m_head + 1) % m_max_buf;
    
    unlock();
    return true;
  }
  
  bool push(const char * buf)
  {
    lock();
    char * p = m_buf[m_tail];
    int len = 0;
    for( ;*buf != '\0' && len < 84; buf++, p++, len++){
      *p = *buf;
    }
    
    if(len == 84){
      m_buf[m_tail][83] = '\0';
      cerr << "Error in " << m_name << "::push(const char*). No null character in the string given " << endl;
      cerr << "    -> string: " << m_buf[m_tail] << endl;
      unlock();
      return false;
    }
    
    *p = *buf;
    int next_tail = (m_tail + 1) % m_max_buf;
    if(m_head == next_tail){
      m_new_nmeas--;
      m_head = (m_head + 1) % m_max_buf;
    }
    m_tail = next_tail;
    
    m_new_nmeas++;
    unlock();
    return true;
  }
};

#endif
