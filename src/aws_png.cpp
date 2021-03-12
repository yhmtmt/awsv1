// Copyright(c) 2021 Yohei Matsumoto, All right reserved. 

// aws_map_depth.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_map_depth.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_map_depth.cpp.  If not, see <http://www.gnu.org/licenses/>.

#include <cstdio>
#include <cstring>
#include <iostream>

#include <png.h>

#include "aws_png.hpp"

using namespace std;


bool s_aws_bmp::read_png(const char * fname)
{
  if(data){
    delete [] data;
    data = nullptr;
  }
    
  FILE * f = fopen(fname, "rb");

  if(!f){
    cerr << "In aws_bmp::read_png, failed to open  " << fname << endl;
    return false;
  }
    
  unsigned int sz;
  
  png_structp png;
  png_infop info;
  png_bytepp datap;
  png_byte type;
  png_byte signature[8];


  sz = fread(signature, 1, sizeof(signature), f);

  if(png_sig_cmp(signature, 0, sizeof(signature))){
    cerr << "Error in aws_bmp::read_png, png_sig_cmp." << endl;
    return false;
  }

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(png == nullptr){
    cerr << "Error in aws_bmp::read_png, png_create_read_struct." << endl;
    return false;
  }
  
  info = png_create_info_struct(png);
  if(info == nullptr){
    cerr << "Error in aws_bmp::read_png, png_create_info_struct." << endl;
    return false;
  }

  png_init_io(png, f);
  png_set_sig_bytes(png, sz);

  png_read_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

  width = png_get_image_width(png, info);
  height= png_get_image_height(png, info);
  depth = png_get_bit_depth(png, info);
  channels = png_get_channels(png, info);
  datap = png_get_rows(png, info);
  rowbytes = png_get_rowbytes(png, info);
  type = png_get_color_type(png, info);
  
  data = new unsigned char[rowbytes * height];
  if(!data){
    cerr << "In s_aws_bmp::read_png, failed to allocate memory." << endl;
    png_destroy_read_struct(&png, &info, nullptr);
    return false;
  }

  for (int j = 0; j < height; j++){
    memcpy(data + j * rowbytes, datap[j], rowbytes);
  }
  png_destroy_read_struct(&png, &info, nullptr);
  
  fclose(f);
  return true;
}


bool s_aws_bmp::write_png(const char * fname)
{
  FILE * f = fopen(fname, "wb");
  
  if(!f){
    cerr << "In aws_bmp::write_png, failed to open " << fname << endl;
    return false;
  }

  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if(!png){
    cerr << "Error in s_aws_bmp::write_png, png_create_write_struct." << endl;
    return false;
  }
  
  png_infop info = png_create_info_struct(png);
  if(!info){
    cerr << "Error in s_aws_bmp::write_png, png_create_write_struct." << endl;
    png_destroy_write_struct(&png, &info);
    return false;
  }
  
  png_bytepp datap;
  png_byte type;
  if(channels == 1){
    type = PNG_COLOR_TYPE_GRAY;
  }else if(channels == 2){
    type = PNG_COLOR_TYPE_GRAY_ALPHA;
  }else if(channels == 3){
    type = PNG_COLOR_TYPE_RGB;
  }else if(channels == 4){
    type = PNG_COLOR_TYPE_RGB_ALPHA;
  }else{
    cerr << "Error in s_aws_bmp::write_png, unsupported channels " << channels << endl;
    return false;
  }
  png_init_io(png, f);
  png_set_IHDR(png, info, width, height, depth, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  
  datap = (png_bytepp)png_malloc(png, sizeof(png_bytep) * height);
  rowbytes = width * channels * (depth)/8;
  png_set_rows(png, info, datap);
  
  for(int j = 0; j < height; j++){
    datap[j] = (png_bytep)png_malloc(png, rowbytes);
    memcpy(datap[j], data + j * rowbytes, rowbytes);
  }  
  png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);

  for(int j = 0; j < height; j++){
    png_free(png, datap[j]);
  }
  png_free(png, datap);

  png_destroy_write_struct(&png, &info);
  
  fclose(f);
  
  return true;
}
