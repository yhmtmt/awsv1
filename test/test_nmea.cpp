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

