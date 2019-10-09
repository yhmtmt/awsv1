// Copyright(c) 2012 Yohei Matsumoto, Tokyo University of Marine
// Science and Technology, All right reserved. 

// aws_coord.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_coord.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_coord.h.  If not, see <http://www.gnu.org/licenses/>. 

#ifndef AWS_COORD_H
#define AWS_COORD_H

#include "aws_const.hpp"

void bihtoecef(const double lat, const double lon, const double alt,
	double & x, double & y, double & z);

void eceftobih(const double x, const double y, const double z, double & lat, double & lon, double & alt);

void wrldtoecef(const double * Rrot, 
		const double xorg, const double yorg, const double zorg, 
		const double xwrld, const double ywrld, const double zwrld,
		double & xecef, double & yecef, double & zecef
		);
void eceftowrld(const double * Rrot, 
		const double xorg, const double yorg, const double zorg, 
		const double xecef, const double yecef, const double zecef,
		double & xwrld, double & ywrld, double & zwrld
		);

void getwrldrot(const double lat, const double lon, const double * Rwrld);

#endif
