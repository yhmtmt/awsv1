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
#include <Eigen/Core>
#include <Eigen/LU>


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


// calculate c0*a0 + c1*a1
inline float interpolate_angle_rad(float a0, float a1,
				   float c0 = 0.5, float c1 = 0.5)
{
  double diff = a0 - a1;
  if(diff > PI){
    a1 += PI * 2;
  }else if(diff < -PI){
    a1 -= PI * 2;
  }
  double ws = c0 * a0 + c1 * a1;
  return normalize_angle_rad((float)ws);
}
// Gives rotation matrix corresponding to specified roll, pitch and yaw.
inline Eigen::Matrix3d rotation_matrix(const float roll, const float pitch,
				       const float yaw)
{
  double cr = cos(roll), sr = sin(roll);
  double cp = cos(pitch), sp = sin(pitch);
  double cy = cos(yaw), sy = sin(yaw);
  double spcr = sp * cr;
  double spsr = sp * sr;
  Eigen::Matrix3d R;
  R <<
    cy * cp, cy * spsr - sy * cr, cy * spcr + sy * sr,
    sy * cp, sy * spsr + cy * cr, sy * spcr - cy * sr,
    -sp,     cp * sr,             cp * cr;
  return R;
}

// Gives cross product matrix [ax] in "a x b" where a and b are the 3D vector.
inline Eigen::Matrix3d left_cross_product_matrix(const float x, const float y,
						 const float z)
{
  Eigen::Matrix3d ax;
  ax <<
    0, -z, y,
    z,  0, -x,
    -y, x, 0;
  return ax;
}

#endif
