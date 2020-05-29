// Copyright(c) 2020 Yohei Matsumoto, All right reserved. 

// ch_binary_data_queue.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_binary_data_queue.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_binary_data_queue.hpp.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CH_BINAR_DATA_QUEUE_HPP
#define CH_BINAR_DATA_QUEUE_HPP

#include "channel_base.hpp"

template<unsigned short buffer_size = 64,
	 unsigned short data_size = 256>
class ch_binary_data_queue: public ch_base
{
protected:
  unsigned char data_queue[buffer_size][data_size];
  unsigned short data_len[buffer_size];
  
  int m_head, m_tail, m_num;
public:  
  ch_binary_data_queue(const char * name): ch_base(name), 
				  m_head(0), m_tail(0), m_num(0)
  {    
  }
  
  virtual ~ch_binary_data_queue()
  {
  }

  void push(const unsigned char * data, unsigned int len = data_size)
  {
    lock();
    if(len > data_size)
      cerr << "in channel " << m_name << ".push(), data size " << len << " passed exceeded maximum data size " << data_size << endl;
    
    data_len[m_tail] = min((unsigned short)len, data_size);
    memcpy(data_queue[m_tail], data, data_len[m_tail]);

    m_tail = (m_tail + 1) % buffer_size;
    if(m_num == buffer_size){
      m_head = (m_head + 1) % buffer_size;
    }else{
      m_num++;
    }
    unlock();
  }
  
  void pop(unsigned char * data, unsigned int & len)
  {
    lock();
    if(m_num){
      len = data_len[m_head];
      if(len > data_size){
	 cerr << "in channel " << m_name << ".pop(), data size " << len << " passed exceeded maximum data size " << data_size << endl;
      }
      memcpy(data, data_queue[m_head], data_len[m_head]);
      
      m_head = (m_head + 1) % buffer_size;
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
      + sizeof(unsigned short) * (buffer_size + 1);
  }
  
  virtual size_t write_buf(const char *buf)
  {
    // buffer layout
    // | <number of data> | <data length 1> | <data 1> 
    // | <data length 2> | <data 2> | ... | <data length n> | <data n> |
    unsigned short num = *((unsigned short *)buf);
    unsigned int len = sizeof(num);
    lock();
    for(int i = 0; i < num; i++, m_tail = (m_tail + 1) % buffer_size){
      const unsigned short * p = (const unsigned short*)(buf + len);
      memcpy(data_queue[m_tail], (const unsigned char*)(p + 1), *p);
      data_len[m_tail] = *p;
      len += sizeof(*p) + *p;
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
    unsigned short & num = *((unsigned short *)buf);
    unsigned int len = sizeof(unsigned short);
    lock();
    for(num = 0;m_num != num; ++num, m_head = (m_head + 1) % buffer_size){
      unsigned short * p = (unsigned short*)(buf + len);
      *p = data_len[m_head];
      memcpy((unsigned char*)(p + 1), data_queue[m_head], *p);
      len += sizeof(unsigned short) + *p;
    }
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

#endif
