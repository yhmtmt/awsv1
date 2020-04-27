// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// ch_state.cpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_state.cpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_state.cpp.  If not, see <http://www.gnu.org/licenses/>.

#include "aws_math.hpp"
#include "ch_state.hpp"

///////////////////////////////////////////////////////////////////////// ch_state
size_t ch_state::write_buf(const char * buf)
{
  const long long *lptr = (const long long*)buf;	
  const float * ptr = (const float*)(lptr + 6);
  const double * dptr = (const double*)(ptr + 12);
  
  set_attitude(lptr[2], ptr[0], ptr[1], ptr[2]);
  set_velocity(lptr[3], ptr[3], ptr[4]);
  
  lock();
  tpos = lptr[0];
  lat = dptr[0];
  lon = dptr[1];
  x = dptr[3];
  y = dptr[4];
  z = dptr[5];
  memcpy((void*)R, (void*)(dptr+6), sizeof(double)* 9);
  
  talt = lptr[1];
  alt = dptr[2];
    
  tdp = lptr[4];
  depth = ptr[5];
  
  twx = lptr[5];
  bar = ptr[6];
  temp_air = ptr[7];
  hmdr = ptr[8];
  dew = ptr[9];
  dir_wnd_t = ptr[10];
  wspd_mps = ptr[11];
  
  unlock();
  return get_dsize();
}

size_t ch_state::read_buf(char * buf)
{
  lock();
  long long * lptr = (long long *)buf;
  lptr[0] = tpos;
  lptr[1] = talt;
  lptr[2] = tatt;
  lptr[3] = tvel;
  lptr[4] = tdp;
  lptr[5] = twx;
  
  float * ptr = (float*)(lptr + 6);
  ptr[0] = roll;
  ptr[1] = pitch;
  ptr[2] = yaw;
  ptr[3] = cog;
  ptr[4] = sog;
  ptr[5] = depth;
  ptr[6] =  bar;
  ptr[7] = temp_air;
  ptr[8] = hmdr;
  ptr[9] = dew;
  ptr[10] = dir_wnd_t;
  ptr[11] = wspd_mps;
  
  double * dptr = (double*)(ptr + 12);
  dptr[0] = lat;
  dptr[1] = lon;
  dptr[2] = alt;
  dptr[3] = x;
  dptr[4] = y;
  dptr[5] = z;
  
  memcpy((void*)(dptr+6), (void*)(R), sizeof(double)* 9);
  unlock();
  return get_dsize();
}

int ch_state::write(FILE * pf, long long tcur)
{
  if (!pf)
    return 0;
  
  int sz = 0;
  if (tdp <= tdpf && tvel <= tvelf && tatt <= tattf && tpos <= tposf
      && talt <= taltf){
    return sz;
  }
  
  sz = sizeof(long long)* 6;
  
  lock();
  fwrite((void*)&tcur, sizeof(long long), 1, pf);
  fwrite((void*)&tpos, sizeof(long long), 1, pf);
  
  fwrite((void*)&lat, sizeof(double), 1, pf);
  fwrite((void*)&lon, sizeof(double), 1, pf);
  tposf = tpos;
  
  fwrite((void*)&talt, sizeof(long long), 1, pf);	
  fwrite((void*)&alt, sizeof(double), 1, pf);
  
  sz += sizeof(float)* 3;
  
  fwrite((void*)&tatt, sizeof(long long), 1, pf);
  fwrite((void*)&roll, sizeof(float), 1, pf);
  fwrite((void*)&pitch, sizeof(float), 1, pf);
  fwrite((void*)&yaw, sizeof(float), 1, pf);
  tattf = tatt;
  sz += sizeof(float)* 3;

  fwrite((void*)&tvel, sizeof(long long), 1, pf);
  fwrite((void*)&cog, sizeof(float), 1, pf);
  fwrite((void*)&sog, sizeof(float), 1, pf);
  tvelf = tvel;
  sz += sizeof(float)* 2;
  
  fwrite((void*)&tdp, sizeof(long long), 1, pf);
  fwrite((void*)&depth, sizeof(float), 1, pf);
  tdpf = tdp;
  sz += sizeof(float);
  
  fwrite((void*)&twx, sizeof(long long), 1, pf);
  fwrite((void*)&bar, sizeof(float), 1, pf);
  fwrite((void*)&temp_air, sizeof(float), 1, pf);
  fwrite((void*)&hmdr, sizeof(float), 1, pf);
  fwrite((void*)&dew, sizeof(float), 1, pf);
  fwrite((void*)&dir_wnd_t, sizeof(float), 1, pf);
  fwrite((void*)&wspd_mps, sizeof(float), 1, pf);
  twxf = twx;
  sz += sizeof(float) * 6;
  
  m_tfile = tcur;
  unlock();
  return sz;
}

int ch_state::read(FILE * pf, long long tcur)
{
  if (!pf)
    return 0;
  
  int sz = 0;
  if (tposf < tcur && tposf != tpos){
    set_position(tposf, latf, lonf);
  }
  
  if(taltf < tcur && taltf != talt){
    set_alt(taltf, altf);
  }
  
  if (tattf < tcur && tattf != tatt){
    set_attitude(tattf, rollf, pitchf, yawf);
  }
  
  if (tvelf < tcur && tvelf != tvel){
    set_velocity(tvelf, cogf, sogf);
  }
  
  if (tdpf < tcur && tdpf != tdp){
    set_depth(tdpf, depthf);
  }  
  
  while (!feof(pf)){
    if (m_tfile > tcur){
      break;
    }
    
    lock();
    size_t res = 0;
    res += fread((void*)&m_tfile, sizeof(long long), 1, pf);
    
    res += fread((void*)&tposf, sizeof(long long), 1, pf);
    res += fread((void*)&latf, sizeof(double), 1, pf);
    res += fread((void*)&lonf, sizeof(double), 1, pf);

    res += fread((void*)&taltf, sizeof(long long), 1, pf);
    res += fread((void*)&altf, sizeof(double), 1, pf);
    
    res += fread((void*)&tattf, sizeof(long long), 1, pf);
    res += fread((void*)&rollf, sizeof(float), 1, pf);
    res += fread((void*)&pitchf, sizeof(float), 1, pf);
    res += fread((void*)&yawf, sizeof(float), 1, pf);
    
    res += fread((void*)&tvelf, sizeof(long long), 1, pf);
    res += fread((void*)&cogf, sizeof(float), 1, pf);
    res += fread((void*)&sogf, sizeof(float), 1, pf);
		
    res += fread((void*)&tdpf, sizeof(long long), 1, pf);
    res += fread((void*)&depthf, sizeof(float), 1, pf);

    res += fread((void*)&twxf, sizeof(long long), 1, pf);
    res += fread((void*)&barf, sizeof(float), 1, pf);
    res += fread((void*)&temp_airf, sizeof(float), 1, pf);
    res += fread((void*)&hmdrf, sizeof(float), 1, pf);
    res += fread((void*)&dewf, sizeof(float), 1, pf);
    res += fread((void*)&dir_wnd_tf, sizeof(float), 1, pf);
    res += fread((void*)&wspd_mpsf, sizeof(float), 1, pf);
    sz = res;
    unlock();
  }
  return sz;
}

bool ch_state::log2txt(FILE * pbf, FILE * ptf)
{
  fprintf(ptf, "t, tpos, talt, tatt, tvel, tdp, twx, lat, lon, alt, yaw, pitch, roll, cog, sog, depth, bar, temp_air, hmdr, dew, dir_wnd_t, wspd_mps\n");
  while (!feof(pbf)){
    long long t, tmax = 0;
	  
    size_t res = 0;
    res += fread((void*)&m_tfile, sizeof(long long), 1, pbf);
    
    res += fread((void*)&t, sizeof(long long), 1, pbf);
    res += fread((void*)&lat, sizeof(double), 1, pbf);
    res += fread((void*)&lon, sizeof(double), 1, pbf);
    tposf = tpos = t;
    
    res += fread((void*)&t, sizeof(long long), 1, pbf);
    res += fread((void*)&alt, sizeof(float), 1, pbf);
    taltf = talt = t;		
    
    res += fread((void*)&t, sizeof(long long), 1, pbf);
    res += fread((void*)&roll, sizeof(float), 1, pbf);
    res += fread((void*)&pitch, sizeof(float), 1, pbf);
    res += fread((void*)&yaw, sizeof(float), 1, pbf);
    tattf = tatt = t;
    
    res += fread((void*)&t, sizeof(long long), 1, pbf);
    res += fread((void*)&cog, sizeof(float), 1, pbf);
    res += fread((void*)&sog, sizeof(float), 1, pbf);
    
    tvelf = tvel = t;
    
    res += fread((void*)&t, sizeof(long long), 1, pbf);
    res += fread((void*)&depth, sizeof(float), 1, pbf);
    tdpf = tdp = t;
    
    res += fread((void*)&t, sizeof(long long), 1, pbf);
    res += fread((void*)&bar, sizeof(float), 1, pbf);
    res += fread((void*)&temp_air, sizeof(float), 1, pbf);
    res += fread((void*)&hmdr, sizeof(float), 1, pbf);
    res += fread((void*)&dew, sizeof(float), 1, pbf);
    res += fread((void*)&dir_wnd_t, sizeof(float), 1, pbf);
    res += fread((void*)&wspd_mps, sizeof(float), 1, pbf);        
    twxf = twx = t;
    
    fprintf(ptf, "%lld, %lld, %lld, %lld, %lld, %lld, %lld, %+013.8lf, %+013.8lf, %+06.1lf, %+06.1f, %+06.1f, %+06.1f, %+06.1f, %+06.1f, %+06.1f, %+06.1f, %+06.1f,%+06.1f,%+06.1f,%+06.1f,%+06.1f \n",
	    m_tfile, tpos, talt, tatt, tvel, tdp, twx, lat, lon, alt, yaw, pitch, roll, cog, sog, depth, bar, temp_air, hmdr, dew, dir_wnd_t, wspd_mps);
  }
  return true;
}

//////////////////////////////////////////////////////////////////////////////////////// ch_eng_state

const char * strStatEng1[NMEA2000::EngineStatus1_MAX+1] = {
    "Check Engine", "Over Temperature", "Low Oil Pressure", "Low Oil Level", "Low Fuel Pressure",
    "Low System Voltage", "Low Coolant Level", "Water Flow", "Water In Fuel", "Charge Indicator",
    "Preheat Indicator", "High Boost Pressure", "Rev Limit Exceeded", "EGR System",
    "Throttle Position Sensor", "Emergency Stop"
};;

const char * strStatEng2[NMEA2000::EngineStatus2_MAX+1] = {
  "Warning Level1", "Warning Level2", "Power Reduction", "Maintenance Needed",
  "Engine Comm Error", "Subor Secondary Throttle", "Neutral Start Protect",
  "Engine Shutting Down"
};

const char * strStatGear[NMEA2000::GearStatus_MAX+1] = {
  "Forward", "Neutral", "Reverse"
};

size_t ch_eng_state::write_buf(const char * buf)
{
  lock();
  const long long * lptr = (const long long*)buf;
  t = lptr[0];
  trapid = lptr[1];
  tdyn = lptr[2];
  ttran = lptr[3];
  ttrip = lptr[4];
  
  const float * fptr = (const float*)(lptr + 5);
  rpm = fptr[0];
  toil = fptr[1];
  temp = fptr[2];
  valt = fptr[3];
  frate = fptr[4];
  tgoil = fptr[5];
  flavg = fptr[6];
  fleco = fptr[7];
  flinst = fptr[8];

  const unsigned char * ucptr = (const unsigned char*)(fptr + 9);
  trim = ucptr[0];
  ld = ucptr[1];
  tq = ucptr[2];
  
  const int * iptr = (const int *)(ucptr + 3);
  poil = iptr[0];
  pclnt = iptr[1];
  pfl = iptr[2];
  pgoil = iptr[3];
  flused = iptr[4];

  const unsigned int * uiptr = (const unsigned int*)(iptr + 5);
  teng = uiptr[0];

  const NMEA2000::EngineStatus1 * se1ptr = (const NMEA2000::EngineStatus1 *)(uiptr + 1);
  stat1 = *se1ptr;

  const NMEA2000::EngineStatus2 * se2ptr = (const NMEA2000::EngineStatus2 *)(se1ptr + 1);
  stat2 = *se2ptr;

  const NMEA2000::GearStatus * sgptr = (const NMEA2000::GearStatus *)(se2ptr +1);
  gear = *sgptr;

  unlock();
  return get_dsize();
}

size_t ch_eng_state::write_buf_back(const char * buf)
{
  lock();
  const long long * lptr = (const long long*)buf;
  tf = lptr[0];
  trapidf = lptr[1];
  tdynf = lptr[2];
  ttranf = lptr[3];
  ttripf = lptr[4];
  
  const float * fptr = (const float*)(lptr + 5);
  rpmf = fptr[0];
  toilf = fptr[1];
  tempf = fptr[2];
  valtf = fptr[3];
  fratef = fptr[4];
  tgoilf = fptr[5];
  flavgf = fptr[6];
  flecof = fptr[7];
  flinstf = fptr[8];

  const unsigned char * ucptr = (const unsigned char*)(fptr + 9);
  trimf = ucptr[0];
  ldf = ucptr[1];
  tqf = ucptr[2];
  
  const int * iptr = (const int *)(ucptr + 3);
  poilf = iptr[0];
  pclntf = iptr[1];
  pflf = iptr[2];
  pgoilf = iptr[3];
  flusedf = iptr[4];

  const unsigned int * uiptr = (const unsigned int*)(iptr + 5);
  tengf = uiptr[0];

  const NMEA2000::EngineStatus1 * se1ptr = (const NMEA2000::EngineStatus1 *)(uiptr + 1);
  stat1f = *se1ptr;

  const NMEA2000::EngineStatus2 * se2ptr = (const NMEA2000::EngineStatus2 *)(se1ptr + 1);
  stat2f = *se2ptr;

  const NMEA2000::GearStatus * sgptr = (const NMEA2000::GearStatus *)(se2ptr +1);
  gearf = *sgptr;

  unlock();
  return get_dsize();
}

size_t ch_eng_state::read_buf(char * buf)
{
  lock();
  long long * lptr = (long long*)buf;
  lptr[0] = t;
  lptr[1] =  trapid;
  lptr[2] = tdyn;
  lptr[3] = ttran;
  lptr[4] = ttrip;
  
  float * fptr = (float*)(lptr + 5);
  fptr[0] = rpm;
  fptr[1] = toil;
  fptr[2] = temp;
  fptr[3] = valt;
  fptr[4] = frate;
  fptr[5] = tgoil;
  fptr[6] = flavg;
  fptr[7] = fleco;
  fptr[8] = flinst;

  unsigned char * ucptr = (unsigned char*)(fptr + 9);
  ucptr[0] = trim;
  ucptr[1] = ld;
  ucptr[2] = tq;
  
  int * iptr = (int *)(ucptr + 3);
  iptr[0] = poil;
  iptr[1] = pclnt;
  iptr[2] = pfl;
  iptr[3] = pgoil;
  iptr[4] = flused;

  unsigned int * uiptr = (unsigned int*)(iptr + 5);
  uiptr[0] = teng;

  NMEA2000::EngineStatus1 * se1ptr = (NMEA2000::EngineStatus1 *)(uiptr + 1);
  *se1ptr = stat1;

  NMEA2000::EngineStatus2 * se2ptr = (NMEA2000::EngineStatus2 *)(se1ptr + 1);
  *se2ptr = stat2;

  NMEA2000::GearStatus * sgptr = (NMEA2000::GearStatus *)(se2ptr +1);
  *sgptr = gear;
  
  unlock();
  return get_dsize();
}

int ch_eng_state::write(FILE * pf, long long tcur)
{
  if (!pf)
    return 0;

  if (t <= tf){
    return 0;
  }

  int sz = (int)get_dsize();

  tf = tcur;
  read_buf(buf);
  (*(long long*) buf) = tcur;
  fwrite(buf, sizeof(char), sz, pf);
  
  return sz + sizeof(long long);
}

int ch_eng_state::read(FILE * pf, long long tcur)
{
  if (!pf)
    return 0;

  if(trapidf <= tcur){
    set_rapid(trapidf, rpmf, trimf);
  }

  if(tdynf <= tcur){
    set_dynamic(tdynf, poilf, toilf, tempf,
		valtf, fratef, tengf, pclntf, pflf, stat1f, stat2f, ldf, tqf);
  }

  if(ttranf <= tcur){
    set_tran(ttranf, gearf, pgoilf, tgoilf);
  }

  if(ttripf <= tcur){
    set_trip(ttripf, flusedf, flavgf, flecof, flinstf);
  }

  // reading next data record
  size_t sz = get_dsize();
  
  while (!feof(pf)){
    if (tf > tcur){
      break;
    }

    int res;
    res = fread(buf, sizeof(char), sz, pf);
    write_buf_back(buf);
  }

  return 0;
}

bool ch_eng_state::log2txt(FILE * pbf, FILE * ptf)
{
  fprintf(ptf, "trec, trapid, tdyn, ttran, ttrip, rpm, trim, poil, toil, temp, valt, frate, teng, pclnt, pfl, stat1, stat2, ld, tq, gear, pgoil, tgoil, flused, flavg, fleco, flinst\n");
  size_t sz = get_dsize();
  while (!feof(pbf)){

    int res;
    res = fread(buf, sizeof(char), sz, pbf);
    write_buf(buf);

    fprintf(ptf, "%lld, %lld, %lld, %lld, %lld,", t, trapid, tdyn, ttran, ttrip);
    const char * str1, * str2, * str3;
    str1 = str2 = str3 = "null";
    
    if((int)stat1 <= (int)NMEA2000::EngineStatus1_MAX && (int)stat1 >= 0){
      str1 = strStatEng1[stat1];
    }
    if((int)stat2 <= (int)NMEA2000::EngineStatus2_MAX && (int)stat2 >= 0){
      str2 = strStatEng2[stat2];
    }
    if((int)gear <= (int)NMEA2000::GearStatus_MAX && (int) gear >= 0){
      str3 = strStatGear[gear];
    }
    
    fprintf(ptf, "%f, %d, %d, %03.2f, %02.1f, %02.2f, %03.2f, %u, %d, %d, %s, %s, %u, %u,",
	    rpm, (int)trim,
	    poil, toil, temp, valt, frate, teng, pclnt, pfl, str1,
	    str2, (unsigned int)ld, (unsigned int)tq);

    fprintf(ptf, "%s, %d, %03.2f,", str3, pgoil, tgoil);
    fprintf(ptf, "%d, %03.2f, %03.2f, %03.2f", flused, flavg, fleco, flinst);
    
    fprintf(ptf, "\n");
  }
  return true;
}

void ch_eng_state::print(ostream & out)
{
  out << "engstate rpm=" << rpm << endl;
}
