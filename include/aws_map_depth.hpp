// Copyright(c) 2021 Yohei Matsumoto, All right reserved. 

// aws_map_depth.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_map_depth.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_map_depth.hpp.  If not, see <http://www.gnu.org/licenses/>.

// bitmap is created as the 4 channels 1024x1024 image,
// but the 4 channels store two 16bit signed integer.
// upper 16bit indicates meter, and the lower 16bit indicates millimeter.
// negative value in the upper 16bit  means unknown point,
// and that in the lower 16bit means invalid point.
// bitmap coordinate is the same as the opengl texture coordinate,
// i.e. bottom-left corner is (0,0).
#define DEPTH_MAP_WIDTH 1024
#define DEPTH_MAP_HEIGHT 1024
#define DEPTH_MAP_CHANNELS 4
#define DEPTH_MAP_DEPTH 8


// This layer data is for storing depth map as bitmap data.
// To insert the data into MapDataBase, we need to retrieve information of
// the node stores the data we have.
// Create
class Depth: public LayerData
{
protected:
  s_aws_bmp bmp; // depth bitmap (as 4byte integer)
  
  vec3 vtx[3];   // 3 points form valid triangle area.
  vec3 vec[3];   // corresponding vector
  vec3 zvec, xvec, yvec;
  vec2 vtxi[3];  // bmp corresponding 3 points
  vec2 veci[3];  // corresponding vector
  double scale;  // scale ECEF to bmp
  
  vec3 vtx_center;
  float r;

public:
  Depth();
  virtual ~Depth();
protected:
  // reduce the data structure to meet the size limit
  virtual bool _reduce(const size_t sz_lim) ;
  
  // merge given layerData to this layerData
  virtual bool _merge(const LayerData & layerData) ;
  
  // release all internal data structure but does not mean
  // the destruction of this object
  virtual void _release() ;			  
  
  // remove data with specified id.
  virtual bool _remove(const unsigned int id) ;
public:
  // returns LayerType value.
  virtual const LayerType getLayerType() const
  {
    return lt_depth;
  };
  
  // save data to ofile stream.
  virtual bool save();
  virtual bool save(ofstream & ofile);
    
  // load data from ifile stream.
  virtual bool load();
  virtual bool load(ifstream & ifile);

  // split the layer data into nodes given
  virtual bool split(list<Node*> & nodes, Node * pParentNode = NULL) const ;
  // returns clone of the instance
  virtual LayerData * clone() const ;
    
  // returns size in memory 
  virtual size_t size() const ;
    
  // returns minimum distance between objects in meter		
  virtual float resolution() const ;

  // returns radius of the object's distribution in meter
  virtual float radius() const ;
    
  // returns center of the object's distribution
  virtual vec3 center() const ;

  // dump the data inside as text 
  virtual void print() const ;

  bool init();
  bool init(const vec3 * _vtx);
};
