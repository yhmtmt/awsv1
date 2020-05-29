#ifndef CH_AWS1_CTRL_HPP
#define CH_AWS1_CTRL_HPP
// Copyright(c) 2019-2020 Yohei Matsumoto, All right reserved. 

// ch_aws1_ctrl.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_aws1_ctrl.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_aws1_ctrl.hpp.  If not, see <http://www.gnu.org/licenses/>.

#include "ch_binary_data_queue.hpp"

#include "control_generated.h"

typedef ch_binary_data_queue<64, 128> ch_ctrl_data;

// map a value with 3 threasholds (for rudder contrl and states)
inline  int map_oval(int val,
		     int vmax, int vnut, int vmin,
		     int omax, int onut, int omin
		     )
{
  int dvmax = val - vmax;
  int dvnut = val - vnut;
  int dvmin = val - vmin;
  int dvmax_vnut = vmax - vnut;
  int dvnut_vmin = vnut - vmin;
  
  if (abs(dvmax) <= abs(dvmax_vnut) && abs(dvnut) < abs(dvmax_vnut))
    return (int)((double)((omax - onut) * (dvnut)) / (double)(dvmax_vnut)) + onut;
  else if (abs(dvnut) <= abs(dvnut_vmin) && abs(dvmin) < abs(dvnut_vmin))
    return (int)((double)((onut - omin) * (dvnut)) / (double)(dvnut_vmin)) + onut;
  else if (abs(dvmax) < abs(dvmin))
    return omax;
  else
    return omin;
}

// map a value with 5 threasholds (for engines)
inline  int map_oval(int val,
		     int vmax, int vfnut, int vnut, int vbnut, int vmin,
		     int omax, int ofnut, int onut, int obnut, int omin
		     )
{
  int dvmax = val - vmax;
  int dvfnut = val - vfnut;
  int dvnut = val - vnut;
  int dvbnut = val - vbnut;
  int dvmin = val - vmin;
  int dvmax_vfnut = vmax - vfnut;
  int dvfnut_vnut = vfnut - vnut;
  int dvnut_vbnut = vnut - vbnut;
  int dvbnut_vmin = vbnut - vmin;
  
  if (abs(dvmax) <= abs(dvmax_vfnut) && abs(dvfnut) < abs(dvmax_vfnut))
    return (int)((double)((omax - ofnut) * (dvfnut)) / (double)(dvmax_vfnut)) + ofnut;
  else if (abs(dvfnut) <= abs(dvfnut_vnut) && abs(dvnut) < abs(dvfnut_vnut))
    return (int)((double)((ofnut - onut) * (dvnut)) / (double)(dvfnut_vnut)) + onut;
  else if (abs(dvnut) <= abs(dvnut_vbnut) && abs(dvbnut) < abs(dvnut_vbnut))
    return (int)((double)((onut - obnut) * (dvbnut)) / (double)(dvnut_vbnut)) + obnut;
  else if (abs(dvbnut) <= abs(dvbnut_vmin) && abs(dvmin) < abs(dvbnut_vmin))
    return (int)((double)((obnut - omin) * dvmin) / (double)(dvbnut_vmin)) + omin;
  else if (abs(dvmax) < abs(dvmin))
    return omax;
  else
    return omin;
}

#endif
