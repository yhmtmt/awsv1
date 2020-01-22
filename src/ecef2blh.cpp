#include <iostream>
#include <iomanip>
using namespace std;
#include "aws_coord.hpp"

int main(int argc, char ** argv)
{
  if(argc != 4){
    cout << "Usage: ecef2blh <x> <y> <z>" << endl;
    return 1;
  }

  double x = atof(argv[1]);
  double y = atof(argv[2]);
  double z = atof(argv[3]);

  double lat, lon, alt;
  lat = lon = alt = 0.0;
  eceftoblh(x, y, z, lat, lon, alt);
  lat *= 180.0 / PI;
  lon *= 180.0 / PI;
  cout << setprecision(9) << lat << " " << setprecision(10) << lon << " " << setprecision(7) << alt << endl;
  
  return 0;
}
