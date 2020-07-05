#ifndef AWS_MAP_H
#define AWS_MAP_H
// Copyright(c) 2017 Yohei Matsumoto, All right reserved. 

// aws_map.h is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// aws_map.h is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with aws_map.h.  If not, see <http://www.gnu.org/licenses/>. 
#define GR 1.61803398875 // golden ratio
#define _AWS_MAP_DEBUG

namespace AWSMap2 {
  
  struct vec3{
    union{
      double x;
      double lat;
    };
    
    union{
      double y;
      double lon;
    };
    
    union {
      double z;
      double alt;
    };
    
    vec3() :x(0), y(0), z(0)
    {
    }
    
    vec3(const double _x, const double _y, const double _z) : x(_x), y(_y), z(_z)
    {
    }
    
    const vec3 & operator *= (const double s)
    {
      x *= s; y *= s; z *= s;
      return *this;
    }
    
    const vec3 & operator += (const vec3 & r)
    {
      x += r.x; y += r.y; z += r.z;
      return *this;
    }
  };


  inline bool operator == (const vec3 & l, const vec3 & r)
  {
    return l.x == r.x && l.y == r.y && l.z == r.z;
  }
  
  inline vec3 operator - (const vec3 & l, const vec3 & r)
  {
    return vec3(l.x - r.x, l.y - r.y, l.z - r.z);
  }
  
  inline vec3 operator + (const vec3 & l, const vec3 & r)
  {
    return vec3(l.x + r.x, l.y + r.y, l.z + r.z);
  }
  
  inline vec3 operator * (const vec3 & l, const double & r)
  {
    return vec3(l.x * r, l.y * r, l.z * r);
  }
  
  inline double dot(const vec3 & l, const vec3 & r)
  {
    return l.x * r.x + l.y * r.y + l.z * r.z;
  }
  
  struct vec2{
    union{
      double x;
      double lat;
    };
    
    union{
      double y;
      double lon;
    };

    vec2() :x(0.), y(0.)
    {
    }
    
    vec2(const double _x, const double _y) :x(_x), y(_y)
    {
    }
  };
  
  inline bool operator == (const vec2 & l, const vec2 & r)
  {
    return l.x == r.x && l.y == r.y;
  }
  
  
  inline double l2Norm2(const vec3 & pt0, const vec3 & pt1)
  {
    double tmp, result = 0.0;
    tmp = pt0.x - pt1.x;
    result = tmp * tmp;
    
    tmp = pt0.y - pt1.y;
    result += tmp * tmp;
    
    tmp = pt0.z - pt1.z;
    result += tmp * tmp;
    
    return result;
  }
  
  inline double l2Norm(const vec3 & pt0, const vec3 & pt1)
  {
    return sqrt(l2Norm2(pt0, pt1));
  }
  
  enum {
    MAX_PATH_LEN=1024
  };
  
  enum LayerType {
    lt_coast_line=0, lt_undef
  };

  extern const char * strLayerType[lt_undef];
  LayerType getLayerType(const char * str);
  class Node;
  class LayerData;
  class LayerDataPtr;

  // MapDataBase provide access to the databese.
  // The nodes and their data is casched so that the total size is kept
  // less than their limits given. (The data least recently used is basically
  // discarded first.)
  
  // Methods: 
  // "request" method is used to retrieve layer data in a region specified.
  // If all the data requested is on the cache, the class returns their layer
  // data immediately. Otherwise, the missing nodes and their layer data are
  // identified and loaded.

  // "insert" method is used to add a certain layer data to the data base.
  // the layer data is automatically splitted into multiple nodes to satisfy
  // the data size limits. (The layer data class should implemented splitter.)

  // "erase" method is used to remove a ceratin layer data from the data base.
  // The layer data should be specified by that provided layer data with
  // request method. The class does not provide update method inside layer data.
  // If you need to modify the layer data, first erase the data, and then
  // insert newly created updated layer data.

  // 
  class MapDataBase
  {
  private:
    static unsigned int maxSizeLayerData[lt_undef]; // maximum size of
                                                    // each LayerData instance
    static unsigned int maxNumNodes;		    // maximum number of
                                                    // Node instances
    static unsigned int maxTotalSizeLayerData;	    // maximum total size of
                                                    // LayerData instances
    static char * path;				    // local storage path to
                                                    // save Node and LayerData. 
    
  public:
    static unsigned int getMaxSizeLayerData(const LayerType & layerType)
    {
      return maxSizeLayerData[layerType];
    }
    
    static void setMaxSizeLayerData(const LayerType & layerType,
				    const unsigned int size)
    {
      maxSizeLayerData[layerType] = size;
    }
    
    static unsigned int getMaxNumNodes()
    {
      return maxNumNodes;
    }
    static void setMaxNumNodes(const unsigned int _maxNumNodes)
    {
      maxNumNodes = _maxNumNodes;
    }
    
    static unsigned int getMaxTotalSizeLayerData()
    {
      return maxTotalSizeLayerData;
    }
    
    static void setMaxTotalSizeLayerData(const unsigned int _maxTotalSizeLayerData)
    {
      maxTotalSizeLayerData = _maxTotalSizeLayerData;
    }
    
    static const char * getPath();
    static void setPath(const char * path);
    
  private:
    Node * pNodes[20];     // 20 triangles of the first icosahedron
    mutex mtx;
  public:
    MapDataBase();
    virtual ~MapDataBase();
    
    bool init();
    
    // request layerData within the circle specified with (center, radius).
    void request(list<list<LayerDataPtr>> & layerDatum,
		 const list<LayerType> & layerTypes,
		 const vec3 & center, const float radius,
		 const float resolution = 0);

    // insert an instance of LayerData to the location.
    bool insert(const LayerData * layerData);
    
    // erase an instance of LayerData.
    // The instance should be got via request method
    bool erase(const LayerData * layerData);
    
    // restruct MapDataBase on the memory, as the number of
    // nodes and total size of layer data are to be less than their limits.
    // (this method should be called periodically)
    void restruct();
    
    // save MapDataBase (only the nodes and data updated)
    bool save();
  };
  
  class Node
  {
  private:
    // Node list for memory management.
    static Node * head, * tail;
    
    // Number of nodes in the node list.
    static unsigned int numNodesAlive;
    
    // Insert newly instantiated node to the node list. 
    static void insert(Node * pNode);

    // Remove pNode from node list.
    static void pop(Node * pNode);

    // Move pNode to the tail of the node list.
    // called when the node is accessed.
    static void accessed(Node * pNode); 
  public:
    
    // Remove nodes if the limit of  maximum number of nodes are violated.
    // Nodes with no downlink node least recently used are removed.
    static void restruct(); 

    // Loads child node.
    static Node * load(Node * pNodeUp, unsigned int idChild);
    
    static const unsigned int getNumNodesAlive()
    {
      return numNodesAlive;
    }
    
    static const int getMaxLevel()
    {
      char maxLevel = 0;
      for (Node * pn = head; pn != NULL; pn = pn->next)
	maxLevel = max((int)pn->level, (int)maxLevel);
      return (int) maxLevel;
    }
    
  private:
    Node * prev, * next;// link pointers for memory management
    unsigned char level;
    int refcount;
    bool bupdate;		// update flag. asserted when the layerDataList or downLink is updated
    unsigned char id;	// id of the node in the upper node. (0 to 3 for ordinal nodes. 0 to 19 for top level nodes.)
    Node * upLink;		// Up link. NULL for top 20 nodes
    bool bdownLink;		// false until the downLink is created.
    Node * downLink[4]; // Down link. 
    vec2 vtx_blh[3];	// blh coordinte of the node's triangle
    void calc_ecef();
    vec3 vtx_ecef[3];   // ecef coordinate of the node's triangle (calculated automatically in construction phase) 
    vec3 vec_ecef[2];		// vtx_ecef[1] - vtx_ecef[0], vtx_ecef[2] - vtx_ecef[0]

    map<LayerType, LayerData*> layerDataList;
    
    // create downLink nodes, and assert bdownLink flag. 
    // the function is called only in addLayerData when the addition exceeds the 
    // size limit of the layer type. 
    bool createDownLink();
    
    // getPath(list<unsigned char>) helps getPath(char*, unsigned int) to generate path string to this node.
    void getPath(list<unsigned char> & path_id);
    
    // insertLayerData helps addLayerData. 
    void insertLayerData(LayerData * pLayerData);
    
    // getLayerData returns layerData of layerType in this node 
    LayerData * getLayerData(LayerType layerType);
    
    // distributeLayerData helps addLayerData. 
    bool distributeLayerData(const LayerData & layerData);
    
    // relleaseLayerData releases all the layer data in the node.
    void releaseLayerData();
    
  public:
    Node();
    Node(const unsigned char _id, Node * _upLink, const vec2 vtx_blh0, const vec2 vtx_blh1, const vec2 vtx_blh2);
    
    virtual ~Node();
    
    // setId(unsigned char) set node index in the upper layer.
    void setId(const unsigned char _id){
      id = _id;
    }
    
    const unsigned char getId()
    {
      return id;
    }
    
    const vec3 & getVtxECEF(int i) const
    {
      if (i >= 0 && i < 3)
	return vtx_ecef[i];
      return vtx_ecef[0];
    }
    
    int getLevel()
    {
      return level;
    }
    
    bool isLocked();
    
    void lock()
    {
      refcount++;
    }
    
    void unlock()
    {
      refcount--;
    }
    
    // getPath(char*, unsigned int) returns the path string
    // the length is less than the specified limit.
    void getPath(char * path, unsigned int maxlen);
    
    // save Node data and layer data recursively for all downlinks
    // this function is called only from MapDataBase::save()
    bool save();
    
    // collision(vec3) determines whether the specified point collides
    // with the node.
    const bool collision(const vec3 & location, const double err = 0.0f);
    
    // collision(vec3, float) determines whether the specified circle
    // collides with the node.
    const bool collision(const vec3 & center, const float radius);
    
    
    const void collision_downlink(const vector<vec3> & pts, vector<char> & inodes);

    // getLayerData called from MapDataBase::request
    void getLayerData(list<list<LayerDataPtr>> & layerData, 
		      const list<LayerType> & layerType, const vec3 & center,
		      const float radius, const float resolution = 0);
    
    // addLayerData adds the layer data given in the argument.
    // the function is invoked from LayerData::split, and the split is called from MapDataBase::insert
    // This function recursively call split() and itself so that the size of the layer data does not exceeds its limit.
    bool addLayerData(const LayerData & layerData, 
		      const size_t sz_node_data_lim = 0x4FFFFF /* 4MB */);
    
    // deleteLayerData deletes the layer data given in the argument.
    // The method delete the layer data in the node when the pointer is exactly the same as that given in the argument.
    // so the pointer given as the argument should be got via getLayerData
    bool deleteLayerData(const LayerData * layerData);	
  };
  
  class LayerData
  {
    friend class LayerDataPtr;
    // static section
  private:
    static LayerData * head, * tail;
    static unsigned int totalSize;
  protected:
    static void insert(LayerData * pLayerData);
    static void pop(LayerData * pLayerData);
  public:
    static void accessed(LayerData * pLayerData);
    static void resize(unsigned int size_diff)
    {
      totalSize += size_diff;
    }
    
    static const unsigned int getTotalSize()
    {
      return totalSize;
    }
    
    static void restruct();
    static LayerData * create(const LayerType layerType); // factory function
    
  protected:
    LayerData * prev, * next;
    int  refcount;
    bool bupdate;
    bool bactive;
    
    Node * pNode;
    
    void genFileName(char * fname, size_t len_max)
    {
      char path[2048];
      pNode->getPath(path, 2048);
      snprintf(fname, len_max, "%s/%s.dat", path, strLayerType[getLayerType()]);
    }
    
  public:
    LayerData() : prev(NULL), next(NULL), pNode(NULL), refcount(0), bupdate(false), bactive(false) {};
    virtual ~LayerData() {};
    
    void setNode(Node * _pNode) { pNode = _pNode; };
    Node * getNode() const { return pNode; };
    
    void setActive(){
      bactive = true;
      LayerData::insert(this);
    }
    
    bool isActive(){
      return bactive;
    }
    
    bool isLocked()
    {
      return refcount > 0;
    }
    
    void lock() {
      refcount++;
    }
    
    void unlock() {
      refcount--;
    }
    
    // major interfaces 
    bool save();
    bool load();
    void release();
    bool reduce(const size_t sz_lim);
    bool merge(const LayerData & layerData);
    
    // interfaces to be implemented in sub-classes.
  protected:
    // reduce the data structure to meet the size limit
    virtual bool _reduce(const size_t sz_lim) = 0;
    
    // merge given layerData to this layerData
    virtual bool _merge(const LayerData & layerData) = 0;
    
    // release all internal data structure but does not mean
    // the destruction of this object
    virtual void _release() = 0;			  
    
  public:
    // returns LayerType value.
    virtual const LayerType getLayerType() const = 0;
    
    // save data to ofile stream.
    virtual bool save(ofstream & ofile) = 0;
    
    // load data from ifile stream.
    virtual bool load(ifstream & ifile) = 0;

    // split the layer data into nodes given
    virtual bool split(list<Node*> & nodes, Node * pParentNode = NULL) const = 0;
    // returns clone of the instance
    virtual LayerData * clone() const = 0;
    
    // returns size in memory 
    virtual size_t size() const = 0;
    
    // returns minimum distance between objects in meter		
    virtual float resolution() const = 0;

    // returns radius of the object's distribution in meter
    virtual float radius() const = 0;
    
    // returns center of the object's distribution
    virtual vec3 center() const = 0;

    // dump the data inside as text 
    virtual void print() const = 0;
  };
  
  #include "aws_map_point.hpp"
  #include "aws_map_coast_line.hpp"
    
  class LayerDataPtr
  {
  private:
    const LayerData * ptr;
  public:
    LayerDataPtr() :ptr(NULL)
    {
    }
    
    LayerDataPtr(const LayerDataPtr & ldp) :ptr(ldp.ptr)
    {
      const_cast<LayerData*>(ptr)->lock();
    }
    
    LayerDataPtr(const LayerData * _ptr) :ptr(_ptr)
    {
      const_cast<LayerData*>(ptr)->lock();
    }
    
    ~LayerDataPtr()
    {
      const_cast<LayerData*>(ptr)->accessed(const_cast<LayerData*>(ptr));
      const_cast<LayerData*>(ptr)->unlock();
      if (!const_cast<LayerData*>(ptr)->getNode()) {
	  const_cast<LayerData*>(ptr)->pop(const_cast<LayerData*>(ptr));
	  delete ptr;
      }
    }
    
    const LayerData & operator * () const
    {
      return *ptr;
    }
    
    const LayerData * operator ->() const
    {
      return ptr;
    }    
  };
};

struct Point3f{
  float x,y,z;
  Point3f():x(0),y(0),z(0){};
  Point3f(float _x, float _y, float _z):x(_x), y(_y), z(_z){};
};

struct Point2f{
  float x,y;
  Point2f():x(0),y(0){};
  Point2f(float _x, float _y):x(_x), y(_y){};
};

class c_icosahedron
{
private:
  unsigned int nv, nf, ne; // number of vertices, faces, edges
  
  Point3f *v; // vertex in ecef
  Point2f *q; // vertex in lat/lon
  unsigned int ** f;   // face (index)
  unsigned int ** e;   // edge (index)
  
public:
  c_icosahedron();
  c_icosahedron(const c_icosahedron & icshdrn);
  ~c_icosahedron();
  Point3f get_mid_point(unsigned int ie) const
  {
    return Point3f(
		   (v[e[ie][0]].x + v[e[ie][1]].x) * 0.5,
		   (v[e[ie][0]].y + v[e[ie][1]].y) * 0.5,
		   (v[e[ie][0]].z + v[e[ie][1]].z) * 0.5);  
  }
  
  unsigned int get_edge(unsigned int iv0, unsigned int iv1) const
  {
    if (iv0 > iv1){
      int t = iv0;
      iv0 = iv1;
      iv1 = t;
    }
    for (unsigned int ie = 0; ie < ne; ie++)
      {
	if (e[ie][0] == iv0 && e[ie][1] == iv1)
	  return ie;
      }
    
    return UINT_MAX;
  }
  
  const unsigned int get_nv() const
  {
    return nv;
  }
  
  const unsigned int get_nf() const
  {
    return nf;
  }
  
  const unsigned int get_ne() const
  {
    return ne;
  }
  
  Point3f * getv(){
    return v;
  }
  
  Point2f * getq(){
    return q;
  }
  
  unsigned int ** getf(){
    return f;
  }
};

#endif
