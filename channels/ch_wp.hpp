#ifndef CH_WP_HPP
#define CH_WP_HPP
// Copyright(c) 2016-2020 Yohei Matsumoto, All right reserved. 

// ch_wp.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_wp.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_wp.hpp.  If not, see <http://www.gnu.org/licenses/>.

#include "channel_base.hpp"
#include "aws_coord.hpp"

struct s_wp
{
  double lat, lon; // longitude lattitude
  double x, y, z;	// corresponding ECEF to lat, lon
  double rx, ry, rz; // Relative position in my own ship coordinate
  float rarv;		// Arrival threashold radius
  float v;		// velocity to go
  long long t;	// arrival time
  bool update_rpos;
  
  s_wp() :lat(0.), lon(0.), x(0.), y(0.), z(0.), rarv(0.), v(0.), t(-1),
	  update_rpos(false)
  {
  }
  
  s_wp(double _lat, double _lon, float _rarv, float _v) :
    lat(_lat), lon(_lon), x(0.), y(0.), z(0.), rarv(_rarv), v(_v), t(-1),
    update_rpos(false)
  {
    blhtoecef(lat, lon, 0., x, y, z);
  }
  
  s_wp(bool bnull): lat(FLT_MAX), lon(FLT_MAX), x(FLT_MAX), y(FLT_MAX), z(FLT_MAX), rarv(FLT_MAX), 
		    v(FLT_MAX), t(LLONG_MAX), update_rpos(bnull){
  }
  
  ~s_wp()
  {
  }
  
  bool is_null(){
    return lat == FLT_MAX;
  }
  
  void update_pos_rel(const double * Rorg,
		      const double xorg, const double yorg, const double zorg)
  {
    eceftowrld(Rorg, xorg, yorg, zorg, x, y, z, rx, ry, rz);
    update_rpos = true;
  }
  
  void set_arrival_time(const long long _t)
  {
    t = _t;
  }
  
  const long long get_arrival_time()
  {
    return t;
  }
};

// contains waypoints
// has insert, delete, access method
class ch_wp: public ch_base
{
public:
  enum e_cmd{
    cmd_save, cmd_load, cmd_none
  };
protected:
  e_cmd cmd;                       // save/load command to waypoint manager
  int id;                          // route index. used to save/load route file.
  
  static s_wp wp_null;             // null waypoint for return value
  
  list<s_wp*> wps;                 // list of waypoints 
  list<s_wp*>::iterator itr;       // iterator manipulator used

  // Manipulators focus
  int focus;                        // index of the focused wp
  list<s_wp*>::iterator itr_focus;  // iterator of the focused wp

  // Next navigation target     
  int inext;                        // index of the next wp
  list<s_wp*>::iterator itr_next;   // iterator of the next wp

  // for autopilot 
  float dist_next;                  // distance to the next wp
  float course_next;                 // course to the next wp
  float xdiff_next;                 // cross track difference 

  // finding next navigation target.
  // The earliest waypoint with no arrival flag is selected.
  void find_next(){
    for(itr = wps.begin(), inext = 0; itr != wps.end() && (*itr)->get_arrival_time() > 0; inext++, itr++);
    itr_next = itr;
  }
  
public:
  ch_wp(const char * name) :ch_base(name), focus(0), dist_next(0.), course_next(0), xdiff_next(0), cmd(cmd_none), id(0)
  {
    itr_next = itr_focus = itr = wps.begin();
  }
  
  void clear()
  {
    for(itr = wps.begin(); itr != wps.end(); itr++)
      delete *itr;
    wps.clear();
    focus = 0;
    itr_next = itr_focus = itr = wps.begin();
  }
  
  void set_cmd(const e_cmd _cmd)
  {
    cmd = _cmd;
  }
  
  const e_cmd get_cmd()
  {
    return cmd;
  }
  
  void set_route_id(const int _id)
  {
    id = _id;
  }
  
  const int get_route_id()
  {
    return id;
  }
  
  void set_target_course(const float dist, const float course,
			 const float xdiff)
  {	  
    dist_next = dist;
    course_next = course;
    xdiff_next = xdiff;
  }
  
  void get_target_course(float & dist, float & course, float & xdiff)
  {
    dist = dist_next;
    course = course_next;
    xdiff = xdiff_next;
  }
  
  
  bool is_finished(){
    return itr_next == wps.end();
  }
  
  s_wp & get_next_wp(){		
    return **itr_next;
  }
  
  s_wp & get_prev_wp(){
    if(itr_next == wps.begin()){
      return **itr_next;
    }
    return **std::prev(itr_next);
  }
  
  void set_next_wp(){
    if (itr_next != wps.end()){
      inext++;
      itr_next++;
    }
  }
  
  void ins(float lat, float lon, float rarv, float v = 0.){
    s_wp * pwp = new s_wp(lat, lon, rarv, v);
    itr_focus = wps.insert(itr_focus, pwp);
    focus++;
    itr_focus++;
    
    find_next();
  }
  
  void ers(){
    if(wps.end() != itr_focus){
      delete *itr_focus;
      itr_focus = wps.erase(itr_focus);
    }
    if(itr_focus == wps.end())
      focus = (int) wps.size();
    
    find_next();
  }

  // reverse waypoint list
  void rev(){
    wps.reverse();
    set_focus(get_focus());
    find_next();
  }

  // refresh waypoint list (arrival flag is cleaned up)
  void ref(){ 
    for(itr = wps.begin(); itr != wps.end(); itr++)
      (*itr)->set_arrival_time(-1);
    find_next();
  }
  
  void set_focus(int i)
  {
    int j;
    for(j = 0, itr_focus = wps.begin(); itr_focus != wps.end() && j < i; j++, itr_focus++);
    focus = j;
  }
  
  int get_focus()
  {
    return focus;
  }
  
  int get_next()
  {
    return inext;
  }

  void next_focus()
  {
    if(itr_focus != wps.end())
      itr_focus++;
    
    if(itr_focus == wps.end()){
      focus = (int) wps.size();
    }else{
      focus++;
    }
  }
  
  void prev_focus()
  {
    if(itr_focus != wps.begin())
      itr_focus--;
    if(itr_focus == wps.begin())
      focus = 0;
    else
      focus--;
  }
  
  s_wp & get_focused_wp()
  {
    if (itr_focus == wps.end())
      return wp_null;
    return **itr_focus;
  }
  
  bool is_focused()
  {
    bool r = itr == itr_focus;
    return r;
  }
  
  s_wp & cur(){
    return **itr;
  }
  
  bool is_end(){
    return itr == wps.end();
  }
  
  bool is_begin(){
    bool r = itr == wps.begin();
    return r;
  }
  
  void  begin(){
    itr = wps.begin();
  }
  
  void end(){
    itr = wps.end();
  }
  
  s_wp & seek(int i){
    for(itr = wps.begin(); i!= 0 && itr != wps.end(); itr++, i--);
    s_wp & r = **itr;
    return r;
  }
  
  void next(){
    if(wps.end() != itr)
      itr++;
  }
  
  void prev(){
    if(wps.begin() != itr)
      itr--;
  }
  
  int get_num_wps(){
    int r = (int) wps.size();
    return r;
  }
};

#endif
