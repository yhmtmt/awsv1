#ifndef AWS_NMEA_AIS_H
#define AWS_NMEA_AIS_H
// Copyright(c) 2016 Yohei Matsumoto, All right reserved. 

// aws_nmea_ais.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_nmea_ais.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_nmea_ais.h.  If not, see <http://www.gnu.org/licenses/>. 

//s_binary_message helps generating AIS binary messages.
struct s_binary_message{
  int len; // bit length
  int sq; // sequence id
  int type; // message 8 or 14
  int ch; // channel 0/*auto*/, 1 /*A*/, 2 /*B*/, 3 /*A and B*/
  int txtseq;
  bool ackreq;
  unsigned int mmsi; // for message 6 or 12
  unsigned char msg[120]; // message up to 120 bytes
  char nmea[86]; // nmea buffer
  s_binary_message():len(0), sq(0), mmsi(0), type(8), ch(0), txtseq(0), ackreq(false){
  }
  
  ~s_binary_message(){
  }
  
  // text DAC 1 FI 0
  bool set_msg_text(char * buf);
  
  // interpreting data in buf as hex data. The characters in buf should be 0 to 9 or a to f.
  bool set_msg_hex(char * buf);
  
  // interpreting data in buf as 6bit character string. The characters should be in the armoring table.
  bool set_msg_c6(char * buf);
  
  // interpreting data in buf as 8bit character string. Any data byte can be OK. 
  bool set_msg_c8(char * buf);
  
  // interpreting data in buf as binary string. The characters given in the buffer should be 0 or 1. 
  bool set_msg_bin(char * buf);
  
  // binary message pvc sends an absolue position and velosity as message 8
  // This message does not care about DAC/FI/AckReq/TextSeq 
  bool set_msg_pvc(
		   unsigned char id, /* 8bit */
		   unsigned short sog /* 10bit 0.1 knot / unit */,
		   unsigned short cog /* 12bit 0.1 deg / unit */,
		   int lon /* 28bit 10000 minutes  = 1/600000 deg /unit*/ , 
		   int lat /* 27bit 10000 minutes  = 1/600000 deg /unit*/);
  bool get_msg_pvc(
		   unsigned char & id, /* 8bit */
		   unsigned short & sog,
		   unsigned short & cog,
		   int & lon, 
		   int & lat);
  
  // pvc2 sends an absolute positon and velocity as message 8
  // DAC/FI/AckReq/TextSeq are set as zero.
  bool set_msg_pvc2(
		    unsigned char id, /* 8bit */
		    unsigned short sog /* 10bit 0.1 knot / unit */,
		    unsigned short cog /* 12bit 0.1 deg / unit */,
		    int lon /* 28bit 10000 minutes  = 1/600000 deg /unit*/ , 
		    int lat /* 27bit 10000 minutes  = 1/600000 deg /unit*/);
  bool get_msg_pvc2(
		    unsigned char & id, /* 8bit */
		    unsigned short & sog,
		    unsigned short & cog,
		    int & lon, 
		    int & lat);
  
  // pvc3 sends a relative position to the own ship and the velocity
  // DAC/FI/AckReq/TextSeq are set as zero
  bool set_msg_pvc3(
		    unsigned char id, /* 8bit */
		    unsigned short sog /* 10bit 0.1 knot / unit */,
		    unsigned short cog /* 12bit 0.1 deg / unit */,
		    unsigned short dist /* 10bit 0.1 mile / unit */,
		    unsigned short bear /* 12bit 0.1 deg / unit */);
  bool get_msg_pvc3(
		    unsigned char & id, /* 8bit */
		    unsigned short & sog /* 10bit 0.1 knot / unit */,
		    unsigned short & cog /* 12bit 0.1 deg / unit */,
		    unsigned short & dist /* 10bit 0.1 mile / unit */,
		    unsigned short & bear /* 12bit 0.1 deg / unit */);
  
  // pvc4 sends an absolute position, the velocity and UTC sec as message 8
  // DAC/FI/AckReq/TextSeq are set as zero
  bool set_msg_pvc4(
		    unsigned char id, /* 8bit */
		    unsigned short sog /* 10bit 0.1 knot / unit */,
		    unsigned short cog /* 12bit 0.1 deg / unit */,
		    int lon /* 28bit 10000 minutes  = 1/600000 deg /unit*/ , 
		    int lat /* 27bit 10000 minutes  = 1/600000 deg /unit*/,
		    unsigned char sec /* 6bit 1sec / unit */);
  bool get_msg_pvc4(
		    unsigned char & id, /* 8bit */
		    unsigned short & sog,
		    unsigned short & cog,
		    int & lon, 
		    int & lat,
		    unsigned char & sec /* 6bit 1sec / unit */);
  
  // pvc5 sends an relative position, the velocity and UTC sec as message 8
  // DAC/FI/AckReq/TextSeq are set as zero.
  bool set_msg_pvc5(
		    unsigned char id, /* 8bit */
		    unsigned short sog /* 10bit 0.1 knot / unit */,
		    unsigned short cog /* 12bit 0.1 deg / unit */,
		    unsigned short dist /* 10bit 0.1 mile / unit */,
		    unsigned short bear /* 12bit 0.1 deg / unit */,
		    unsigned char sec /* 6bit 1sec / unit */);
  bool get_msg_pvc5(
		    unsigned char & id, /* 8bit */
		    unsigned short & sog /* 10bit 0.1 knot / unit */,
		    unsigned short & cog /* 12bit 0.1 deg / unit */,
		    unsigned short & dist /* 10bit 0.1 mile / unit */,
		    unsigned short & bear /* 12bit 0.1 deg / unit */,
		    unsigned char & sec /* 6bit 1sec / unit */);
  
  // generating BBM or ABM nmea sentence. The results are stored in nmeas.
  // Note that multiple sentences can be produced.
  bool gen_nmea(const char * toker, vector<string> & nmeas);
};

char armor(char c);
unsigned char decchar(unsigned char c6);
unsigned char encchar(unsigned char c8);
const char * get_ship_type_name(unsigned char uc);


//////////////////////// c_vdm (from AIS)
class c_vdm;

struct s_pl{
  s_pl * pnext;
  char payload[256];
  int pl_size;
  short fcounts; // number of frangments
  short fnumber; // fragment number
  short seqmsgid; // sequential message id for multi sentence messaage
  bool is_chan_A; 
  short num_padded_zeros;
  bool cs; // check sum
  s_pl():pnext(NULL), pl_size(0), fcounts(0), fnumber(0),
	 seqmsgid(-1), is_chan_A(true), 
	 cs(false)
  {}
  
  bool is_complete()
  {
    return  fcounts == fnumber;
  }
  void dearmor(const char * str);
};

class c_vdm: public c_nmea_dat
{
public:
  bool m_vdo;
  unsigned char m_repeat; // repeat indicator
  unsigned int m_mmsi; // mmsi number
  bool m_is_chan_A;
  
  c_vdm(): m_vdo(false), m_repeat(0), m_mmsi(0), m_is_chan_A(true)
  {
  }
  
  ~c_vdm()
  {
  }
  
  virtual void dec_payload(s_pl * ppl)
  {
    m_is_chan_A = ppl->is_chan_A;
    char * dat = ppl->payload;
    m_repeat = (dat[1] & 0x30) >> 4;
    
    m_mmsi = ((dat[1] & 0x0F) << 26) |
      ((dat[2] & 0x3F) << 20) | 
      ((dat[3] & 0x3F) << 14) |
      ((dat[4] & 0x3F) << 8) |
      ((dat[5] & 0x3F) << 2) |
      ((dat[6] & 0x30) >> 4);
  };

  virtual void dec_payload(s_pl * ppl, const long long t) = 0;
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_NONE;
  }

  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_VDM;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_VDM;};
};


class c_vdm_msg1:public c_vdm
{// class A position report
public:
  char m_status; // navigation status
  float m_turn; // Rate of Turn degrees/min
  float m_speed; // knot
  char m_accuracy; // DGPS = 1, GPS = 0
  int m_lon_min;// minute
  int m_lat_min;// minute
  float m_lon; // degree
  float m_lat; // degree
  float m_course; // xxx.x degree
  unsigned short m_heading; // degree
  unsigned char m_second; // UTC second
  char m_maneuver; // 0:na 1: no special 2:special
  char m_raim; // RAIM flag
  unsigned int m_radio; // radio status
  virtual void dec_payload(s_pl * ppl);
  virtual ostream & show(ostream & out) const;
  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::PositionReportClassA 
      positionReportClassA(m_repeat, (NMEA0183::NavigationStatus) m_status,
			   m_accuracy==1,
			   m_second, (NMEA0183::ManeuverIndicator) m_maneuver,
			   m_raim != 0,
			   (short)m_turn,
			   (unsigned short)m_speed * 10,
			   (unsigned short)m_course * 10,
			   m_heading,
			   m_mmsi,
			   m_lon_min, m_lat_min);    
    auto payload = builder.CreateStruct(positionReportClassA);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()    
  {
    return NMEA0183::VDMPayload_PositionReportClassA;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_VDM1;};
};

class c_vdm_msg4: public c_vdm
{// class A base station report
public:
  unsigned short m_year;
  char m_month, m_day, m_hour, m_minute;
  char m_accuracy; // DGPS = 1, GPS = 0
  int m_lon_min, m_lat_min; // position in minute
  float m_lon; // degree
  float m_lat; // degree
  unsigned char m_second; // UTC second
  char m_epfd; // positioning device
  char m_raim; // RAIM flag
  unsigned int m_radio; // radio status
  
  virtual void dec_payload(s_pl * ppl);
  virtual ostream & show(ostream & out) const;

  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::BaseStationReport
      baseStationReport(m_repeat,
			m_month, m_day, m_hour, m_minute, m_second,
			(NMEA0183::EPFDFixType)m_epfd, m_raim, m_accuracy == 1,
			m_year,
			m_mmsi,
			m_lon_min, m_lat_min);    
    auto payload = builder.CreateStruct(baseStationReport);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_BaseStationReport;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_VDM4;};
};

class c_vdm_msg5: public c_vdm
{// class A static information
public:
  unsigned char m_ais_version;
  unsigned int m_imo;
  unsigned char m_callsign[8];
  unsigned char m_shipname[21];
  unsigned char m_shiptype;
  short m_to_bow, m_to_stern;
  unsigned char m_to_port, m_to_starboard;
  char m_epfd;
  unsigned short m_year;
  unsigned char m_month, m_day, m_hour, m_minute;
  unsigned short m_draught;
  bool m_dte;
  unsigned char m_destination[21];
  
  virtual void dec_payload(s_pl * ppl);
  virtual ostream & show(ostream & out) const;

  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::StaticAndVoyageRelatedData    
      staticAndVoyageRelatedData(m_repeat,
				 m_ais_version,
				 (NMEA0183::EPFDFixType)m_epfd,
				 m_month, m_day, m_hour, m_minute,
				 m_to_port, m_to_starboard,
				 m_dte,
				 (NMEA0183::ShipType)m_shiptype,
				 m_to_bow, m_to_stern,
				 m_mmsi,
				 m_imo,
				 m_draught);
    for (int i = 0; i < 7; i++)
      staticAndVoyageRelatedData.mutable_callsign()->Mutate(i, m_callsign[i]);
    for (int i = 0; i < 20; i++)
      staticAndVoyageRelatedData.mutable_shipName()->Mutate(i, m_shipname[i]);
    for (int i = 0; i < 20; i++)
      staticAndVoyageRelatedData.mutable_destination()->Mutate(i,
							       m_destination[i]);
    
    auto payload = builder.CreateStruct(staticAndVoyageRelatedData);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_StaticAndVoyageRelatedData;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_VDM5;};
};

class c_vdm_msg6: public c_vdm
{
public:
  bool m_retransmit;
  unsigned char m_seqno;
  unsigned int m_mmsi_dst;
  unsigned short m_dac;
  unsigned short m_fid;
  s_binary_message m_msg;
  unsigned short m_msg_size;
  virtual void dec_payload(s_pl * ppl);
  virtual ostream & show(ostream & out) const;
 
  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::BinaryAddressedMessage
      binaryAddressedMessage(m_repeat,
			     m_seqno, m_retransmit,
			     m_fid, m_dac,
			     m_mmsi,
			     m_mmsi_dst);
    for (int i = 0; i < 115; i++)
      binaryAddressedMessage.mutable_data()->Mutate(i, m_msg.msg[i]);
    
    auto payload = builder.CreateStruct(binaryAddressedMessage);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
   
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_BinaryAddressedMessage;
  }  
  virtual e_nd_type get_type() const
  {return ENDT_VDM6;}
};

class c_vdm_msg8: public c_vdm
{
public:
  unsigned short m_dac;
  unsigned short m_fid;
  s_binary_message m_msg;
  //	char m_msg[128];
  unsigned short m_msg_size;
  virtual void dec_payload(s_pl * ppl);
  virtual ostream & show(ostream & out) const;
  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::BinaryBroadcastMessage
      binaryBroadcastMessage(m_repeat,
			     m_fid, m_dac,
			     m_mmsi);
    for (int i = 0; i < 119; i++)
      binaryBroadcastMessage.mutable_data()->Mutate(i, m_msg.msg[i]);
    
    auto payload = builder.CreateStruct(binaryBroadcastMessage);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_BinaryBroadcastMessage;
  }  
  virtual e_nd_type get_type() const
  {return ENDT_VDM8;};
};

class c_vdm_msg18: public c_vdm
{// class B position report
public:
  float m_speed; // knot
  char m_accuracy; // DGPS = 1, GPS = 0
  unsigned short m_lon_min, m_lat_min; // positin in minutes
  float m_lon; // degree
  float m_lat; // degree
  float m_course; // xxx.x degree
  unsigned short m_heading; // degree
  unsigned char m_second; // UTC second
  bool m_cs;
  bool m_disp;
  bool m_dsc;
  bool m_band;
  bool m_msg22;
  bool m_assigned;
  bool m_raim; 
  unsigned int m_radio; // radio status
  
  virtual ostream & show(ostream & out) const;
  virtual void dec_payload(s_pl * ppl);
  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::StandardClassBCSPositionReport
      standardClassBCSPositionReport(m_repeat, m_accuracy == 1,
				     m_second, 0/*not defined yet*/,
				     m_cs, m_disp, m_dsc, m_band, m_msg22,
				     m_assigned, m_raim,
				     (unsigned short) (m_speed * 10),
				     (unsigned short) (m_course * 10),
				     m_heading,
				     m_mmsi, m_lon_min, m_lat_min);
    
    auto payload = builder.CreateStruct(standardClassBCSPositionReport);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_StandardClassBCSPositionReport;
  }
  
  virtual e_nd_type get_type() const 
  {return ENDT_VDM18;};
};

class c_vdm_msg19: public c_vdm
{// class B position report ex
public:
  float m_speed; // knot
  char m_accuracy; // DGPS = 1, GPS = 0
  int m_lon_min, m_lat_min; // position in minutes;
  float m_lon; // degree
  float m_lat; // degree
  float m_course; // xxx.x degree
  unsigned short m_heading; // degree
  unsigned char m_second; // UTC second
  bool m_assigned;
  bool m_raim; 
  bool m_dte;
  unsigned int m_radio; // radio status
  unsigned char m_shipname[21];
  unsigned char m_shiptype;
  short m_to_bow, m_to_stern;
  char m_epfd;
  
  unsigned char m_to_port, m_to_starboard;
  
  virtual ostream & show(ostream & out) const;
  virtual void dec_payload(s_pl * ppl);
  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::ExtendedClassBCSPositionReport
      extendedClassBCSPositionReport(m_repeat, m_accuracy == 1,
				     m_second, 0/*not defined yet*/,
				     m_assigned, m_raim,
				     (NMEA0183::ShipType)m_shiptype,
				     (NMEA0183::EPFDFixType)m_epfd,
				     m_dte,
				     m_to_port, m_to_starboard,
				     (unsigned short) (m_speed * 10),
				     (unsigned short) (m_course * 10),
				     m_heading,
				     m_to_bow, m_to_stern, 
				     m_mmsi, m_lon_min, m_lat_min);
    for (int i = 0; i < 20; i++)
      extendedClassBCSPositionReport.mutable_shipName()->Mutate(i,
								m_shipname[i]);
    auto payload = builder.CreateStruct(extendedClassBCSPositionReport);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_ExtendedClassBCSPositionReport;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_VDM19;};
};

class c_vdm_msg24: public c_vdm
{// class B static information
public:
  unsigned char m_part_no;
  unsigned char m_shipname[21];
  
  unsigned char m_shiptype;
  unsigned char m_vendorid[3];
  unsigned char m_callsign[8];
  
  short m_to_bow, m_to_stern;
  unsigned char m_to_port, m_to_starboard;
  unsigned char m_model;
  unsigned int m_serial;
  unsigned int m_ms_mmsi;
  
  char m_epfd;
  
  virtual ostream & show(ostream & out) const;
  virtual void dec_payload(s_pl * ppl);
  virtual void dec_payload(s_pl * ppl, const long long t)
  {
    dec_payload(ppl);
    builder.Clear();
    NMEA0183::StaticDataReport
      staticDataReport(m_repeat, m_part_no,
		       (NMEA0183::ShipType)m_shiptype,
		       m_to_port, m_to_starboard,
		       m_model,
		       m_to_bow, m_to_stern, 
		       m_mmsi,
		       m_serial,
		       m_ms_mmsi);
    for (int i = 0; i < 20; i++)
      staticDataReport.mutable_shipName()->Mutate(i, m_shipname[i]);
    for(int i = 0; i < 7; i++)
      staticDataReport.mutable_callsign()->Mutate(i, m_callsign[i]);
    for(int i = 0; i < 3; i++)
      staticDataReport.mutable_vendorID()->Mutate(i, m_vendorid[i]);
    
    auto payload = builder.CreateStruct(staticDataReport);
    auto vdm = CreateVDM(builder,
			 m_vdo,
			 (m_is_chan_A ?
			  NMEA0183::AISChannel_A : NMEA0183::AISChannel_B),
			 get_vdm_payload_type(),
			 payload.Union());
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   vdm.Union());
    
    builder.Finish(data);
  }
  
  virtual const NMEA0183::VDMPayload get_vdm_payload_type()
  {
    return NMEA0183::VDMPayload_StaticDataReport;
  }
  
  virtual e_nd_type get_type() const
  {return ENDT_VDM24;};
};

//////////////////////// c_abk
class c_abk: public c_nmea_dat
{
public:
  unsigned char m_cs; // check sum
  unsigned int m_mmsi;
  enum e_recv_chan{ NA, CHA, CHB} m_rch;
  unsigned short m_msg_id;
  unsigned short m_seq;
  unsigned short m_stat;
  
  c_abk(): m_mmsi(0), m_rch(NA), m_msg_id(0), m_seq(0), m_stat(0){};
  ~c_abk(){};
  
  virtual bool dec(const char * str);
  virtual bool decode(const char * str, const long long t = -1){
    if(!dec(str))
      return false;
    builder.Clear();
    NMEA0183::ABK abk(m_mmsi, m_msg_id, m_seq, m_stat);
    auto payload = builder.CreateStruct(abk); 
    auto data = CreateData(builder,
			   t,
			   get_payload_type(),
			   payload.Union());
    
    builder.Finish(data);        
  }

  virtual NMEA0183::Payload get_payload_type()
  {
    return NMEA0183::Payload_ABK;
  }
  
  virtual ostream & show(ostream & out) const;
  virtual e_nd_type get_type() const
  {return ENDT_ABK;};
};

/////////////////////////////////////////////////////////// vdm decoder
class c_vdm_dec
{
protected:
  bool m_vdo;

  struct s_vdm_obj{
    unsigned char id;
    c_vdm * dat;
    s_vdm_obj():dat(nullptr)
    {
    }

    s_vdm_obj(const unsigned char id_, c_vdm * dat_):dat(dat_)
    {
      id = id_;
    }

    ~s_vdm_obj()
    {
      if(dat)
	delete dat;
      dat = nullptr;
    }
    
    bool match(const unsigned char id_){
      return id == id_;
    }    
  };
  
  vector<s_vdm_obj> vdm_objs;
  c_vdm * create_vdm_dat(const unsigned char message_id)
  {
    switch(message_id){
    case 1:
    case 2:
    case 3:return new c_vdm_msg1;
    case 4:return new c_vdm_msg4;
    case 5:return new c_vdm_msg5;
    case 6:return new c_vdm_msg6;
    case 8:return new c_vdm_msg8;
    case 18:return new c_vdm_msg18;
    case 19:return new c_vdm_msg19;
    case 24:return new c_vdm_msg24;
    default:
      break;
    }
    return nullptr;
  }
  
  s_pl * m_pool;
  list<s_pl*> m_tmp; // temporaly store multi fragments message
  
  c_vdm_dec * m_pnext;
  
  char m_type; // message type
  
  
  c_vdm * dec_payload(s_pl * ppl, const long long t);
  
  s_pl * alloc(){
    if(m_pool == NULL)
      return new s_pl;
    s_pl * tmp = m_pool;
    m_pool = m_pool->pnext;
    tmp->pl_size = 0;
    return tmp;
  }
  
  void free(s_pl * ptr){
    ptr->pnext = m_pool;
    m_pool = ptr;
  }
  
  void clear();
public:
  c_vdm_dec():m_pool(NULL), m_vdo(false)
    {
    }
  
  ~c_vdm_dec()
  {
    while(m_pool){
      s_pl * ppl = m_pool->pnext;
      delete m_pool;
      m_pool = ppl;
    }
  }

  bool add_vdm_dat(const unsigned char id)
  {
    c_vdm * dat = create_vdm_dat(id);
    if(!dat)
      return false;

    for(int i = 0; i < vdm_objs.size(); i++){
      if(vdm_objs[i].match(id))
	return false;
    }
    
    vdm_objs.push_back(s_vdm_obj(id, dat));
    return true;      
  }
  
  void set_vdo(){
    m_vdo = true;
  }
  
  c_vdm * decode(const char * str, long long t = -1);
};

#endif
