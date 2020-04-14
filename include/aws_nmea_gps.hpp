#ifndef AWS_NMEA_GPS_H
#define AWS_NMEA_GPS_H
// Copyright(c) 2016-2020 Yohei Matsumoto, All right reserved. 

// aws_nmea_gps.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_nmea_gps.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_nmea_gps.h.  If not, see <http://www.gnu.org/licenses/>. 


enum e_gp_dir{
  EGP_N, EGP_S, EGP_E, EGP_W
};

// class for $gga
class c_gga: public c_nmea_dat
{
public:
  short m_h, m_m;
  float m_s;
  NMEA0183::GPSFixStatus m_fix_status;
  short m_num_sats;
  double m_lon_deg/* (deg) */, m_lat_deg/* (deg) */;
  e_gp_dir m_lon_dir, m_lat_dir;
  float m_hdop, m_alt/* (m) */, m_geos/* (m) */, m_dgps_age /* (sec) */;
  short m_dgps_station;
  
  c_gga(): m_h(0), m_m(0), m_s(0.),
	   m_num_sats(0), m_lon_deg(0.), m_lat_deg(0.),
	   m_lon_dir(EGP_N), m_lat_dir(EGP_E), m_hdop(0), m_alt(0.),
	   m_geos(0.), m_dgps_age(0.), m_dgps_station(-1)
  {
  }

  virtual bool dec(const char * str);
  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    auto payload = builder.CreateStruct(NMEA0183::GGA(
						      m_h,
						      m_m,
						      (uint16_t)(m_s * 1000),
						      m_fix_status,
						      m_num_sats,
						      m_dgps_station,
						      m_hdop,
						      m_alt,
						      (m_lat_dir == EGP_N ? m_lat_deg : -m_lat_deg),
						      (m_lon_dir == EGP_E ? m_lon_deg : -m_lon_deg)));
						    
						    
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_GGA;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "GPGGA >";
    out << " UTC:" << m_h << ":" << m_m << ":" << m_s << endl;
    out << " SAT:" << m_num_sats;
    out << " POS(lt, ln, h):" 
	<< m_lat_deg << (m_lat_dir == EGP_N ? "N":"S") << ","
	<< m_lon_deg << (m_lon_dir == EGP_E ? "E":"W") << "," 
	<< m_alt << "(" << m_geos << ")";
    out << " HDOP:" << m_hdop;
    out << " DGPS ID:" << m_dgps_station 
	<< " DGPS Age:" << m_dgps_age;
    out << " CS:" << (m_cs ? "yes" : "no") << endl;
    return out;
  }
  virtual e_nd_type get_type() const
  {return ENDT_GGA;};
};

class c_gsa: public c_nmea_dat
{
public:
  NMEA0183::SelectionMeasurementMode smm; // Selection of the measurement mode {1: Auto, 2: Manual}
  NMEA0183::MeasurementMode mm; // Measurement mode {1: No Measurement, 2: 2D, 3: 3D}
  unsigned char sused[12]; // Satellited used for the calculation (upto 12 sats)
  float pdop, hdop, vdop;
  
  c_gsa():smm(NMEA0183::SelectionMeasurementMode_Manual),
	  mm(NMEA0183::MeasurementMode_NoMeasurement),
	  pdop(0.), hdop(0.), vdop(0.)
  {
    for(int i = 0; i< 12; i++)
      sused[i] = 0;
  }

  virtual bool dec(const char * str);
  
  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::GSA gsa(smm, mm, pdop, hdop, vdop);
    for (int i = 0; i < 12; i++)
      gsa.mutable_satellites()->Mutate(i, sused[i]);    
    auto payload = builder.CreateStruct(gsa); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_GSA;
  }

  virtual ostream & show(ostream & out) const
  {
    return out;
  };
  virtual e_nd_type get_type() const
  {
    return ENDT_GSA;
  };
};

class c_gsv: public c_nmea_dat
{
public:
  unsigned char ns /* number of sentence */, si /* sentence index */;
  unsigned short nsats_usable; // number of usable satellite.
  unsigned short sat[4], el[4], az[4], sn[4];
  c_gsv():ns(0), si(0), nsats_usable(0)
  {
    for(int i = 0; i < 4; i++){
      sat[i] = el[i] = sn[i] = 0;
    }
  }
  
  virtual bool dec(const char * str);

  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::GSV gsv(ns, si, nsats_usable,
		      NMEA0183::GSVSatelliteInformation(sat[0], el[0], az[0], sn[0]),
		      NMEA0183::GSVSatelliteInformation(sat[1], el[1], az[1], sn[1]),
		      NMEA0183::GSVSatelliteInformation(sat[2], el[2], az[2], sn[2]),
		      NMEA0183::GSVSatelliteInformation(sat[3], el[3], az[3], sn[3])
		      );
    auto payload = builder.CreateStruct(gsv); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_GSV;
  }
  
  virtual ostream & show(ostream & out) const
  {
    return out;
  };

  virtual e_nd_type get_type() const
  {
    return ENDT_GSV;
  };
};

// class for $rmc
class c_rmc: public c_nmea_dat
{
public:
  unsigned char m_h, m_m;
  float m_s;
  bool m_v;
  double m_lon_deg/* (deg) */, m_lat_deg/* (deg) */;
  e_gp_dir m_lon_dir, m_lat_dir;
  double m_vel, m_crs, m_crs_var;
  unsigned char m_yr, m_mn, m_dy;
  e_gp_dir m_crs_var_dir;
  NMEA0183::GPSFixStatus fs;
  c_rmc(): m_h(0), m_m(0), m_s(0.0), m_v(false),
	   m_lon_deg(0.0), m_lat_deg(0.0),
	   m_lon_dir(EGP_E), m_lat_dir(EGP_N),
	   m_vel(0.0), m_crs(0.0), m_crs_var(0.0),
	   m_yr(0), m_mn(0), m_dy(0), m_crs_var_dir(EGP_E)
  {
  }

  virtual bool dec(const char * str);

  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::RMC rmc(m_v, m_yr, m_mn, m_dy, m_h, m_m, m_s * 1000,
		      fs, m_vel, m_crs,
		      (m_crs_var_dir == EGP_E ? m_crs_var: -m_crs_var),
		      (m_lat_dir == EGP_N ? m_lat_deg : -m_lat_deg),
		      (m_lon_dir == EGP_E ? m_lon_deg : -m_lon_deg));
    auto payload = builder.CreateStruct(rmc); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_RMC;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "GPRMC>";
    out << " UTC:" << m_dy << "," << m_mn << "," << m_yr << "," << m_h << ":" << m_m << ":" << m_s;
    out << " Pos(lt, ln, h):" 
	<< m_lat_deg << (m_lat_dir == EGP_N ? "N":"S") << ","
	<< m_lon_deg << (m_lon_dir == EGP_E ? "E":"W");
    out << " CRS(var):" << m_crs << "(" << m_crs_var << (m_crs_var_dir == EGP_E ? "E":"W") << ")";
    out << " SPD:" << m_vel;
    out << " CS:" << (m_cs ? "yes" : "no") << endl;
    return out;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_RMC;};
};

class c_vtg: public c_nmea_dat
{
public:
  float crs_t, crs_m, v_n, v_k;
  NMEA0183::GPSFixStatus fs;
  c_vtg():crs_t(0.), crs_m(0.), v_n(0.), v_k(0.)
  {
  }

  virtual bool dec(const char * str);

  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::VTG vtg(fs, crs_t, crs_m, v_n, v_k);
    auto payload = builder.CreateStruct(vtg); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_VTG;
  }
  
  virtual ostream & show(ostream & out) const
  {
    return out;
  };

  virtual e_nd_type get_type() const
  {
    return ENDT_VTG;
  };
};

class c_zda: public c_nmea_dat
{
public:
  unsigned char m_h, m_m;
  float m_s;
  unsigned char m_dy, m_mn;
  unsigned short m_yr;
  unsigned char m_lzh, m_lzm;
  
  c_zda(): m_h(0), m_m(0), m_s(0), m_dy(0), m_mn(0),
	   m_yr(0), m_lzh(0), m_lzm(0)
  {
  }

  virtual bool dec(const char * str);
  
  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::ZDA zda(m_h, m_m, m_mn, m_dy, m_lzh, m_lzm,
		      m_s * 1000, m_yr);
    auto payload = builder.CreateStruct(zda); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_ZDA;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "ZDA>";
    out << " UTC:" << m_dy << "," << m_mn << "," << m_yr << "," << m_h << ":" << m_m << ":" << m_s;
    out << " CS:" << (m_cs ? "yes" : "no") << endl;
    return out;
  }
  virtual e_nd_type get_type() const
  {return ENDT_ZDA;};
};

class c_gll: public c_nmea_dat
{
 public:
  double lon, lat; // in degree
  e_gp_dir lon_dir, lat_dir;
  short hour, mint, msec; // UTC time. hour,minuite,msec
  bool available; // A(valid) or V(invalid)
  NMEA0183::GPSFixStatus fs;
 c_gll():lon(0),lat(0),lon_dir(EGP_E),lat_dir(EGP_N),
	 hour(0),mint(0),msec(0)
  {
  }

  virtual bool dec(const char * str);

  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::GLL gll(fs, available, hour, mint, msec,
		      (lat_dir == EGP_N ? lat : -lat),
		      (lon_dir == EGP_E ? lon : -lon));
    auto payload = builder.CreateStruct(gll); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_GLL;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "GPGLL>";
    out << " UTC: " << hour << ":" << mint << ":" << msec / 1000 << ":" << msec % 1000;
    out << " POS: " << lat << (lat_dir == EGP_N  ? "N":"S") << ","
	<< lon << (lon_dir == EGP_E ? "E":"W");
    out << " Available: " << (available ? "yes":"no") << endl;
    return out;
  }

  virtual e_nd_type get_type() const
  {return ENDT_GLL; };    
};

class c_hdt: public c_nmea_dat
{
 public:
  float hdg; // heading in degree
  
  c_hdt():hdg(0)
  {
  }

  virtual bool dec(const char * str);

  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::HDT hdt(hdg);
    auto payload = builder.CreateStruct(hdg); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_HDT;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "GPHDT>";
    out << " HDG: " << hdg << endl;
    return out;
  }

  virtual e_nd_type get_type() const
  {return ENDT_HDT;};    
};

class c_hev: public c_nmea_dat
{
public:
  float hev; // heave in meter

  c_hev():hev(0)
  {
  }
  
  virtual bool dec(const char * str);

   virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();

    auto payload = builder.CreateStruct(NMEA0183::HEV(hev)); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_HEV;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "GPHEV>";
    out << " HEV: " << hev << endl;
    return out;
  }

  virtual e_nd_type get_type() const
  {return ENDT_HEV;};    
};

class c_rot: public c_nmea_dat
{
 public:
  float rot;
  bool available;
  
  c_rot():rot(0),available(false)
  {
  }

  virtual bool dec(const char * str);

  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();

    auto payload = builder.CreateStruct(NMEA0183::ROT(available, rot)); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_ROT;
  }
  virtual ostream & show(ostream & out) const
  {
    out << "GPROT>";
    out << " ROT: " << rot <<
      " Available: " << (available ? "Yes":"No") << endl;
    return out;
  }

  virtual e_nd_type get_type() const
  {return ENDT_ROT;};
};


//////////////////////////////////////////////////// hemisphere's psat message

class c_psat: public c_nmea_dat
{
public:
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_PSAT;
  }
  
  virtual NMEA0183::PSATPayload get_psat_payload_type()
  {
    return NMEA0183::PSATPayload_NONE;
  }

};

class c_psat_hpr: public c_psat
{
 public:
  unsigned char hour, mint, sec;
  float hdg, pitch, roll; // attitude in degree;
  bool gyro; // true: values are from gyro, false: from gps

  c_psat_hpr():hour(0),mint(0),sec(0),hdg(0),pitch(0),roll(0),gyro(false)
  {
  }
  virtual bool dec(const char * str);
  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();

    auto payload = builder.CreateStruct(NMEA0183::HPR(hour, mint, sec, hdg,
						      pitch, roll, gyro));
    auto psat = CreatePSAT(builder, NMEA0183::PSATPayload_HPR, payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   psat.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::PSATPayload get_psat_payload_type()
  {
    return NMEA0183::PSATPayload_HPR;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << " PSAT,HPR>";
    out << " HDG: " << hdg << ", PITCH: " << pitch << ", ROLL: " << roll;
    out << " Source: " << (gyro ? "Gyro":"GPS") << endl;
    return out;
  }
  
  virtual e_nd_type get_type() const
  {
    return ENDT_PSAT_HPR;
  }
};

class c_psat_dec
{
protected:
  c_psat_hpr hpr;

  struct s_psat_obj{
    char id[3];
    c_nmea_dat * dat;
    s_psat_obj(const char * id_, c_nmea_dat * dat_):dat(dat_)
    {
      id[0] = id_[0];      id[1] = id_[1];      id[2] = id_[2];      
    }

    ~s_psat_obj(){
      if(dat){
	delete dat;
	dat = nullptr;
      }
    }

    bool match(const char * id_){
      return id[0] == id_[0] && id[1] == id_[1] && id[2] == id_[2];
    }
  };

  vector<s_psat_obj> psat_objs;
  c_nmea_dat * create_psat_dat(const char * sentence_id)
  {
    for(int i = 0 ; i <= NMEA0183::PSATPayload_MAX; i++){
      if(string(NMEA0183::EnumNamesPSATPayload()[i]) ==
	 string(sentence_id))
	switch(i){
	case NMEA0183::PSATPayload_HPR: return new c_psat_hpr;
	default:
	  break;
	}    
    }
    return nullptr;
  }
  
public:
  c_psat_dec()
  {
  }

  bool add_psat_dat(const char * sentence_id)
  {
    c_nmea_dat * dat = create_psat_dat(sentence_id);
    if(!dat)
      return false;

    for (int i = 0; i < psat_objs.size(); i++)
      if(psat_objs[i].match(sentence_id))
	return false;
    
    psat_objs.push_back(s_psat_obj(sentence_id, dat));
    return true;
  }
  
  c_nmea_dat * decode(const char * str, const long long t = 0);  
};


///////////////////////////////////////// Airmar weather station specific NMEA
class c_mda: public c_nmea_dat
{
public:
  float iom, bar, temp_air, temp_wtr, hmdr, hmda, dpt, dir_wnd_t, dir_wnd_m,  wspd_kts, wspd_mps;
  
  c_mda():iom(0.f), bar(0.f), temp_air(0.f), temp_wtr(0.f), hmdr(0.f), hmda(0.f), dpt(0.f), dir_wnd_t(0.f), dir_wnd_m(0.f), wspd_kts(0.f), wspd_mps(0.f)
  {
  }
  
  virtual bool dec(const char * str);

   virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::MDA mda(iom, bar, temp_air, temp_wtr, hmdr, hmda, dpt,
		      dir_wnd_t, dir_wnd_m, wspd_kts,wspd_mps);
    auto payload = builder.CreateStruct(mda); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_MDA;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "MDA>";
    out << " IOM: " << iom 
	<< " BAR: " << bar 
	<< " Tair: " << temp_air 
	<< " Twtr: " << temp_wtr 
	<< " HMDRel: " << hmdr 
	<< " HMDAbs: " << hmda 
	<< " DewPt: " << dpt 
	<< " TrueWindDir: " << dir_wnd_t
	<< " MagWindDir: " << dir_wnd_m
	<< " WinSpd(kts): " << wspd_kts
	<< " WinSpd(mps): " << wspd_mps << endl;    
    return out;
  }

  virtual e_nd_type get_type() const
  {return ENDT_MDA;};
};



class c_wmv: public c_nmea_dat
{
public:
  float wangl, wspd;
  NMEA0183::SpeedUnit spd_unit;
  bool relative; // otherwise, theoretical 
  bool valid;  
  c_wmv():wangl(0.f), wspd(0.f), spd_unit(NMEA0183::SpeedUnit_kmph),
	  relative(false), valid(false)
  {
  }
  
  virtual bool dec(const char * str);
   virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::WMV wmv((relative ?
		       NMEA0183::WindAngleMode_Relative :
		       NMEA0183::WindAngleMode_Theoretical),
		      spd_unit, wangl, wspd);
    auto payload = builder.CreateStruct(wmv); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_WMV;
  }
  
  virtual ostream & show(ostream & out) const
  {
    return out;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_WMV;};
};


// here we only assume type B xdr sentence defined by AIRMAR.
class c_xdr: public c_nmea_dat
{
public:
  float pitch, roll;
  
  c_xdr():pitch(0.f), roll(0.f)
  {
  }
  
  virtual bool dec(const char * str);
   virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::XDR xdr(pitch, roll);
    auto payload = builder.CreateStruct(xdr); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);    
  }
  
  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_XDR;
  }
  
  virtual ostream & show(ostream & out) const
  {
    out << "XDR(TypeB)>";
    out << " pitch: " << pitch <<
      " roll: " << roll << endl;
    return out;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_XDR;};
};

#endif
