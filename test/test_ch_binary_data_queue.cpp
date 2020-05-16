#include <cstdlib>
#include <ctime>
#include <cstring>
#include <iostream>


using namespace std;

#include "gtest/gtest.h"
#include "ch_binary_data_queue.hpp"

typedef ch_binary_data_queue<32, 32> ch_test_queue;

class ChBinaryDataQueueTest: public ::testing::Test
{
protected:
  ch_test_queue chan0, chan1;
  unsigned char data[64][32];
  unsigned short len[64];
  virtual void SetUp(){
    srand(0);
    for(int i = 0; i < 64; i++){
      len[i] = rand() % 32;      
      for(int j = 0; j < len[i]; j++){
	data[i][j] = (unsigned char) (rand() % 256);
      }
    }
  }
  
public:
  ChBinaryDataQueueTest():chan0("chan0"), chan1("chan1"){};
  
};


TEST_F(ChBinaryDataQueueTest, PushPop)
{
  size_t sz;

  unsigned char buf[32];
  auto test = [&] (int n){
    for(int i = 0; i < n; i++){
      chan0.push(data[i], len[i]);
    }
    for(int i = (n > 32 ? n-32 : 0); i < n; i++){
      chan0.pop(buf, sz);
      ASSERT_EQ(sz, len[i]);
      ASSERT_TRUE(0 == memcmp(buf, data[i], sz));
    }    
  };
  
  test(1);
  test(16);
  test(32);
  test(48);
}

TEST_F(ChBinaryDataQueueTest, ReadWriteBuf)
{
  size_t sz0, sz1;
  unsigned char * buf = new unsigned char[chan0.get_dsize()];

  auto test = [&] (int n){
    for(int i = 0; i < n; i++){
      chan0.push(data[i], len[i]);
    }
    sz0 = chan0.read_buf((char*)buf);
    ASSERT_TRUE(sz0 < chan0.get_dsize());
    sz1 = chan1.write_buf((const char*)buf);
    ASSERT_EQ(sz0, sz1);
    for(int i = (n > 32 ? n-32 : 0); i < n; i++){
      chan1.pop(buf, sz0);
      ASSERT_EQ(sz0, len[i]);
      ASSERT_TRUE(0 == memcmp(buf, data[i], sz0));
    }
  };

  test(1);
  test(16);
  test(32);
  test(48);

  delete[] buf;
}
  
