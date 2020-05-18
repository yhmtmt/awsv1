#include <iostream>
#include <cmath>
#include <list>
#include <utility>
using namespace std;

#include "gtest/gtest.h"
#include "aws_nmea.hpp"

class NMEATest: public ::testing::Test
{
protected:
  static const char * sentences[29];
  c_nmea_dec dec;
  virtual void SetUp(){
    dec.add_nmea0183_decoder("RMC");
    dec.add_nmea0183_decoder("GGA");    
    dec.add_nmea0183_decoder("GSA");
    dec.add_nmea0183_decoder("GSV");
    dec.add_nmea0183_decoder("VTG");
    dec.add_nmea0183_decoder("GLL");
    dec.add_nmea0183_decoder("ZDA");
    dec.add_psat_decoder("HPR");

    dec.add_nmea0183_vdm_decoder(1); // 1
    dec.add_nmea0183_vdm_decoder(2); // 2
    dec.add_nmea0183_vdm_decoder(3); // 3
    dec.add_nmea0183_vdm_decoder(5); // 5
    dec.add_nmea0183_vdm_decoder(6); // 6
    dec.add_nmea0183_vdm_decoder(8); // 7   
    dec.add_nmea0183_vdm_decoder(18);// B
    dec.add_nmea0183_vdm_decoder(19);// C
    dec.add_nmea0183_vdm_decoder(24);// H

    dec.add_nmea0183_vdo_decoder(1);
    dec.add_nmea0183_vdo_decoder(2);
    dec.add_nmea0183_vdo_decoder(3);
    dec.add_nmea0183_vdo_decoder(5);
    dec.add_nmea0183_vdo_decoder(6);
    dec.add_nmea0183_vdo_decoder(8);        
    dec.add_nmea0183_vdo_decoder(18);
    dec.add_nmea0183_vdo_decoder(19);
    dec.add_nmea0183_vdo_decoder(24);
  }
};

const char * NMEATest::sentences[29] = {
  "$GPRMC,085120.307,A,3541.1493,N,13945.3994,E,000.0,240.3,181211,,,A*6A",
  "$GPGGA,085120.307,3541.1493,N,13945.3994,E,1,08,1.0,6.9,M,35.9,M,,0000*5E",
  "$GPGSA,A,3,29,26,05,10,02,27,08,15,,,,,1.8,1.0,1.5*3E",
  "$GPGSV,3,1,12,26,72,352,28,05,65,066,37,15,50,268,35,27,33,189,37*7F",
  "$GPVTG,240.3,T,,M,000.0,N,000.0,K,A*08",
  // 5
  "$GPGLL,4916.45,N,12311.12,W,225444,A,*1D",
  "$GPZDA,201530.00,04,07,2002,00,00*60",
  "$PSAT,HPR,170921.60,27.77,-3.18,,N*24",
  "!AIVDM,1,1,,A,13u?etPv2;0n:dDPwUM1U1Cb069D,0*24",
  "!AIVDM,1,1,,A,400TcdiuiT7VDR>3nIfr6>i00000,0*78",
  //10
  "!AIVDM,2,1,0,A,58wt8Ui`g??r21`7S=:22058<v05Htp000000015>8OA;0sk,0*7B",
  "!AIVDM,2,2,0,A,eQ8823mDm3kP00000000000,2*5D",
  "!AIVDM,1,1,4,B,6>jR0600V:C0>da4P106P00,2*02",
  "!AIVDM,2,1,9,B,61c2;qLPH1m@wsm6ARhp<ji6ATHd<C8f=Bhk>34k;S8i=3To,0*2C",
  "!AIVDM,2,2,9,B,Djhi=3Di<2pp=34k>4D,2*03",
  //15
  "!AIVDM,1,1,1,B,8>h8nkP0Glr=<hFI0D6??wvlFR06EuOwgwl?wnSwe7wvlOw?sAwwnSGmwvh0,0*17",
  "!AIVDO,1,1,,A,95M2oQ@41Tr4L4H@eRvQ;2h20000,0*0D",
  "!AIVDM,1,1,,B,;8u:8CAuiT7Bm2CIM=fsDJ100000,0*51",
  "!AIVDM,1,1,,B,>>M4fWA<59B1@E=@,0*17",
  "!AIVDM,1,1,,A,B6CdCm0t3`tba35f@V9faHi7kP06,0*58",
  //20
  "!AIVDM,2,1,0,B,C8u:8C@t7@TnGCKfm6Po`e6N`:Va0L2J;06HV50JV?SjBPL3,0*28",
  "!AIVDM,2,2,0,B,11RP,0*17",
  "!AIVDO,2,1,5,B,E1c2;q@b44ah4ah0h:2ab@70VRpU<Bgpm4:gP50HH`Th`QF5,0*7B",
  "!AIVDO,2,2,5,B,1CQ1A83PCAH0,0*60",
  "!AIVDO,1,1,,B,H1c2;qA@PU>0U>060<h5=>0:1Dp,2*7D",
  //25
  "!AIVDO,1,1,,B,H1c2;qDTijklmno31<<C970`43<1,0*28",
  "!AIVDM,1,1,,A,KCQ9r=hrFUnH7P00,0*41",
  "!AIVDM,1,1,,B,KC5E2b@U19PFdLbMuc5=ROv62<7m,0*16",
  "!AIVDM,1,1,,B,K5DfMB9FLsM?P00d,0*70"
};


TEST_F(NMEATest, GPRMCTest)
{
  // testing gprmc decode
  const c_nmea_dat * dat = dec.decode(sentences[0]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_RMC);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::RMC * rmc = data->payload_as_RMC();
  ASSERT_TRUE(rmc != nullptr);
  ASSERT_TRUE(rmc->measured() == true);
  ASSERT_TRUE(rmc->year() == 11);
  ASSERT_TRUE(rmc->month() == 12);
  ASSERT_TRUE(rmc->day() == 18);  
  ASSERT_TRUE(rmc->hour() == 8);
  ASSERT_TRUE(rmc->minute() == 51);  
  ASSERT_TRUE(rmc->msec() == 20307);
  ASSERT_TRUE(rmc->fixStatus() == NMEA0183::GPSFixStatus_GPSF);
  ASSERT_FLOAT_EQ(rmc->sog(), 0);
  ASSERT_FLOAT_EQ(rmc->cog(), 240.3);
  ASSERT_FLOAT_EQ(rmc->var(), 0);
  ASSERT_FLOAT_EQ(rmc->latitude(), 35.0 + (41.1493 / 60.0));
  ASSERT_FLOAT_EQ(rmc->longitude(), 139.0 + (45.3994 / 60.0));
}

TEST_F(NMEATest, GPGGATest)
{
  const c_nmea_dat * dat = dec.decode(sentences[1]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_GGA);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::GGA * gga = data->payload_as_GGA();
  ASSERT_TRUE(gga != nullptr);
  ASSERT_TRUE(gga->numSatellites() == 8);  
  ASSERT_TRUE(gga->DGPSStation() == 0);  
  ASSERT_TRUE(gga->hour() == 8);
  ASSERT_TRUE(gga->minute() == 51);  
  ASSERT_TRUE(gga->msec() == 20307);
  ASSERT_TRUE(gga->fixStatus() == NMEA0183::GPSFixStatus_GPSF);
  ASSERT_FLOAT_EQ(gga->altitude(), 6.9);  
  ASSERT_FLOAT_EQ(gga->dop(), 1.0);
  ASSERT_FLOAT_EQ(gga->geoid(),35.9); 
  ASSERT_FLOAT_EQ(gga->latitude(), 35.0 + (41.1493 / 60.0));
  ASSERT_FLOAT_EQ(gga->longitude(), 139.0 + (45.3994 / 60.0));

  // encode test
  c_gga gga_dec, gga_enc;
  char buf[85];
  gga_dec.decode(sentences[1]);
  gga_dec.encode(buf);
  gga_enc.decode(buf);
  ASSERT_EQ(gga_dec.m_h, gga_enc.m_h);
  ASSERT_EQ(gga_dec.m_m, gga_enc.m_m);
  ASSERT_FLOAT_EQ(gga_dec.m_s, gga_enc.m_s);  
  ASSERT_FLOAT_EQ(gga_dec.m_lat_deg, gga_enc.m_lat_deg);
  ASSERT_FLOAT_EQ(gga_dec.m_lon_deg, gga_enc.m_lon_deg);
  ASSERT_EQ(gga_dec.m_fix_status, gga_enc.m_fix_status);
  ASSERT_EQ(gga_dec.m_num_sats, gga_enc.m_num_sats);  
  ASSERT_EQ(gga_dec.m_lon_dir, gga_enc.m_lon_dir);
  ASSERT_EQ(gga_dec.m_lat_dir, gga_enc.m_lat_dir);  
  ASSERT_FLOAT_EQ(gga_dec.m_hdop, gga_enc.m_hdop);
  ASSERT_FLOAT_EQ(gga_dec.m_alt, gga_enc.m_alt);
  ASSERT_FLOAT_EQ(gga_dec.m_geos, gga_enc.m_geos);
  ASSERT_FLOAT_EQ(gga_dec.m_dgps_age, gga_enc.m_dgps_age);    
  ASSERT_EQ(gga_dec.m_dgps_station, gga_enc.m_dgps_station);  
}

TEST_F(NMEATest, GPGSATest)
{
  const c_nmea_dat * dat = dec.decode(sentences[2]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_GSA);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::GSA * gsa = data->payload_as_GSA();
  ASSERT_TRUE(gsa != nullptr);
  ASSERT_EQ(gsa->selectionMeasurementMode(), NMEA0183::SelectionMeasurementMode_Auto);
  ASSERT_EQ(gsa->measurementMode(), NMEA0183::MeasurementMode_ThreeDimensional);
  ASSERT_EQ(gsa->satellites()->Data()[0], 29);
  ASSERT_EQ(gsa->satellites()->Data()[1], 26);
  ASSERT_EQ(gsa->satellites()->Data()[2], 05);
  ASSERT_EQ(gsa->satellites()->Data()[3], 10);
  ASSERT_EQ(gsa->satellites()->Data()[4], 2);
  ASSERT_EQ(gsa->satellites()->Data()[5], 27);
  ASSERT_EQ(gsa->satellites()->Data()[6], 8);
  ASSERT_EQ(gsa->satellites()->Data()[7], 15);
  ASSERT_EQ(gsa->satellites()->Data()[8], 0);
  ASSERT_EQ(gsa->satellites()->Data()[9], 0);
  ASSERT_EQ(gsa->satellites()->Data()[10], 0);
  ASSERT_EQ(gsa->satellites()->Data()[11], 0);      
  
  ASSERT_FLOAT_EQ(gsa->pdop(), 1.8);  
  ASSERT_FLOAT_EQ(gsa->hdop(), 1.0);
  ASSERT_FLOAT_EQ(gsa->vdop(), 1.5); 
}

TEST_F(NMEATest, GPGSVTest)
{
  const c_nmea_dat * dat = dec.decode(sentences[3]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_GSV);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::GSV * gsv = data->payload_as_GSV();
  ASSERT_TRUE(gsv != nullptr);
  ASSERT_EQ(gsv->numSentences(), 3);
  ASSERT_EQ(gsv->sentence(), 1);
  ASSERT_EQ(gsv->numSatellitesUsable(), 12);
  ASSERT_EQ(gsv->satelliteInformation0().satellite(), 26);
  ASSERT_EQ(gsv->satelliteInformation0().elevation(), 72);
  ASSERT_EQ(gsv->satelliteInformation0().azimus(), 352);
  ASSERT_EQ(gsv->satelliteInformation0().sn(), 28);  
  ASSERT_EQ(gsv->satelliteInformation1().satellite(), 5);
  ASSERT_EQ(gsv->satelliteInformation1().elevation(), 65);
  ASSERT_EQ(gsv->satelliteInformation1().azimus(), 66);
  ASSERT_EQ(gsv->satelliteInformation1().sn(), 37);  
  ASSERT_EQ(gsv->satelliteInformation2().satellite(), 15);
  ASSERT_EQ(gsv->satelliteInformation2().elevation(), 50);
  ASSERT_EQ(gsv->satelliteInformation2().azimus(), 268);
  ASSERT_EQ(gsv->satelliteInformation2().sn(), 35);  
  ASSERT_EQ(gsv->satelliteInformation3().satellite(), 27);
  ASSERT_EQ(gsv->satelliteInformation3().elevation(), 33);
  ASSERT_EQ(gsv->satelliteInformation3().azimus(), 189);
  ASSERT_EQ(gsv->satelliteInformation3().sn(), 37);  
}

TEST_F(NMEATest, GPGLLTest)
{
  const c_nmea_dat * dat = dec.decode(sentences[5]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_GLL);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::GLL * gll = data->payload_as_GLL();
  ASSERT_TRUE(gll != nullptr);
  ASSERT_EQ(gll->hour(), 22);
  ASSERT_EQ(gll->minute(), 54);  
  ASSERT_EQ(gll->msec(), 44000);
  ASSERT_EQ(gll->measured(), true);
  ASSERT_EQ(gll->fixStatus(), NMEA0183::GPSFixStatus_LOST);
  ASSERT_FLOAT_EQ(gll->latitude(), 49.0 + (16.45 / 60.0));
  ASSERT_FLOAT_EQ(gll->longitude(), -(123.0 + (11.12 / 60.0)));
}


TEST_F(NMEATest, GPVTGTest)
{
  const c_nmea_dat * dat = dec.decode(sentences[4]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_VTG);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::VTG * vtg = data->payload_as_VTG();
  ASSERT_TRUE(vtg != nullptr);
  ASSERT_TRUE(vtg->fixStatus() == NMEA0183::GPSFixStatus_GPSF);
  ASSERT_FLOAT_EQ(vtg->cogTrue(), 240.3);  
  ASSERT_FLOAT_EQ(vtg->cogMag(), 0);
  ASSERT_FLOAT_EQ(vtg->sogN(),0); 
  ASSERT_FLOAT_EQ(vtg->sogK(), 0);

  // encode test
  c_vtg vtg_dec, vtg_enc;
  char buf[85];
  vtg_dec.decode(sentences[4]);
  vtg_dec.encode(buf);
  vtg_enc.decode(buf);
  ASSERT_FLOAT_EQ(vtg_dec.crs_t, vtg_enc.crs_t);  
  ASSERT_FLOAT_EQ(vtg_dec.crs_m, vtg_enc.crs_m);
  ASSERT_FLOAT_EQ(vtg_dec.v_n, vtg_enc.v_n);
  ASSERT_FLOAT_EQ(vtg_dec.v_k, vtg_enc.v_k);  
  ASSERT_EQ(vtg_dec.fs, vtg_enc.fs);  
}

TEST_F(NMEATest, GPZDATest)
{
  const c_nmea_dat * dat = dec.decode(sentences[6]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_ZDA);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::ZDA * zda = data->payload_as_ZDA();
  ASSERT_TRUE(zda != nullptr);
  ASSERT_TRUE(zda->year() == 2002);
  ASSERT_TRUE(zda->month() == 7);
  ASSERT_TRUE(zda->day() == 4);  
  ASSERT_TRUE(zda->hour() == 20);
  ASSERT_TRUE(zda->minute() == 15);  
  ASSERT_TRUE(zda->msec() == 30000);
  ASSERT_TRUE(zda->lzh() == 0);
  ASSERT_TRUE(zda->lzm() == 0);
  
   // encode test
  c_zda zda_dec, zda_enc;
  char buf[85];
  zda_dec.decode(sentences[6]);
  zda_dec.encode(buf);
  cout << "buf:" << buf << endl;
  zda_enc.decode(buf);
  ASSERT_EQ(zda_dec.m_h, zda_enc.m_h);
  ASSERT_EQ(zda_dec.m_m, zda_enc.m_m); 
  ASSERT_FLOAT_EQ(zda_dec.m_s, zda_enc.m_s);  
  ASSERT_EQ(zda_dec.m_dy, zda_enc.m_dy);
  ASSERT_EQ(zda_dec.m_mn, zda_enc.m_mn);
  ASSERT_EQ(zda_dec.m_yr, zda_enc.m_yr);
  ASSERT_EQ(zda_dec.m_lzh, zda_enc.m_lzh);
  ASSERT_EQ(zda_dec.m_lzm, zda_enc.m_lzm);    
}


TEST_F(NMEATest, PSATHPRTest)
{
  const c_nmea_dat * dat = dec.decode(sentences[7]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_PSAT);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::PSAT * psat = data->payload_as_PSAT();
  ASSERT_TRUE(psat != nullptr);
  const NMEA0183::HPR * hpr = psat->payload_as_HPR();
  ASSERT_TRUE(hpr != nullptr);
  ASSERT_EQ(hpr->hour(), 17);
  ASSERT_EQ(hpr->minute(), 9);
  ASSERT_EQ(hpr->msec(), 21600);
  ASSERT_FLOAT_EQ(hpr->heading(), 27.77);
  ASSERT_FLOAT_EQ(hpr->pitch(), -3.18);
  ASSERT_FLOAT_EQ(hpr->roll(), 0);
  ASSERT_TRUE(hpr->gyro() == false);

   // encode test
  c_psat_hpr hpr_dec, hpr_enc;
  char buf[85];
  hpr_dec.decode(sentences[7] + 10);
  hpr_dec.encode(buf);
  hpr_enc.decode(buf + 10);
  ASSERT_EQ(hpr_dec.hour, hpr_enc.hour);
  ASSERT_EQ(hpr_dec.mint, hpr_enc.mint); 
  ASSERT_FLOAT_EQ(hpr_dec.sec, hpr_enc.sec);  
  ASSERT_FLOAT_EQ(hpr_dec.hdg, hpr_enc.hdg);
  ASSERT_FLOAT_EQ(hpr_dec.pitch, hpr_enc.pitch);
  ASSERT_FLOAT_EQ(hpr_dec.roll, hpr_enc.roll);
  ASSERT_EQ(hpr_dec.gyro, hpr_enc.gyro);
}

TEST_F(NMEATest, VDMTest1)
{
  const c_nmea_dat * dat = dec.decode(sentences[8]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_VDM);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::VDM * vdm = data->payload_as_VDM();
  ASSERT_TRUE(vdm->isVDO() == false);  
  ASSERT_TRUE(vdm != nullptr);
  const NMEA0183::PositionReportClassA * pl =
    vdm->payload_as_PositionReportClassA();
  ASSERT_TRUE(pl != nullptr);
  ASSERT_EQ(pl->repeat(), 0);
  ASSERT_EQ(pl->mmsi(), 265547250);
  ASSERT_EQ(pl->status(), NMEA0183::NavigationStatus_UnderWayUsingEngine);
  ASSERT_EQ(pl->speed(), 139);
  ASSERT_EQ(pl->dgps(), false);
  ASSERT_FLOAT_EQ((double)pl->longitude()/600000., 11.8329767);
  ASSERT_FLOAT_EQ((double)pl->latitude()/600000., 57.6603533);
  ASSERT_FLOAT_EQ(pl->turn(), -2.8569787);    
  ASSERT_EQ(pl->course(), 404);
  ASSERT_EQ(pl->heading(), 41);
  ASSERT_EQ(pl->second(), 53);
  ASSERT_EQ(pl->maneuver(), NMEA0183::ManeuverIndicator_NotAvailable);
  ASSERT_EQ(pl->raim(), false);
}

TEST_F(NMEATest, VDMTest5)
{
  const c_nmea_dat * dat = dec.decode(sentences[10]);
  ASSERT_TRUE(dat == nullptr);
  dat = dec.decode(sentences[11]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_VDM);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::VDM * vdm = data->payload_as_VDM();
  ASSERT_TRUE(vdm->isVDO() == false);  
  ASSERT_TRUE(vdm != nullptr);
  const NMEA0183::StaticAndVoyageRelatedData * pl =
    vdm->payload_as_StaticAndVoyageRelatedData();
  ASSERT_TRUE(pl != nullptr);
  ASSERT_EQ(pl->repeat(), 0);
  ASSERT_EQ(pl->mmsi(), 603916439);
  ASSERT_EQ(pl->aisVersion(), 0);
  ASSERT_EQ(pl->imo(), 439303422);
  ASSERT_EQ(string((const char*)pl->callsign()->Data(), 7), string("``ZA83R"));
  ASSERT_EQ(string((const char*)pl->shipName()->Data(), 20), string("```ARCO`AVON\0\0\0\0\0\0\0\0", 20));
  ASSERT_EQ(pl->shipType(), NMEA0183::ShipType_PassengerNoAdditionalInformation);
  ASSERT_EQ(pl->toBow(), 113);
  ASSERT_EQ(pl->toStern(), 31);
  ASSERT_EQ(pl->toPort(), 17);
  ASSERT_EQ(pl->toStarboard(), 11);
  ASSERT_EQ(pl->month(), 3);
  ASSERT_EQ(pl->day(), 23);
  ASSERT_EQ(pl->hour(), 19);  
  ASSERT_EQ(pl->minute(), 45);
  ASSERT_EQ(pl->draught(), 132); // 13.2m
  ASSERT_EQ(string((const char*) pl->destination()->Data(), 20), string("``HOUSTON@@@@@@@@@@@"));
  ASSERT_EQ(pl->dte(), true);
}


TEST_F(NMEATest, VDMTest18)
{
  const c_nmea_dat * dat = dec.decode(sentences[19]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_VDM);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::VDM * vdm = data->payload_as_VDM();
  ASSERT_TRUE(vdm->isVDO() == false);  
  ASSERT_TRUE(vdm != nullptr);

  const NMEA0183::StandardClassBCSPositionReport * pl =
    vdm->payload_as_StandardClassBCSPositionReport();
    
  ASSERT_TRUE(pl != nullptr);
  ASSERT_EQ(pl->repeat(), 0);
  ASSERT_EQ(pl->mmsi(), 423302100);
  ASSERT_EQ(pl->speed(), 14);
  ASSERT_EQ(pl->course(), 1770);
  ASSERT_EQ(pl->dgps(), true);
  ASSERT_FLOAT_EQ((double)pl->longitude() / 600000.0, 53+0.6598/60.0);
  ASSERT_FLOAT_EQ((double)pl->latitude() / 600000.0, 40 + 0.3170/60.0);
  ASSERT_EQ(pl->heading(), 177);
  ASSERT_EQ(pl->second(), 34);
  ASSERT_EQ(pl->cs(), true);
  ASSERT_EQ(pl->display(), true);
  ASSERT_EQ(pl->dsc(), true);
  ASSERT_EQ(pl->band(), true);
  ASSERT_EQ(pl->msg22(), true);
  ASSERT_EQ(pl->assigned(), false);
  ASSERT_EQ(pl->raim(), false);
}

TEST_F(NMEATest, VDMTest19)
{
  const c_nmea_dat * dat = dec.decode(sentences[20]);
  ASSERT_TRUE(dat == nullptr);
  dat = dec.decode(sentences[21]);  
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_VDM);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::VDM * vdm = data->payload_as_VDM();
  ASSERT_TRUE(vdm->isVDO() == false);  
  ASSERT_TRUE(vdm != nullptr);

  const NMEA0183::ExtendedClassBCSPositionReport * pl =
    vdm->payload_as_ExtendedClassBCSPositionReport();
  
  ASSERT_TRUE(pl != nullptr);
  ASSERT_EQ(pl->repeat(), 0);
  ASSERT_EQ(pl->mmsi(), 601000013);
  ASSERT_EQ(pl->speed(), 29);
  ASSERT_EQ(pl->dgps(), false);
  ASSERT_FLOAT_EQ((double)pl->longitude()/600000., 32 + 11.9718/60.0);
  ASSERT_FLOAT_EQ((double)pl->latitude()/600000., -(29 + 50.2488/60.0));  
  ASSERT_EQ(pl->course(), 890);;
  ASSERT_EQ(pl->heading(), 90);
  ASSERT_EQ(pl->second(), 12);  
  ASSERT_EQ(string((const char*)pl->shipName()->Data(), 20), string("TEST`NAME`CLSB`MSG19", 20));
  ASSERT_EQ(pl->shipType(), NMEA0183::ShipType_PleasureCraft);
  ASSERT_EQ(pl->toBow(), 7);
  ASSERT_EQ(pl->toStern(), 6);
  ASSERT_EQ(pl->toPort(), 2);
  ASSERT_EQ(pl->toStarboard(), 3);
  ASSERT_EQ(pl->epfd(), NMEA0183::EPFDFixType_GPS);
  ASSERT_EQ(pl->raim(), false);
  ASSERT_EQ(pl->assigned(), false);
  ASSERT_EQ(pl->dte(), true);
}

TEST_F(NMEATest, VDOTest24)
{
  // Part A
  const c_nmea_dat * dat = dec.decode(sentences[24]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_VDM);
  const uint8_t * buffer = dat->get_buffer_pointer();
  size_t size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  const NMEA0183::Data * data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  const NMEA0183::VDM * vdm = data->payload_as_VDM();
  ASSERT_TRUE(vdm->isVDO() == true);  
  ASSERT_TRUE(vdm != nullptr);

  const NMEA0183::StaticDataReport * pl =
    vdm->payload_as_StaticDataReport();
  
  ASSERT_TRUE(pl != nullptr);
  ASSERT_EQ(pl->repeat(), 0);
  ASSERT_EQ(pl->mmsi(), 112233445);
  ASSERT_EQ(pl->partno(), 0);
  ASSERT_EQ(string((const char*)pl->shipName()->Data(), 20), string("THIS`IS`A`CLASS`B`UN", 20));
  

  // Part B
  dat = dec.decode(sentences[25]);
  ASSERT_TRUE(dat != nullptr);
  ASSERT_TRUE(dat->get_payload_type() == NMEA0183::Payload_VDM);
  buffer = dat->get_buffer_pointer();
  size = dat->get_buffer_size();
  ASSERT_TRUE(size < 256);
  ASSERT_TRUE(buffer != nullptr);
  data = NMEA0183::GetData(buffer);
  ASSERT_TRUE(data != nullptr);
  vdm = data->payload_as_VDM();
  ASSERT_TRUE(vdm->isVDO() == true);  
  ASSERT_TRUE(vdm != nullptr);

  pl = vdm->payload_as_StaticDataReport();

  ASSERT_EQ(pl->repeat(), 0);
  ASSERT_EQ(pl->mmsi(), 112233445);
  ASSERT_EQ(pl->partno(), 1);
  ASSERT_EQ(pl->shipType(), NMEA0183::ShipType_Sailing);
  ASSERT_EQ(string((const char*)pl->callsign()->Data(), 7), string("CALLSIG"));   ASSERT_EQ(pl->toBow(), 5);
  ASSERT_EQ(pl->toStern(), 4);
  ASSERT_EQ(pl->toPort(), 3);
  ASSERT_EQ(pl->toStarboard(), 12);  
}


