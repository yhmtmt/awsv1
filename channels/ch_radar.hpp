#ifndef CH_RADAR_HPP
#define CH_RADAR_HPP
// Copyright(c) 2019 Yohei Matsumoto, All right reserved.

// ch_radar.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ch_radar.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ch_radar.hpp.  If not, see <http://www.gnu.org/licenses/>.

#include "channel_base.hpp"
#include "ch_binary_data_queue.hpp"
#include "radar_type_generated.h"

// This constants are temporal ones.  Make the file independent from
// radar type.
#define GARMIN_XHD_SPOKES 1440
#define GARMIN_XHD_MAX_SPOKE_LEN 705

struct GeoPosition
{
  double lat;
  double lon;
};

/*
enum RadarControlState {
  RCS_OFF = -1,
  RCS_MANUAL = 0,
  RCS_AUTO_1,
  RCS_AUTO_2,
  RCS_AUTO_3,
  RCS_AUTO_4,
  RCS_AUTO_5,
  RCS_AUTO_6,
  RCS_AUTO_7,
  RCS_AUTO_8,
  RCS_AUTO_9
};
*/

enum radar_command_id
{
  RC_TXOFF,
  RC_TXON,
  RC_RANGE,
  RC_BEARING_ALIGNMENT,
  RC_NO_TRANSMIT_START,
  RC_NO_TRANSMIT_END,
  RC_GAIN,
  RC_SEA,
  RC_RAIN,
  RC_INTERFERENCE_REJECTION,
  RC_SCAN_SPEED,
  RC_TIMED_IDLE,
  RC_TIMED_RUN,
  RC_IMG,
  RC_NONE
};

struct radar_command
{
  radar_command_id id;
  int val;
  RadarControlState state;

  radar_command(const radar_command_id _id, const int _val, const RadarControlState _state) : id(_id), val(_val), state(_state)
  {
  }
};

class ch_radar_state : public ch_base
{
protected:
  int m_state, m_range, m_gain, m_gain_state, m_scan_speed,
      m_bearing_alignment, m_interference_rejection,
      m_rain_clutter, m_rain_mode, m_sea_clutter,
      m_sea_mode, m_no_transmit_start,
      m_no_transmit_start_state, m_no_transmit_end,
      m_no_transmit_end_state, m_timed_idle,
      m_timed_idle_mode, m_timed_run, m_next_state_change;

public:
  ch_radar_state(const char *name) : ch_base(name),
                                     m_state(0), m_range(0), m_gain(0),
                                     m_gain_state(-1), m_scan_speed(0),
                                     m_bearing_alignment(0),
                                     m_interference_rejection(0),
                                     m_rain_clutter(0), m_rain_mode(-1),
                                     m_sea_clutter(0), m_sea_mode(-1),
                                     m_no_transmit_start(0),
                                     m_no_transmit_start_state(-1),
                                     m_no_transmit_end(0),
                                     m_no_transmit_end_state(-1),
                                     m_timed_idle(0), m_timed_idle_mode(0),
                                     m_timed_run(0), m_next_state_change(0)
  {
  }
  virtual ~ch_radar_state()
  {
  }
  void set_state(const int state)
  {
    lock();
    m_state = state;
    unlock();
  }

  const int get_state()
  {
    int state;
    lock();
    state = m_state;
    unlock();
    return state;
  }

  void set_scan_speed(const int scan_speed)
  {
    lock();
    m_scan_speed = scan_speed;
    unlock();
  }

  const int get_scan_speed()
  {
    int scan_speed;
    lock();
    scan_speed = m_scan_speed;
    unlock();
    return scan_speed;
  }

  void set_range(const int range)
  {
    lock();
    m_range = range;
    unlock();
  }

  const int get_range()
  {
    int range;
    lock();
    range = m_range;
    unlock();
    return range;
  }

  void set_gain(const int gain, const int state)
  {
    lock();
    m_gain = gain;
    m_gain_state = state;
    unlock();
  }

  int get_gain()
  {
    int gain;
    lock();
    gain = m_gain;
    unlock();
    return gain;
  }

  int get_gain_state()
  {
    int gain_state;
    lock();
    gain_state = m_gain_state;
    unlock();
    return gain_state;
  }

  void set_bearing_alignment(const int bearing_alignment)
  {
    lock();
    m_bearing_alignment = bearing_alignment;
    unlock();
  }

  int get_bearing_alignment()
  {
    int bearing_alignment;
    lock();
    bearing_alignment = m_bearing_alignment;
    unlock();
    return bearing_alignment;
  }

  void set_interference_rejection(const int interference_rejection)
  {
    lock();
    m_interference_rejection = interference_rejection;
    unlock();
  }

  int get_interference_rejection()
  {
    int interference_rejection;
    lock();
    interference_rejection = m_interference_rejection;
    unlock();
    return interference_rejection;
  }

  void set_rain(const int rain, const int mode)
  {
    lock();
    m_rain_clutter = rain;
    m_rain_mode = m_rain_mode;
    unlock();
  }

  int get_rain()
  {
    int rain;
    lock();
    rain = m_rain_clutter;
    unlock();
    return rain;
  }

  int get_rain_mode()
  {
    int rain_mode;
    lock();
    rain_mode = m_rain_mode;
    unlock();
    return rain_mode;
  }

  void set_sea(const int sea, const int mode)
  {
    lock();
    m_sea_clutter = sea;
    m_sea_mode = mode;
    unlock();
  }

  int get_sea()
  {
    int sea;
    lock();
    sea = m_sea_clutter;
    unlock();
    return sea;
  }

  int get_sea_mode()
  {
    int sea_mode;
    lock();
    sea_mode = m_sea_mode;
    unlock();
    return sea_mode;
  }

  void set_no_transmit_start(const int no_transmit_start, const int state)
  {
    lock();
    m_no_transmit_start = no_transmit_start;
    m_no_transmit_start_state = state;
    unlock();
  }

  int get_no_transmit_start()
  {
    int no_transmit_start;
    lock();
    no_transmit_start = m_no_transmit_start;
    unlock();
    return no_transmit_start;
  }

  int get_no_transmit_start_state()
  {
    int no_transmit_start_state;
    lock();
    no_transmit_start_state = m_no_transmit_start_state;
    unlock();
    return no_transmit_start_state;
  }

  void set_no_transmit_end(const int no_transmit_end, const int state)
  {
    lock();
    m_no_transmit_end = no_transmit_end;
    m_no_transmit_end_state = state;
    unlock();
  }

  int get_no_transmit_end()
  {
    int no_transmit_end;
    lock();
    no_transmit_end = m_no_transmit_end;
    unlock();
    return no_transmit_end;
  }

  int get_no_transmit_end_state()
  {
    int no_transmit_end_state;
    lock();
    no_transmit_end_state = m_no_transmit_end_state;
    unlock();
    return no_transmit_end_state;
  }

  void set_timed_idle(const int timed_idle, const int mode)
  {
    lock();
    m_timed_idle = timed_idle;
    m_timed_idle_mode = mode;
    unlock();
  }

  int get_timed_idle()
  {
    int timed_idle;
    lock();
    timed_idle = m_timed_idle;
    unlock();
    return timed_idle;
  }

  int get_timed_idle_mode()
  {
    int timed_idle_mode;
    lock();
    timed_idle_mode = m_timed_idle_mode;
    unlock();
    return timed_idle_mode;
  }

  void set_timed_run(const int timed_run)
  {
    lock();
    m_timed_run = timed_run;
    unlock();
  }

  int get_timed_run()
  {
    int timed_run;
    lock();
    timed_run = m_timed_run;
    unlock();
    return timed_run;
  }

  void set_next_state_change(const int next_state_change)
  {
    lock();
    m_next_state_change = next_state_change;
    unlock();
  }

  int get_next_state_change()
  {
    int next_state_change;
    lock();
    next_state_change = m_next_state_change;
    unlock();
    return next_state_change;
  }
};

class ch_radar_ctrl : public ch_base
{
protected:
  queue<radar_command> command_queue;

public:
  ch_radar_ctrl(const char *name) : ch_base(name)
  {
  }

  virtual ~ch_radar_ctrl()
  {
  }

  void push(const radar_command_id id,
            const int val = 0, const RadarControlState state = RadarControlState_OFF)
  {

    lock();
    command_queue.push(radar_command(id, val, state));
    unlock();
  }

  bool pop(radar_command_id &id, int &val, RadarControlState &state)
  {
    lock();
    if (command_queue.empty())
    {
      unlock();
      return false;
    }

    radar_command &rc = command_queue.front();
    id = rc.id;
    val = rc.val;
    state = rc.state;
    command_queue.pop();
    unlock();
    return true;
  }
};

struct s_radar_point
{
  short bearing;   // 0 to GARMIN_XHD_SPOKES
  short distance;  // 0 to GARMIN_XHD_MAX_SPOKE_LEN - 1
  float range;     // maximum distance in meter (typically 1852)
  float yaw;       // yaw
  double lat, lon; // own ship position

  s_radar_point() : bearing(-1), distance(-1), yaw(0.f), lat(0.f), lon(0.f){};

  // returns distance in meter
  const float get_distance()
  {
    return (float)(distance * range * (1.0 / (double)GARMIN_XHD_MAX_SPOKE_LEN));
  }

  // returns bearing in radian
  const float get_bearing()
  {
    return (float)(bearing * (2.0 * PI / (double)GARMIN_XHD_SPOKES));
  }
};

struct s_radar_line
{
  int bearing;
  size_t len;
  long long time;
  unsigned long long number; 
  s_radar_line() : bearing(-1), number(0){};
};

class ch_radar_image : public ch_base
{
protected:
  s_radar_line *m_history;
  unsigned char *lines[2048];
  long long tprev_update;
  int bearing_prev_update;
  int m_range_meters;
  int m_data_length;
  unsigned long long total_points_found;
  unsigned long long spoke_count;
  int recent_spoke[32];
  int num_recent_spokes;
  int recent_head, recent_tail;
  unsigned char line_tmp[GARMIN_XHD_MAX_SPOKE_LEN];
public:
  ch_radar_image(const char *name) : ch_base(name), bearing_prev_update(-1),
                                     m_range_meters(1852),
                                     m_data_length(GARMIN_XHD_MAX_SPOKE_LEN),
                                     total_points_found(0), spoke_count(0),
                                     num_recent_spokes(0),
                                     recent_head(0), recent_tail(0)
  {
    m_history = (s_radar_line *)calloc(GARMIN_XHD_SPOKES, sizeof(s_radar_line));
    unsigned char *ptr = (unsigned char *)calloc(2048 * 1024,
                                                 sizeof(unsigned char));
    for (size_t i = 0; i < GARMIN_XHD_SPOKES; i++)
    {
      lines[i] = ptr;
      ptr += sizeof(unsigned char) * 1024;
    }
  }

  virtual ~ch_radar_image()
  {
    if (m_history)
    {
      free(m_history);
    }

    if (lines[0])
    {
      free(lines[0]);
    }
  }

  void reset_image()
  {
    lock();
    for (size_t i = 0; i < GARMIN_XHD_SPOKES; i++)
    {
      memset(lines[i], 0, GARMIN_XHD_MAX_SPOKE_LEN);
      m_history[i].bearing = -1;
      m_history[i].len = 0;
      m_history[i].time = 0;
      m_history[i].number = 0;
    }
    unlock();
  }

  const unsigned long long get_total_points_found()
  {
    return total_points_found;
  }

  void set_spoke(const long long t,
                 const int bearing, const unsigned char *data,
                 const size_t len, const int range_meters,
                 const int vth = 16, const int overwrap = 10,
		 const bool sidesupression = true)
  {
    if(len >=  GARMIN_XHD_MAX_SPOKE_LEN){
      cerr << "Invalid spoke length detected in ch_radar::set_spoke." << endl;
      return;
    }
    
    total_points_found = 0;
    if (m_range_meters != range_meters)
    {
      reset_image();
      bearing_prev_update = bearing - 1;
      if (bearing_prev_update < 0)
        bearing_prev_update += GARMIN_XHD_SPOKES;
    }
    lock();
    m_range_meters = range_meters;
    m_data_length = len;
    m_history[bearing].bearing = bearing;
    m_history[bearing].len = len;
    m_history[bearing].time = t;
    m_history[bearing].number = spoke_count;
  
    int fixed_spoke = -1;
    // update recent_spoke list
    if(num_recent_spokes == overwrap){
      // pop oldest spoke in the list, and extract first reflection 
      fixed_spoke = recent_spoke[recent_head];
      recent_head = (recent_head + 1) % overwrap;
      num_recent_spokes--;
    }

    recent_spoke[recent_tail] = bearing;
    num_recent_spokes++;
    recent_tail = (recent_tail + 1) % overwrap;

    // Spreading spoke energy into spokes overwrapped with the sidelobe.
    // beam width in the number of spokes
    int wspokes = overwrap * 2 + 1;

    // updateing new front spoke
    for (int r = 0; r < len; r++)
    {
        line_tmp[r] = (unsigned char)(data[r] / wspokes);
    }

    // spreading energy into backward spokes
    for (int i = -overwrap; i < 0; i++)
    {
      int ib = bearing + i;
      if (ib < 0)
        ib += GARMIN_XHD_SPOKES;
      unsigned char *line = lines[ib];
      if (spoke_count - m_history[ib].number > overwrap)
      { // for old spoke, the value is renewed
        for (int r = 0; r < len; r++)
          line[r] = line_tmp[r];
      }
      else // for recent spoke, the value is accumulated
      {
        for (int r = 0; r < len; r++)
        {
          line[r] = (unsigned char)min(255, (int)line[r] + line_tmp[r]);
        }
      }
    }

    // spreading energy into forward spokes
    for (int i = 0; i < overwrap + 1; i++)
    {
      int ib = (bearing + i) % GARMIN_XHD_SPOKES;
      unsigned char *line = lines[ib];
      if (spoke_count - m_history[ib].number > overwrap)
      { // for old spoke, the value is renewed
        for (int r = 0; r < len; r++)
          line[r] = line_tmp[r];
      }
      else // for recent spoke, the value is accumulated
      {
        for (int r = 0; r < len; r++)
        {
          line[r] = (unsigned char)min(255, (int)line[r] + line_tmp[r]);
        }
      }
    }

    // Sidelobe suppression
    // At the finished bearing [bearing - overwrap], value less than vth means
    // blob is not found inside the beam of current bearing.
    // It means that the continuous reflections on a same range in spokes the bearings are less than [bearing - overwrap]
    // includes sidelobe reflections on both side of the blob to be suppressed.
    //
    // for each range r
    //  if line[bearing - overwrap][r] < vth  && line[bearing - overwrap - 1][r] > vth
    //    count the spokes the blob is in the range as blob_width
    //    if blob_width > 2 * overwrap + 1
    //        side overwrap scopes are filled with zero
    //     else
    //       leave the center as the value, otehre spokes are filled with zero
    if (sidesupression)
    {
      unsigned char * line0 = lines[fixed_spoke];

      bool first_return_found = false;
      for (int r = 0; r < len; r++)
      {
        if(first_return_found){
          line0[r] = 0;
        }

        if (line0[r] < vth)
        {
          line0[r] = 0;

          int l = 0;
          int ib2 = fixed_spoke - 1;
          while (1)
          {
            if (ib2 < 0)
              ib2 += GARMIN_XHD_SPOKES;
            if(m_history[fixed_spoke].number - m_history[ib2].number > overwrap)
              break;
            unsigned char *line = lines[ib2];
            if (line[r] >= vth)
            {
              l++;
              ib2--;
            }
            else
            {
              break;
            }
          }

          if (l > 0)
          { // blob found
            int zero_len = zero_len = min(l / 2, overwrap);
            int i0 = fixed_spoke - 1;
            int i1 = i0 - zero_len;
            int i2 = fixed_spoke - (l - zero_len) - 1;
            int i3 = i2 - zero_len;

            if (i3 < 0)
            {
              i3 += GARMIN_XHD_SPOKES;
              if (i2 < 0)
              {
                i2 += GARMIN_XHD_SPOKES;
                if (i1 < 0)
                {
                  i1 += GARMIN_XHD_SPOKES;
                  if (i0 < 0)
                    i0 += GARMIN_XHD_SPOKES;
                }
              }
            }
            unsigned char val = 0;
            while (i0 != i3)
            {
              if (i0 == i2)
                val = 0;
              if (i0 == i1)
                val = 255;

              unsigned char *line = lines[i0];
              line[r] = val;
              if(val != 0)
                total_points_found++;
              i0--;
              if (i0 < 0)
                i0 += GARMIN_XHD_SPOKES;
            }
          }
        }else{
          first_return_found = true;
        }
      }
    }
    bearing_prev_update = bearing;
    spoke_count++;
    unlock();
  }

  int get_scaled_range_meters()
  {
    lock();
    int range_meters = (int)(0.5 + (double)m_range_meters *
                                       ((double)GARMIN_XHD_MAX_SPOKE_LEN / (double)m_data_length));
    unlock();
    return range_meters;
  }

  int get_range_meters()
  {
    lock();
    int range_meters = m_range_meters;
    unlock();
    return range_meters;
  }

  const unsigned char *get_lines()
  {
    return lines[0];
  }
};

#endif
