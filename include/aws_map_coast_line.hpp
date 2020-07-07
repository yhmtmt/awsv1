// Copyright(c) 2017-2020 Yohei Matsumoto, All right reserved. 

// aws_map_coast_line.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_map_coast_line.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_map_coast_line.hpp.  If not, see <http://www.gnu.org/licenses/>. 
class CoastLine : public LayerData
{
protected:
  static const vector<vec3> null_vec_vec3;
  static const vector<vec2> null_vec_vec2;
    
  struct s_line {
    vector<vec2> pts;
    vector<vec3> pts_ecef;
      
    size_t size() {
      return sizeof(unsigned int) + (sizeof(vec2) + sizeof(vec3)) * pts.size();
    }
  };
  size_t total_size;
  double dist_min;
  double pt_radius;
  vec3 pt_center;
  vec2 pt_center_blh;
    
  vector<s_line*> lines;
  void add(list<vec2> & line);
  int try_reduce(int nred);
  void update_properties();
public:
  CoastLine();
  virtual ~CoastLine();
    
  const unsigned int getNumLines() const
  {
    return lines.size();
  }
    
  const vector<vec3> & getPointsECEF(unsigned int id) const
  {
    if (id >= lines.size())
      return null_vec_vec3;
    return lines[id]->pts_ecef;
  }
    
  const vector<vec2> & getPointsBLH(unsigned int id) const
  {
    if (id >= lines.size())
      return null_vec_vec2;
    return lines[id]->pts;
  }
    
  bool loadJPJIS(const char * fname);
protected:
  virtual bool _reduce(const size_t sz_lim);
  virtual bool _merge(const LayerData & layerData);
  virtual bool _remove(const unsigned int id);  
  virtual void _release();
public:
  virtual const LayerType getLayerType() const { return lt_coast_line; };
  virtual bool save(ofstream & ofile);
  virtual bool load(ifstream & ifile);
  virtual bool split(list<Node*> & nodes, Node * pParentNode = NULL) const;
  virtual LayerData * clone() const;
  virtual size_t size() const;
  virtual float resolution() const;
  virtual float radius() const; // returns radius of the object's distribution in meter
  virtual vec3 center() const; // returns center of the object's distribution
  virtual void setCenter(const vec3 & _center)
  {
    pt_center = _center;
  }
  virtual void setRadius(const float _radius)
  {
    pt_radius = _radius;
  }
  virtual void print() const;
};
