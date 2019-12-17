// Copyright(c) 2019 Yohei Matsumoto, All right reserved. 

// factory is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// factory is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with factory.  If not, see <http://www.gnu.org/licenses/>. 

///////////////////////////////////////////////// setting up channel factory
#include "aws.hpp"
#include "ch_sample.hpp"
#include "ch_vector.hpp"
#include "ch_nmea.hpp"
#include "ch_state.hpp"
#include "ch_aws1_sys.hpp"
#include "ch_aws1_ctrl.hpp"
#include "ch_map.hpp"
#include "ch_obj.hpp"
#include "ch_wp.hpp"
#include "ch_radar.hpp"

// Initialization function. 
// This function is called at the begining of the aws process start. If you
// need to initialize global and static data structure related to channel
// please insert your initialization code here.
void ch_base::init()
{
  register_factory();
}

// Uninitialization function. 
// This function is called at the end of the aws process. If you add your
// initialization code, correspodning destruction code should be added to
// this function.
void ch_base::uninit()
{
}

// Registration function.
// This function is called at the beginning of the aws process. If you add
// your own channels to the system, please register your channel by inserting
// the code "register_factory<class>("class string").
void ch_base::register_factory()
{
  register_factory<ch_sample>("sample");
  register_factory<ch_nmea>("nmea");
  
  register_factory<ch_aws1_ctrl_inst>("aws1_ctrl_inst");
  register_factory<ch_aws1_ctrl_stat>("aws1_ctrl_stat");
  register_factory<ch_aws1_ap_inst>("aws1_ap_inst");
  register_factory<ch_ring<char, 1024> >("crbuf");
  register_factory<ch_ring<char, 2048> >("crbuf2k");
  register_factory<ch_ring<char, 4096> >("crbuf4k");
  register_factory<ch_ring<char, 8192> >("crbuf8k");
  register_factory<ch_state>("state");
  register_factory<ch_eng_state>("engstate");
  register_factory<ch_map>("map");
  register_factory<ch_ais_obj>("ais_obj");
  register_factory<ch_wp>("wp");
  register_factory<ch_aws1_sys>("aws1_sys"); 
  register_factory<ch_radar_image>("radar_image");
  register_factory<ch_radar_ctrl>("radar_ctrl");
  register_factory<ch_radar_state>("radar_state");
}
