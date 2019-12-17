// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_base is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_base is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_base.  If not, see <http://www.gnu.org/licenses/>. 

#include "aws.hpp"

CHMap ch_base::m_chmap;

ch_base * ch_base::create(const char * type_name, const char * chan_name)
{	
  CHMap::iterator itr = m_chmap.find(type_name);
  if(itr == m_chmap.end())
    return NULL;
  
  CreateChannel creator = itr->second;
  
  return (ch_base*) creator(chan_name);
}
