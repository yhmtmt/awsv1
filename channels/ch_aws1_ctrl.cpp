// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_aws1_ctrl.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_aws1_ctrl.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_aws1_ctrl.cpp.  If not, see <http://www.gnu.org/licenses/>.

#include "ch_aws1_ctrl.hpp"

const char * str_aws1_ctrl_src[ACS_NONE] = 
{
  "ui","rmt", "ap1", "ap2", "fset"
};


const char * str_aws1_ap_mode[EAP_NONE] =
{
  "cursor", "wp", "wpav", "stay"
};
