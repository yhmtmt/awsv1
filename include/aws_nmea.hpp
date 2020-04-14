// Copyright(c) 2019-2020 Yohei Matsumoto, All right reserved. 

// aws_nmea.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_nmea.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_nmea.hpp.  If not, see <http://www.gnu.org/licenses/>. 

#ifndef AWS_NMEA_HPP
#define AWS_NMEA_HPP

#include "nmea0183_generated.h"

// NMEA data type
enum e_nd_type{
  /* GPS related NMEA message */
  ENDT_GGA, ENDT_GSA, ENDT_GSV, ENDT_RMC, ENDT_VTG, ENDT_ZDA, ENDT_GLL,
  ENDT_HDT, ENDT_HEV, ENDT_ROT,
  /* Hemisphere V104 specific message */  
  ENDT_PSAT,
  /* Airmar WX220 specific message */
  ENDT_MDA, ENDT_WMV, ENDT_XDR,
  /* ARPA related NMEA message */
  ENDT_TTM,
  /* Fish Finder's NMEA message */
  ENDT_DBT, ENDT_MTW,
  /* AIS related NMEA message */
  ENDT_VDM, ENDT_VDO, ENDT_ABK, 
  /* Autopilot related NMEA message */
  ENDT_APB, ENDT_AAM, ENDT_BOD, ENDT_BWC, ENDT_XTE, ENDT_RMB, ENDT_APA,
  /* Undefined NMEA message */
  ENDT_UNDEF,
  
  /* sub type for decoded VDM message */
  ENDT_VDM1, ENDT_VDM4, ENDT_VDM5, ENDT_VDM6,
  ENDT_VDM8, ENDT_VDM18, ENDT_VDM19, ENDT_VDM24,

  /* Hemisphere V104 specific message */  
  ENDT_PSAT_HPR
};

extern const char * str_nd_type[ENDT_UNDEF];
e_nd_type get_nd_type(const char * str);

bool eval_nmea_chksum(const char * str);
unsigned char calc_nmea_chksum(const char * str);
unsigned int htoi(const char * str);
bool parstrcmp(const char * str1, const char * str2);
int parstrcpy(char * str, const char * src, int num);
int parstrcpy(char * str, const char * src, char delim, int max_buf = 32);

class c_nmea_dec;

/////////////////////////// c_nmea_dat and its inheritant
class c_nmea_dat
{
protected:
  flatbuffers::FlatBufferBuilder builder;
public:
  c_nmea_dat():builder(256){}
  
  char m_toker[2];
  bool m_cs;
  virtual ostream & show(ostream & out) const
  {
    return out;
  };

  virtual bool dec(const char * str){ return true; };
  virtual bool decode(const char * str, const long long t = -1){
    return true;
  }
  
  const uint8_t * get_buffer_pointer() const
  { return builder.GetBufferPointer(); }
  
  flatbuffers::uoffset_t get_buffer_size() const
  { return builder.GetSize();};
  
  virtual NMEA0183::Payload get_payload_type(){
    return NMEA0183::Payload_NONE;
  }
  
  virtual e_nd_type get_type() const = 0;
};

#include "aws_nmea_gps.hpp"

/////////////////////// c_ttm (from ARPA)
class c_ttm: public c_nmea_dat
{
public:
  short m_id;				// field1 the target number 0 to 99 (but we found 100 in JRC's ARPA)
  float m_dist;			// field 2 the target distance (mile or kilometer, specified later)
  float m_bear;			// field 3 the target bearing from own ship (in degree)
  bool m_is_bear_true;	// field 4 true if target bearing is true direction otherwise relative direction
  float m_spd;			// field 5 the target speed in knot or km/h.
  float m_crs;			// field 6 the target course
  float m_is_crs_true;	// field 7 true if the target cousre is true direction otherwise relative direction
  float m_dcpa;			// field 8 Distance of closest point of approach.
  float m_tcpa;			// field 9 Time of closest point of approach
  char m_dist_unit;		// field 10 Distance unit. K=Kilometer, N=Nautical-mile, S=Statute-mile
  char m_data[20];		// field 11 User data (typicaly a name)
  char m_state;			// field 12 Tracking status. L=Lost, Q=Under Acquisition, T=Under Tracking
  bool m_is_ref;			// field 13 Reference target flag
  char m_utc_h, m_utc_m, m_utc_s, m_utc_ms;			 
  // field 14 UTC hour, minute, second, milisecond
  bool m_is_auto;			// field 15 true if the target is automaticaly acquisited
  
  virtual ostream & show(ostream & out) const
  {
    out << "TTM>";
    out << " ID:" << m_id 
	<< " DIST:" << m_dist << "(" << m_dist_unit << "M)"
	<< " BEAR:" << m_bear << (m_is_bear_true ? "(deg, T)" : "(deg, R)")
	<< " SPD:" << m_spd << (m_dist_unit == 'K' ? "(K/h)" : (m_dist_unit == 'N' ? "(KT)" : (m_dist_unit == 'S' ? "(MPH)" : "(xxx)")))
	<< " CRS:" << m_crs << (m_is_crs_true ? "(deg, T)" : "(deg, R)")
	<< " DCPA:" << m_dcpa << "(" << m_dist_unit << "M)"
	<< " TCPA:" << m_tcpa << "(MIN)"
	<< " DATA:" << m_data
	<< " STATE:" << (m_state == 'L' ? "LOST": (m_state == 'Q' ? "ACQ" : (m_state == 'T' ? "TRC" : "???")))
	<< " REF:" << (m_is_ref ? "TRUE":"FALSE")
	<< " UTC:" << (int) m_utc_h << ":" << (int) m_utc_m << ":" << (int) m_utc_s << "." << (int) m_utc_ms
	<< " AUTO:" << (m_is_auto ? "TRUE":"FALSE") << endl;
    return out;
  }
  
  virtual bool dec(const char * str);
  
  virtual e_nd_type get_type() const
  {return ENDT_TTM;};
};


//////////////////////// c_bdt (from fish finder)
class c_dbt: public c_nmea_dat
{
public:
  float dfe /* depth in feet */, dm /* depth in meter */, dfa /*depth in fathomas*/;
  
  c_dbt():dfe(0), dm(0), dfa(0)
  {
  }

  virtual bool dec(const char * str);
  
  virtual e_nd_type get_type() const
  {
    return ENDT_DBT;
  }
  virtual ostream & show(ostream & out) const
  {
    out << "DBT: " << dfe << "ft " << dm << "m " << dfa << "Ft" << endl;
    return out;
  }
};

//////////////////////// c_mtw (from fish finder)
class c_mtw: public c_nmea_dat
{
public:
  float t;
  c_mtw():t(0.0)
  {
  }
  
  virtual bool dec(const char * str);
  
  virtual e_nd_type get_type() const
  {
    return ENDT_MTW;
  }
};

#include "aws_nmea_ais.hpp"

// nmea decoder class 
// Usage : Instantiate an object, and call decode method with NMEA string as an argument. 
// * decode method returns an NMEA data object, the object is allocated in the decoder object.
// * The decoder object should not be used in multiple threads. 
class c_nmea_dec
{
protected:
  struct s_nmea0183_obj{
    char id[3]; // 3 characters
    c_nmea_dat * dat;
    s_nmea0183_obj():dat(nullptr)
    {
    }
    s_nmea0183_obj(const char * id_, c_nmea_dat * dat_):dat(dat_)
    {
      id[0] = id_[0];      id[1] = id_[1];      id[2] = id_[2];
    }
    
    ~s_nmea0183_obj(){
      if(dat){
	delete dat;
	dat = nullptr;
      }
    }
    
    bool match(const char * id_){
      return id[0] == id_[0] && id[1] == id_[1] && id[2] == id_[2];
    }    
  };
  
  vector<s_nmea0183_obj> nmea0183_objs;

  c_nmea_dat * create_nmea0183_dat(const char * sentence_id)
  {
    for(int i = 0; i <= NMEA0183::Payload_MAX; i++){
      if(string(NMEA0183::EnumNamesPayload()[i]) == string(sentence_id)){
	switch (i){
	case NMEA0183::Payload_NONE:break;
    	case NMEA0183::Payload_GGA:return new c_gga;
    	case NMEA0183::Payload_GSA:return new c_gsa;
    	case NMEA0183::Payload_GSV:return new c_gsv;
    	case NMEA0183::Payload_RMC:return new c_rmc;
    	case NMEA0183::Payload_VTG:return new c_vtg;
    	case NMEA0183::Payload_ZDA:return new c_zda;
    	case NMEA0183::Payload_GLL:return new c_gll;
    	case NMEA0183::Payload_HDT:return new c_hdt;
    	case NMEA0183::Payload_HEV:return new c_hev;
    	case NMEA0183::Payload_ROT:return new c_rot;
    	case NMEA0183::Payload_MDA:return new c_mda;
    	case NMEA0183::Payload_WMV:return new c_wmv;
    	case NMEA0183::Payload_XDR:return new c_xdr;
    	case NMEA0183::Payload_TTM:return new c_ttm;
    	case NMEA0183::Payload_DBT:return new c_dbt;
    	case NMEA0183::Payload_MTW:return new c_mtw;
    	case NMEA0183::Payload_ABK:return new c_abk;
	default:
	  break;
	}
      }
    }

    return nullptr;
  }
        
  c_psat_dec psatdec;
  c_vdm_dec vdmdec;
  c_vdm_dec vdodec;
public:
  c_nmea_dec()
  {
    vdodec.set_vdo();
  }
  
  const c_nmea_dat * decode(const char * str, const long long t = -1);
  
  bool add_nmea0183_decoder(const char * sentence_id){
    c_nmea_dat * dat = create_nmea0183_dat(sentence_id);
    if(!dat)
      return false;
    
    for (int i = 0; i < nmea0183_objs.size(); i++)
      if(nmea0183_objs[i].match(sentence_id))
	return false; // the sentence has already been registered
  
    nmea0183_objs.push_back(s_nmea0183_obj(sentence_id, dat));
    return true;
  }
  
  bool add_nmea0183_vdm_decoder(int message_id)
  {
    return vdmdec.add_vdm_dat(message_id);
  }
  
  bool add_nmea0183_vdo_decoder(int message_id)
  {
    return vdodec.add_vdm_dat(message_id);
  }
  
  bool add_psat_decoder(const char * sentence_id)
  {
    return psatdec.add_psat_dat(sentence_id);
  }
  
};


#endif
