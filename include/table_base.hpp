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
  string data;
  mutex mtx;

  map<string, f_base*> refs;
public:
  t_base(const string & type_, const string & name_):type(type_), name(name_)
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
    data = move(data_);
  }
  
  void set(string & data_)
  {
    unique_lock<mutex> lock(mtx);
    data = move(data_);
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
  
  template <class T> bool is_type()
  {
    return type == typeid(T).name();
  }

  bool is_type(const string & type_)
  {
    return type_ == type;
  }

  const string & get_data()
  {
    return data;
  }
  
  template <class T> const T * get(){
    if(is_type<T>())
      return nullptr;
    
    return flatbuffers::GetRoot<T>((const void*)data.c_str());
  }  
};

#endif
