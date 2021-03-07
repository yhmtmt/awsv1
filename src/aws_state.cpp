#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
#include "aws_coord.hpp"
#include "aws_clock.hpp"
#include "aws_math.hpp"
#include "aws_state.hpp"


//////////////////////////////////////////////////////////// class c_aws1_state
c_aws1_state::c_aws1_state(int _size_buf, int size_sampler, long long dt_sampler):
  size_buf(_size_buf), 
  num_rev(0), tail_rev(0), trev_max(-1), trev_min(-1),
  num_gll(0), tail_gll(0), tgll_max(-1), tgll_min(-1),
  num_vtg(0), tail_vtg(0), tvtg_max(-1), tvtg_min(-1),
  num_hpr(0), tail_hpr(0), thpr_max(-1), thpr_min(-1), tatt_delay(200 * MSEC),
  num_eng(0), tail_eng(0), teng_max(-1), teng_min(-1),
  num_rud(0), tail_rud(0), trud_max(-1), trud_min(-1),
  num_hev(0), tail_hev(0), thev_max(-1), thev_min(-1),
  num_mda(0), tail_mda(0), tmda_max(-1), tmda_min(-1),
  num_dbt(0), tail_dbt(0), tdbt_max(-1), tdbt_min(-1),
  num_engd(0), tail_engd(0), tengd_max(-1), tengd_min(-1),
  sampler(size_sampler, dt_sampler)
{
  trev.resize(size_buf);
  rev.resize(size_buf);
  trim.resize(size_buf);
  tgll.resize(size_buf);
  lat.resize(size_buf);
  lon.resize(size_buf);
  x.resize(size_buf);
  y.resize(size_buf);
  z.resize(size_buf);
  tvtg.resize(size_buf);
  sog.resize(size_buf);
  cog.resize(size_buf);
  thpr.resize(size_buf);
  roll.resize(size_buf);
  pitch.resize(size_buf);
  yaw.resize(size_buf);
  teng.resize(size_buf);
  eng.resize(size_buf);
  trud.resize(size_buf);
  rud.resize(size_buf);
  thev.resize(size_buf);
  hev.resize(size_buf);
  tmda.resize(size_buf);
  wdir.resize(size_buf);
  wspd.resize(size_buf);
  hmd.resize(size_buf);
  tmpa.resize(size_buf);
  dwpt.resize(size_buf);
  bar.resize(size_buf);
  tdbt.resize(size_buf);
  depth.resize(size_buf);
  tengd.resize(size_buf);
  tmpeng.resize(size_buf);
  valt.resize(size_buf);
  frate.resize(size_buf);
  hour.resize(size_buf);

  init<long long>(trev, -1);    
  init<long long>(tgll, -1);
  init<long long>(tvtg, -1);
  init<long long>(thpr, -1);
  init<long long>(teng, -1);
  init<long long>(trud, -1);
  init<long long>(thev, -1);
  init<long long>(tmda, -1);
  init<long long>(tdbt, -1);
  init<long long>(tengd, -1);
}


void c_aws1_state::add_engr(const long long _trev, const float _rev,
			    const int _trim)
{
  if(_trev < trev_max)
    return;

  trev_max = _trev;
  append(trev, tail_rev, _trev);
  append(rev, tail_rev, _rev);
  append(trim, tail_rev, _trim);
  tail_rev = (tail_rev + 1) % size_buf;
  if(num_rev < trev.size()){
    trev_min = trev[0];
    num_rev++;
  }else
    trev_min = trev[tail_rev];

  sampler.sample_10hz_data(*this);
}

void c_aws1_state:: add_engd(const long long _tengd, const float _tmpeng,
			     const float _valt, const float _frate,
			     const double _hour)
{
  if(_tengd < tengd_max)
    return;

  tengd_max = _tengd;
  append(tengd, tail_engd, _tengd);
  append(tmpeng, tail_engd, _tmpeng);
  append(valt, tail_engd, _valt);
  append(frate, tail_engd, _frate);
  append(hour, tail_engd, _hour);
  tail_engd = (tail_engd + 1) % size_buf;
  if(num_engd < tengd.size()){
    tengd_min = tengd[0];
    num_engd++;
  }else
    tengd_min = tengd[tail_engd];

  sampler.sample_1hz_data(*this);  
}

void c_aws1_state::add_position(const long long _tpos, const double _lat, const double _lon)
{
  if(_tpos < tgll_max)
    return;

  tgll_max = _tpos;
  append(tgll, tail_gll, _tpos);
  append(lat, tail_gll, _lat);
  append(lon, tail_gll, _lon);
  double _x, _y, _z;
  blhtoecef(_lat, _lon, 0, _x, _y, _z);
  append(x, tail_gll, _x);
  append(y, tail_gll, _y);
  append(z, tail_gll, _z);
  tail_gll = (tail_gll + 1) % size_buf;
  if(num_gll < tgll.size()){
    tgll_min = tgll[0];
    num_gll++;
  }else
    tgll_min = tgll[tail_gll];
  
  sampler.sample_10hz_data(*this);
}


void c_aws1_state::add_velocity(const long long _tvel, const float _sog, const float _cog)
{
  if(_tvel < tvtg_max)
    return;

  tvtg_max = _tvel;
  append(tvtg, tail_vtg, _tvel);
  append(sog, tail_vtg, _sog);
  append(cog, tail_vtg, _cog);
  tail_vtg = (tail_vtg + 1) % size_buf;
  if(num_vtg < tvtg.size()){
    tvtg_min = tvtg[0];
    num_vtg++;
  }else
    tvtg_min = tvtg[tail_vtg];

  sampler.sample_10hz_data(*this);  
}
  
void c_aws1_state::add_attitude(const long long _tatt,
				const float _roll, const float _pitch,
				const float _yaw)
{
  if(_tatt  < thpr_max)
    return;

  thpr_max = _tatt;
  append(thpr, tail_hpr, _tatt);
  append(roll, tail_hpr, _roll);
  append(pitch, tail_hpr, _pitch);
  append(yaw, tail_hpr, _yaw);
  tail_hpr = (tail_hpr + 1) % size_buf;
  if(num_hpr < thpr.size()){
    thpr_min = thpr[0];
    num_hpr++;
  }else
    thpr_min = thpr[tail_hpr];

  sampler.sample_10hz_data(*this);  
}

void c_aws1_state::add_heave(const long long _thev, const float _hev)
{
  if(_thev < thev_max)
    return;

  thev_max = _thev;
  append(thev, tail_hev, _thev);
  append(hev, tail_hev, _hev);
  tail_hev = (tail_hev + 1) % size_buf;
  if(num_hev < thev.size()){
    thev_min = thev[0];
    num_hev++;
  }else
    thev_min = thev[tail_hev];

  sampler.sample_10hz_data(*this);  
}

void c_aws1_state::add_mda(const long long _tmda,
			   const float _wdir, const float _wspd,
			   const float _hmd, const float _tmpa,
			   const float _dwpt, const float _bar)
  
{
  if(_tmda < tmda_max)
    return;

  tmda_max = _tmda;
  append(tmda, tail_mda, _tmda);
  append(wdir, tail_mda, _wdir);
  append(wspd, tail_mda, _wspd);
  append(hmd, tail_mda, _hmd);
  append(tmpa, tail_mda, _tmpa);
  append(dwpt, tail_mda, _dwpt);
  append(bar, tail_mda, _bar);
  tail_mda = (tail_mda + 1) % size_buf;
  if(num_mda < tmda.size()){
    tmda_min = tmda[0];
    num_mda++;
  }else
    tmda_min = tmda[tail_mda];

  sampler.sample_1hz_data(*this);  
}
  
void c_aws1_state::add_depth(const long long _tdbt, const float _depth)
{
  if(_tdbt < tdbt_max)
    return;

  tdbt_max = _tdbt;
  append(tdbt, tail_dbt, _tdbt);
  append(depth, tail_dbt, _depth);    
  tail_dbt = (tail_dbt + 1) % size_buf;
  if(num_dbt < tdbt.size()){
    tdbt_min = tdbt[0];
    num_dbt++;
  }else
    tdbt_min = tdbt[tail_dbt];

  sampler.sample_1hz_data(*this);  
}

void c_aws1_state::add_eng(const long long _teng,  int _eng)
{
  if(_teng < teng_max)
    return;

  teng_max = _teng;
  append(teng, tail_eng, _teng);
  append(eng, tail_eng, _eng);    
  tail_eng = (tail_eng + 1) % size_buf;
  if(num_eng < teng.size()){
    teng_min = teng[0];
    num_eng++;
  }else
    teng_min = teng[tail_eng];

  sampler.sample_10hz_data(*this);  
}

void c_aws1_state::add_rud(const long long _trud, int _rud)
{
  if(_trud < trud_max)
    return;

  trud_max = _trud;
  append(trud, tail_rud, _trud);
  append(rud, tail_rud, _rud);
  tail_rud = (tail_rud + 1) % size_buf;
  if(num_rud < trud.size()){
    trud_min = trud[0];
    num_rud++;
  }else
    trud_min = trud[tail_rud];

  sampler.sample_10hz_data(*this);  
}
void c_aws1_state::sample_10hz_data(const long long t,
				    double & _lat, double & _lon, double & _hev,
				    double & _x, double & _y, double & _z,
				    float & _u, float & _v, 
				    float & _roll, float & _pitch, float  & _yaw,
				    float & _rev, int & _trim,
				    int & _eng, int & _rud)
{
  int i0, i1;
  float alpha, ialpha;
  
  find_sample_index(tgll, tail_gll, t, i0, i1);
  calc_sample_coef(tgll, t, i0, i1, alpha, ialpha);

  _lat = lat[i0] * alpha + lat[i1] * ialpha;
  _lon = lon[i0] * alpha + lon[i1] * ialpha;
  _x = x[i0] * alpha + x[i1] * ialpha;
  _y = y[i0] * alpha + y[i1] * ialpha;
  _z = z[i0] * alpha + z[i1] * ialpha;

  find_sample_index(thev, tail_hev, t, i0, i1);
  calc_sample_coef(thev, t, i0, i1, alpha, ialpha);

  _hev = hev[i0] * alpha + hev[i1] * ialpha;
  find_sample_index(tvtg, tail_vtg, t, i0, i1);
  calc_sample_coef(tvtg, t, i0, i1, alpha, ialpha);

  float _cog = interpolate_angle_rad(cog[i0], cog[i1], alpha, ialpha);
  float _sog = sog[i0] * alpha + sog[i1] * ialpha;

  long long tcor = t + tatt_delay;
  find_sample_index(thpr, tail_hpr, tcor, i0, i1);
  calc_sample_coef(thpr, tcor, i0, i1, alpha, ialpha);

  _roll = roll[i0] * alpha + roll[i1] * ialpha;
  _pitch = pitch[i0] * alpha + pitch[i1] * ialpha;
  _yaw = interpolate_angle_rad(yaw[i0], yaw[i1], alpha, ialpha);

  double dir = _cog - _yaw;
  _u = cos(dir) * _sog;
  _v = sin(dir) * _sog;
  
  find_sample_index(trev, tail_rev, t, i0, i1);
  calc_sample_coef(trev, t, i0, i1, alpha, ialpha);
  _rev = rev[i0] * alpha + rev[i1] * ialpha;
  _trim = trim[i0] * alpha + trim[i1] * ialpha;

  find_sample_index(teng, tail_eng, t, i0, i1);
  _eng = eng[i0];

  find_sample_index(trud, tail_rud, t, i0, i1); 
  _rud = rud[i0];
  find_sample_index(tengd, tail_engd, t, i0, i1);
}

void c_aws1_state::sample_1hz_data(const long long t, float & _tmpeng,
				   float & _valt, float & _frate,
				   double & _hour, float & _wdir, float & _wspd,
				   float & _hmd, float & _tmpa,
				   float & _dwpt, float & _bar,
				   float & _depth)
{
  int i0, i1;
  float alpha, ialpha;
  find_sample_index(tengd, tail_engd, t, i0, i1);
  calc_sample_coef(tengd, t, i0, i1, alpha, ialpha);
  _tmpeng = tmpeng[i0] * alpha + tmpeng[i1] * ialpha;
  _valt = valt[i0] * alpha + valt[i1] * ialpha;
  _frate = frate[i0] * alpha + frate[i1] * ialpha;
  _hour = hour[i0] * alpha + hour[i1] * ialpha;
  
  find_sample_index(tmda, tail_mda, t, i0, i1);
  calc_sample_coef(tmda, t, i0, i1, alpha, ialpha);
  
  _wdir = wdir[i0] * alpha + wdir[i1] * ialpha;
  _wspd = wspd[i0] * alpha + wspd[i1] * ialpha;
  _hmd = hmd[i0] * alpha + hmd[i1] * ialpha;
  _tmpa = tmpa[i0] * alpha + tmpa[i1] * ialpha;
  _dwpt = dwpt[i0] * alpha + dwpt[i1] * ialpha;
  _bar = bar[i0] * alpha + bar[i1] * ialpha;

  find_sample_index(tdbt, tail_dbt, t, i0, i1);
  calc_sample_coef(tdbt, t, i0, i1, alpha, ialpha);
  _depth = depth[i0] * alpha + depth[i1] * ialpha;   
}


bool c_aws1_state::get_10hz_data(const int idx,
				 double & _lat, double & _lon, double & _hev,
				 double & _x, double & _y, double & _z,
				 float & _u, float & _v, float & _w,
				 float & _roll, float & _pitch, float  & _yaw,
				 float & _dr_dt, float & _dp_dt, float & _dy_dt,
				 float & _rev, int & _trim,
				 int & _eng, int & _rud)
{
  if(idx > 0) // prediction is not sapported
    return false;

  if(idx <= -sampler.num10hz) // past sample out of buffer
    return false;
  
  int i = sampler.tail10hz - idx - 1;
  if(i < 0)
    i += sampler.size_buf;
  i %= sampler.size_buf;

  _lat = sampler.lat[i];
  _lon = sampler.lon[i];
  _hev = sampler.hev[i];
  _x = sampler.x[i];
  _y = sampler.y[i];
  _z = sampler.z[i];

  _u = sampler.ucor[i];
  _v = sampler.vcor[i];
  _w = sampler.wcor[i];

  _roll = sampler.roll[i];
  _pitch = sampler.pitch[i];
  _yaw = sampler.yaw[i];
  
  _dr_dt = sampler.dr_dt[i];
  _dp_dt = sampler.dp_dt[i];
  _dy_dt = sampler.dy_dt[i];

  _eng = sampler.eng[i];
  _rud = sampler.rud[i];

  _rev = sampler.rev[i];
  _trim = sampler.trim[i];  
}

bool c_aws1_state::get_1hz_data(const int idx, float & _tmpeng,
				float & _valt, float & _frate,
				double & _hour, float & _wdir, float & _wspd,
				float & _hmd, float & _tmpa,
				float & _dwpt, float & _bar,
				float & _depth)
{
  if(idx > 0) // prediction is not sapported
    return false;

  if(idx <= -sampler.num1hz) // past sample out of buffer
    return false;
  
  int i = sampler.tail1hz - idx - 1;
  if(i < 0)
    i += sampler.size_buf;
  i %= sampler.size_buf;
  
  _tmpeng = sampler.tmpeng[i];
  _valt = sampler.valt[i];
  _frate = sampler.frate[i];
  _hour = sampler.hour[i];
  
  _wdir = sampler.wdir[i];
  _wspd = sampler.wspd[i];
  _hmd = sampler.hmd[i];
  _tmpa = sampler.tmpa[i];
  _dwpt = sampler.dwpt[i];
  _bar = sampler.bar[i];
  _depth = sampler.depth[i];    
}

//////////////////////////////////////////////// class c_aws1_state_sampler
c_aws1_state_sampler::c_aws1_state_sampler(int _size_buf, long long _dt):
  size_buf(_size_buf), tail10hz(0), tail1hz(0),
  num10hz(0), num1hz(0), t10hz(-1), t1hz(-1),
  dt(_dt),
  xant(0), yant(0), zant(-1.04)
{
  inv_dt_sec = (double) SEC / (double) dt ;
  
  rev.resize(size_buf);
  trim.resize(size_buf);
  lat.resize(size_buf);
  lon.resize(size_buf);
  x.resize(size_buf);
  y.resize(size_buf);
  z.resize(size_buf);
  u.resize(size_buf);
  v.resize(size_buf);
  w.resize(size_buf);
  ucor.resize(size_buf);
  vcor.resize(size_buf);
  wcor.resize(size_buf);
  du_dt.resize(size_buf);
  dv_dt.resize(size_buf);
  roll.resize(size_buf);
  pitch.resize(size_buf);
  yaw.resize(size_buf);
  dr_dt.resize(size_buf);
  dp_dt.resize(size_buf);
  dy_dt.resize(size_buf);
  eng.resize(size_buf);
  rud.resize(size_buf);
  hev.resize(size_buf);
  wdir.resize(size_buf);
  wspd.resize(size_buf);
  hmd.resize(size_buf);
  tmpa.resize(size_buf);
  dwpt.resize(size_buf);
  bar.resize(size_buf);
  depth.resize(size_buf);
  tmpeng.resize(size_buf);
  valt.resize(size_buf);
  frate.resize(size_buf);
  hour.resize(size_buf);
}


const long long c_aws1_state_sampler::get_time_10hz_data(int idx)
{
  if(idx < num10hz)
    return t10hz + dt * idx;
  return -1;
}

const long long c_aws1_state_sampler::get_time_1hz_data(int idx)
{
  if(idx < num1hz)
    return t1hz + dt * idx;
  return -1;
}

const long long c_aws1_state_sampler::get_cycle_time()
{
  return dt;
}

void c_aws1_state_sampler::sample_1hz_data(c_aws1_state & st)
{
  while(1){
    long long tnext = t1hz + dt;
    if(!st.is_1hz_data_samplable(tnext))
      return;

    float _tmpeng, _valt, _frate;
    double _hour;
    float _wdir, _wspd, _hmd, _tmpa, _dwpt, _bar, _depth;
    st.sample_1hz_data(tnext, _tmpeng, _valt, _frate, _hour,
		       _wdir, _wspd, _hmd, _tmpa, _dwpt, _bar,
		       _depth);
    tmpeng[tail1hz] = _tmpeng;
    valt[tail1hz] = _valt;
    frate[tail1hz] = _frate;
    hour[tail1hz] = _hour;
    wdir[tail1hz] = _wdir;
    wspd[tail1hz] = _wspd;
    hmd[tail1hz] = _hmd;
    tmpa[tail1hz] = _tmpa;
    dwpt[tail1hz] = _dwpt;
    bar[tail1hz] = _bar;
    depth[tail1hz] = _depth;
    
    num1hz++;
    tail1hz = (tail1hz + 1) % size_buf;
    t1hz = tnext;
  }
}


void c_aws1_state_sampler::sample_10hz_data(c_aws1_state & st)
{
  while(1){
    long long tnext = t10hz + dt;
    if(!st.is_10hz_data_samplable(tnext))
      return;
    
    int _eng, _rud;
    float _rev;
    int _trim;  
    double _lat, _lon, _hev, _x, _y, _z;
    float _u, _v, _roll, _pitch, _yaw;  
    st.sample_10hz_data(tnext, _lat, _lon, _hev, _x, _y, _z,
			_u, _v, _roll, _pitch, _yaw, _rev, _trim, _eng, _rud);
    
    rev[tail10hz] = _rev;
    trim[tail10hz] = _trim;
    lat[tail10hz] = _lat;
    lon[tail10hz] = _lon;
    x[tail10hz] = _x;
    y[tail10hz] = _y;
    z[tail10hz] = _z;
    u[tail10hz] = _u;
    v[tail10hz] = _v;
    roll[tail10hz] = _roll;
    pitch[tail10hz] = _pitch;
    yaw[tail10hz] = _yaw;
    eng[tail10hz] = _eng;
    rud[tail10hz] = _rud;
    hev[tail10hz] = (float)_hev;
    
    num10hz++;
    int ip = tail10hz;
    int i0 = tail10hz - 1;
    if(i0 < 0)
      i0 += size_buf;
    
    if(num10hz == 1){ // is not differentiable
      dr_dt[tail10hz] = 0;
      dp_dt[tail10hz] = 0;
      dy_dt[tail10hz] = 0;
      w[tail10hz] = 0;
      du_dt[tail10hz] = 0;
      dv_dt[tail10hz] = 0;
      ucor[tail10hz] = 0;
      vcor[tail10hz] = 0;
      wcor[tail10hz] = 0;
    }
    
    if(num10hz > 1){ // backward difference
      dr_dt[ip] = (float)(normalize_angle_rad(roll[ip] - roll[i0]) * inv_dt_sec);
      dp_dt[ip] = (float)(normalize_angle_rad(pitch[ip] - pitch[i0]) * inv_dt_sec);
      dy_dt[ip] = (float)(normalize_angle_rad(yaw[ip] - yaw[i0]) * inv_dt_sec);
      
      w[ip] = (float)((_hev - hev[i0]) * inv_dt_sec);
      du_dt[ip] = (float)((_u - u[i0]) * inv_dt_sec);
      dv_dt[ip] = (float)((_v - v[i0]) * inv_dt_sec);
      
      float _ucor, _vcor, _wcor;
      correct_velocity(u[ip], v[ip], w[ip], dr_dt[ip], dp_dt[ip], dy_dt[ip],
		       xant, yant, zant, _ucor, _vcor, _wcor);
      ucor[ip] = _ucor;
      vcor[ip] = _vcor;
      wcor[ip] = _wcor;
    }
    
    int in = i0 - 1;
    if(in < 0)
      in += size_buf;
    
    if(num10hz > 2){ // symmetric difference
      double inv_2dt = 0.5 * inv_dt_sec;
      dr_dt[i0] = (float)(normalize_angle_rad(roll[ip] - roll[in]) * inv_2dt);
      dp_dt[i0] = (float)(normalize_angle_rad(pitch[ip] - pitch[in]) * inv_2dt);
      dy_dt[i0] = (float)(normalize_angle_rad(yaw[ip] - yaw[in]) * inv_2dt);
      w[ip] = (float)((hev[ip] - hev[in]) * inv_2dt);
      du_dt[ip] = (float)((u[ip] - u[in]) * inv_2dt);
      dv_dt[ip] = (float)((v[ip] - v[in]) * inv_2dt);
      float _ucor, _vcor, _wcor;
      correct_velocity(u[i0], v[i0], w[i0], dr_dt[i0], dp_dt[i0], dy_dt[i0],
		       xant, yant, zant, _ucor, _vcor, _wcor);
      ucor[i0] = _ucor;
      vcor[i0] = _vcor;
      wcor[i0] = _wcor;    
    }
    
    tail10hz = (tail10hz + 1) % size_buf;
    t10hz = tnext;    
  }
}  
  
