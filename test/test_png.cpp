#include <iostream>
#include <vector>
#include <list>
#include <cmath>
#include <cstdlib>
#include <climits>
#include <cfloat>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <mutex>

using namespace std;

#if __GNUC__ < 9
#include <experimental/filesystem>
namespace fs = experimental::filesystem;
#else
#include <filesystem>
namespace fs = filesystem;
#endif

#include "gtest/gtest.h"

#include <string.h>
#include <assert.h>

#include "aws_png.hpp"

class PngTest:public ::testing::Test
{
public:
  unsigned int widths[4] = { 100, 128, 512, 1024 };
  unsigned int heights[4] = { 360, 512, 738, 1024 };
  unsigned int channels[4] = { 1, 2, 3, 4};
  unsigned int depths[2] = {8, 16};
  
protected:
  s_aws_bmp * create_bmp(unsigned int width, unsigned int height,
			 unsigned int channels, unsigned int depth){
    s_aws_bmp * bmp = new s_aws_bmp(width, height, channels, depth);
    if(!bmp)
      return nullptr;

    if(!bmp->data)
      return nullptr;
      
    int n = width * height;
    int bytes = channels * (depth / 8);
    srand(0);
    for (int i = 0; i < n; i++){
      bmp->data[i] = rand() % 0xFF;
    }
    return bmp;
  }
  
  PngTest()
  {
  }
  
  virtual void SetUp(){
  }

  virtual void TearDown(){
  }
  
};

  
TEST_F(PngTest, ReadWrite)
{
  for (int i = 0; i < 4; i ++){
    for(int j = 0; j < 4; j++){
      for(int k = 0; k < 4; k++){
	for(int l = 0; l < 2; l++){
	  s_aws_bmp * bmpsrc = create_bmp(widths[i], heights[j], channels[k], depths[l]);
	  ASSERT_TRUE(bmpsrc != nullptr);
	  
	  char fname[1024];
	  snprintf(fname, 1024, "w%d_h%d_c%d_d%d.png",
		   widths[i], heights[j], channels[k], depths[l]);
	  
	  ASSERT_TRUE(bmpsrc->write_png(fname));

	  s_aws_bmp * bmprep = new s_aws_bmp;
	  ASSERT_TRUE(bmprep != nullptr);

	  ASSERT_TRUE(bmprep->read_png(fname));

	  int n = bmprep->width * bmprep->height *
	    bmprep->channels * (bmprep->depth / 8);
	  for(int m = 0; m < n; m++){
	    ASSERT_EQ(bmprep->data[m], bmpsrc->data[m]);
	  }
	  delete bmprep;
	  delete bmpsrc;
	}
      }
    }
  }
}
