// Copyright(c) 2017-2020 Yohei Matsumoto, All right reserved. 

// aws_map_point.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_map_point.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_map_point.hpp.  If not, see <http://www.gnu.org/licenses/>

class Points : public LayerData
{
protected:
  struct s_points{
    long long tstart, tend; // recorded time
    double range; // scanner range 
    vector<vec3> pts; // points measured 
    vector<vec3> vec; // vector toward scanner center 
    size_t size() {
      return sizeof(unsigned int) + sizeof(long long) + (sizeof(unsigned int) + sizeof(vec3)) * pts.size();
    }
  };
    
  virtual bool _reduce(const size_t sz_lim) = 0;        // reduce the data structure to meet the size limit
  virtual bool _merge(const LayerData & layerData) = 0; // merge given layerData to this layerData
  virtual void _release() = 0;			  // release all internal data structure but does not mean the destruction of this object
  virtual bool _remove(const unsigned int id) = 0;
public:
  Points();
  virtual ~Points();
    
  virtual const LayerType getLayerType() const = 0;     // returns LayerType value.
  virtual bool save(ofstream & ofile) = 0;              // save data to ofile stream.
  virtual bool load(ifstream & ifile) = 0;              // load data from ifile stream.
  virtual bool split(list<Node*> & nodes, Node * pParentNode = NULL) const = 0; // split the layer data into nodes given
  virtual LayerData * clone() const = 0;	// returns clone of the instance
  virtual size_t size() const = 0;		// returns size in memory 
  virtual float resolution() const = 0;	// returns minimum distance between objects in meter		
  virtual float radius() const = 0;		// returns radius of the object's distribution in meter
  virtual vec3 center() const = 0;	// returns center of the object's distribution
  virtual void print() const = 0;   
};
