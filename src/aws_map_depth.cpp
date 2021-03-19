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

#include <string.h>
#include <assert.h>

#include "aws_coord.hpp"
#include "aws_stdlib.hpp"
#include "aws_map.hpp"

namespace AWSMap2{
  Depth::Depth(): r(-1.0f)
  {
    bmp.width = DEPTH_MAP_WIDTH;
    bmp.height = DEPTH_MAP_HEIGHT;
    bmp.channels = DEPTH_MAP_CHANNELS;
    bmp.depth = DEPTH_MAP_DEPTH;
  }

  Depth::~Depth()
  {
  }

  bool Depth::init()
  {
    if(!pNode){
      return false;
    }

    for(int i = 0; i < 3; i++){
      vtx[i] = pNode->getVtxECEF(i);      
    }
    
    return init(vtx);
  }

  bool Depth::init(const vec3 * _vtx)
  {
    for(int i = 0; i < 3; i++){
      vtx[i] = _vtx[i];
    }

    // calculating radius
    vec[0] = vtx[1] - vtx[0];
    vec[1] = vtx[2] - vtx[0];
    vec[2] = vec[1] - vec[0];
    double s0 = sqrt(dot(vec[0], vec[0]));
    double s1 = sqrt(dot(vec[1], vec[1]));
    double s2 = sqrt(dot(vec[2], vec[2]));
    
    double c = dot(vec[0], vec[1]) / (s0 * s1);
    double s = sqrt(1 - c*c);
    r = 0.5 * s2 / s;

    // calculating rotation matrix
    vec3 Z(0, 0, 1.0); // Z axis in ECEF coordinate system
    
    zvec = cross(vec[0], vec[1]); // normal vector of the surface
    zvec *= (1.0 / sqrt(dot(zvec, zvec))); // normalize
    yvec = Z - zvec * dot(Z, zvec) ;
    yvec *= (1.0 / sqrt(dot(yvec, yvec))); // normalize
    xvec = cross(yvec, zvec);
    xvec *= (1.0 / sqrt(dot(xvec, xvec))); // normalize

    // transform vtx into planar northup coordinate
    vec3 vtx0p(dot(vtx[0],xvec), dot(vtx[0],yvec), dot(vtx[0], zvec));
    vec3 vtx1p(dot(vtx[1],xvec), dot(vtx[1],yvec), dot(vtx[1], zvec));
    vec3 vtx2p(dot(vtx[2],xvec), dot(vtx[2],yvec), dot(vtx[2], zvec));    
   
    double xmin, ymin;
    double xmax, ymax;
    xmin = min(min(vtx0p.x, vtx1p.x), vtx2p.x);
    xmax = max(max(vtx0p.x, vtx1p.x), vtx2p.x);
    ymin = min(min(vtx0p.y, vtx1p.y), vtx2p.y);
    ymax = max(max(vtx0p.y, vtx1p.y), vtx2p.y);

    double w = xmax - xmin;
    double h = ymax - ymin;
    scale = min((double)(DEPTH_MAP_WIDTH - 1)/ w,
		 (double)(DEPTH_MAP_HEIGHT - 1) / h);
    double sw = scale * w;
    double sh = scale * h;
    
    float xmini, ymini;
    // float xmaxi, ymaxi;
    xmini = (float)(((double)(DEPTH_MAP_WIDTH - 1) - sw) * 0.5);
    // xmaxi = (float)((double)(DEPTH_MAP_WIDTH - 1) - xmini);
    ymini = (float)(((double)(DEPTH_MAP_HEIGHT - 1) - sh) * 0.5);
    // ymaxi = (float)((double)(DEPTH_MAP_HEIGHT - 1) - ymini);

    vtxi[0] = vec2(scale * (vtx0p.x - xmin) + xmini,
		   scale * (vtx0p.y - ymin) + ymini);
    vtxi[1] = vec2(scale * (vtx1p.x - xmin) + xmini,
		   scale * (vtx1p.y - ymin) + ymini);
    vtxi[2] = vec2(scale * (vtx2p.x - xmin) + xmini,
		   scale * (vtx2p.y - ymin) + ymini);

    if(!bmp.init(bmp.width, bmp.height, bmp.channels, bmp.depth)){
      return false;
    }

    short * p = bmp.datai16;

    for (int j = 0; j < bmp.height; j++){
      for (int i = 0; i < bmp.width; i++, p+=2){	
	p[0] = p[1] = -1;
      }
    }
    return true;
  }

  // We do not need to reduce because the size of the Depth data is fixed.
  bool Depth::_reduce(const size_t sz_lim)
  {
    // No implementation 
    return true;
  }

  
  bool Depth::_merge(const LayerData & layerData)
  {
    if(r < 0){// the object has not been initialized with node data.
      if(!init())
	return false; // failed to initialize
    }
    
    // project overwrpapping part of the triangle in layerData
    const Depth * pdata = dynamic_cast<const Depth *>(&layerData);
    if(!pdata){
      return false;
    }

    if(vtx_center == pdata->vtx_center){
      memcpy(bmp.data, pdata->bmp.data, bmp.get_data_size());
      return true;
    }
    
    // if a,b,c is in the effective triangle,
    // sample value around a veci[0] + b veci[1], and fill the pixel value.
    unsigned int size_of_pixel = 2;
    unsigned int size_of_rows = size_of_pixel * pdata->bmp.width;
    
    double invD = 1.0 / (veci[0].x * veci[1].y - veci[0].y * veci[1].x);
    vec2 iv0(veci[1].y * invD, -veci[0].y * invD);
    vec2 iv1(-veci[1].x * invD, veci[0].x * invD);
    
    // for all effective pixels in the bmp    
    for (int j = 0; j < bmp.height; j++){    
      for (int i = 0; i < bmp.width; i++){
	// calculate pixel vector like as s veci[0] + t veci[1]
	vec2 x((double)i - vtxi[0].x, (double)j - vtxi[0].y);
	double s = iv0.x * x.x + iv1.x * x.y;
	double t = iv0.y * x.x + iv1.y * x.y;	

	if(s < 0 || s > 1 || t < 0 || t > 1 || s + t > 1) 
	  continue;// is not inside triangle
	
	// if s,t is in the effective pixel,	
	//  make point vector p as s vec[0] + t vec[1] + vtx[0]
	
	vec3 p = vec[0] * s + vec[1] * t + vtx[0];
	
	// find crossing point on the surface in layerData
	// p' as the solution [ vec'[0], vec'[1], -p][a, b, c]^t = vtx'[0]
	// where vec' and vtx' is the vec and vtx in layerData.
	double a, b, c;
	proj2tri(a, b, c, p, pdata->vtx[0], pdata->vec[0], pdata->vec[1]);
	if(a < 0 || a > 1 || b < 0 || b > 1)
	  continue;

	x = pdata->veci[0] * a + pdata->veci[1] * b + pdata->vtxi[0];

	// sampling at x with bi-linear interpolation
	int x0 = int(x.x), y0 = int(x.y), x1 = x0 + 1, y1 = y0 + 1;
	double alpha = x.x - (double)x0;
	double beta = x.y - (double)y0;
	double ialpha = 1.0 - alpha;
	double ibeta = 1.0 - beta;
	const short * pd00 = pdata->bmp.datai16 + x0 * size_of_pixel + y0 * size_of_rows;
	const short * pd10 = pd00 + size_of_pixel;
	const short * pd01 = pd00 + size_of_rows;
	const short * pd11 = pd01 + size_of_pixel;
       
	short * ps = bmp.datai16 + i * size_of_pixel + j * size_of_rows;
	ps[0] = ibeta * (ialpha * pd00[0] + alpha * pd10[0])
	  + beta * (ialpha * pd01[0] + alpha * pd11[0]);
	ps[1] = ibeta * (ialpha * pd00[1] + alpha * pd10[1])
	  + beta * (ialpha * pd01[1] + alpha * pd11[1]);
	ps[0] += ps[1] / 1000;
	ps[1] = ps[1] % 1000;
      }
    }
    return true;
  }

  void Depth::_release()
  {
    delete[] bmp.data;
    bmp.data = nullptr;
  }

  bool Depth::_remove(const unsigned int id)
  {
    return true;
  }

  bool Depth::save()
  {
    if (!bupdate){
      return true;
    }
    
    if (!pNode)
      return false;

    if(!bmp.data)
      return false;
      
    char fname[2048];
    genFileName(fname, 2048);
#ifdef _AWS_MAP_DEBUG
    cout << "saving " << fname << endl;
#endif
    if(!bmp.write_png(fname))
      return false;
          
    bupdate = false;
    return true;
  }
  
  bool Depth::save(ofstream & ofile)
  {
    return true;
  }

  bool Depth::load()
  {
     if (!pNode)
      return false;
    
    char fname[2048];
    genFileName(fname, 2048);
#ifdef _AWS_MAP_DEBUG
    cout << "loading " << fname << endl;
#endif
    if(!bmp.read_png(fname))
      return false;
    
    bupdate = false;
    
    setActive();
    
    return true;
  }
  
  bool Depth::load(ifstream & ifile)
  {
    return true;
  }

  // only digs down to the correct node to be added.
  bool Depth::split(list<Node*> & nodes, Node * pParentNode) const
  {
    double err = 0.0;
    for(auto itr = nodes.begin(); itr != nodes.end(); itr++){
      if((*itr)->collision(vtx_center, err))
	(*itr)->addLayerData(*this);
    }
  }

  LayerData * Depth::clone() const
  {
    Depth * p = new Depth();
    p->bmp.init(bmp.width, bmp.height, bmp.channels, bmp.depth);

    memcpy(p->bmp.data, bmp.data, bmp.get_data_size());
    
    p->vtx[0] = vtx[0];
    p->vtx[1] = vtx[1];
    p->vtx[2] = vtx[2];
    p->vtxi[0] = vtxi[0];
    p->vtxi[1] = vtxi[1];
    p->vtxi[2] = vtxi[2];
    
    return p;
  }

  size_t Depth::size() const
  {    
    unsigned int sz= sizeof(s_aws_bmp) + sizeof(vtx) + sizeof(vtxi);
    sz += bmp.get_data_size();
    return sz;
      
  }

  float Depth::resolution() const
  {
  }

  float Depth::radius() const
  {
    return r;
  }

  vec3 Depth::center() const
  {
    return vtx_center;
  }

  void Depth::print() const
  {
  }  
};

