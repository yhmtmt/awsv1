#ifdef PY_EXPORT
#include <boost/python.hpp>
#endif

#ifndef AWS_STATE_HPP
#define AWS_STATE_HPP


// this function calculate antenna rotation induced velocity variation
void correct_velocity(const float u, const float v, const float w,
		      const float dr_dt, const float dp_dt, const float dy_dt,
		      const float xant, const float yant, const float zant,
		      float & ucor, float & vcor, float & wcor)
{
  ucor = u - (-dy_dt * yant + dp_dt * zant);
  vcor = v - ( dy_dt * xant - dr_dt * zant);
  wcor = w - (-dp_dt * xant + dr_dt * yant);
}


class c_aws1_state;
class c_aws1_state_sampler
{
public:  
  int size_buf;
  int num;
  int tail;
  long long t;
  long long dt;
  double inv_dt_sec;
  vector<float> rev;
  vector<int> trim;
  vector<double> lat;
  vector<double> lon;
  vector<double> x, y, z;
  vector<float> u, v, w;
  vector<float> ucor, vcor, wcor;
  vector<float> du_dt, dv_dt;
  vector<float> roll;
  vector<float> pitch;
  vector<float> yaw;
  vector<float> dr_dt;
  vector<float> dp_dt;
  vector<float> dy_dt;
  vector<int> eng;
  vector<int> rud;
  vector<float> hev;
  vector<float> wdir;
  vector<float> wspd;
  vector<float> hmd;
  vector<float> tmpa;
  vector<float> dwpt;
  vector<float> bar;
  vector<float> depth;
  vector<float> tmpeng;
  vector<float> valt;
  vector<float> frate;
  vector<double> hour;

  float xant, yant, zant;

  c_aws1_state_sampler(int _size_buf, long long _dt);

  ~c_aws1_state_sampler()
  {
  }

  void set_time(const long long _t)
  {
    t = _t;
    clear();
  }

  void set_gps_antenna_position(const float _xant,
				const float _yant, const float _zant)
  {
    xant = _xant;
    yant = _yant;
    zant = _zant;
  }

  
  const long long get_time(int idx = 0);
  const long long get_cycle_time();  

  void clear()
  {
    num = tail = 0; 
  }
  
  bool sample(c_aws1_state & st);
};

class c_aws1_state
{
private:
  int size_buf;
  vector<long long> trev;
  vector<float> rev;
  vector<int> trim;
  
  vector<long long> tgll;
  vector<double> lat;
  vector<double> lon;
  vector<double> x, y, z;
  
  vector<long long> tvtg;
  vector<float> sog;
  vector<float> cog;

  long long tatt_delay;
  vector<long long> thpr;
  vector<float> roll;
  vector<float> pitch;
  vector<float> yaw;
 
  vector<long long> teng;
  vector<int> eng;
  
  vector<long long> trud;
  vector<int> rud;
  
  vector<long long> thev;
  vector<float> hev;
  
  vector<long long> tmda;
  vector<float> wdir;
  vector<float> wspd;
  vector<float> hmd;
  vector<float> tmpa;
  vector<float> dwpt;
  vector<float> bar;
  
  vector<long long> tdbt;
  vector<float> depth;
  
  vector<long long> tengd;
  vector<float> tmpeng;
  vector<float> valt;
  vector<float> frate;
  vector<double> hour;
  
  int num_rev, tail_rev;
  long long trev_max, trev_min;
  int num_gll, tail_gll;
  long long tgll_max, tgll_min;
  int num_vtg, tail_vtg;
  long long tvtg_max, tvtg_min;
  int num_hpr, tail_hpr;
  long long thpr_max, thpr_min;
  int num_eng, tail_eng;
  long long teng_max, teng_min;
  int num_rud, tail_rud;
  long long trud_max, trud_min;
  int num_hev, tail_hev;
  long long thev_max, thev_min;
  int num_mda, tail_mda;
  long long tmda_max, tmda_min;
  int num_dbt, tail_dbt;
  long long tdbt_max, tdbt_min;
  int num_engd, tail_engd;
  long long tengd_max, tengd_min;
  
  template <class T> void append(vector<T> & hist, int tail, const T & val)
  {
    hist[tail] = val;
  }

  bool is_samplable(const long long tmax, const long long tmin, const long long tsample){    
    if(tsample > tmax || tsample < tmin)
      return false;
    return true;
  }
    
  bool is_symmetric_differentiable(const long long tmax, const long long tmin, const long long tdiff, const long long dt){
    if(!is_samplable(tmax, tmin, tdiff + dt))
      return false;
    if(!is_samplable(tmax, tmin, tdiff - dt))
      return false;
    return true;
  }

  bool is_backward_differentiable(const long long tmax, const long long tmin, const long long tdiff, const long long dt){
    if(!is_samplable(tmax, tmin, tdiff))
      return false;
    if(!is_samplable(tmax, tmin, tdiff - dt))
      return false;    
    return true;
  }
  
  template <class T> void init(vector<T> & hist, const T val)
  {
    fill(hist.begin(), hist.end(), val);
  }
 
  void find_sample_index(const vector<long long> & ts, const int tail,
			 const long long & tsmpl,
			 int & i0, int & i1)
  {
    i1 = tail - 1;
    if(i1 < 0)
      i1 += ts.size();

    if(ts[i1] < tsmpl){    // tsmpl is larger than tmax
      i0 = i1;
      i1 = size_buf;
      return;
    }

    if(ts[tail] > tsmpl){    // tsmpl is less than tmin
      i0 = -1;
      i1 = tail;
      return;
    }
      
    i0 = i1  - 1;
    if(i0 < 0)
      i0 += ts.size();    
      
    while(i1 != tail){
      if(ts[i0] <= tsmpl && ts[i1] >= tsmpl)
	break;
      i1 -= 1;
      if(i1 < 0)
	i1 += ts.size();
      i0 = i1  - 1;
      if(i0 < 0)
	i0 += ts.size();      
    }

    if(i1 == tail)
      i0 = i1 = -1;
    
  }

  void calc_sample_coef(const vector<long long> & ts, const long long & tsmpl,
			const int i0, const int i1,
			float & alpha, float & ialpha)
  {
    double dt_inv = 1.0/(double)(ts[i1] - ts[i0]);
    alpha = (float)((double)(tsmpl - ts[i1]) * dt_inv);
    ialpha = 1.0 - alpha;        
  }

  c_aws1_state_sampler sampler;
public:

  bool is_samplable(const long long t)    
  {
    // check if position data is samplable
    if(!is_samplable(tgll_max, tgll_min, t))
      return false;
    
    if(!is_samplable(thev_max, thev_min, t))
      return false;

    // check if kinetics data is samplable    
    if(!is_samplable(tvtg_max, tvtg_min, t))
      return false;
    
    if(!is_samplable(thpr_max, thpr_min, t + tatt_delay))
      return false;

    // check if control data is samplable
    if(teng_min < 0)
      return false;

    if(trud_min < 0)
      return false;

    if(tengd_min < 0)
      return false;
    
    if(!is_samplable(trev_max, trev_min, t))
      return false;

    // ehck if environment data is samplable
    if(!is_samplable(tmda_max, tmda_min, t))
      return false;

    if(!is_samplable(tdbt_max, tdbt_min, t))
      return false;

    return true;
  }
  
  c_aws1_state(int _size_buf, int size_sampler,
	       long long dt_sampler = 100 * MSEC);
			      
  ~c_aws1_state(){}   

  void add_engr(const long long _trev, const float _rev, const int _trim);
  void add_engd(const long long _tengd, const float _tmpeng,
		const float _valt, const float _frate, const double _hour); 
  void add_position(const long long _tpos, const double _lat, const double _lon);
  void add_velocity(const long long _tvel, const float _sog, const float _cog);
  void add_attitude(const long long _tatt,
		    const float _roll, const float _pitch, const float _yaw);
  
  void add_heave(const long long _thev, const float _hev);
  void add_mda(const long long _tmda,
	       const float _wdir, const float _wspd,
	       const float _hmd, const float _tmpa,
	       const float _dwpt, const float _bar);  
  void add_depth(const long long _tdbt, const float _depth);
  void add_eng(const long long _teng,  int _eng);
  void add_rud(const long long _trud, int _rud);
 
  void sample_position(const long long t,
		       double & _lat, double & _lon, double & _hev,
		       double & _x, double & _y, double & _z);
  
  void sample_kinetics(const long long t, 
		       float & _u, float & _v, 
		       float & _r, float & _p, float  & _y);

  void sample_control(const long long t, int & _eng, int & _rud,
		      float & _rev, int & _trim,
		      float & _tmpeng, float & _valt, float & _frate,
		      double & _hour);

  void sample_environment(const long long t, float & _wdir, float & _wspd,
			  float & _hmd, float & _tmpa,
			  float & _dwpt, float & _bar,
			  float & _depth);

  const long long get_time(int idx = 0)
  {
    return sampler.get_time(idx);
  }

  bool get_position(const int idx,
		       double & _lat, double & _lon, double & _hev,
		       double & _x, double & _y, double & _z);
  
  
  bool get_kinetics(const int idx, 
		    float & _u, float & _v, float & _w,
		    float & _r, float & _p, float  & _y,
		    float & _dr_dt, float & _dp_dt, float & _dy_dt);


  bool get_control(const int idx, int & _eng, int & _rud,
		      float & _rev, int & _trim,
		      float & _tmpeng, float & _valt, float & _frate,
		      double & _hour);

  bool get_environment(const int idx, float & _wdir, float & _wspd,
			  float & _hmd, float & _tmpa,
			  float & _dwpt, float & _bar,
			  float & _depth);
#ifdef PY_EXPORT
  boost::python::tuple get_position_py(const int idx)
  {
     double _lat, _lon, _hev, _x, _y, _z;
     if(!get_position(idx, _lat, _lon, _hev, _x, _y, _z))
       return boost::python::make_tuple(false, _lat, _lon, _hev, _x, _y, _z);     
     return boost::python::make_tuple(true, _lat, _lon, _hev, _x, _y, _z);     
  };
  
  boost::python::tuple get_kinetics_py(const int idx)
  {
    float _u, _v, _w, _r, _p,  _y, _dr_dt, _dp_dt, _dy_dt;
    if(!get_kinetics(idx, _u, _v, _w, _r, _p,  _y, _dr_dt, _dp_dt, _dy_dt))
      return boost::python::make_tuple(false, _u, _v, _w, _r, _p,  _y, _dr_dt, _dp_dt, _dy_dt);
    return boost::python::make_tuple(true, _u, _v, _w, _r, _p,  _y, _dr_dt, _dp_dt, _dy_dt);
  }
  

  boost::python::tuple get_control_py(const int idx)
  {
    int _eng, _rud;
    float _rev;
    int _trim;
    float _tmpeng, _valt, _frate;
    double _hour;
    if(!get_control(idx, _eng, _rud, _rev, _trim, _tmpeng, _valt, _frate, _hour))
      return boost::python::make_tuple(false, _eng, _rud, _rev, _trim, _tmpeng, _valt, _frate, _hour);

    return boost::python::make_tuple(true, _eng, _rud, _rev, _trim, _tmpeng, _valt, _frate, _hour);
  }
  

  boost::python::tuple get_environment_py(const int idx)
  {
    float _wdir, _wspd, _hmd, _tmpa, _dwpt, _bar, _depth;
    if(!get_environment(idx, _wdir, _wspd, _hmd, _tmpa, _dwpt, _bar, _depth))
      return boost::python::make_tuple(false, _wdir, _wspd, _hmd, _tmpa, _dwpt, _bar, _depth);

    return boost::python::make_tuple(true, _wdir, _wspd, _hmd, _tmpa, _dwpt, _bar, _depth);
  }

#endif
  
  void set_time(const long long & t)
  {
    sampler.set_time(t + abs(tatt_delay));
  }
  
  void set_gps_antenna_position(const float x, const float  y, const float z)
  {
    sampler.set_gps_antenna_position(x, y, z);
  }

  void set_gps_attitude_delay(const long long delay = 200 * MSEC)
  {
    tatt_delay = delay;
  }

  void clear()
  {
    num_rev = 0; tail_rev = 0; trev_max = -1; trev_min = -1;
    num_gll = 0; tail_gll = 0; tgll_max = -1; tgll_min = -1;
    num_vtg = 0; tail_vtg = 0; tvtg_max = -1; tvtg_min = -1;
    num_hpr = 0; tail_hpr = 0; thpr_max = -1; thpr_min = -1; 
    num_eng = 0; tail_eng = 0; teng_max = -1; teng_min = -1;
    num_rud = 0; tail_rud = 0; trud_max = -1; trud_min = -1;
    num_hev = 0; tail_hev = 0; thev_max = -1; thev_min = -1;
    num_mda = 0; tail_mda = 0; tmda_max = -1; tmda_min = -1;
    num_dbt = 0; tail_dbt = 0; tdbt_max = -1; tdbt_min = -1;
    num_engd = 0; tail_engd = 0; tengd_max = -1; tengd_min = -1;
    
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
    sampler.clear();
  }
  
};





#ifdef PY_EXPORT
BOOST_PYTHON_MODULE( libaws_state ){
  namespace python = boost::python;
  python::class_<c_aws1_state>("c_aws1_state",python::init<int,int,long long>())
    .def("clear", &c_aws1_state::clear)
    .def("set_gps_attitude_delay", &c_aws1_state::set_gps_attitude_delay)
    .def("set_gps_antenna_position", &c_aws1_state::set_gps_antenna_position)
    .def("set_time", &c_aws1_state::set_time)
    .def("get_position", &c_aws1_state::get_position_py)
    .def("get_kinetics", &c_aws1_state::get_kinetics_py)
    .def("get_control", &c_aws1_state::get_control_py)
    .def("get_environment", &c_aws1_state::get_environment_py)
    .def("get_time", &c_aws1_state::get_time)
    .def("add_engr", &c_aws1_state::add_engr)
    .def("add_engd", &c_aws1_state::add_engd)
    .def("add_position", &c_aws1_state::add_position)
    .def("add_velocity", &c_aws1_state::add_velocity)
    .def("add_attitude", &c_aws1_state::add_attitude)
    .def("add_heave", &c_aws1_state::add_heave)
    .def("add_mda", &c_aws1_state::add_mda)
    .def("add_depth", &c_aws1_state::add_depth)
    .def("add_eng", &c_aws1_state::add_eng)
    .def("add_rud", &c_aws1_state::add_rud)
    ;
}
#endif

#endif
