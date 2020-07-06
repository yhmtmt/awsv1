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
#include "gtest/gtest.h"

#include <string.h>
#include <assert.h>

#include "aws_coord.hpp"
#include "aws_stdlib.hpp"
#include "aws_map.hpp"

class MapTest:public ::testing::Test
{
protected:
  MapTest()
  {
  }
  
  virtual void SetUp(){
  }
};

TEST_F(MapTest, Init)
{
    cout << "Data file is in " << PATH_TEST_INPUT_DATA << endl;  
}
