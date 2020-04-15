#ifndef CH_OBJ_HPP
#define CH_OBJ_HPP
// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_obj.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_obj.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_obj.h.  If not, see <http://www.gnu.org/licenses/>.

#include "channel_base.hpp"
#include "aws_coord.hpp"

// Object source (Defines how the object is detected.)
enum e_obj_src
{
  EOS_UNDEF = 0, EOS_VMAN=0x1, EOS_VAUTO=0x2, EOS_AIS=0x4, EOS_SV=0x8
};

// Object state (How the object is treated in the filters observing)
enum e_obj_trck_state
{
  EOST_UNDEF = 0, EOST_TRCK=0x1, EOST_DET=0x2, EOST_LOST=0x4, EOST_NOINT=0x8
};

// Object type (The type of the object, corresponding to the sub class)
enum e_obj_type
{
	EOT_UNKNOWN = 0, EOT_SHIP=0x1, EOT_OBST=0x2
};

// Data related to the object
enum e_obj_data_type
{
  EOD_UNDEF=0, EOD_IMG=0x1, EOD_IMRECT=0x2, EOD_MDL3D=0x4, 
  EOD_POS_BLH=0x8, EOD_VEL_BLH = 0x10, 
  EOD_POS_ECEF=0x20, EOD_VEL_ECEF = 0x40,
  EOD_POS_REL=0x80, EOD_VEL_REL = 0x100,
  EOD_ATTD=0x200, EOD_AIS=0x400, EOD_R = 0x800, EOD_POS_BD = 0x1000,
  EOD_TDCPA=0x2000
};

// The base class of the objects in aws
class c_obj
{
protected:
  e_obj_type m_type;			// Type of the object
  e_obj_src m_src;			// Source sensor of the object information
  e_obj_trck_state m_tst;		// Tracking status.
  e_obj_data_type m_dtype;	// Data types the object hold
  
  int m_id_track;				// tracking id
  
  long long m_t;				// Time updated.
  double m_lat, m_lon, m_alt;	// BLH coordinate
  float m_cog, m_sog;			// Velocity in BLH coordinate (knots, degree)
  double m_x, m_y, m_z;		// ECEF  coordinate (meter)
  float m_vx, m_vy, m_vz;		// Velocity in ECEF (meter / sec)
  double m_xr, m_yr, m_zr;		// Relative orthogonal coordinate (centered at my own ship, meter)
  float m_vxr, m_vyr, m_vzr;	// velocity in orthogonal coordinate centered at my own ship (meter per sec)
  float m_vrx, m_vry;			// relative velocity in relative coordinate (meter / sec)
  float m_nvxr, m_nvyr;		// normalized relative velocity vector in relative coordinate
  float m_nhdgx, m_nhdgy;		// heading vector
  float m_bear, m_dist;		// bearing and distance
  float m_roll, m_pitch, m_yaw; // The attitude Roll pitch yaw
  
  float m_tcpa, m_dcpa;
  float m_s0, m_dsdt;			// standard deviation of initial fix, and for prediction  
public:
  c_obj();
  virtual ~c_obj();
  
  virtual void print(ostream & out)
  {
  }
  
  void set_tracking_id(const int _id)
  {
    m_id_track = _id;
  }
  
  const int get_tracking_id()
  {
    return m_id_track;
  }
  
  void set(const e_obj_type & type){
    m_type = type;
  }
  
  const e_obj_type & get_type(){
    return m_type;
  }
  
  void set(e_obj_src & src){
    m_src = src;
  }
  
  const e_obj_src & get_src(){
    return m_src;
  }
  
  void set(const e_obj_trck_state & tst){
    m_tst = tst;
  }
  
  const e_obj_trck_state & get_tst(){
    return m_tst;
  }
  
  void set(const e_obj_data_type & dtype){
    m_dtype = dtype;
  }
  
  const e_obj_data_type  & get_dtype(){
    return m_dtype;
  }
  
  void set_time(const long long t){
    m_t = t;
  }
  
  const long long get_time(){
    return m_t;
  }
  
  // Position and velocity in BLH coordinate.
  void set_pos_blh(const double lat, const double lon, const double alt){
    m_lat = lat;
    m_lon = lon;
    m_alt = alt;
    m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_BLH);
  }
  
  void set_blh_from_ecef(){
    m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_BLH);
    eceftoblh(m_x, m_y, m_z, m_lat, m_lon, m_alt);	
    m_lat *= (float)(180. / PI);
    m_lon *= (float)(180. / PI);
  }
  
  void reset_blh(){
    m_dtype = (e_obj_data_type)(m_dtype & ~EOD_POS_BLH & ~EOD_VEL_BLH);
  }
  
  bool get_pos_blh(double & lat, double & lon, double & alt){
    lat = m_lat;
    lon = m_lon;
    alt = m_alt;
    return (m_dtype & EOD_POS_BLH) != 0;
  }
  
  void set_vel_blh(const float cog, const float sog){
    m_cog = cog;
    m_sog = sog;
    float theta = (float)(m_cog * (PI / 180.));
    float c = cos(theta);
    float s = sin(theta);
    m_nvxr = s;
    m_nvyr = c;
    m_dtype = (e_obj_data_type)(m_dtype | EOD_VEL_BLH);
  }
  
  bool get_vel_blh(float & cog, float & sog){
    cog = m_cog;
    sog = m_sog;
    return (m_dtype & EOD_VEL_BLH) != 0;
  }
  
  // Position and velocity in ECEF coordinate.
  void set_pos_ecef(const double x, const double y, const double z){
    m_x = x;
    m_y = y;
    m_z = z;
    m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_ECEF);
  }
  
  void set_ecef_from_blh()
  {
    m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_ECEF);
    blhtoecef(m_lat * (PI/180.), m_lon * (PI/180.), m_alt, m_x, m_y, m_z);
  }
  
  void set_ecef_from_rel(const double * Rorg,
			 const double xorg, const double yorg,
			 const double zorg)
  {
    if(m_dtype & EOD_POS_REL){
      wrldtoecef(Rorg, xorg, yorg, zorg, m_xr, m_yr, m_zr, m_x, m_y, m_z);
      m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_ECEF);
    }
  }
  
  void reset_ecef(){
    m_dtype = (e_obj_data_type)(m_dtype & ~EOD_POS_ECEF & ~EOD_VEL_ECEF);
  }
  
  bool get_pos_ecef(double & x, double & y, double & z){
    x = m_x;
    y = m_y;
    z = m_z;
    return (m_dtype & EOD_POS_ECEF) != 0;
  }
  
  void set_vel_ecef(const float vx, const float vy, const float vz){
    m_vx = vx;
    m_vy = vy;
    m_vz = vz;
    m_dtype = (e_obj_data_type)(m_dtype | EOD_VEL_ECEF);
  }
  
  void set_vel_ecef_from_blh(const double * Rorg)
  {
    if(m_dtype & EOD_POS_BLH){	
      float v = (float)(m_sog * KNOT);
      float c = m_nvyr, s = m_nvxr;
      m_vx = (float)(s * Rorg[0] + c * Rorg[3]);
      m_vy = (float)(s * Rorg[1] + c * Rorg[4]);
      m_vz = (float)(s * Rorg[2] + c * Rorg[5]);
      m_vx *= v;
      m_vy *= v;
      m_vz *= v;
      m_dtype = (e_obj_data_type)(m_dtype | EOD_VEL_ECEF);
    }
  }
  
  bool get_vel_ecef(float & vx, float & vy, float & vz){
    vx = m_vx;
    vy = m_vy;
    vz = m_vz;
    return (m_dtype & EOD_VEL_ECEF) != 0;
  }
  
  // For position and velocity in relative coordinate to my own ship.
  void set_pos_rel(const float xr, const float yr, const float zr){
    m_xr = xr;
    m_yr = yr;
    m_zr = zr;
    m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_REL);
  }
  
  void set_pos_rel_from_ecef(const double * Rorg, float xorg, float yorg, float zorg)
  {
    if(m_dtype & EOD_POS_ECEF){
      eceftowrld(Rorg, xorg, yorg, zorg, m_x, m_y, m_z, m_xr, m_yr, m_zr);
      m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_REL);	
    }
  }
  
  void reset_rel(){
    m_dtype = (e_obj_data_type)(m_dtype & ~EOD_POS_REL & ~EOD_VEL_REL);
  }
  
  bool get_pos_rel(double & xr, double & yr, double & zr){
    xr = m_xr;
    yr = m_yr;
    zr = m_zr;
    return (m_dtype & EOD_POS_REL) != 0;
  }
  
  void set_vel_rel(const float & vxr, const float & vyr, const float & vzr){
    m_vxr = vxr;
    m_vyr = vyr;
    m_vzr = vzr;
    m_dtype = (e_obj_data_type)(m_dtype | EOD_VEL_REL);
  }
  
  bool get_vel_vec2d(float & nvxr, float &nvyr){
    nvxr = m_nvxr;
    nvyr = m_nvyr;
    return (m_dtype & EOD_VEL_BLH) != 0;
  }
  
  void set_vel_rel_from_ecef(const double * Rorg)
  {
    if(m_dtype & EOD_VEL_ECEF){
      m_vxr = (float)(m_vx * Rorg[0] + m_vy * Rorg[1] + m_vz * Rorg[2]);
      m_vyr = (float)(m_vx * Rorg[3] + m_vy * Rorg[4] + m_vz * Rorg[5]);
      m_vzr = (float)(m_vx * Rorg[6] + m_vy * Rorg[7] + m_vz * Rorg[8]);
      m_dtype = (e_obj_data_type)(m_dtype | EOD_VEL_REL);
    }
  }
  
  void set_vel_rel_from_blh()
  {
    if(m_dtype & EOD_VEL_BLH){
      float v = (float)(m_sog * KNOT);
      m_vxr = (float) (v * m_nvxr);
      m_vyr = (float) (v * m_nvyr);
      m_vzr = 0.f;
      m_dtype = (e_obj_data_type)(m_dtype | EOD_VEL_REL);
    }
  }
  
  bool get_vel_rel(float & vxr, float & vyr, float & vzr){
    vxr = m_vxr;
    vyr = m_vyr;
    vzr = m_vzr;
    return (m_dtype & EOD_VEL_REL) != 0;
  }
  
  void set_att(const float roll, const float pitch, const float yaw)
  {
    m_roll = roll;
    m_pitch = pitch;
    m_yaw = yaw;
    float th = (float)(m_yaw * (PI / 180.));
    
    m_nhdgx = (float)sin(th);
    m_nhdgy = (float)cos(th);
    
    m_dtype = (e_obj_data_type)(m_dtype | EOD_ATTD);
  }
  
  void reset_att()
  {
    m_dtype = (e_obj_data_type)(m_dtype & ~EOD_ATTD);
  }
  
  bool get_att(float & roll, float & pitch, float & yaw)
  {
    roll = m_roll;
    pitch = m_pitch;
    yaw = m_yaw;
    return (m_dtype & EOD_ATTD) != 0;
  }
  
  bool get_hdg_vec2d(float & nhdgx, float & nhdgy)
  {
    nhdgx = m_nhdgx;
    nhdgy = m_nhdgy;
    return (m_dtype & EOD_ATTD) != 0;
  }
  
  void set_pos_bd(float bear, float dist)
  {
    m_bear = bear;
    m_dist = dist;
    m_dtype = EOD_POS_BD;
  }
  
  void set_pos_rel_from_bd()
  {
    double th = (m_bear * (PI / 180.));
    double c = cos(th), s = sin(th);
    m_xr = (float) (m_dist * s); 
    m_yr = (float)(m_dist * c);
    m_zr = 0;
    m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_REL);
  }
  
  void set_pos_bd_from_rel()
  {
    m_bear = atan2(m_xr, m_yr);
    m_dist = sqrt(m_xr * m_xr + m_yr * m_yr);
    m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_BD);
  }
  
  bool get_pos_bd(float & bear, float & dist)
  {
    bear = m_bear;
    dist = m_dist;
    return (m_dtype & EOD_POS_BD) != 0;
  }
  
  void reset_bd()
  {
    m_dtype = (e_obj_data_type)(m_dtype & ~EOD_POS_BD);
  }
  
  bool calc_tdcpa(const long long t, float vx, float vy)
  {
    if (m_dtype & EOD_POS_REL){
      float dt = (float)((t - m_t) / (double) SEC);
      float vrx = (float)(m_vxr - vx);
      float vry = (float)(m_vyr - vy);
      float xr = (float)(vrx * dt + m_xr);
      float yr = (float)(vry * dt + m_yr);
      float D2 = (float)(xr * xr + yr * yr);
      if ((m_dtype & EOD_POS_BD) == 0){
	m_bear = (float)atan2(m_xr, m_yr);
	m_dist = (float)sqrt(D2);
	m_dtype = (e_obj_data_type)(m_dtype | EOD_POS_BD);
      }
      
      m_tcpa = (float)(-D2 / (xr * vrx + yr * vry));
      
      float xcpa = (float)(m_tcpa * vrx + m_xr);
      float ycpa = (float)(m_tcpa * vry + m_yr);
      float dcpa2 = (float)(xcpa * xcpa + ycpa * ycpa);
      m_dcpa = (float)sqrt(dcpa2);
      m_dtype = (e_obj_data_type)(m_dtype | EOD_TDCPA);
      m_vrx = vrx;
      m_vry = vry;
      return true;
    }
    return false;
  }
  
  bool get_tdcpa(float & tcpa, float & dcpa)
  {
    tcpa = m_tcpa;
    dcpa = m_dcpa;
    
    return (m_dtype & EOD_TDCPA) != 0;
  }
  
  void reset_tdcpa(){
    m_dtype = (e_obj_data_type)(m_dtype & ~EOD_TDCPA);
  }
  
  bool get_prediction(const long long t, float & x, float & y, float & s)
  {
    if ((m_dtype & EOD_TDCPA) == 0)
      return false;
    
    float dt = (float)((t - m_t) * (1.0 / (double)SEC));
    
    s = m_s0 + dt * m_dsdt;
    x = (float)(m_xr + m_vrx * dt);
    y = (float)(m_yr + m_vry * dt);
    
    return true;
  }
};


// AIS object
class c_ais_obj: public c_obj
{
 protected:
  unsigned int m_mmsi;
public:
  c_ais_obj();
  c_ais_obj(const c_ais_obj & obj);
  c_ais_obj(const long long t, const unsigned int mmsi, 
	    const double lat, const double lon,
	    const float cog, const float sog, const float hdg);
  virtual ~c_ais_obj();
  
  virtual void print(ostream & out){
    out << "AIS Object MMSI: " << m_mmsi << 
      " lat " << m_lat << " lon " << m_lon 
	<< " xr " << m_xr << " yr " << m_yr << " zr " << m_zr << endl;
  }
  
  void set(const long long t, const unsigned int mmsi, 
	   double lat, double lon, float cog, float sog, float hdg)
  {
    m_type = EOT_SHIP;
    m_src = EOS_AIS;
    if(m_tst == EOST_DET || m_tst == EOST_TRCK)
      m_tst = EOST_TRCK;
    m_dtype= (e_obj_data_type)(EOD_AIS | EOD_ATTD);
    
    m_yaw = hdg;
    set_time(t);
    m_mmsi = mmsi;
    set_pos_blh(lat, lon, 0.);
    set_vel_blh(cog, sog);
    reset_ecef();
    reset_rel();
  }
  
  void update(const long long t, double lat, double lon, 
	      float cog, float sog, float hdg){
    m_t = t;
    m_dtype = (e_obj_data_type)(EOD_AIS | EOD_ATTD);
    m_yaw = hdg;
    set_pos_blh(lat, lon, 0.);
    set_vel_blh(cog, sog);
    reset_ecef();
    reset_rel();
  }
  
  void set(const c_ais_obj & obj){
    set(obj.m_t, obj.m_mmsi, obj.m_lat, obj.m_lon,
	obj.m_cog, obj.m_sog, obj.m_yaw);
  }
  
  void update(const c_ais_obj & obj){
    update(obj.m_t, obj.m_lat, obj.m_lon, obj.m_cog, obj.m_sog, obj.m_yaw);
  }
  
  const unsigned int get_mmsi(){
    return m_mmsi;
  }
  
  static size_t get_dsize()
  {
    return sizeof(long long) + sizeof(unsigned int) + sizeof(float) * 5;
  }
  
  int write(FILE * pf)
  {
    fwrite((void*)&m_t , sizeof(long long), 1, pf);
    fwrite((void*)&m_mmsi, sizeof(unsigned int), 1, pf);
    fwrite((void*)&m_lat, sizeof(float), 1, pf);
    fwrite((void*)&m_lon, sizeof(float), 1, pf);
    fwrite((void*)&m_cog, sizeof(float), 1, pf);
    fwrite((void*)&m_sog, sizeof(float), 1, pf);
    fwrite((void*)&m_yaw, sizeof(float), 1, pf);
    return sizeof(long long) + sizeof(unsigned int) + sizeof(float) * 5;
  }
  
  int read(FILE * pf)
  {
    size_t res;
    res = fread((void*)&m_t , sizeof(long long), 1, pf);
    if(!res)
      return 0;
    res = fread((void*)&m_mmsi, sizeof(unsigned int), 1, pf);
    if(!res)
      return 0;
    res = fread((void*)&m_lat, sizeof(float), 1, pf);
    if(!res)
      return 0;
    res = fread((void*)&m_lon, sizeof(float), 1, pf);
    if(!res)
      return 0;
    res = fread((void*)&m_cog, sizeof(float), 1, pf);
    if(!res)
      return 0;
    res = fread((void*)&m_sog, sizeof(float), 1, pf);
    if(!res)
      return 0;
    res = fread((void*)&m_yaw, sizeof(float), 1, pf);
    if(!res)
      return 0;
    return sizeof(long long) + sizeof(float) * 5;		
  }
  
  void write_buf(const char * buf)
  {
    m_t = *((long long*) buf);
    buf += sizeof(long long);
    
    m_mmsi = *((unsigned int*) buf);
    buf += sizeof(unsigned int);
    
    m_lat = *((float*)buf);
    buf += sizeof(float);
    
    m_lon = *((float*)buf);
    buf += sizeof(float);
    
    m_cog = *((float*)buf);
    buf += sizeof(float);
    
    m_sog = *((float*)buf);
    buf += sizeof(float);

    m_yaw = *((float*)buf);
    buf += sizeof(float);
  }
  
  void read_buf(char * buf)
  {
    *((long long*) buf) = m_t;
    buf += sizeof(long long);
    
    *((unsigned int*) buf) = m_mmsi;
    buf += sizeof(unsigned int);
    
    *((float*)buf) = m_lat;
    buf += sizeof(float);
    
    *((float*)buf) = m_lon;
    buf += sizeof(float);
    
    *((float*)buf) = m_cog;
    buf += sizeof(float);
    
    *((float*)buf) = m_sog;
    buf += sizeof(float);
    
    *((float*)buf) = m_yaw;
    buf += sizeof(float);
  }
  
  static void read_buf_null(char * buf){
    memset((void*)buf, 0, get_dsize());
  }
};


// contains recent object list, expected object list
// has insert, delete, and search method
class ch_obj: public ch_base
{
protected:
  list<c_obj*> objs;
  list<c_obj*>::iterator itr;
public:
  ch_obj(const char * name): ch_base(name)
  {
    itr = objs.begin();
  }
  
  virtual ~ch_obj()
  {
  }
  
  void ins(c_obj * pobj){
    itr = objs.insert(itr, pobj);
  }
  
  void ers(){
    itr = objs.erase(itr);
  }
  
  c_obj * cur(){
    return *itr;
  }
  
  bool is_end(){
    return itr == objs.end();
  }
  
  bool is_begin(){
    return itr == objs.begin();
  }
  
  c_obj * begin(){
    return *(itr = objs.begin());
  }
  
  void end(){
    itr = objs.end();
  }
  
  void next(){
    if(objs.end() != itr)
      itr++;
  }
  
  void prev(){
    if(objs.begin() != itr)
      itr--;
  }
  
  int get_num_objs(){
    return (int) objs.size();
  }
};

class ch_ais_obj:public ch_base
{
protected:
  map<unsigned int, c_ais_obj *> objs;
  map<unsigned int, c_ais_obj *>::iterator itr;
  list<c_ais_obj*> updates;
  long long m_tfile;
public:
  ch_ais_obj(const char * name): ch_base(name), m_tfile(0)
  {
    itr = objs.begin();
  }
  
  virtual ~ch_ais_obj()
  {
    lock();
    begin();
    for (; !is_end(); next())
      delete (itr->second);
    objs.clear();
    updates.clear();
    unlock();
  }
  
  void push(const long long t, const unsigned int mmsi,
	    double lat, double lon, float cog, float sog, float hdg)
  {
    if(mmsi == 0 || mmsi > 999999999){
      //cout << "illegal mmsi detected: mmsi " << mmsi <<  endl;
      return ;
    }
    lock();
    itr = objs.find(mmsi);
    if(itr != objs.end()){
      c_ais_obj & obj = *(itr->second);
      obj.update(t, lat, lon, cog, sog, hdg);
      obj.set_ecef_from_blh();
      updates.push_back(&obj);
    }else{
      c_ais_obj * pobj = new c_ais_obj(t, mmsi, lat, lon, cog, sog, hdg);
      objs.insert(map<unsigned int, c_ais_obj *>::value_type(mmsi, pobj));
      pobj->set_ecef_from_blh();
      updates.push_back(pobj);
    }
    unlock();
  }
  
  void reset_updates()
  {
    lock();
    updates.clear();
    unlock();
  }
  
  void update_rel_pos_and_vel(const double * R, const float x,
			      const float y, const float z)
  {
    lock();
    for (itr = objs.begin(); itr != objs.end(); itr++){
      c_ais_obj * pobj = itr->second;
      
      pobj->set_pos_rel_from_ecef(R, x, y, z);
      pobj->set_vel_ecef_from_blh(R);
      pobj->set_vel_rel_from_blh();
      pobj->set_pos_bd_from_rel();
    }
    unlock();
  }
  
  void set_track(const int _id){
    int id = 0;
    for (itr = objs.begin(); itr != objs.end(); itr++){
      c_ais_obj * pobj = itr->second;
      if (id == _id){
	pobj->set_tracking_id(0);
      }else{
	pobj->set_tracking_id(-1);
      }
      id++;
    }
  }
  
  const int get_tracking_id(){
    c_ais_obj & obj = *(itr->second);
    return obj.get_tracking_id();
  }
  
  void calc_tdcpa(const long long t, float vx, float vy)
  {
    lock();
    for (itr = objs.begin(); itr != objs.end(); itr++){
      c_ais_obj * pobj = itr->second;
      pobj->calc_tdcpa(t, vx, vy);
    }
    unlock();
  }
  
  void remove_out(float range)
  {
    lock();
    float r2 = (float)(range * range);
    for(itr = objs.begin(); itr != objs.end();){
      c_ais_obj * pobj = itr->second;
      double x, y, z;
      pobj->get_pos_rel(x, y, z);
      float d = (float)(x * x + y * y + z * z);
      if(d > r2){
	updates.remove(pobj);
	delete itr->second;			
	itr = objs.erase(itr);
      }else{
	itr++;
      }
    }
    unlock();
  }
  
  void remove_old(const long long told){
    lock();
    for(itr = objs.begin(); itr != objs.end();){
      c_ais_obj * pobj = itr->second;
      if(pobj->get_time() < told){
	updates.remove(pobj);
	itr = objs.erase(itr);
	delete pobj;
      }else{
	itr++;
      }
    }
    unlock();
  }
  
  // Note: 
  // get_cur_state, is_end, is_begin, begin, end, next, prev do not lock mutex. 
  // If you use them, lock/unlock methods should be called by their user.
  
  bool get_cur_state(double & x, double & y, double & z,
		     float & vx, float & vy, float & vz, float & yw){
    c_ais_obj & obj = *(itr->second);
    bool flag = true;
    float r, p;
    if(obj.get_dtype() & EOD_POS_REL){
      obj.get_pos_rel(x, y, z);
    }
    else{
      flag = false;
    }
    
    if(obj.get_dtype() & EOD_VEL_REL){
      obj.get_vel_rel(vx, vy, vz);
    }
    else{
      flag = false;
    }
    
    if(obj.get_dtype() & EOD_ATTD){	
      obj.get_att(r, p, yw);
    }
    else{
      flag = false;
    }
    return flag;
  }
  
  bool get_tdcpa(float & tcpa, float & dcpa){
    return (itr->second)->get_tdcpa(tcpa, dcpa);
  }
  
  bool get_pos_bd(float & bear, float & dist)
  {
    return (itr->second)->get_pos_bd(bear, dist);
  }
  
  bool get_prediction(const long long t, float & x, float & y, float & s)
  {
    return (itr->second)->get_prediction(t, x, y, s);
  }
  
  bool is_end(){
    bool r = itr == objs.end();
    return r;
  }
  
  bool is_begin(){
    bool r = itr == objs.begin();
    return r;
  }
  
  void begin(){
    itr = objs.begin();
  }
  
  void end(){
    itr = objs.end();
  }
  
  c_ais_obj & cur()
  {
    return *(itr->second);
  }
  
  void next(){
    if(objs.end() != itr)
      itr++;
  }
  
  void prev(){
    if(objs.begin() != itr)
      itr--;
  }
  
  int get_num_objs(){
    int r;
    r = (int) objs.size();
    return r;
  }
  
  virtual size_t get_dsize()
  {
    return c_ais_obj::get_dsize();
  }
  
  virtual size_t write_buf(const char * buf)
  {
    lock();
    c_ais_obj obj_new;
    
    obj_new.write_buf(buf);
    if(obj_new.get_mmsi() != 0){
      itr = objs.find(obj_new.get_mmsi());
      if(itr != objs.end()){
	c_ais_obj & obj = *(itr->second);
	obj.update(obj_new);
	obj.set_ecef_from_blh();
	updates.push_back(itr->second);
      }else{
	c_ais_obj * pobj = new c_ais_obj(obj_new);
	objs.insert(map<unsigned int, c_ais_obj *>::value_type(pobj->get_mmsi(), pobj));
	pobj->set_ecef_from_blh();
	updates.push_back(pobj);
      }
    }
    unlock();
    return get_dsize();
  }
  
  virtual size_t read_buf(char * buf)
  {
    lock();
    if(updates.size()){
      c_ais_obj * pobj = *(updates.begin());
      updates.pop_front();
      pobj->read_buf(buf);
    }else{
      c_ais_obj::read_buf_null(buf);
    }
    unlock();
    return get_dsize();
  }
  
  virtual int write(FILE * pf, long long tcur)
  {
    int sz = 0;
    
    if(pf){
      lock();
      long long tnew = m_tfile;
      for(itr = objs.begin(); itr != objs.end(); itr++){
	c_ais_obj & obj = *(itr->second);
	if(obj.get_time() > m_tfile){
	  tnew = max(tnew, obj.get_time());
	  sz += obj.write(pf);
	}
      }
      m_tfile = tnew;
      unlock();
    }
    return sz;
  }
  
  virtual int read(FILE * pf, long long tcur)
  {
    c_ais_obj obj;
    int sz = 0;
    if(pf){
      lock();
      while(m_tfile < tcur && !feof(pf)){
	obj.read(pf);
	m_tfile = obj.get_time();
	itr = objs.find(obj.get_mmsi());
	if(itr != objs.end()){
	  c_ais_obj & obj = *(itr->second);
	  obj.update(obj);
	  obj.set_ecef_from_blh();
	  updates.push_back(itr->second);
	}else{
	  c_ais_obj * pobj = new c_ais_obj(obj);
	  objs.insert(map<unsigned int, c_ais_obj *>::value_type(obj.get_mmsi(), pobj));
	  pobj->set_ecef_from_blh();
	  updates.push_back(pobj);
	}				
      }
      unlock();
    }
    return sz;
  }
  
  virtual bool log2txt(FILE * pbf, FILE * ptf)
  {
    c_ais_obj obj;
    fprintf(ptf, "t, mmsi, lat, lon, cog, sog, hdg\n");
    int sz = 0;
    if(pbf){
      while(!feof(pbf)){
	obj.read(pbf);
	double lat, lon, alt;
	float cog, sog, roll, pitch, yaw;
	obj.get_pos_blh(lat, lon, alt);
	obj.get_vel_blh(cog, sog);
	obj.get_att(roll, pitch, yaw);
	fprintf(ptf, "%lld, %u, %+013.8f, %+013.8f, %+06.1f, %+06.1f, %+06.1f\n", 
		obj.get_time(), obj.get_mmsi(), lat, lon, cog, sog, yaw);
      }
    }
    return true;
  }
};

#endif
