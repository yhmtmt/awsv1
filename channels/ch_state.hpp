#ifndef CH_STATE_HPP
#define CH_STATE_HPP
// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_state.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_state.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_state.hpp.  If not, see <http://www.gnu.org/licenses/>.

#include "channel_base.hpp"
#include "aws_state.hpp"
#define SAMPLER_SIZE 100

// state channel contains row sensor data.
class ch_state: public ch_base
{
protected:
  long long m_tfile;          // current time record of the log file playing

  c_aws1_state st;
  
  Eigen::Vector3d x_gps;      // GPS antenna (V104)  position.
  
  // Attitude data (V104)
  long long tatt;             // attitude time stamp
  float roll, pitch, yaw;     // roll(deg), pitch(deg), yaw(deg)
  long long tattf;            // from log file
  float rollf, pitchf, yawf;  // from log file

  float roll_rad, pitch_rad, yaw_rad; // radian version
  float droll, dpitch, dyaw;          // derivative
  Eigen::Matrix3d R_body;             // ship body rotation matrix
  Eigen::Vector3d v_rot;              // GPS antena speed due to ship rotation
  
  // Position data (V104)
  long long tpos;             // position time stamp
  double lon, lat;            // longitude(deg), latitude(deg)
  long long tposf;            // from log file
  double lonf, latf;          // from log file

  // Heave data (V104)
  long long talt;             // altitude(heave) time stamp
  double alt;                 // altitude
  long long taltf;            // from log file
  double altf;                // from log file
  
  // Velocity data (V104)
  long long tvel;
  float cog, sog;             // cog and sog in deg and kts
  long long tvelf;
  float cogf, sogf;           // from log file

  // Depth data (GPS fish finder)
  long long tdp;              // Depth time stamp
  float depth;                // Depth
  long long tdpf;             // from log file
  float depthf;               // from log file
  
  // Weather data (WX220)
  long long twx;   // weather data time stamp 
  float bar;       /*air pressure (bar)*/
  float temp_air;  /* air temperature (C)*/
  float hmdr;      /* relative humidity (%)*/
  float dew;       /* dew point (C) */
  float dir_wnd_t; /* True wind direction (deg)*/
  float wspd_mps;  /* wind speed (m/s) */ 
  long long twxf;  // from log file
  float barf;      // from log file
  float temp_airf; // from log file
  float hmdrf;     // from log file
  float dewf;      // from log file
  float dir_wnd_tf;// from log file
  float wspd_mpsf; // from log file
  
  // calculated data 
  double x, y, z;             // ecef coordinate
  double xf, yf, zf;          // from log file
  double R[9];                // Rotation matrix for ENU transformation

  float vx, vy;	              // velocity vector
  float nvx, nvy;             // course vector (normal vector)
  long long tcvel;            // time stamp of corrected velocity 
  float ccog, csog;           // corrected cog and sog
  float cvx, cvy;             // corrected velocity vector
  float cnvx, cnvy;           // corrected course vector
  
public:
  ch_state(const char * name): ch_base(name),
			       x_gps(0,0,0),
			       m_tfile(0), tatt(0), tpos(0), tvel(0), tdp(0),
			       tattf(0), tposf(0), tvelf(0), tdpf(0), 
			       roll(0), pitch(0), yaw(0),
			       roll_rad(0), pitch_rad(0), yaw_rad(0), 
			       droll(0), dpitch(0), dyaw(0), 
			       lon(0), lat(0), alt(0),
			       x(0), y(0), z(0),
			       cog(0), sog(0), depth(0),
			       ccog(0.0f), csog(0.0f),
			       st(SAMPLER_SIZE, SAMPLER_SIZE)
  {
    for(int i = 0; i < 9; i++) R[i] = 0.0;
    R[0] = R[4] = R[8] = 1.0;
  }

  void set_sampling_start_time(const long long t)
  {
    lock();
    st.set_time(t);
    unlock();
  }
  
  void set_gps_ant_pos(const Eigen::Vector3d & _x_gps)
  {
    lock();
    x_gps = _x_gps;
    st.set_gps_antenna_position(_x_gps[0], _x_gps[1], _x_gps[2]);
    unlock();
  }

  void set_gps_attitude_delay(const long long delay)
  {
    lock();
    st.set_gps_attitude_delay(delay);
    unlock();
  }
  
  
  void set_attitude(const long long _tatt,
		    const float _r, const float _p, const float _y)
  {
    lock();
    if(tatt != _tatt){
      double _roll_rad = (roll * (PI / 180.f));
      double _pitch_rad = (pitch * (PI / 180.f));
      double _yaw_rad = (yaw * (PI / 180.f));
      
      double idt = (double) SEC / (double) (_tatt - tatt);
      dyaw = (float)(idt * normalize_angle_rad(_yaw_rad - yaw_rad));
      dpitch = (float)(idt * normalize_angle_rad(_pitch_rad - pitch_rad));
      droll = (float)(idt * normalize_angle_rad(_roll_rad - roll_rad));      
      
      roll_rad = _roll_rad;
      pitch_rad = _pitch_rad;
      yaw_rad = _yaw_rad;
      R_body = rotation_matrix(roll_rad, pitch_rad, yaw_rad);
      Eigen::Matrix3d rx = left_cross_product_matrix(droll, dpitch, dyaw);
      v_rot = rx * R_body * x_gps;
      tatt = _tatt;
      roll = _r; 
      pitch = _p;
      yaw = _y;
      
      st.add_attitude(_tatt, _roll_rad, _pitch_rad, _yaw_rad);
    }
    unlock();
  }
  
  void set_position(const long long _tpos, const double _lat, const double _lon)
  {
    lock();
    if(tpos != _tpos){
      tpos = _tpos;
      lat = _lat;
      lon = _lon;
      
      double lat_rad = (lat * (PI / 180.)), lon_rad = (lon * (PI / 180.));
      
      getwrldrot(lat_rad, lon_rad, R);
      blhtoecef(lat_rad, lon_rad, alt, x, y, z);
      
      st.add_position(_tpos, lat_rad, lon_rad);
    }
    unlock();
  }
  
  void set_alt(const long long _talt, const double _alt)
  {
    lock();
    talt = _talt;
    alt = _alt;
    
    st.add_heave(_talt, _alt);
    unlock();
  }

  void get_alt(long long & _talt, float & _alt)
  {
    lock();
    _talt = talt;
    _alt = alt;
    unlock();
  }
  
  void set_velocity(const long long & _tvel, const float _cog, const float _sog)
  {
    lock();
    if(tvel != _tvel){
      tvel = _tvel;
      cog = _cog;
      sog = _sog;
      float th = (float)(cog * (PI / 180.));
      nvx = (float)sin(th);
      nvy = (float)cos(th);
      float mps = (float)(sog * KNOT);
      vx = (float)(mps * nvx);
      vy = (float)(mps * nvy);
      // Note: in body fixed coordinate x is bow direction, but in world coordinate, x is east direction
      cvx = vx - v_rot(1);
      cvy = vy - v_rot(0);

      st.add_velocity(_tvel, mps, th); 
    }
    unlock();
  }
 
  void set_depth(const long long & _tdp, const float _depth){
    lock();
    tdp = _tdp;
    depth = _depth;

    st.add_depth(_tdp, _depth);
    unlock();
  }
  
  void set_weather(const long long & _twx, const float _bar,
		   const float _temp_air, const float _hmdr,
		   const float _dew, const float _dir_wnd_t,
		   const float _wspd_mps)
  {
    lock();
    twx = _twx;
    bar = _bar;
    temp_air = _temp_air;
    hmdr = _hmdr;
    dew = _dew;
    dir_wnd_t = _dir_wnd_t;
    wspd_mps = _wspd_mps;


    st.add_mda(_twx, _dir_wnd_t * (PI / 180.), _wspd_mps, _hmdr, _temp_air,
	       _dew, _bar);
    unlock();
  }


  void set_engr(const long long _tengr, const float _rev, const int _trim)
  {
    lock();
    st.add_engr(_tengr, _rev, _trim);
    unlock();
  }

  void set_engd(const long long _tengd, const float _tmpeng, const float _valt, const float _frate, const double _hour)
  {
    lock();
    st.add_engd(_tengd, _tmpeng, _valt, _frate, _hour);
    unlock();    
  }

  void set_eng(const long long _teng, const int _eng)
  {
    lock();
    st.add_eng(_teng, _eng);
    unlock();
  }

  void set_rud(const long long _trud, const int _rud)
  {
    lock();
    st.add_rud(_trud, _rud);
    unlock();
  }
  
  
  void get_weather(long long & _twx, float & _bar, float & _temp_air,
		   float & _hmdr, float & _dew,
		   float & _dir_wnd_t, float & _wspd_mps)
  {
    lock();
    _twx = twx;
    _bar = bar;
    _temp_air = temp_air;
    _hmdr = hmdr;
    _dew = dew;
    _dir_wnd_t = dir_wnd_t;
    _wspd_mps = wspd_mps;
    unlock();
  }
  
  void get_attitude(long long & _tatt, float & _r, float & _p, float & _y)
  {
    lock();
    _tatt = tatt;
    _r = roll;
    _p = pitch;
    _y = yaw;
    unlock();
  }
  
  void get_position(long long & _tpos, double & _lat, double & _lon, 
		    double & _x, double & _y, double & _z, double * Renu)
  {
    lock();
    _tpos = tpos;
    _lat = lat;
    _lon = lon;
    _x = x;
    _y = y;
    _z = z;
    memcpy(Renu, R, sizeof(R));
    unlock();
  }
  
  void get_position(long long & _tpos, double & _lat, double & _lon)
  {
    lock();
    _tpos = tpos;
    _lat = lat;
    _lon = lon;
    unlock();
  }
  
  void get_position_ecef(long long & _tpos, double & _x, double & _y, double & _z)
  {
    lock();
    _tpos = tpos;
    _x = x;
    _y = y;
    _z = z;
    unlock();
  }
  
  void get_enu_rotation(long long & _tpos, double * Rret)
  {
    lock();
    _tpos = tpos;
    memcpy(Rret, R, sizeof(R));
    unlock();
  }

  void get_velocity(long long & _tvel, float & _cog, float & _sog)
  {
    lock();
    _tvel = tvel;
    _cog = cog;
    _sog = sog;
    unlock();
  }

  void get_corrected_velocity(long long & _tcvel, float & _ccog, float & _csog)
  {
    lock();
    _tcvel = tcvel;
    _ccog = atan2(cvx, cvy) * (180.f / PI);
    _csog = sqrt(cvx * cvx + cvy * cvy) * (1. / KNOT);
    unlock();
  }
  
  void get_velocity_vector(long long & _tvel, float & _vx, float & _vy)
  {
    lock();
    _tvel = tvel;
    _vx = vx;
    _vy = vy;
    unlock();
  }

  void get_corrected_velocity_vector(long long & _tcvel, float & _cvx, float & _cvy)
  {
    lock();
    _tcvel = tcvel;
    _cvx = cvx;
    _cvy = cvy;
    unlock();
  }
  
  void get_norm_velocity_vector(long long & _tvel, float & _nvx, float & _nvy)
  {
    lock();
    _tvel = tvel;
    _nvx = nvx;
    _nvy = nvy;
    unlock();
  }

  void get_normal_corrected_velocity_vector(long long & _tcvel, float & _cnvx, float & _cnvy)
  {
    lock();
    _tcvel = tvel;
    _cnvx = cnvx;
    _cnvy = cnvy;
    unlock();
  }
  
  void get_depth(long long & _tdp, float & _depth)
  {
    lock();
    _tdp = tdp;
    _depth = depth;
    unlock();
  }
  
  bool get_10hz_data(long long & _t,
		     double & _lat, double & _lon, double & _hev,
		     double & _x, double & _y, double & _z,
		     float & _u, float & _v, float & _w, 
		     float & _roll, float & _pitch, float  & _yaw,
		     float & _dr_dt, float & _dp_dt, float & _dy_dt,
		     float & _rev, int & _trim,
		     int & _eng, int & _rud, const int idx = 0)
  {
    lock();
    _t = st.get_time_10hz_data(idx);
    bool res = st.get_10hz_data(idx, _lat, _lon, _hev, _x, _y, _z, _u, _v, _w,
				_roll, _pitch, _yaw, _dr_dt, _dp_dt, _dy_dt,
				_rev, _trim, _eng, _rud);
    unlock();

    return res;
  }

  bool get_1hz_data(long long & _t,  float & _tmpeng,
		    float & _valt, float & _frate,
		    double & _hour, float & _wdir, float & _wspd,
		    float & _hmd, float & _tmpa,
		    float & _dwpt, float & _bar,
		    float & _depth, const int idx = 0)
  {
    lock();
    _t = st.get_time_1hz_data(idx);    
    bool res = st.get_1hz_data(idx, _tmpeng,
			       _valt, _frate,
			       _hour, _wdir, _wspd,
			       _hmd, _tmpa,
			       _dwpt, _bar,
			       _depth);
    unlock();
    return res;
  }
  
  
  virtual size_t get_dsize()
  {
    return sizeof(long long) * 6 + sizeof(float) * 12 + sizeof(double) * 15;
  }
  
  virtual size_t write_buf(const char *buf);
  virtual size_t read_buf(char * buf);
  
  virtual void print(ostream & out)
  {
    out << "channel " << m_name << ":";
    out << "LATLON:" << lat << "," << lon;
    out << " RPY:" << roll << "," << pitch << "," << yaw;
    out << "cogsog:" << cog << "," << sog;
    out << endl;
  }
  
  virtual int write(FILE * pf, long long tcur);
  
  virtual int read(FILE * pf, long long tcur);
  
  virtual bool log2txt(FILE * pbf, FILE * ptf);
};



extern const char * strStatEng1[NMEA2000::EngineStatus1_MAX +1];

extern const char * strStatEng2[NMEA2000::EngineStatus2_MAX +1];

extern const char * strStatGear[NMEA2000::GearStatus_MAX+1];

class ch_eng_state: public ch_base
{
private:
  long long t, tf;
  
  long long trapid, trapidf;
  float rpm, rpmf; // x0.25
  unsigned char trim, trimf; // x1
  
  long long tdyn, tdynf;
  int poil, poilf; // oil pressure
  float toil, toilf; // oil temperature
  float temp, tempf; // Engine? temperature
  float valt, valtf; // alternator potential 
  float frate, fratef; // fuel rate (L/h)
  unsigned int teng, tengf; // total engine hour
  int pclnt, pclntf; // coolant pressure
  int pfl, pflf; // fuel pressure
  NMEA2000::EngineStatus1 stat1, stat1f; // Status 1
  NMEA2000::EngineStatus2 stat2, stat2f; // Status 2
  unsigned char ld, ldf; // load
  unsigned char tq, tqf; // torque
  
  long long ttran, ttranf;
  NMEA2000::GearStatus gear, gearf;
  int pgoil, pgoilf;
  float tgoil, tgoilf;
  
  long long ttrip, ttripf;
  int flused, flusedf; // fuel used
  float flavg, flavgf; // fuel rate average
  float fleco, flecof; // fuel rate eco
  float flinst, flinstf; // Instantaneous fuel rate
  
  char *buf;
public:
  ch_eng_state(const char * name):ch_base(name), buf(NULL), tf(0),
				  poil(0), poilf(0),
				  temp(273+60), tempf(273+60),
				  valt(14.1), valtf(14.1),
				  frate(1.0), fratef(1.0),
				  teng(0), tengf(0),
				  stat1(NMEA2000::EngineStatus1_CheckEngine),
				  stat1f(NMEA2000::EngineStatus1_CheckEngine),
				  stat2(NMEA2000::EngineStatus2_WarningLevel2),
				  stat2f(NMEA2000::EngineStatus2_WarningLevel2),
				  ld(0), ldf(0), tq(0), tqf(0), 
				  gear(NMEA2000::GearStatus_Neutral),
				  gearf(NMEA2000::GearStatus_Neutral),
				  pgoil(0), pgoilf(0),
				  flused(0), flusedf(0),
				  flavg(0), flavgf(0),
				  fleco(0), flecof(0),
				  flinst(0), flinstf(0)
				  
  {
   buf = new char[get_dsize()];
  }
  ~ch_eng_state()
  {
    delete[] buf;
  }
  
  void set_rapid(const long long _t, const float _rpm, const unsigned char _trim)
  {
    lock();
    t = trapid = _t;
    rpm = _rpm;
    trim = _trim;
    unlock();
  }
  
  void get_rapid(long long & _t, float & _rpm, unsigned char & _trim)
  {
    lock();
    _t = trapid;
    _rpm = rpm;
    _trim = trim;
    unlock();
  }

  void set_dynamic(const long long _t, const int _poil, const float _toil,
		   const float _temp,  const float _valt, const float _frate,
		   const unsigned int _teng, const int _pclnt, const int _pfl,
		   const NMEA2000::EngineStatus1 _stat1,
		   const NMEA2000::EngineStatus2 _stat2,
		   const unsigned char _ld, const unsigned char _tq)
  {
    lock();
    t = tdyn = _t;
    poil = _poil;
    toil = _toil;
    temp = _temp;
    valt = _valt;
    frate = _frate;
    teng = _teng;
    pclnt = _pclnt;
    pfl = _pfl;
    stat1 = _stat1;
    stat2 = _stat2;
    ld = _ld;
    tq = _tq;
    unlock();
  }

  void get_dynamic(long long & _t,  int & _poil,  float & _toil,
		   float & _temp, float & _valt,  float & _frate,
		   unsigned int & _teng,  int & _pclnt,  int & _pfl,
		   NMEA2000::EngineStatus1 & _stat1,
		   NMEA2000::EngineStatus2 & _stat2,
		   unsigned char & _ld, unsigned char & _tq)
  {
    lock();
    _t = tdyn;
    _poil = poil;
    _toil = toil;
    _temp = temp;
    _valt = valt;
    _frate = frate;
    _teng = teng;
    _pclnt = pclnt;
    _pfl = pfl;
    _stat1 = stat1;
    _stat2 = stat2;
    _ld = ld;
    _tq = tq;
    unlock();    
  }

  void set_tran(const long long _t, const NMEA2000::GearStatus _gear, const int _pgoil,
		const float _tgoil)
  {
    lock();
    t = ttran = _t;
    gear = _gear;
    pgoil = _pgoil;
    tgoil = _tgoil;
    unlock();
  }
  
  void get_tran(long long & _t, NMEA2000::GearStatus & _gear, int & _pgoil, float & _tgoil)
  {
    lock();
    _t = ttran;
    _gear = gear;
    _pgoil = pgoil;
    _tgoil = tgoil;
    unlock();
  }
  
  void set_trip(const long long _t, const int _flused, const float _flavg,
		const float _fleco, const float _flinst)
  {
    lock();
    t = ttrip = _t;
    flused = _flused;
    flavg = _flavg;
    fleco = _fleco;
    flinst = _flinst;
    unlock();
  }
  
  void get_trip(long long & _t, int & _flused, float & _flavg, float & _fleco,
		float & _flinst)
  {
    lock();
    _t = ttrip;
    _flused = flused;
    _flavg = flavg;
    _fleco = fleco;
    _flinst = flinst;
    unlock();
  }
  
  virtual size_t get_dsize(){
    return sizeof(long long)*5 + sizeof(float)*9 + sizeof(unsigned char)*3
      + sizeof(int)*5 + sizeof(unsigned int) + sizeof(NMEA2000::EngineStatus1)
      + sizeof(NMEA2000::EngineStatus2) + sizeof(NMEA2000::GearStatus);
  }
  
  virtual size_t write_buf_back(const char * buf);
  virtual size_t write_buf(const char * buf);
  virtual size_t read_buf(char * buf);
  virtual int write(FILE * pf, long long tcur);
  virtual int read(FILE * Pf, long long tcur);
  virtual void print(ostream & out);
  virtual bool log2txt(FILE * pbf, FILE * ptf);
};


class ch_time_sync: public ch_base
{
private:
  long long time_measured;
  long long time_delta;

public:
  ch_time_sync(const char * name): ch_base(name)
  {
  }

  void set_time_delta(const long long measured, const long long delta)
  {
    lock();
    time_measured = measured;
    time_delta = delta;
    unlock();
  }

  void get_time_delta(long long & measured, long long & delta)
  {
    lock();
    measured = time_measured;
    delta = time_delta;
    unlock();
  }
};
  
#endif
