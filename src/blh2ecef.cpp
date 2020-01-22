#include <iostream>
#include <iomanip>
using namespace std;
#include "aws_coord.hpp"

int main(int argc, char ** argv)
{
  if(argc != 3 && argc != 4){
    cout << "Usage: blh2ecef <lat> <lon> [<alt>]" << endl;
    return 1;
  }

  double lat = atof(argv[1]) * PI / 180.0;
  double lon = atof(argv[2]) * PI / 180.0;
  double alt = 0.0;
  if(argc == 4)
    alt = atof(argv[3]);

  double x, y, z;
  x = y = z = 0.0;
  blhtoecef(lat, lon, alt, x, y, z);
  cout << setprecision(9) << x << " " << y << " " << z << endl;
  
  return 0;
}
