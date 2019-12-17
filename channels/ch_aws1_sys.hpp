#ifndef CH_AWS1_SYS_HPP
#define CH_AWS1_SYS_HPP
// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_aws1_ctrl.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_aws1_ctrl.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_aws1_ctrl.hpp.  If not, see <http://www.gnu.org/licenses/>.

#include "channel_base.hpp"


class ch_aws1_sys: public ch_base
{
protected:
public:
  ch_aws1_sys(const char * name): ch_base(name)
  {
  }
  
  virtual ~ch_aws1_sys()
  {
  }
};

#endif
