#ifndef T_BASE_H
#define T_BASE_H
// Copyright(c) 2019 Yohei Matsumoto,  All right reserved. 

// table_base.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// table_base.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with table_base.hpp.  If not, see <http://www.gnu.org/licenses/>.

class f_base;

class t_base
{
protected:
  string name;
  string type;
  size_t size;
  shared_ptr<const char> data;
  mutex mtx;

  map<string, f_base*> refs;
public:
  t_base(const string & type_, const string & name_):type(type_), name(name_),
						     size(0), data(nullptr)
  {
  }
  
  virtual ~t_base();
  
  void lock(){
    mtx.lock();
  }

  void unlock(){
    mtx.unlock();
  }

  void set(const string & data_)
  {
    unique_lock<mutex> lock(mtx);
    size = data_.size();
    char * p = (char*)malloc(size);
    memcpy((void*)p, data_.data(), size);    
    data = shared_ptr<const char>(p, free);
  }
 
  bool set_flt_ref(const string & flt_tbl_name, f_base * flt);

  void del_flt_ref(f_base * flt);
  
  const string & get_name()
  {
    return name;
  }

  const string & get_type()
  {
    return type;
  }
 
  bool is_type(const string & type_)
  {    
    return type_ == type;
  }

  bool is_same(shared_ptr<const char> & data_)
  {
    return data_ == data;
  }
  
  shared_ptr<const char> get_data()
  {
    unique_lock<mutex> lock(mtx);
    return data;
  }

  const size_t get_data_size()
  {
    unique_lock<mutex> lock(mtx);
    return size;
  }

  template<class T> const T * get()
  {
    return flatbuffers::GetRoot<T>((const void*)data.get());
  }
};


#endif
