#ifndef CH_MAP_HPP
#define CH_MAP_HPP
// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_map.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_map.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_map.hpp.  If not, see <http://www.gnu.org/licenses/>.

#include "channel_base.hpp"

// contains map information - multiple layered, dynamically updatable map.
// has insert, delete, configuration methods
class ch_map: public ch_base
{
protected:
  bool bupdate, bready;
  float m_range, m_resolution;
  bool m_blayer_type[AWSMap2::lt_undef];
  list<AWSMap2::LayerDataPtr> m_layer_datum[AWSMap2::lt_undef];
  
  AWSMap2::vec3 m_cecef; // x, y, z
public:
  ch_map(const char * name):ch_base(name), bupdate(true), bready(false), m_resolution(10), m_range(10000), m_cecef()
  {
    m_blayer_type[AWSMap2::lt_coast_line] = true;
  }
  
  virtual ~ch_map()
  {
  }
  
  bool is_update(){
    return bupdate;
  }
  
  void set_update(){
    bupdate = true;
  }
  
  void reset_update(){
    bupdate = false;
  }
  
  bool is_ready(){
    return bready;
  }
  
  void set_ready(){
    bready = true;
  }
  
  void reset_ready(){
    bready = false;
  }
  
  
  void enable_layer(const AWSMap2::LayerType layer_type)
  {
    m_blayer_type[layer_type] = true;
  }
  
  void disable_layer(const AWSMap2::LayerType layer_type)
  {
    m_blayer_type[layer_type] = false;
  }
  
  bool is_layer_enabled(const AWSMap2::LayerType layer_type)
  {
    return m_blayer_type[layer_type];
  }
  
  void set_resolution(const float resolution)
  {
    m_resolution = resolution;
  }
  
  const float get_resolution()
  {
    return m_resolution;
  }
  
  void set_range(const float range)
  {
    m_range = range;
  }
  
  const float get_range()
  {
    return m_range;
  }
  
  void set_center(const float x, const float y, const float z)
  {
    m_cecef.x = x;
    m_cecef.y = y;
    m_cecef.z = z;
  }
  
  const AWSMap2::vec3 get_center()
  {
    return m_cecef;
  }
  
  void set_layer_data(const AWSMap2::LayerType layer_type, list<AWSMap2::LayerDataPtr> & layer_data)
  {
    m_layer_datum[layer_type].clear();
    //m_layer_datum[layer_type].insert(m_layer_datum[layer_type].end(), layer_data.begin(), layer_data.end());	    
    m_layer_datum[layer_type] = layer_data;  
  }

  const list<AWSMap2::LayerDataPtr> & get_layer_data(const AWSMap2::LayerType layer_type)
  {
    return m_layer_datum[layer_type];
  }
  
  void clear_layer_datum()
  {
    for (int layer_type = 0; layer_type < (int)AWSMap2::lt_undef; layer_type++){
      m_layer_datum[layer_type].clear();
    }
  }
};

#endif
