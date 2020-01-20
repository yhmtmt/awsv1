#include <iostream>
#include <cmath>

#include "gtest/gtest.h"
#include "time.h"
#include "aws_clock.hpp"

class ClockTest: public ::testing::Test{
protected:
  c_clock clk;
  long long period;
  virtual void SetUp(){
    period = 100000;
    clk.set_period(period); // 10 msec
  }
};


bool avg_jit(double t){return t < 1000;};    // 100usec deviation allowed
bool tper_err(long long t){return t < 1000;};
bool ttl_err(long long t){return t < 10000;};// 1msec deviation allowed

TEST_F(ClockTest, OnlineTest){
  long long t[101];
  clk.start(0, true);
  t[0] = clk.get_time();
  for(int i = 0; i < 100; i++){
    clk.wait();
    t[i+1] = clk.get_time();
  }
  
  long long t100 = t[100] - t[0];
  double tjit = 0;
  
  for(int i = 0; i < 100; i++){
    long long tper = std::abs(t[i+1] - t[i]) - period;
    tjit = (double)(tper * tper);
  }
  tjit /= 100.0;
  
  long long terr = std::abs(t100 - 100*period);

  EXPECT_PRED1(avg_jit, tjit);
  EXPECT_PRED1(ttl_err, terr);
  clk.stop();
  //  EXPECT_EQ(t100, 10000000);
}

TEST_F(ClockTest, OfflineTest){
  long long t[101];
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  long long tstart = (long long)((long long) ts.tv_sec * SEC +
				 (long long) (ts.tv_nsec / 100));
  
  clk.start(tstart, false);
  t[0] = clk.get_time();
  for(int i = 0; i < 100; i++){
    clk.wait();
    t[i+1] = clk.get_time();
    if(i == 50){ // in 50th cycle, the clock is paused by 1sec, then restarted.
      clk.pause();
      long long t0 = clk.get_time();
      sleep(1);
      long long t1 = clk.get_time();
      EXPECT_EQ(t0, t1);
      clk.restart();
    }
  }
  long long t100 = t[100] - t[0];
  double tjit = 0;

  for(int i = 0; i < 100; i++){
    long long tper = std::abs(t[i+1] - t[i]) - period;
    EXPECT_PRED1(tper_err, tper);
    tjit = (double)(tper * tper);
  }
  tjit /= 100.0;
  
  long long terr = std::abs(t100 - 100*period);

  EXPECT_PRED1(avg_jit, tjit);
  EXPECT_PRED1(ttl_err, terr);
  clk.stop();
}
