// Copyright(c) 2012 Yohei Matsumoto, Tokyo University of Marine
// Science and Technology, All right reserved. 

// aws_coord.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_coord.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_coord.cpp.  If not, see <http://www.gnu.org/licenses/>. 
#include <iostream>
#include <cmath>
using namespace std;

#include "aws_coord.hpp"
#include <proj_api.h>

void eceftoblh(const float x, const float y, const float z, float & lat, float & lon, float & alt)
{
	double p = sqrt(x* x + y * y);
	double th = atan(z * AE / (p * BE));
	double s = sin(th);
	double c = cos(th);
	s = s * s * s;
	c = c * c* c;
	lat = (float)atan2(z + EE2_B * s, p - EE2_A * c);
	lon = (float)atan2(y, x);
	s = (float)sin(lat);
	alt = (float)(p / cos(lat) - AE / sqrt(1 - EE2 * s * s));
}

void blhtoecef(const double lat, const double lon, const double alt,
	double & x, double & y, double & z)
{
  double slat = sin(lat);
  double clat = cos(lat);
  double slon = sin(lon);
  double clon = cos(lon);
  double N = AE / sqrt(1 - EE2 * slat * slat);
  
  double tmp = (N + alt) * clat;
  x = (tmp * clon);
  y = (tmp * slon);
  z = ((N * (1 - EE2) + alt) * slat);
}


void eceftoblh(const double x, const double y, const double z, double & lat, double & lon, double & alt)
{
	double p = sqrt(x* x + y * y);
	double th = atan(z * AE / (p * BE));
	double s = sin(th);
	double c = cos(th);
	s = s * s * s;
	c = c * c* c;
	lat = atan2(z + EE2_B * s, p - EE2_A * c);
	lon = atan2(y, x);
	s = sin(lat);
	alt = (p / cos(lat) - AE / sqrt(1 - EE2 * s * s));
}

void wrldtoecef(const double * Rrot, 
		const double xorg, const double yorg, const double zorg, 
		const double xwrld, const double ywrld, const double zwrld,
		double & xecef, double & yecef, double & zecef
		)
{

  xecef = (double)(Rrot[0] * xwrld + Rrot[3] * ywrld + Rrot[6] * zwrld + xorg);
  yecef = (double)(Rrot[1] * xwrld + Rrot[4] * ywrld + Rrot[7] * zwrld + yorg);
  zecef = (double)(Rrot[2] * xwrld + Rrot[5] * ywrld + Rrot[8] * zwrld + zorg);  
}

void eceftowrld(const double * Rrot, 
		const double xorg, const double yorg, const double zorg, 
		const double xecef, const double yecef, const double zecef,
		double & xwrld, double & ywrld, double & zwrld
		)
{
  double x = (double)(xecef - xorg);	
  double y = (double)(yecef - yorg);
  double z = (double)(zecef - zorg);
  
  xwrld = (double)(Rrot[0] * x + Rrot[1] * y + Rrot[2] * z);
  ywrld = (double)(Rrot[3] * x + Rrot[4] * y + Rrot[5] * z);
  zwrld = (double)(Rrot[6] * x + Rrot[7] * y + Rrot[8] * z);
}

void getwrldrot(const double lat, const double lon, double * Rwrld)
{
  double c, s;
  double * tmp0 = Rwrld;
  double tmp1[9], tmp2[9];  
  // pi/2
  c = 0;
  s = 1;
  tmp0[0] = c  ; tmp0[1] = s  ; tmp0[2] = 0.f;
  tmp0[3] = -s ; tmp0[4] = c  ; tmp0[5] = 0.f;
  tmp0[6] = 0.f; tmp0[7] = 0.f; tmp0[8] = 1.f;
  
  // pi/2 - lat
  c = sin(lat);
  s = cos(lat);

  tmp1[0] = c  ; tmp1[1] = 0.f; tmp1[2] = -s ;
  tmp1[3] = 0.f; tmp1[4] = 1.f; tmp1[5] = 0.f;
  tmp1[6] = s  ; tmp1[7] = 0.f; tmp1[8] =  c ;

  tmp2[0] = tmp0[0] * tmp1[0]; tmp2[1] = tmp0[1]; tmp2[2] = tmp0[0] * tmp1[2];
  tmp2[3] = tmp0[3] * tmp1[0]; tmp2[4] = tmp0[4]; tmp2[5] = tmp0[0] * tmp1[2];
  tmp2[6] = tmp1[6]          ; tmp2[7] = 0.f    ; tmp2[8] = tmp1[8];
  
  // lon
  c = cos(lon);
  s = sin(lon);
  tmp1[0] = c  ; tmp1[1] = s  ; tmp1[2] = 0.f;
  tmp1[3] = -s ; tmp1[4] = c  ; tmp1[5] = 0.f;
  tmp1[6] = 0.f; tmp1[7] = 0.f; tmp1[8] = 1.f;
  

  Rwrld[0] = tmp2[0] * tmp1[0] + tmp2[1] * tmp1[3];
  Rwrld[1] = tmp2[0] * tmp1[1] + tmp2[1] * tmp1[4];
  Rwrld[2] = tmp2[2];

  Rwrld[3] = tmp2[3] * tmp1[0] + tmp2[4] * tmp1[3];
  Rwrld[4] = tmp2[3] * tmp1[1] + tmp2[4] * tmp1[4];
  Rwrld[5] = tmp2[5];
  
  Rwrld[6] = tmp2[6] * tmp1[0];
  Rwrld[7] = tmp2[6] * tmp1[1];
  Rwrld[8] = tmp2[8];
}

