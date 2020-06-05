// Copyright(c) 2016-2020 Yohei Matsumoto, All right reserved. 

// aws_nmea_gps.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_nmea_gps.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_nmea_gps.cpp.  If not, see <http://www.gnu.org/licenses/>. 
#include <cstdio>
#include <stdlib.h>
#include <wchar.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include "aws_sock.hpp"
#include "aws_thread.hpp"

using namespace std;

#include "aws_nmea.hpp"

/////////////////////////////////////////// gga decoder
bool c_gga::decode(const char * str, const long long t)
{
  m_h = m_m = 0;
  m_s = 0.0f;
  m_fix_status = NMEA0183::GPSFixStatus_LOST;
  m_num_sats = 0;
  m_lon_deg = m_lat_deg = 0.0;
  m_lon_dir = EGP_N;
  m_lat_dir = EGP_E;
  m_hdop = m_alt = m_geos = m_dgps_age = 0.0;
  m_dgps_station = 0;
    
  if(!dec(str))
    return false;
  builder.Clear();
  auto payload = CreateGGA(builder,
			   m_h,
			   m_m,
			   (uint16_t)(m_s * 1000),
			   m_fix_status,
			   m_num_sats,
			   m_dgps_station,
			   m_hdop,
			   m_alt,
			   m_geos,
			   (m_lat_dir == EGP_N ? m_lat_deg : -m_lat_deg),
			   (m_lon_dir == EGP_E ? m_lon_deg : -m_lon_deg));

  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}

bool c_gga::dec(const char * str)
{
  c_nmea_dat::dec(str);
  
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  while(ipar < 15){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $GPGGA 
      break;
    case 1: // TIME hhmmss
      parstrcpy(tok, buf, 2);
      m_h = (short) atoi(tok);
      parstrcpy(tok, buf+2, 2);
      m_m = (short) atoi(tok);
      parstrcpy(tok, buf+4, '\0');
      m_s = (float) atof(tok);
      break;
    case 2: // LAT
      parstrcpy(tok, buf, 2);
      m_lat_deg = atof(tok);
      parstrcpy(tok, buf+2, '\0');
      m_lat_deg += atof(tok) / 60.0;
      break;
    case 3: // N or S
      if(buf[0] == 'N')
	m_lat_dir = EGP_N;
      else
	m_lat_dir = EGP_S;
      break;				
    case 4: // LON
      parstrcpy(tok, buf, 3);
      m_lon_deg = atof(tok);
      parstrcpy(tok, buf + 3, '\0');
      m_lon_deg += atof(tok) / 60;
      break;
    case 5: // E or W
      if(buf[0] == 'E')
	m_lon_dir = EGP_E;
      else
	m_lon_dir = EGP_W;
      break;
    case 6: // Fix Stats
      m_fix_status = (NMEA0183::GPSFixStatus)(buf[0] - '0');
    case 7: // NUM_SATS
      m_num_sats = atoi(buf);
      break;
    case 8: // HDOP
      m_hdop = (float) atof(buf);
      break;
    case 9: // Altitude
      m_alt = (float) atof(buf);
      break;
    case 10: // M
      break;
    case 11:// Geoidal separation
      m_geos = (float) atof(buf);
      break;
    case 12: // M
      break;
    case 13: // dgps age
      m_dgps_age = (float) atof(buf);
      break;
    case 14: // dgps station id
      if(parstrcpy(tok, buf, '*'))
	m_dgps_station = atoi(buf);
      break;
    }
    
    ipar++;
  }
  
  return true;
}

bool c_gga::encode(char *str)
{
  c_nmea_dat::encode(str);
  str[3] = 'G';
  str[4] = 'G';
  str[5] = 'A';
  str[6] = ',';
  char * p = str + 7;
  p += snprintf(p, 12, "%02d%02d%02.3f,", m_h, m_m, m_s);
  {    
    int lat_deg = (int)m_lat_deg;        
    double lat_min = (m_lat_deg - lat_deg) * 60;
    if(m_lat_deg > 0)
      p += snprintf(p, 13, "%02d%02.4f,N,", lat_deg, lat_min);
    else
      p += snprintf(p, 13, "%02d%02.4f,S,", -lat_deg, lat_min);    
  }
  {
    int lon_deg = (int)m_lon_deg;
    double lon_min = (m_lon_deg - lon_deg) * 60;
    if(m_lon_deg > 0)
      p += snprintf(p, 14, "%03d%02.4f,E,",lon_deg, lon_min);
    else
      p += snprintf(p, 14, "%03d%02.4f,W,",lon_deg, lon_min);
  }

  p += snprintf(p, 3, "%01d,", (int) m_fix_status);
  p += snprintf(p, 4, "%02d,", m_num_sats);  
  p += snprintf(p, 5, "%1.1f,", m_hdop);  
  p += snprintf(p, 13, "%6.2f,M,", m_alt);
  p += snprintf(p, 10, "%3.2f,M,", m_geos);
  if(m_fix_status == NMEA0183::GPSFixStatus_DGPSF)
    p += snprintf(p, 14, "%3.3f,%04d*", m_dgps_age, m_dgps_station);
  else
    p += snprintf(p, 3, ",*");
  
  p += snprintf(p, 3, "%02x", calc_nmea_chksum(str));
  return true;
}


/////////////////////////////////////////// gsa decoder
bool c_gsa::decode(const char * str, const long long t)
{
  smm = NMEA0183::SelectionMeasurementMode_Auto;
  mm = NMEA0183::MeasurementMode_NoMeasurement;
  memset(sused, 0, 12);
  pdop = hdop = vdop = 0.0f;
    
  if(!dec(str))
    return false;
  builder.Clear();
  auto vec = builder.CreateVector(sused, 12);
  auto payload = CreateGSA(builder, smm, mm, vec, pdop, hdop, vdop);
  
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}

bool c_gsa::dec(const char * str)
{
  c_nmea_dat::dec(str);    
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];

  while(ipar < 18){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }

    switch(ipar){
    case 0: // $GPGSA
      break;
    case 1: // Selected measurement mode
      if(buf[0] == 'A')
	smm = NMEA0183::SelectionMeasurementMode_Auto;
      else if(buf[0] == 'M')
	smm = NMEA0183::SelectionMeasurementMode_Manual;
      else
	smm = NMEA0183::SelectionMeasurementMode_Manual;
      break;
    case 2: // Measurement mode
      if(buf[0] == '1')
	mm = NMEA0183::MeasurementMode_NoMeasurement;
      else if(buf[0] == '2')
	mm = NMEA0183::MeasurementMode_TwoDimensional;
      else if(buf[0] == '3')
	mm = NMEA0183::MeasurementMode_ThreeDimensional;
      break;
    case 3: // sat1
    case 4: // sat2
    case 5: // sat3
    case 6: // sat4
    case 7: // sat5
    case 8: // sat6
    case 9: // sat7
    case 10: // sat8
    case 11: // sat9
    case 12: // sat10
    case 13: // sat11
    case 14: // sat12
      if(buf[0] != '\0'){
	sused[ipar - 3] = (unsigned char) atoi(buf);
      }
      break;
    case 15: // PDOP
      pdop = (float) atof(buf);
      break;
    case 16: // HDOP
      hdop = (float) atof(buf);
      break;
    case 17: // VDOP
      vdop = (float) atof(buf);
      break;
    }
    ipar++;
  }

  return true;
}

/////////////////////////////////////////// gsv decoder
bool c_gsv::decode(const char * str, const long long t)
{
  ns = 0;
  nsats_usable = 0;
  memset(sat, 0, sizeof(unsigned short) * 4);
  memset(el, 0, sizeof(unsigned short) * 4);
  memset(az, 0, sizeof(unsigned short) * 4);
  memset(sn, 0, sizeof(unsigned short) * 4);    
   
  if(!dec(str))
    return false;
  builder.Clear();
  auto sat0 = NMEA0183::GSVSatelliteInformation(sat[0], el[0], az[0], sn[0]);
  auto sat1 = NMEA0183::GSVSatelliteInformation(sat[1], el[1], az[1], sn[1]);
  auto sat2 = NMEA0183::GSVSatelliteInformation(sat[2], el[2], az[2], sn[2]);
  auto sat3 = NMEA0183::GSVSatelliteInformation(sat[3], el[3], az[3], sn[3]);
  
  auto payload = CreateGSV(builder,ns, si, (unsigned char)nsats_usable,
			   &sat0, &sat1, &sat2, &sat3);

  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}
  

bool c_gsv::dec(const char * str)
{
  c_nmea_dat::dec(str);    
  int i = 0;
  int ipar = 0, npar = 20;
  int isat = 0;
  int len;
  char buf[32];

  while(ipar < npar){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }

    switch(ipar){
    case 0: // $GPGSV
      break;
    case 1: // Number of sentences
      if(buf[0] < '0' || buf[0] > '4')
	ns = 0;
      else
	ns = buf[0] - '0';
      break;
    case 2: // Sentence index
      if(buf[0] < '0' || buf[0] > '4')
	si = 0;
      else
	si = buf[0] - '0';
      break;
    case 3: // Number of usable satellites 
      nsats_usable = atoi(buf);
      npar = nsats_usable - (si - 1) * 4;
      npar = min(4, npar);
      npar = npar * 4 + 4;
      isat = 0;
      break;
    case 4: // sat1
    case 8: // sat2
    case 12: // sat3
    case 16: // sat4
      sat[isat] = (unsigned short) atoi(buf);
      break;
    case 5: // elevation of sat1
    case 9: // elevation of sat2
    case 13: // elevation of sat3
    case 17: // elevation of sat4
      el[isat] = (unsigned short) atoi(buf);
      break;
    case 6: // azimuth of sat1
    case 10: // azimuth of sat2
    case 14: // azimuth of sat3
    case 18: // azimuth of sat4
      az[isat] = (unsigned short) atoi(buf);
      break;
    case 7: // sn
    case 11: // sn
    case 15: // sn
    case 19: // sn
      sn[isat] = (unsigned short) atoi(buf);
      isat++;
      break;
    }
    ipar++;
  }

  return true;
}

/////////////////////////////////////////// rmc decoder
bool c_rmc::decode(const char * str, const long long t)
{
  m_h = m_m = m_yr = m_mn = m_dy = 0;
  m_s = 0.0f;
  m_lon_deg = m_lat_deg = 0.0;
  m_lon_dir = EGP_E;
  m_lat_dir = EGP_N;
  m_vel  = m_crs = m_crs_var = 0.0;
  m_crs_var_dir = EGP_E;
  fs = NMEA0183::GPSFixStatus_LOST;
  
  if(!dec(str))
    return false;
  builder.Clear();
  auto payload = CreateRMC(builder, m_v, m_yr, m_mn, m_dy, m_h, m_m, m_s * 1000,
		    fs, m_vel, m_crs,
		    (m_crs_var_dir == EGP_E ? m_crs_var: -m_crs_var),
		    (m_lat_dir == EGP_N ? m_lat_deg : -m_lat_deg),
		    (m_lon_dir == EGP_E ? m_lon_deg : -m_lon_deg));
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
    
}

bool c_rmc::dec(const char * str)
{
    c_nmea_dat::dec(str);  
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];

  while(ipar < 13){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }

    switch(ipar){
    case 0: // $GPRMC
      break;
    case 1: // TIME hhmmss
      parstrcpy(tok, buf, 2);
      m_h = (short) atoi(tok);
      parstrcpy(tok, buf+2, 2);
      m_m = (short) atoi(tok);
      parstrcpy(tok, buf+4, '\0');
      m_s = (float) atof(tok);
      break;
    case 2: // Validity flag
      if(buf[0] == 'A')
	m_v = true;
      else
	m_v = false;
      break;
    case 3: // Lat
      parstrcpy(tok, buf, 2);
      m_lat_deg = (float) atof(tok);
      parstrcpy(tok, buf+2, '\0');
      m_lat_deg += atof(tok) / 60;
      break;
    case 4: // N or S
      if(buf[0] == 'N')
	m_lat_dir = EGP_N;
      else
	m_lat_dir = EGP_S;
      break;				
    case 5: // LON
      parstrcpy(tok, buf, 3);
      m_lon_deg = (float) atof(tok);
      parstrcpy(tok, buf + 3, '\0');
      m_lon_deg += atof(tok) / 60;
      break;
    case 6: // E or W
      if(buf[0] == 'E')
	m_lon_dir = EGP_E;
      else
	m_lon_dir = EGP_W;
      break;
    case 7: // Speed
      m_vel = atof(buf);
      break;
    case 8: // Course
      m_crs = atof(buf);
      break;
    case 9: // Date
      parstrcpy(tok, buf, 2);
      m_dy = atoi(tok);
      parstrcpy(tok, buf+2, 2);
      m_mn = atoi(tok);
      parstrcpy(tok, buf+4, 2);
      m_yr = atoi(tok);
      break;
    case 10: // Course Variation
      m_crs_var = atof(buf);
      break;
    case 11: // Direction of Variation
      if(buf[0] == 'E')
	m_crs_var_dir = EGP_E;
      else
	m_crs_var_dir = EGP_W;
      break;
    case 12:
      if(buf[0] == 'N')
	fs = NMEA0183::GPSFixStatus_LOST;
      else if(buf[0] == 'A')
	fs = NMEA0183::GPSFixStatus_GPSF;
      else if(buf[0] == 'D')
	fs = NMEA0183::GPSFixStatus_DGPSF;
      else if(buf[0] == 'E')
	fs = NMEA0183::GPSFixStatus_ESTM;
      break;
    }
    ipar++;
  }

  return true;
}

/////////////////////////////////////////// vtg decoder
bool c_vtg::decode(const char * str, const long long t)
{
  crs_t = crs_m = v_n = v_k = 0.0f;
    
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = CreateVTG(builder, fs, crs_t, crs_m, v_n, v_k);
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}


bool c_vtg::dec(const char * str)
{
  c_nmea_dat::dec(str);  
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];

  while(ipar < 10){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }

    switch(ipar){
    case 0: // $**VTG
      break;
    case 1: // course (True)
      crs_t = (float)atof(buf);
      break;
    case 2: // 'T'
      if(buf[0] != 'T')
	goto vtgerror;
      break;
    case 3: // course (Magnetic)
      crs_m = (float)atof(buf);
      break;
    case 4: // 'M'
      if(buf[0] != 'M')
	goto vtgerror;
      break;				
    case 5: // velocity (kts)
      v_n = (float)atof(buf);
      break;
    case 6: // 'N'
      if(buf[0] != 'N')
	goto vtgerror;
      break;
    case 7: // velocity (km/h)
      v_k = (float)atof(buf);
      break;
    case 8: // 'K'
      if(buf[0] != 'K')
	goto vtgerror;
      break;
    case 9: // Fix status
      if(buf[0] == 'N')
	fs = NMEA0183::GPSFixStatus_LOST;
      else if(buf[0] == 'A')
	fs = NMEA0183::GPSFixStatus_GPSF;
      else if(buf[0] == 'D')
	fs = NMEA0183::GPSFixStatus_DGPSF;
      else if(buf[0] == 'E')
	fs = NMEA0183::GPSFixStatus_ESTM;
      else
	goto vtgerror;
    }
    ipar++;
  }

  return true;
 vtgerror:
  return false;
}

bool c_vtg::encode(char * str)
{
  c_nmea_dat::encode(str);  
  str[3] = 'V';
  str[4] = 'T';
  str[5] = 'G';
  str[6] = ',';
  char * p = str + 7;
  p += snprintf(p, 9, "%03.1f,T,", crs_t);
  p += snprintf(p, 9, "%03.1f,M,", crs_m);
  p += snprintf(p, 9, "%03.1f,N,", v_n);
  p += snprintf(p, 9, "%03.1f,K,", v_k);  
  switch(fs){
  case NMEA0183::GPSFixStatus_LOST:
    *p = 'N'; ++p;
    break;
  case NMEA0183::GPSFixStatus_GPSF:
    *p = 'A'; ++p;
    break;
  case NMEA0183::GPSFixStatus_DGPSF:
    *p = 'D'; ++p;
    break;
  case NMEA0183::GPSFixStatus_ESTM:
    *p = 'E'; ++p;
    break;    
  }
  *p = '*'; ++p;
  p += snprintf(p, 3, "%02x", calc_nmea_chksum(str));
  return true;
}

////////////////////////////////////////////////zda decoder
bool c_zda::decode(const char * str, const long long t)
{
  m_h = m_m = m_dy = m_mn = m_lzh = m_lzm = 0;
  m_s = 0.f;
  m_yr = 0;
    
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = NMEA0183::CreateZDA(builder, m_h, m_m, m_mn, m_dy,
				     m_lzh, m_lzm,
				     (unsigned short)(m_s * 1000),
				     (unsigned short)m_yr);
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}


bool c_zda::dec(const char * str)
{
  c_nmea_dat::dec(str);  
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 7){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $GPZDA
      break;
    case 1: // TIME hhmmss
      parstrcpy(tok, buf, 2);
      m_h = (short) atoi(tok);
      parstrcpy(tok, buf+2, 2);
      m_m = (short) atoi(tok);
      parstrcpy(tok, buf+4, '\0');
      m_s = (float) atof(tok);
      break;
    case 2: // day
      m_dy = atoi(buf);
      break;
    case 3: // month
      m_mn = atoi(buf);
      break;
    case 4: // year
      m_yr = atoi(buf);
      break;				
    case 5: // local zone hour offset
      m_lzh = atoi(buf);
      break;
    case 6: // local zone minute offset
      m_lzm = atoi(buf);
      break;
    }
    ipar++;
  }
  
  return true;
}

bool c_zda::encode(char * str)
{
  c_nmea_dat::encode(str);
  str[3] = 'Z';
  str[4] = 'D';
  str[5] = 'A';
  str[6] = ',';
  char * p = str + 7;
  p += snprintf(p, 12, "%02d%02d%02.3f,", (int)m_h, (int)m_m, m_s);
  p += snprintf(p, 19, "%02d,%02d,%04d,%02d,%02d*", (int)m_dy, (int)m_mn, m_yr,
		m_lzh, m_lzm);
  *p = '*'; ++p;
  p += snprintf(p, 3, "%02x", calc_nmea_chksum(str));
  return true;
}


////////////////////////////////////////////////gll decoder
bool c_gll::decode(const char * str, const long long t)
{
  fs = NMEA0183::GPSFixStatus_LOST;
  lon = lat = 0.0;
  hour = mint = msec = 0;
  available = false;
    
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = CreateGLL(builder, fs, available, hour, mint, msec,
			   (lat_dir == EGP_N ? lat : -lat),
			   (lon_dir == EGP_E ? lon : -lon));
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}


bool c_gll::dec(const char * str)
{
  c_nmea_dat::dec(str);    
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 8){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $GPGLL
      break;
    case 1: // lattitude: ddmm.mmmm
      parstrcpy(tok, buf, 2);
      lat = (double) atoi(tok);
      parstrcpy(tok, buf+2, '\0');
      lat += (double) (atof(tok) * (1.0/60.0));
      break;
    case 2: // N or S
      if(buf[0] == 'N')
	lat_dir = EGP_N;
      else
	lat_dir = EGP_S;
      break;				    
    case 3: // longitude dddmm.mmmm
      parstrcpy(tok, buf, 3);
      lon = atof(tok);
      parstrcpy(tok, buf + 3, '\0');
      lon += atof(tok) * (1.0/ 60.0);
      break;
    case 4: // E or W
      if(buf[0] == 'E')
	lon_dir = EGP_E;
      else
	lon_dir = EGP_W;
      break;				
    case 5: // time hhmmss.ss
      parstrcpy(tok, buf, 2);
      hour = (short) atoi(tok);
      parstrcpy(tok, buf+2, 2);
      mint = (short) atoi(tok);
      parstrcpy(tok, buf+4, '\0');
      msec = (short) (atof(tok) * 1000);
      break;
    case 6: // availability
      if(buf[0] == 'A')
	available = true;
      else
	available = false;
      break;
    case 7:
      if(buf[0] == 'N')
	fs = NMEA0183::GPSFixStatus_LOST;
      else if(buf[0] == 'A')
	fs = NMEA0183::GPSFixStatus_GPSF;
      else if(buf[0] == 'D')
	fs = NMEA0183::GPSFixStatus_DGPSF;
      else if(buf[0] == 'E')
	fs = NMEA0183::GPSFixStatus_ESTM;
      break;      
    }
    ipar++;
  }
  
  return true;
}


////////////////////////////////////////////////hdt decoder
bool c_hdt::decode(const char * str, const long long t)
{
  hdg = 0.0f;
    
  if(!dec(str))
    return false;
  builder.Clear();
  auto payload = NMEA0183::CreateHDT(builder, hdg); 
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}

bool c_hdt::dec(const char * str)
{
  c_nmea_dat::dec(str);    
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 3){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $GPHDT
      break;
    case 1: // heading
      hdg = (float)atof(buf);
      break;
    case 2: // T
      break;				    
    }
    ipar++;
  }
  
  return true;
}

////////////////////////////////////////////////hev decoder
bool c_hev::decode(const char * str, const long long t)
{
  hev = 0.0f;
    
  if(!dec(str))
    return false;

  builder.Clear();
  auto payload = NMEA0183::CreateHEV(builder, hev); 
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}


bool c_hev::dec(const char * str)
{
  c_nmea_dat::dec(str);    
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 3){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $GPHEV
      break;
    case 1: // heading
      hev = (float)atof(buf);
      break;
    case 2: // T
      break;				    
    }
    ipar++;
  }
  
  return true;
}

////////////////////////////////////////////////rot decoder
bool c_rot::decode(const char * str, const long long t)
{
  rot = 0.0f;
    
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = NMEA0183::CreateROT(builder, available, rot); 
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}

bool c_rot::dec(const char * str)
{
  c_nmea_dat::dec(str);  
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 3){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $GPROT
      break;
    case 1: // Rate of Turn
      rot = (float)atof(buf);
      break;
    case 2: // A or not
      if (buf[0] == 'A')
	available = true;
      else
	available = false;
      break;				    
    }
    ipar++;
  }
  
  return true;
}

//////////////////////////////////////////////// hemisphere's psat decoder
c_nmea_dat * c_psat_dec::decode(const char * str, const long long t)
{
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];

  while (ipar < 2){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){
      ipar++;
      continue;
    }
    switch(ipar){
    case 0: // $PSAT
      break;
    case 1: // HPR or GBS or INTLT
      for(int ipsat = 0; ipsat < psat_objs.size(); ipsat++){
	if(psat_objs[ipsat].match(buf)){
	  if(psat_objs[ipsat].dat->decode(&str[i],t)){
	    c_nmea_dat * dat = psat_objs[ipsat].dat;
	    if(dat){
	      dat->m_toker[0] = str[1];
	      dat->m_toker[1] = str[2];
	      dat->m_cs = true;
	    }
	    return dat;	  
	  }
	  return nullptr;
	}
      }
    }
    ipar++;
  }
  
  return nullptr;
}

////////////////////////////////////////////////////// hpr decoder
bool c_psat_hpr::decode(const char * str, const long long t)
{
  hour = mint = 0;
  sec = 0.0f;
  hdg = pitch = roll = 0.0f;
  gyro = false;
    
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = NMEA0183::CreateHPR(builder, hour, mint,
				     (unsigned short)(sec * 1000),
				     hdg,
				     pitch, roll, gyro);
  auto psat = CreatePSAT(builder, NMEA0183::PSATPayload_HPR, payload.Union());
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 psat.Union());
    
  builder.Finish(data);    
}

bool c_psat_hpr::dec(const char * str)
{
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  while(ipar < 5){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    switch(ipar){
    case 0: // time
      parstrcpy(tok, buf, 2);
      hour = (short) atoi(tok);
      parstrcpy(tok, buf+2, 2);
      mint = (short) atoi(tok);
      parstrcpy(tok, buf+4, '\0');
      sec = (float) atof(tok);
      break;
    case 1: // heading
      hdg = (float)atof(buf);
      break;
    case 2: // pitch
      pitch = (float)atof(buf);
      break;
    case 3: // roll
      roll = (float)atof(buf);
      break;
    case 4: // from GPS or Gyro
      if(buf[0] == 'N')
	gyro = false;
      else
	gyro = true;
      break;
    }
    ipar++;
  }
 
  return true;
}


bool c_psat_hpr::encode(char * str)
{
  str[0] = '$';
  str[1] = 'P';
  str[2] = 'S';
  str[3] = 'A';
  str[4] = 'T';
  str[5] = ',';
  str[6] = 'H';
  str[7] = 'P';
  str[8] = 'R';
  str[9] = ',';
  
  char * p = str + 10;
  p += snprintf(p, 12, "%02d%02d%02.3f,", hour, mint, sec);
  p += snprintf(p, 25, "%04.2f,%04.2f,%04.2f,", hdg, pitch, roll);
  *p = gyro ? 'G' : 'N'; ++p;
  *p = '*';++p;
  p += snprintf(p, 3, "%02x", calc_nmea_chksum(str));
  return true;
}

///////////////////////////////////////// Airmar weather station specific NMEA
bool c_mda::decode(const char * str, const long long t)
{
  iom = bar = temp_air = temp_wtr = hmdr = hmda
    = dpt = dir_wnd_t = dir_wnd_m = wspd_kts = wspd_mps = 0.0f;
    
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = NMEA0183::CreateMDA(builder, iom, bar, temp_air,
				     temp_wtr, hmdr, hmda, dpt,
				     dir_wnd_t, dir_wnd_m, wspd_kts,wspd_mps);
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}

bool c_mda::dec(const char * str)
{
  c_nmea_dat::dec(str);      
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 21){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    switch(ipar){
    case 0: // $WIMDA
      break;
    case 1: // iom
      iom = atof(buf);
      break;
    case 2: // I
      if (buf[0] != 'I')
	return false;
      break;
    case 3: // bar
      bar = atof(buf);
      break;
    case 4: // B
      if(buf[0] != 'B')
	return false;
      break;				
    case 5: //temp_air
      temp_air = atof(buf);
      break;
    case 6: // C
      if(buf[0] != 'C')
	return false;
      break;
    case 7: // temp_wtr
      temp_wtr = atof(buf);
      break;
    case 8: // C
      if(buf[0] != 'C')
	return false;
      break;
    case 9: //hmdr
      hmdr = atof(buf);
      break;
    case 10://hmda
      hmda = atof(buf);
      break;
    case 11: // dpt
      dpt = atof(buf);
      break;
    case 12: // C
      if(buf[0] != 'C')
	return false;
      break;
    case 13:
      dir_wnd_t = atof(buf);
      break;
    case 14:
      if(buf[0] != 'T')
	return false;
      break;
    case 15:
      dir_wnd_m = atof(buf);
      break;
    case 16:
      if(buf[0] != 'M')
	return false;
      break;
    case 17:
      wspd_kts = atof(buf);
      break;
    case 18:
      if(buf[0] != 'N')
	return false;
      break;
    case 19:
      wspd_mps = atof(buf);
      break;
    case 20:
      if(buf[0] != 'M')
	return false;
      break;      
    }
    ipar++;
  }
  
  return true;
}

///////////////////////////////////////////////////////////// wmv decoder
bool c_wmv::decode(const char * str, const long long t)
{
  wangl = wspd = 0.0f;
  spd_unit = NMEA0183::SpeedUnit_kmph;
  relative = valid = false;
     
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = CreateWMV(builder, (relative ?
				     NMEA0183::WindAngleMode_Relative :
				     NMEA0183::WindAngleMode_Theoretical),
			   spd_unit, wangl, wspd);
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}

bool c_wmv::dec(const char * str)
{
  c_nmea_dat::dec(str);      
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 6){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $WIWMV
      break;
    case 1: // wangl
      wangl = atof(buf);
      break;
    case 2: // R or T
      if (buf[0] == 'R')
	relative = true;
      else if (buf[0] == 'T')
	relative = false;
      else
	return false;
      break;
    case 3: // wspd
      wspd = atof(buf);
      break;
    case 4: // K or M or N or S
      switch(buf[0]){
      case 'K':
	spd_unit = NMEA0183::SpeedUnit_kmph;
	break;       
      case 'M':
	spd_unit = NMEA0183::SpeedUnit_mps;
	break;
      case 'N':
	spd_unit = NMEA0183::SpeedUnit_knots;
	break;
      case 'S':
	spd_unit = NMEA0183::SpeedUnit_mph;
	break;
      }
      break;				
    case 5: // valid or not
      if(buf[0] == 'A')
	valid = true;
      else if(buf[0] == 'V')
	valid = false;
      else
	return false;	
      break;
    }
    ipar++;
  }
  
  return true;
}

////////////////////////////////////////////////////////// xdr decoder
bool c_xdr::decode(const char * str, const long long t)
{
  pitch = roll = 0.0f;
    
  if(!dec(str))
    return false;
  builder.Clear();

  auto payload = NMEA0183::CreateXDR(builder, pitch, roll); 
  auto data = CreateData(builder,
			 t,
			 get_payload_type(),
			 payload.Union());
    
  builder.Finish(data);    
}


bool c_xdr::dec(const char * str)
{
  c_nmea_dat::dec(str);      
  int i = 0;
  int ipar = 0;
  int len;
  char buf[32];
  char tok[32];
  
  while(ipar < 9){
    len = parstrcpy(buf, &str[i], ',');
    i += len + 1;
    if(len == 0){ 
      ipar++;
      continue;
    }
    
    switch(ipar){
    case 0: // $YXXDR
      break;
    case 1: // A
      if(buf[0] != 'A')
	return false;
      break;
    case 2: // Pitch
      pitch = atof(buf);
      break;
    case 3: // D
      if(buf[0] != 'D')
	return false;
      break;
    case 4: // PITCH
      if(strcmp(buf, "PITCH") != 0 )
	return false;
      break;				
    case 5: // A
      if(buf[0] != 'A')
	return false;
      break;
    case 6: // Roll
      roll = atof(buf);
      break;
    case 7: // D
      if(buf[0] != 'D')
	return false;
      break;
    case 8: // ROLL
      if(strcmp(buf, "ROLL") != 0)
	return false;
      break;
    }
    ipar++;
  }
  
  return true;
}

