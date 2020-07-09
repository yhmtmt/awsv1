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

#if __GNUC__ < 8
#include <experimental/filesystem>
namespace fs = experimental::filesystem;
#else
#include <filesystem>
namespace fs = filesystem;
#endif

#include "gtest/gtest.h"

#include <string.h>
#include <assert.h>

#include "aws_coord.hpp"
#include "aws_stdlib.hpp"
#include "aws_map.hpp"

using namespace AWSMap2;

class MapTest:public ::testing::Test
{
protected:
  MapDataBase mdb;  
  static const char * jpgis_list[4];
  bool is_data_found;
  string work_path;
  MapTest(): is_data_found(false)
  {
  }
  
  virtual void SetUp(){
    is_data_found = fs::exists(PATH_TEST_INPUT_DATA);
    for(int i = 0; i < 4; i++){
      string data_path = PATH_TEST_INPUT_DATA;
      data_path += "/";
      data_path += jpgis_list[i];
      is_data_found &= fs::exists(data_path);
    }

    if(!is_data_found)
      return;
    
    work_path = PATH_TEST_INPUT_DATA;
    work_path += "/work";
    fs::create_directory(work_path.c_str());
    fs::permissions(work_path, fs::perms::owner_all);
    
    MapDataBase::setPath(work_path.c_str());

    mdb.init();
    for (int i = 0; i < 4; i++){
      string data_path = PATH_TEST_INPUT_DATA;
      data_path += "/";
      data_path += jpgis_list[i];
      CoastLine cl;
      
      if(!cl.loadJPJIS(data_path.c_str())){
	cout << "Failed to load " << data_path << endl;
	is_data_found = false;
	break;
      }

      mdb.insert(&cl);
    }    
  }

  virtual void TearDown(){
    cout << "removing " << work_path << endl;
    fs::remove_all(work_path.c_str());
  }
  
};

const char * MapTest::jpgis_list[4] =
  {
    "C23-06_12-g.xml", "C23-06_13-g.xml", "C23-06_14-g.xml", "C23-06_22-g.xml"
  };
  
TEST_F(MapTest, Init)
{
    cout << "Data file is in " << PATH_TEST_INPUT_DATA << endl;
    
}
