// Copyright(c) 2017-2020 Yohei Matsumoto, All right reserved. 

// aws_map_coast_line.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_map_coast_line.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_map_coast_line.cpp.  If not, see <http://www.gnu.org/licenses/>. 


#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include <climits>
#include <cfloat>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <mutex>
using namespace std;

#include <string.h>
#include <assert.h>

#include "aws_coord.hpp"
#include "aws_stdlib.hpp"
#include "aws_map.hpp"

namespace AWSMap2{
  /////////////////////////////////////////////////////////////////// Points
  Points::Points()
  {
  }

  Points::~Points()
  {
  }

  bool Points::save(ofstream & ofile)
  {
  }  
}
