#include "gtest/gtest.h"

class ExTest: public ::testing::Test{
protected:
  virtual void SetUp(){
    val = 0;
  }
  int val;
};

TEST_F(ExTest, SetupTest){
  EXPECT_EQ(val, 0);
}
