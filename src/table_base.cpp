// Copyright(c) 2019 Yohei Matsumoto,  All right reserved. 

// table_base.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// table_base.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with table_base.cpp.  If not, see <http://www.gnu.org/licenses/>.


#include "aws.hpp"

t_base::~t_base()
{
  for(auto itr = refs.begin(); itr != refs.end(); itr++){
    itr->second->del_table(this);
  }
  refs.clear();
}

bool t_base::set_flt_ref(const string & flt_tbl_name, f_base * flt)
{
  unique_lock<mutex> lock(mtx);
  if(!flt->set_table(flt_tbl_name, this))
    return false;
  refs[flt->get_name()] = flt;
}

void t_base::del_flt_ref(f_base * flt)
{
  unique_lock<mutex> lock(mtx);
  flt->del_table(this);
  refs.erase(flt->get_name());    
}
