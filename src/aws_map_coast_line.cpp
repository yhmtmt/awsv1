// Copyright(c) 2017-2020 Yohei Matsumoto, All right reserved. 

// aws_map_coast_line.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_map_coast_line.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_map_coast_line.cpp.  If not, see <http://www.gnu.org/licenses/>. 


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
  
  /////////////////////////////////////////////////////////////////// CoastLine
  const vector<vec3> CoastLine::null_vec_vec3;
  const vector<vec2> CoastLine::null_vec_vec2;
  
  CoastLine::CoastLine() :dist_min(FLT_MAX), total_size(0)
  {
  }
  
  CoastLine::~CoastLine()
  {
  }

  bool CoastLine::save(ofstream & ofile)
  {
    unsigned int nlines = (unsigned int) lines.size();
    ofile.write((const char*)&nlines, sizeof(unsigned int));
    for (auto itr = lines.begin(); itr != lines.end(); itr++){
      vector<vec2> & pts = (*itr)->pts;
      unsigned int length = (unsigned int) pts.size();
      ofile.write((const char*)&length, sizeof(unsigned int));
      for (auto itr_pt = pts.begin(); itr_pt != pts.end(); itr_pt++){	
	vec2 pt = *itr_pt;
	ofile.write((const char*)(&pt), sizeof(vec2));
      }
    }
    return true;
  }

  bool CoastLine::load(ifstream & ifile)
  {
    unsigned int nlines = 0;
    ifile.read((char*)&nlines, sizeof(unsigned int));
    lines.resize(nlines);
    
    for (auto itr = lines.begin(); itr != lines.end(); itr++){
      (*itr) = new s_line;
      
      vector<vec2> & pts = (*itr)->pts;
      vector<vec3> & pts_ecef = (*itr)->pts_ecef;
      unsigned int length;
      ifile.read((char*)&length, sizeof(unsigned int));
      pts.resize(length);
      pts_ecef.resize(length);
      auto itr_pt_ecef = pts_ecef.begin();
      for (auto itr_pt = pts.begin(); itr_pt != pts.end(); itr_pt++, itr_pt_ecef++){
	vec2 pt;
	vec3 pt_ecef;
	ifile.read((char*)(&pt), sizeof(vec2));
	blhtoecef(pt.lat, pt.lon, 0, pt_ecef.x, pt_ecef.y, pt_ecef.z);
	(*itr_pt_ecef) = pt_ecef;
	(*itr_pt) = pt;
      }
    }
    
    update_properties();
    
    return true;
  }
  
  void CoastLine::print() const
  {
    char path[2048] = "Not Active";
    if(pNode)
      pNode->getPath(path, 2048);
    cout << "CoastLine:" << path << endl;
    cout << "\t pos:" << pt_center_blh.lat * 180. / PI << "," << pt_center_blh.lon * 180. / PI
	 << " radius:" << radius()
	 << " mindim:" << dist_min
	 << " lines:" << lines.size() 
	 << " size:" << size() << endl;
  }
  
  void CoastLine::_release()
  {
    for (auto itr = lines.begin(); itr != lines.end(); itr++)
      {
	delete (*itr);
      }
    lines.clear();
  }
  
  size_t CoastLine::size() const
  {
    return total_size;
  }
  
  float CoastLine::resolution() const
  {
    return dist_min;
  }
  
  float CoastLine::radius() const
  {
    return pt_radius;
  }
  
  vec3 CoastLine::center() const
  {
    return pt_center;
  }
  
  bool CoastLine::split(list<Node*> & nodes, Node * pParentNode) const
  {
    vector<CoastLine> cls(nodes.size());
    vector<Node*> vnodes(nodes.size()); // vector version of nodes
    {
      int inode = 0;
      for (auto itr = nodes.begin(); itr != nodes.end(); itr++, inode++) {
	vnodes[inode] = *itr;
      }
    }
    
    for (int iline = 0; iline < lines.size(); iline++) {
      vector<vec3> & pts = lines[iline]->pts_ecef;
      // first find correspondance between points in the line and nodes.
      
      vector<char> asgnc(pts.size(), -1);
      if (pParentNode) {
	pParentNode->collision_downlink(pts, asgnc);
      }
      else {
	for (int ipt = 0; ipt < pts.size() - 1; ipt++) {
	  int inode = 0;
	  for (auto itr = nodes.begin(); itr != nodes.end(); itr++) {
	    Node * pNode = *itr;
	    if (pNode->collision(pts[ipt])) {
	      asgnc[ipt] = inode;
	      break;
	    }
	    inode++;
	  }
	}
      }
      asgnc.back() = -1;
      
      // second split line into lines, while doubling boundary points not to miss links between nodes
      int in = asgnc[0], inn = -1; // node index "in" and next node index "inn"
      int ipts = 0, ipte = 0; // start point index "ipts" and end point index "ipte".
      list<vec2> line_new; // newly added line partially extracted from lines[iline]
      for (int ipt = 1; ipt < pts.size(); ipt++) {
	inn = asgnc[ipt];
	
	if (in != inn) {
	  ipte = ipt + 1; // this "+1" allows to make overwrap on next part.
	  
	  // form new line contains points ipts to ipte.
	  for (int iptl = ipts; iptl < ipte; iptl++) {
	    line_new.push_back(lines[iline]->pts[iptl]);
	  }
	  
	  cls[in].add(line_new);
	  line_new.clear();
	  in = inn;
	  ipts = ipt;
	  continue;
	}
      }
    }
    
    for (int inode = 0; inode < nodes.size(); inode++) {
      cls[inode].update_properties();
#ifdef _AWS_MAP_DEBUG
      cout << "Adding layer data: " << endl;
      cls[inode].print();
      cout << " \tsize: " << cls[inode].size() << endl;
#endif
      if (cls[inode].size() > 0)
	vnodes[inode]->addLayerData(cls[inode]);
    }
    
    return true;
  }


  int CoastLine::try_reduce(int nred)
  {
    // select nred points with shortest distance to both sides in the line (excluding terminal points)
    struct s_red_pt{
      int iline;
      int ipt;
      double dist;
      s_red_pt * prev, *next;
      s_red_pt() :iline(-1), ipt(-1), dist(DBL_MAX), prev(NULL), next(NULL){};
    };
  
    // creating distance data 
    int npts_list = 0;
    vector<s_red_pt*> redpts(lines.size());
  
    for (int iline = 0; iline < lines.size(); iline++){
      redpts[iline] = NULL;
      vector<vec3> & pts = lines[iline]->pts_ecef;
      if(pts.size() > 3)
	npts_list += (int)pts.size() - 3;
    }
  
    // creating sort list
    vector<s_red_pt> mblock(npts_list);
    vector<s_red_pt*> sortlist(npts_list, NULL);
    npts_list = 0;
    for (int iline = 0; iline < lines.size(); iline++){
      if (lines[iline]->pts.size() <= 3) {
	vec2 &pt0 = lines[iline]->pts.front();
	vec2 &pt1 = lines[iline]->pts.back();
	if (pt0.x == pt1.x && pt0.y == pt1.y) {
	  delete lines[iline];
	  lines[iline] = NULL;
	}
	continue;
      }
      vector<vec3> & pts = lines[iline]->pts_ecef;
      s_red_pt * redpt = NULL, * redpt_prev = NULL;
      for (int ipt = 2; ipt < pts.size() - 1; ipt++){
	redpt = &mblock[npts_list];
	redpt->iline = iline;
	redpt->ipt = ipt;
	redpt->dist = l2Norm(pts[ipt], pts[ipt - 1]);
	sortlist[npts_list] = redpt;
	if (!redpt_prev) {
	  redpts[iline] = redpt;
	}
	else {
	  redpt_prev->next = redpt;
	  redpt->prev = redpt_prev;
	}
	redpt_prev = redpt;
	npts_list++;
      }
    }

    struct {
      bool operator () (const s_red_pt * p0, const s_red_pt * p1) const
      {
	return p0->dist < p1->dist;
      }
    } lessthan;
  
    sort(sortlist.begin(), sortlist.end(), lessthan);
  
    // reduction phase
#ifdef _AWS_MAP_DEBUG
    int nredd = 0;
    int prog = 0;	
    cout << "Progress(";
    cout << nred << "/" << sortlist.size() << "):";
#endif
    while (nredd < nred && nredd < sortlist.size()){
#ifdef _AWS_MAP_DEBUG
      if ((nredd * 10 / nred) > prog) {
	prog ++;
	cout << "*";
      }
#endif
      s_red_pt * redpt = sortlist[nredd];
      s_red_pt * redpt0 = redpt->prev;
      s_red_pt * redpt1 = redpt->next;
      int iline = redpt->iline;
      int ipt = redpt->ipt;
      int ipt0 = (redpt0 ? redpt0->ipt : 1);
      int ipt1 = (redpt1 ? redpt1->ipt : lines[iline]->pts.size() - 1);
    
      s_line & line = *lines[iline];
      vector<vec2> & pts = line.pts;
      vector<vec3> & pts_ecef = line.pts_ecef;
      vec2 pt_m;
      vec3 pt_ecef_m;
      pt_ecef_m = (pts_ecef[ipt0] + pts_ecef[ipt]);
      pt_ecef_m *= 0.5;
      double alt;
      eceftoblh(pt_ecef_m.x, pt_ecef_m.y, pt_ecef_m.z, pt_m.lat, pt_m.lon, alt);
    
      if (redpt0){
	int iptm1 = (redpt0->prev ? redpt0->prev->ipt : 1);
	redpt0->dist = l2Norm(pt_ecef_m, pts_ecef[iptm1]);
	redpt0->next = redpt1;
      }
      else {
	redpts[iline] = redpt1;
      }
    
      if (redpt1){
	redpt1->dist = l2Norm(pt_ecef_m, pts_ecef[ipt1]);
	redpt1->prev = redpt0;
      }
      pts[ipt0] = pt_m;
      pts_ecef[ipt0] = pt_ecef_m;
    
      if (redpts[iline] == NULL) { // points has already been 3
	vec2 & pt0 = pts.front();
	vec2 & pt1 = pts.back();
	if (pt0.x == pt1.x && pt0.y == pt1.y) {
	  delete lines[iline];
	  lines[iline] = NULL;
	  nredd += 3;
	}
      }
    
      nredd++;
    
      sort(sortlist.begin() + nredd, sortlist.end(), lessthan);
      if (sortlist.size() == 0)
	break;
    }
#ifdef _AWS_MAP_DEBUG
    cout << endl;
    if (nredd < nred) {
      cout << "Reduction is not completed because of many fragment" << endl;
    }
#endif
    for (int iline = 0; iline < lines.size(); iline++) {
      if (!lines[iline])
	continue;
    
      vector<vec2> & pts = lines[iline]->pts;
      vector<vec3> & pts_ecef = lines[iline]->pts_ecef;
    
      if (pts.size() < 3)
	continue;
    
      s_red_pt * redpt = redpts[iline];
      int ipt = 2;
      for (; redpt != NULL; redpt = redpt->next, ipt++){
	pts[ipt] = pts[redpt->ipt];
	pts_ecef[ipt] = pts_ecef[redpt->ipt];
      }
      pts[ipt] = pts.back();
      pts_ecef[ipt] = pts_ecef.back();
      pts.resize(ipt + 1);
      pts_ecef.resize(ipt + 1);
    }
  
    update_properties();
  
    return nred - nredd;
  }

  bool CoastLine::_reduce(const size_t sz_lim)
  {
    bupdate = true;
    if (size() < sz_lim)
      return true;
  
    // calculate nred; the number of points to be reduced
    unsigned int sz_pts_lim = (unsigned int)(sz_lim);	
    unsigned int sz_pt = (unsigned int)(sizeof(vec2)+sizeof(vec3));
    unsigned int npts = 0;
    for (unsigned int iline = 0; iline < lines.size(); iline++){
      npts += (unsigned int) lines[iline]->pts.size();
    }
  
    unsigned int nred = npts - (sz_pts_lim / sz_pt);
    if (try_reduce(nred) != 0){
      return false;
    }
    return true;
  }

  bool CoastLine::_merge(const LayerData & layerData)
  {
#ifdef _AWS_MAP_DEBUG
    print();
#endif
    bupdate = true;
    const CoastLine * src = dynamic_cast<const CoastLine*>(&layerData);
    const vector<s_line*> & lines_src = src->lines;
	
    // this loop finds connection between lines in layerData and this object.
    for (int iline0 = 0; iline0 < lines_src.size(); iline0++){
    
      s_line & line_src = *lines_src[iline0];
      vector<vec2> & pts_src = line_src.pts;
      vec2 & pt_src_begin = pts_src.front() , & pt_src_end = pts_src.back();
    
      // iline_con_begin is the line index of existing line connected with newly added line lines_src[iline0]
      // and bdst_begin_con_begin is true if lines[iline_con_begin] and lines_src[iline0] is connected with their starting points.
      // iline_con_end is the line index of existing line connected with newly added line lines_src[iline0]
      // and bdst_begin_con_end is true if the start point of lines[iline_con_end] is connected with the end point of lines_src[iline0]
      int iline_con_begin = -1, iline_con_end = -1; 
      bool bdst_begin_con_begin = true, bdst_begin_con_end = true;
      // check connection
      for (int iline1 = 0; iline1 < lines.size(); iline1++){
	if (!lines[iline1])
	  continue;
	s_line & line_dst = *lines[iline1];
	vector<vec2> & pts_dst = line_dst.pts;
	vec2 & pt_dst_begin = pts_dst.front(), & pt_dst_end = pts_dst.back();
	if (iline_con_begin < 0) {
	  if (pt_src_begin == pt_dst_begin) {
	    iline_con_begin = iline1;
	    bdst_begin_con_begin = true;
	  }
	  else if (pt_src_begin == pt_dst_end) {
	    iline_con_begin = iline1;
	    bdst_begin_con_begin = false;
	  }
	  else {
	    iline_con_begin = -1;
	  }
	}
	if (iline_con_end < 0) {
	  if (pt_src_end == pt_dst_begin) {
	    iline_con_end = iline1;
	    bdst_begin_con_end = true;
	  }
	  else if (pt_src_end == pt_dst_end) {
	    iline_con_end = iline1;
	    bdst_begin_con_end = false;
	  }
	  else {
	    iline_con_end = -1;
	  }
	}
      
	if (iline_con_end > 0 && iline_con_begin > 0)
	  break;
      }
    
      s_line * pline_begin = NULL, *pline_end = NULL, *pline_new;
    
      if (iline_con_begin >= 0){
	pline_new = new s_line;
	// the starting point of lines_src[iline0] is connected with lines[iline_con_begin]
	pline_begin = lines[iline_con_begin];
	if (bdst_begin_con_begin){
	  // if the connection is head to head, first reverse the lines[iline_con_begin]
	  reverse(pline_begin->pts.begin(), pline_begin->pts.end());
	  reverse(pline_begin->pts_ecef.begin(), pline_begin->pts_ecef.end());
	}
	// copy lines[iline_con_begin] to newly created object *pline_new
	*pline_new = *pline_begin;
	assert(pline_new->pts.back() == pts_src.front());
	pline_new->pts.pop_back();
	pline_new->pts_ecef.pop_back();
	// insert lines_src[iline0] at the end of pline_new 
	// note that the end point is exactly the point of lines_src[iline0]. This is very important later if the end point is connected with other line.
	pline_new->pts.insert(pline_new->pts.end(), pts_src.begin(), pts_src.end());
	pline_new->pts_ecef.insert(pline_new->pts_ecef.end(), line_src.pts_ecef.begin(),
				   line_src.pts_ecef.end());
      
	// delete old object lines[iline_con_begin] and replace it with pline_new
	delete lines[iline_con_begin];
	lines[iline_con_begin] = pline_new;
      }
    
      if (iline_con_end >= 0){
	// the end point of lines_src[iline0] is connected with lines[iline_con_end]
	pline_end = lines[iline_con_end];
	vector<vec2> & pts = pline_end->pts;
	vector<vec3> & pts_ecef = pline_end->pts_ecef;
      
	if (!bdst_begin_con_end){
	  // if lines_src[iline0] is connected with the end point of lines[iline_con_end] reverse it.
	  reverse(pts.begin(), pts.end());
	  reverse(pts_ecef.begin(), pts_ecef.end());
	}
      
	if (iline_con_begin >= 0) {
	  // if previously start point of lines_src[iline0] is connected with lines[iline_con_begin], 
	  // the end point of lines_src[iline0] is reserved as that of lines[iline_con_begin]
	  pline_new = lines[iline_con_begin];
	  lines[iline_con_begin] = NULL;
	
	}else {
	  pline_new = new s_line;
	  *pline_new = line_src;
	}
      
	assert(pline_new->pts.back() == pts.front());
	pline_new->pts.pop_back();
	pline_new->pts_ecef.pop_back();
	pline_new->pts.insert(pline_new->pts.end(), pts.begin()+1, pts.end());
	pline_new->pts_ecef.insert(pline_new->pts_ecef.end(), pts_ecef.begin()+1, pts_ecef.end());
	delete lines[iline_con_end];
	lines[iline_con_end] = pline_new;
      }
    
      if (iline_con_begin < 0 && iline_con_end < 0) {
	pline_new = new s_line;
	*pline_new = line_src;
	lines.push_back(pline_new);
      }
    }

    // erase null element from lines
    for (vector<s_line*>::iterator itr = lines.begin(); itr != lines.end();){
      if (*itr == NULL)
	itr = lines.erase(itr);
      else
	itr++;
    }
  
    update_properties();
#ifdef _AWS_MAP_DEBUG
    print();
#endif
    return true;
  }

  bool CoastLine::_remove(const unsigned int id)
  {
    if(id >= lines.size())
      return false;
    
    delete lines[id];
    
    lines.erase(lines.begin() + id);
    
    update_properties();
    return true;
  }
  
  LayerData * CoastLine::clone() const
  {
    CoastLine * pnew = new CoastLine;
  
    pnew->pNode = NULL;
    pnew->bupdate = bupdate;
    pnew->dist_min = dist_min;
    pnew->total_size = total_size;
  
    vector<s_line*> & lines_new = pnew->lines;
    lines_new.resize(lines.size());
    auto itr_new = lines_new.begin();
    for (auto itr = lines.begin(); itr != lines.end(); itr++, itr_new++){
      *itr_new = new s_line;
      *(*itr_new) = *(*itr);
    }
  
    return pnew;
  }
  
  void CoastLine::add(list<vec2> & line)
  {
    s_line * pline = new s_line;
    
    pline->pts.resize(line.size());
    pline->pts_ecef.resize(line.size());
    list<vec2>::iterator itr_src = line.begin();
    vector<vec2>::iterator itr_dst = pline->pts.begin();
    vector<vec3>::iterator itr_dst_ecef = pline->pts_ecef.begin();
    for (; itr_src != line.end(); itr_src++, itr_dst++, itr_dst_ecef++) {
      *itr_dst = *itr_src;
      blhtoecef(itr_dst->lat, itr_dst->lon, 0., itr_dst_ecef->x, itr_dst_ecef->y, itr_dst_ecef->z);
    }
    lines.push_back(pline);
    
    // calculating resolution and size
    vector<vec3> & pts = pline->pts_ecef;
    for (int i = 1; i < pts.size(); i++) {
      vec3 & pt0 = pts[i - 1];
      vec3 & pt1 = pts[i];
      dist_min = min(dist_min, l2Norm(pt0, pt1));
    }
    total_size += pline->size();
  }
  
  void CoastLine::update_properties()
  {
    // removing null line
    for (auto itr = lines.begin(); itr != lines.end();) {
      if (*itr == NULL)
	itr = lines.erase(itr);
      else
	itr++;
    }
    
    // calculating center 	
    pt_center = vec3(0, 0, 0);
    unsigned int num_total_points = 0;
    for (int iline = 0; iline < lines.size(); iline++){
      vector<vec3> & pts = lines[iline]->pts_ecef;
      for (int i = 0; i < pts.size(); i++){
	pt_center += pts[i];
      }
      num_total_points += (unsigned int)pts.size();
    }
    pt_center *= (1.0 / (double)num_total_points);
    double alt;
    eceftoblh(pt_center.x, pt_center.y, pt_center.z, pt_center_blh.lat, pt_center_blh.lon, alt);
    
    
    total_size = 0;
    pt_radius = 0;
    dist_min = DBL_MAX;
    for (int iline = 0; iline < lines.size(); iline++){
      // calculating resolution and size
      vector<vec3> & pts = lines[iline]->pts_ecef;
      pt_radius = max(pt_radius, l2Norm(pt_center, pts[0]));
      for (int i = 2; i < pts.size()-1; i++) {
	vec3 & pt0 = pts[i - 1];
	vec3 & pt1 = pts[i];
	double dist = l2Norm(pt0, pt1);
	if (dist == 0) {
	  pts.erase(pts.begin() + i);
	  i--;
	  continue;
	}
	dist_min = min(dist_min, dist);
	pt_radius = max(pt_radius, l2Norm(pt_center, pts[i]));
      }
      total_size += lines[iline]->size();
    }
  }
  
  bool CoastLine::loadJPJIS(const char * fname)
  {
    ifstream fjpgis(fname);
    if (!fjpgis.is_open()) {
      cerr << "Failed to open file " << fname << "." << endl;
      return false;
    }
    
    char buf[1024];
    bool bline = false;
    list<vec2> line;
    while (!fjpgis.eof())
      {
	fjpgis.getline(buf, 1024);
	
	if (!bline) {
	  char * p = buf;
	  for (; *p != '<' && *p != '\0'; p++);
	  
	  if (strcmp(p, "<gml:posList>") == 0) {
	    bline = true;
	  }
	}
	else {
	  char * p = buf;
	  for (; *p != '<' && *p != '\0'; p++);
	  if (strcmp(buf, "</gml:posList>") == 0) {
	    add(line);
	    line.clear();
	    bline = false;
	    continue;
	  }
	  
	  vec2 pt;
	  for (p = buf; *p != ' ' && *p != '\0'; p++);
	  *p = '\0';
	  p++;
	  pt.x = (float)(atof(buf) * PI / 180.);
	  pt.y = (float)(atof(p) * PI / 180.);
	  
	  if (line.size() == 0 || (pt.x != line.back().x || pt.y != line.back().y))
	    line.push_back(pt);
	}
      }
    
    update_properties();
    return true;
  }
}
