// Copyright(c) 2012-2020 Yohei Matsumoto, All right reserved. 

// aws_math.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_math.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_math.hpp.  If not, see <http://www.gnu.org/licenses/>. 

#ifndef AWS_MATH_HPP
#define AWS_MATH_HPP
#include "aws_const.hpp"

inline float normalize_angle_deg(float angle){
  if(angle > 180.0f)
    return angle - 360.0f;
  else if (angle < -180.0f)
    return angle + 360.0f;
  return angle;
}

inline double normalize_angle_deg(double angle){
  if(angle > 180.0)
    return angle - 360.0;
  else if(angle < -180.0)
    return angle + 360.0;
  return angle;
}

inline float normalize_angle_rad(float angle)
{
  if(angle > PI)
    return (float)(angle - PI * 2);
  else if(angle < -PI)
    return (float)(angle + PI * 2);
  return angle;
}

inline double normalize_angle_rad(double angle)
{
  if(angle > PI)
    return angle - PI * 2;
  else if(angle < -PI)
    return angle + PI * 2;
  return angle;
}


#endif
