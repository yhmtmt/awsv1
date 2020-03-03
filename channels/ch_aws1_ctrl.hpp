#ifndef CH_AWS1_CTRL_HPP
#define CH_AWS1_CTRL_HPP
// Copyright(c) 2019-2020 Yohei Matsumoto, All right reserved. 

// ch_aws1_ctrl.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_aws1_ctrl.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_aws1_ctrl.hpp.  If not, see <http://www.gnu.org/licenses/>.

#include "channel_base.hpp"

// Control Source ID. control_aws1 can switch three control sources:
// ui_manager, autopilot, and filter parameters of control_aws1. 
enum e_aws1_ctrl_src{
  ACS_UI,  // ui_manager
  ACS_AP,  // autopilot
  ACS_FSET,// filter parameters of control_aws1
  ACS_NONE
};

// map a value with 3 threasholds (for rudder contrl and states)
inline  int map_oval(int val,
		     int vmax, int vnut, int vmin,
		     int omax, int onut, int omin
		     )
{
  int dvmax = val - vmax;
  int dvnut = val - vnut;
  int dvmin = val - vmin;
  int dvmax_vnut = vmax - vnut;
  int dvnut_vmin = vnut - vmin;
  
  if (abs(dvmax) <= abs(dvmax_vnut) && abs(dvnut) < abs(dvmax_vnut))
    return (int)((double)((omax - onut) * (dvnut)) / (double)(dvmax_vnut)) + onut;
  else if (abs(dvnut) <= abs(dvnut_vmin) && abs(dvmin) < abs(dvnut_vmin))
    return (int)((double)((onut - omin) * (dvnut)) / (double)(dvnut_vmin)) + onut;
  else if (abs(dvmax) < abs(dvmin))
    return omax;
  else
    return omin;
}

// map a value with 5 threasholds (for engines)
inline  int map_oval(int val,
		     int vmax, int vfnut, int vnut, int vbnut, int vmin,
		     int omax, int ofnut, int onut, int obnut, int omin
		     )
{
  int dvmax = val - vmax;
  int dvfnut = val - vfnut;
  int dvnut = val - vnut;
  int dvbnut = val - vbnut;
  int dvmin = val - vmin;
  int dvmax_vfnut = vmax - vfnut;
  int dvfnut_vnut = vfnut - vnut;
  int dvnut_vbnut = vnut - vbnut;
  int dvbnut_vmin = vbnut - vmin;
  
  if (abs(dvmax) <= abs(dvmax_vfnut) && abs(dvfnut) < abs(dvmax_vfnut))
    return (int)((double)((omax - ofnut) * (dvfnut)) / (double)(dvmax_vfnut)) + ofnut;
  else if (abs(dvfnut) <= abs(dvfnut_vnut) && abs(dvnut) < abs(dvfnut_vnut))
    return (int)((double)((ofnut - onut) * (dvnut)) / (double)(dvfnut_vnut)) + onut;
  else if (abs(dvnut) <= abs(dvnut_vbnut) && abs(dvbnut) < abs(dvnut_vbnut))
    return (int)((double)((onut - obnut) * (dvbnut)) / (double)(dvnut_vbnut)) + obnut;
  else if (abs(dvbnut) <= abs(dvbnut_vmin) && abs(dvmin) < abs(dvbnut_vmin))
    return (int)((double)((obnut - omin) * dvmin) / (double)(dvbnut_vmin)) + omin;
  else if (abs(dvmax) < abs(dvmin))
    return omax;
  else
    return omin;
}

extern const char * str_aws1_ctrl_src[ACS_NONE];

struct s_aws1_ctrl_inst{
 // current time
  long long tcur;
  
  // control source
  e_aws1_ctrl_src ctrl_src;
  
  // aws control input (from aws)
  unsigned char rud_aws;
  unsigned char eng_aws;
  
  s_aws1_ctrl_inst(): tcur(0), ctrl_src(ACS_UI), 
		      rud_aws(127), eng_aws(127)
  {
  }
};

struct s_aws1_ctrl_stat{
  // current time
  long long tcur;
  
  // control output
  unsigned char rud;
  unsigned char eng;
  
  // control source
  e_aws1_ctrl_src ctrl_src;
  
  // aws control input (from aws)
  unsigned char rud_aws;
  unsigned char eng_aws;  
 
  // Threashold values of digital potentiometer's
  unsigned char eng_max;
  unsigned char eng_nuf;
  unsigned char eng_nut;
  unsigned char eng_nub;
  unsigned char eng_min;

  unsigned char rud_max;
  unsigned char rud_nut;
  unsigned char rud_min;
  
  s_aws1_ctrl_stat():
  ctrl_src(ACS_UI),
  eng(0x7f), rud(0x7f),  
  eng_max(0x81),eng_nuf(0x80),  eng_nut(0x7f),  
  eng_nub(0x7e), eng_min(0x7d),  
  rud_max(0x80),  rud_nut(0x7f),  rud_min(0x7e)
  {
  }
};

class ch_aws1_ctrl_inst: public ch_base
{
protected:
  s_aws1_ctrl_inst inst;
  long long m_tfile;
public:
  ch_aws1_ctrl_inst(const char * name): ch_base(name), m_tfile(0)
  {
  }
  
  void set(const s_aws1_ctrl_inst & _inst){
    lock();
    inst = _inst;
    unlock();
  }
  
  void get(s_aws1_ctrl_inst & _inst){
    lock();
    _inst = inst;
    unlock();
  }
  
  virtual size_t get_dsize()
  {
    return sizeof(s_aws1_ctrl_inst);
  }
  
  virtual size_t write_buf(const char * buf)
  {
    lock();
    memcpy((void*)&inst, (void*) buf, sizeof(s_aws1_ctrl_inst));
    unlock();
    return get_dsize();
  }
  
  virtual size_t read_buf(char * buf)
  {
    lock();
    memcpy((void*) buf, (void*)&inst, sizeof(s_aws1_ctrl_inst));
    unlock();
    return get_dsize();
  }
  
  virtual void print(ostream & out)
  {
    out << "channel " << m_name 
	<< " rud " <<  (int) inst.rud_aws 
	<< " eng "<<  (int) inst.eng_aws 
	<< endl;
  }
  
  
  // file writer method
  virtual int write(FILE * pf, long long tcur)
  {
    if(m_tfile == inst.tcur){
      return 0;
    }
    
    int sz = 0;
    if(pf){
      lock();
      fwrite((void*)&inst.tcur, sizeof(long long), 1, pf);
      fwrite((void*)&inst.ctrl_src, sizeof(e_aws1_ctrl_src), 1, pf);
      fwrite((void*)&inst.rud_aws, sizeof(unsigned char), 1, pf);
      fwrite((void*)&inst.eng_aws, sizeof(unsigned char), 1, pf);
      sz = sizeof(long long) + sizeof(e_aws1_ctrl_src), sizeof(unsigned char) * 3;
      m_tfile = inst.tcur;
      unlock();
    }
    
    return sz;
  }
  
  // file reader method
  virtual int read(FILE * pf, long long tcur)
  {
    if(!pf)
      return 0;
    
    int sz = 0;
    while(m_tfile <= tcur && !feof(pf)){
      lock();
      size_t res;
      res = fread((void*)&inst.tcur, sizeof(long long), 1, pf);
      if(!res)
	goto eof;
      res = fread((void*)&inst.ctrl_src, sizeof(e_aws1_ctrl_src), 1, pf);
      if(!res)
	goto eof;
      res = fread((void*)&inst.rud_aws, sizeof(unsigned char), 1, pf);
      if(!res)
	goto eof;
      res = fread((void*)&inst.eng_aws, sizeof(unsigned char), 1, pf);
      if(!res)
	goto eof;
      sz = sizeof(long long) + sizeof(e_aws1_ctrl_src), sizeof(unsigned char) * 3;
      m_tfile = inst.tcur;
      unlock();			
    }
    return sz;
  eof:
    unlock();
    return 0;
  }
  
  virtual bool log2txt(FILE * pbf, FILE * ptf)
  {
    int sz = 0;
    fprintf(ptf, "t, acs, rud, eng\n");
    while(!feof(pbf)){
      size_t res;
      res = fread((void*)&inst.tcur, sizeof(long long), 1, pbf);
      if(!res)
	break;
      res = fread((void*)&inst.ctrl_src, sizeof(e_aws1_ctrl_src), 1, pbf);
      if(!res)
	break;
      res = fread((void*)&inst.rud_aws, sizeof(unsigned char), 1, pbf);
      if(!res)
	break;
      res = fread((void*)&inst.eng_aws, sizeof(unsigned char), 1, pbf);
      if(!res)
	break;
      sz = sizeof(long long) + sizeof(e_aws1_ctrl_src), sizeof(unsigned char) * 3;
      m_tfile = inst.tcur;
      
      fprintf(ptf, "%lld, %d, %d, %d\n", inst.tcur, 
	      (int)inst.ctrl_src, (int)inst.rud_aws, (int)inst.eng_aws);
    }
    return true;
  }
};

class ch_aws1_ctrl_stat: public ch_base
{
protected:
  s_aws1_ctrl_stat stat;
  long long m_tfile;
public:
  ch_aws1_ctrl_stat(const char * name): ch_base(name), m_tfile(0)
  {
  }
  
  void set(const s_aws1_ctrl_stat & _stat)
  {
    lock();
    stat = _stat;
    unlock();
  }
  
  void get(s_aws1_ctrl_stat & _stat)
  {
    lock();
    _stat = stat;
    unlock();
  }
  
  virtual size_t get_dsize()
  {
    return sizeof(s_aws1_ctrl_stat);
  }
  
  virtual size_t write_buf(const char * buf)
  {
    lock();
    memcpy((void*)&stat, (void*)buf, sizeof(s_aws1_ctrl_stat));
    unlock();
    return get_dsize();
  }
  
  virtual size_t read_buf(char * buf){
    lock();
    memcpy((void*)buf, (void*)&stat, sizeof(s_aws1_ctrl_stat));
    unlock();
    return get_dsize();
  }
  
  virtual void print(ostream & out)
  {
    out << "channel " << m_name << " rud " <<  (int) stat.rud  << " " 
	<<  (int) stat.rud_aws << " eng " <<  (int) stat.eng << " " << endl;
  }
  
  // file writer method
  virtual int write(FILE * pf, long long tcur)
  {
    if(m_tfile == stat.tcur){
      return 0;
    }
    
    int sz = 0;
    if(pf){
      lock();
      fwrite((void*)&stat, sizeof(stat), 1, pf);
      sz = sizeof(stat);
      m_tfile = stat.tcur;
      unlock();
    }
    
    return sz;
  }
  
  // file reader method
  virtual int read(FILE * pf, long long tcur)
  {
    if(!pf)
      return 0;
    
    int sz = 0;
    while(m_tfile <= tcur && !feof(pf)){
      lock();
      size_t res = fread((void*)&stat, sizeof(stat), 1, pf);
      if(!res)
	goto eof;
      sz += (int)res;
      m_tfile = stat.tcur;
      unlock();
    }
    return sz;
  eof:
    unlock();
    return sz;
  }
  
  virtual bool log2txt(FILE * pbf, FILE * ptf)
  {
    int sz = 0;
    fprintf(ptf, "t, rud, eng\n");
    while(!feof(pbf)){
      lock();
      size_t res = fread((void*)&stat, sizeof(stat), 1, pbf);
      if(!res)
	break;
      
      sz += (int)res;
      m_tfile = stat.tcur;
      fprintf(ptf, "%lld, %d, %d\n", stat.tcur, (int) stat.rud, (int) stat.eng);
      unlock();
    }
    return true;
  }
};


enum e_ap_mode{
    EAP_CURSOR, EAP_WP, EAP_WPAV, EAP_STAY, EAP_FLW_TGT, EAP_STB_MAN, EAP_NONE
};

extern const char * str_aws1_ap_mode[EAP_NONE];

class ch_aws1_ap_inst : public ch_base
{
protected:
  e_ap_mode mode;
  float smax; // maximum speed
  float eng_max, eng_min; // main engine max/min value
  
  // Stay position (used only in EAP_STAY)
  double lat_stay, lon_stay;
  double x_stay, y_stay, z_stay;
  double rx_stay, ry_stay, rz_stay;
  float d_stay, dir_stay;
  
  // Cursor position (used only in EAP_CURSOR)
  float lat_csr, lon_csr;
  float rx_csr, ry_csr;
  float d_csr, dir_csr;

  // Target position (used only in EAP_FLW_TGT)
  float lat_tgt, lon_tgt;
  float rx_tgt, ry_tgt;
  float d_tgt, dir_tgt;
  
  // Target course and Target rpm (used only in EAP_STB_MAN)
  float cog_tgt, rpm_tgt, sog_tgt;
  
  bool brpos;
public:
  ch_aws1_ap_inst(const char * name) :ch_base(name),
				      mode(EAP_WP), lat_stay(0.f), lon_stay(0.f), brpos(false)
  {
  }
  virtual ~ch_aws1_ap_inst()
  {
  }
  
  const e_ap_mode get_mode()
  {
    return mode;
  }
  
  void set_mode(const e_ap_mode _mode)
  {
    lock();
    mode = _mode;
    unlock();
  }
  
  void set_tgt_sog(const float sog)
  {
    lock();
    sog_tgt = sog;
    unlock();
  }
  
  void get_tgt_sog(float & sog)
  {
    lock();
    sog = sog_tgt;
    unlock();
  }
  
  void set_tgt_cog_and_rev(const float cog, const float rpm)
  {
    lock();
    cog_tgt = cog;
    rpm_tgt = rpm;
    unlock();
  }
  
  void get_tgt_cog_and_rev(float & cog, float & rpm)
  {
    lock();
    cog = cog_tgt;
    rpm = rpm_tgt;
    unlock();
  }
  void set_csr_pos(
		   const float lat, const float lon,
		   const float rx, const float ry, 
		   const float d, const float dir)
  {
    lock();
    lat_csr = lat;
    lon_csr = lon;
    rx_csr = rx;
    ry_csr = ry;
    d_csr = d;
    dir_csr = dir;
    unlock();
  }
  
  void get_csr_pos_rel(float & xr, float & yr, float & d, float & dir)
  {
    lock();
    xr = rx_csr;
    yr = ry_csr;
    d = d_csr;
    dir = dir_csr;
    unlock();
  }
  
  void set_stay_pos(const double lat, const double lon)
  {
    lock();
    lat_stay = lat * (PI / 180.f);
    lon_stay = lon * (PI / 180.f);
    blhtoecef(lat_stay, lon_stay, 0., x_stay, y_stay, z_stay);
    brpos = false;
    unlock();
  }
  
  void get_stay_pos(float & lat, float & lon)
  {
    lock();
    lat = lat_stay;
    lon = lon_stay;
    unlock();
  }
  
  bool get_stay_pos_rel(float & xr, float & yr, float & d, float & dir)
  {
    lock();
    xr = (float)rx_stay;
    yr = (float)ry_stay;
    d = d_stay;
    dir = dir_stay;
    unlock();
    return brpos;
  }
  
  void set_tgt_pos(const float lat, const float lon, const float xr, const float yr, const float d, const float dir)
  {
    lock();
    lat_tgt = lat;
    lon_tgt = lon;
    rx_tgt = xr;
    ry_tgt = yr;
    d_tgt = d;
    dir_tgt = dir;
    unlock();
  }
  
  bool get_tgt_pos_rel(float & xr, float & yr, float & d, float & dir)
  {
    lock();
    xr = rx_tgt;
    yr = ry_tgt;
    d = d_tgt;
    dir = dir_tgt;
    unlock();
    return brpos;
  }
  
  void update_pos_rel(const double * Rorg, double & xorg, double & yorg, double & zorg)
  {
    lock();
    eceftowrld(Rorg, xorg, yorg, zorg, x_stay, y_stay, z_stay, rx_stay, ry_stay, rz_stay);
    d_stay = (float)(sqrt(rx_stay * rx_stay + ry_stay * ry_stay));
    dir_stay = (float)(atan2(rx_stay, ry_stay) * 180. / PI);
    brpos = true;
    unlock();
  }
};

#endif
