#include <iostream>
#include <cmath>

#include "gtest/gtest.h"
#include "aws_coord.hpp"

class CoordTest: public ::testing::Test{
protected:
  struct s_posp{
    double lat, lon, alt;
  };

  struct s_posc{
    double x, y, z;
  };

#define NUM_TEST_POINTS 11
  static s_posp posp[NUM_TEST_POINTS];
  static s_posc posc[NUM_TEST_POINTS];
  virtual void SetUp(){
  }
};

CoordTest::s_posp CoordTest::posp[NUM_TEST_POINTS] = {
  {90, 0, 0}, {-90, 0, 0}, {0, 0, 0},
  {35, 135},  {-35, 135},  {35, -135},  {-35, -135},
  {35, 45},  {-35, 45},  {35, -45},  {-35, -45}  
};

CoordTest::s_posc CoordTest::posc[NUM_TEST_POINTS] = {
  {0, 0, BE}, {0, 0, -BE}, {AE, 0, 0},
  {-3698470.29, 3698470.29, 3637866.91}, {-3698470.29, 3698470.29, -3637866.91},
  {-3698470.29, -3698470.29, 3637866.91}, {-3698470.29, -3698470.29, -3637866.91},
  {3698470.29, 3698470.29, 3637866.91}, {3698470.29, 3698470.29, -3637866.91},
  {3698470.29, -3698470.29, 3637866.91}, {3698470.29, -3698470.29, -3637866.91}  
};
  
TEST_F(CoordTest, BLHECEFTest)
{
  double x, y, z;
  x = y = z = 0;
  for (int i = 0; i < NUM_TEST_POINTS; i++){
    blhtoecef(posp[i].lat * PI / 180.0,
	      posp[i].lon * PI / 180.0,
	      posp[i].alt, x, y, z);
    EXPECT_NEAR(posc[i].x, x, 0.01);
    EXPECT_NEAR(posc[i].y, y, 0.01);
    EXPECT_NEAR(posc[i].z, z, 0.01);
  }
}

TEST_F(CoordTest, ECEFBLHTest)
{
  double lat, lon, alt;
  lat = lon = alt = 0;
  for(int i = 0; i < NUM_TEST_POINTS; i++){
    eceftoblh(posc[i].x, posc[i].y, posc[i].z, lat, lon, alt);
    lat *= 180.0/PI;
    lon *= 180.0/PI;
    EXPECT_FLOAT_EQ(posp[i].lat, lat);
    EXPECT_FLOAT_EQ(posp[i].lon, lon);
    EXPECT_NEAR(posp[i].alt, alt, 0.01);
  }
}

double l2_norm(const double a1, const double a2, const double a3)
{
  return a1 * a1 + a2 * a2 + a3 * a3;
}
  
TEST_F(CoordTest, WorldRotationMatrixTest)
{
  double Rwrld[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
  for(int i = 0; i < NUM_TEST_POINTS; i++){
    getwrldrot(posp[i].lat, posp[i].lon, Rwrld);
    EXPECT_FLOAT_EQ(l2_norm(Rwrld[0], Rwrld[1], Rwrld[2]), 1.0);
    EXPECT_FLOAT_EQ(l2_norm(Rwrld[3], Rwrld[4], Rwrld[5]), 1.0);
    EXPECT_FLOAT_EQ(l2_norm(Rwrld[6], Rwrld[7], Rwrld[8]), 1.0);
    EXPECT_FLOAT_EQ(l2_norm(Rwrld[0], Rwrld[3], Rwrld[6]), 1.0);
    EXPECT_FLOAT_EQ(l2_norm(Rwrld[1], Rwrld[4], Rwrld[7]), 1.0);
    EXPECT_FLOAT_EQ(l2_norm(Rwrld[2], Rwrld[5], Rwrld[8]), 1.0);    
  }
}
