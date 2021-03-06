// Copyright(c) 2020 Yohei Matsumoto, All right reserved.

// aws_log.hpp is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_log.hpp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_log.hpp.  If not, see <http://www.gnu.org/licenses/>.

#ifndef AWS_LOG_HPP
#define AWS_LOG_HPP
#if __GNUC__ < 9
#include <experimental/filesystem>
namespace fs = experimental::filesystem;
#else
#include <filesystem>
namespace fs = filesystem;
#endif
#include <algorithm>
#include <regex>

class c_log
{
private:
  string path;
  string prefix;
  unsigned int size_max;
  unsigned int total_size;
  char time_str[32];
  bool bread;

  vector<long long> time_stamps;
  int current_timestamp_index;
  long long current_timestamp;

  ofstream *ofile;
  ifstream *ifile;

  bool open_new_write_file(const long long t)
  {
    if (ofile)
    {
      delete ofile;
    }
    ofile = new ofstream;

    string fname = path + "/" + prefix + get_time_str(t);
    ofile->open(fname, ios_base::binary);
    return ofile->is_open();
  }

  bool open_read_next_file()
  {
    if (ifile)
    {
      delete ifile;
      ifile = nullptr;
    }
    
    if (current_timestamp_index == time_stamps.size())
      return false;

    current_timestamp_index++;
    if (current_timestamp_index == time_stamps.size())
      return false;

    ifile = new ifstream;

    string fname = path + "/" + prefix + get_time_str(time_stamps[current_timestamp_index]);
    cout << "Opening " << fname << endl;    
    ifile->open(fname, ios_base::binary);
    return ifile->is_open();
  }

  bool open_read_file(const long long t)
  {
    current_timestamp_index = get_time_index(t);
    if (current_timestamp_index == -1)
      return false;

    if (ifile)
    {
      delete ifile;
    }
    ifile = new ifstream;

    string fname = path + "/" + prefix + get_time_str(time_stamps[current_timestamp_index]);
    cout << "Opening " << fname << endl;
    ifile->open(fname, ios_base::binary);
    return ifile->is_open();
  }

  const char *get_time_str(const long long t)
  {
    snprintf(time_str, 32, "_%lld.log", t);
    return time_str;
  }

  const int get_time_index(const long long t)
  {
    int sz = time_stamps.size();
    int imin = 0, imax = sz - 1;

    // return if the time is out of range.
    if(t < time_stamps[imin])
      return 0;
    if(t >= time_stamps[imax])
      return imax;

    int i = (imax - imin) >> 1;    
    while (1)
    {
      int j = i + 1;

      if (time_stamps[i] > t)
      {
        if (i == imin) // the logging time epoch is later.
          return imin;
        imax = i;
	i = imin + ((imax - imin) >> 1);
        continue;
      }

      if (time_stamps[j] <= t)
      {
	imin = j;	
        i = imin + ((imax - imin) >> 1);
        continue;
      }

      if (time_stamps[i] <= t && time_stamps[j] > t)
        return i;
    }
    return -1;
  }

public:
  c_log() : path(), prefix(), size_max(0), total_size(0),
            bread(false), current_timestamp_index(-1), current_timestamp(-1),
            ifile(nullptr), ofile(nullptr)
  {
  }

  ~c_log()
  {
    if (ofile){
      if(current_timestamp > 0)
	open_new_write_file(current_timestamp + 1);
      delete ofile;
    }

    if (ifile)
      delete ifile;
  }

  bool init(const string _path, const string _prefix,
            bool _bread = false, const unsigned int _size_max = (1 << 30))
  {
    path = _path;
    prefix = _prefix;
    size_max = _size_max;
    bread = _bread;

    string rstr(prefix);
    rstr += "_[0-9]+.log";
    regex r(rstr);

    // in read mode, list the files with prefix in the path and record all
    // the timestamps of the files.
    if (bread)
    {
      for (auto &p : fs::directory_iterator(path))
      {
        string fname = p.path().filename();
        if (regex_match(fname.begin(), fname.end(), r))
        {
          unsigned int start_pos = prefix.size() + 1;
          unsigned int end_pos = fname.find(".log");
          long long t = atoll(fname.substr(start_pos, end_pos).c_str());
          time_stamps.push_back(t);
        }
      }
    }
    sort(time_stamps.begin(), time_stamps.end());
    cout << "Log Data " << rstr << endl;
    int index = 0;
    for (auto itr = time_stamps.begin(); itr != time_stamps.end(); itr++){
      cout << index << ":" << *itr << endl;
      index++;
    }
	
    return true;
  }

  bool is_replay(){
    return bread;
  }
  
  void destroy()
  {
    if (ofile){
      if(current_timestamp > 0)
	open_new_write_file(current_timestamp + 1);
      delete ofile;
    }
    ofile = nullptr;
    if (ifile)
      delete ifile;
    ifile = nullptr;

    current_timestamp_index = -1;
    current_timestamp = -1;
  }

  bool write(const long long t, const unsigned char *buf,
             const unsigned int buf_size)
  {
    if(buf_size <= 0){
      cerr << "The data size is not correct. Data size " << buf_size << endl;
      return false;
    }
      
    if(t < current_timestamp){
      cerr << "Timestamp is not consistent. Previous time is " << current_timestamp << " and now " << t << endl;
      return false;
    }
    if (bread)
      return false;

    if (!ofile ||
        (total_size + buf_size + sizeof(t) + sizeof(buf_size) > size_max))
    {
      if (!open_new_write_file(t))
        return false;
      total_size = 0;
    }

    ofile->write((const char *)&t, sizeof(t));
    ofile->write((const char *)&buf_size, sizeof(buf_size));
    ofile->write((const char *)buf, buf_size);

    total_size += buf_size + sizeof(t) + sizeof(buf_size);
    current_timestamp = t;
    return true;
  }

  bool read(long long &t, unsigned char *buf, unsigned int &buf_size)
  {
    if (!bread)
    {
      return false;
    }

    if (current_timestamp_index == -1)
    {
      if (!open_read_file(t))
      {
        return false;
      }
      ifile->read((char *)&current_timestamp, sizeof(current_timestamp));
    }

    buf_size = 0;

    if(ifile == nullptr)
      return false;
    
    if (current_timestamp <= t)
    {
      ifile->read((char *)&buf_size, sizeof(buf_size));
      ifile->read((char *)buf, buf_size);
      t = current_timestamp;
      ifile->read((char *)&current_timestamp, sizeof(current_timestamp));
      
      if(current_timestamp < t){
	cerr << "Timestamp is not consistent. Previous timestamp is " << t << " and now " << current_timestamp << endl;
	if(open_read_next_file()){
          ifile->read((char *)&current_timestamp, sizeof(current_timestamp));	
	}else{
	  return false;
	}
      }
      
      if (ifile->eof())
      {
        if (open_read_next_file())
        {
          ifile->read((char *)&current_timestamp, sizeof(current_timestamp));
        }
      }
    }

    return true;
  }

  int get_num_read_files()
  {
    return time_stamps.size();
  }
};

#endif
