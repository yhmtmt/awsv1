// Copyright(c) 2021 Yohei Matsumoto, All right reserved. 

// aws_png.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_png.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_png.hpp.  If not, see <http://www.gnu.org/licenses/>.
#ifndef AWS_PNG_HPP
#define AWS_PNG_HPP

struct s_aws_bmp{
  unsigned int width, height;
  unsigned int channels, depth, rowbytes;
  union{
    unsigned char * data;
    long long * datai64;    
    int * datai32;
    short * datai16;
    unsigned short * datau16;
    unsigned int * datau32;
    unsigned long long * datau64;
    float * dataf32;
    double * dataf64;
  };
  
  s_aws_bmp():data(nullptr), width(-1), height(-1), channels(0){};
  s_aws_bmp(unsigned int width, unsigned int height,
	    unsigned int channels, unsigned int depth)
    :data(nullptr), width(width), height(height),
     channels(channels), depth(depth)
  {
    data = new unsigned char[get_data_size()];
  }

  bool init(unsigned int width, unsigned int height,
	    unsigned int channels, unsigned int depth);
  ~s_aws_bmp(){ delete[] data; data = nullptr; };
  
  size_t get_data_size() const
  {
    return width * height * channels * (depth / 8);    
  }
  
  bool read_png(const char * fname);
  bool write_png(const char * fname);
};

#endif
