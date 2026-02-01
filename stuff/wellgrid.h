#include <list>
#include <vector>
#include <map>
#include <valarray>
#include <libh5/hdf5.h>

#ifndef __PUBLIC_STUFF__
#define __PUBLIC_STUFF__

/////////////// C O N R E C /////////////////////

typedef struct { double x, y; } coord;
typedef struct { int v1, v2, v3; } simplex;
typedef std::list<int> curve;
typedef std::list<curve> path;
typedef std::list<simplex> complex;
typedef std::vector<coord> vertex;
typedef std::valarray<float> f_array;

void conrec(int ss[6], const float* fn, int nz, const float* z, \
  vertex &ver, path *stroke, path *fill);

void blin(const coord& p, const coord& q, vertex& ver);

////////////////// G R I D ////////////////////

// class CDaoDatabase;

class geom_grid
{
public:
  struct field_struct
  { 
    int cx, cy; 
    double x1, x2, y1, y2; 
    const int *seam_list;
    const float **seam_data;
  };

  int size() const;
  int index(const double *src) const;
  void bbox(double bb[6]) const;
  bool cell(const double *src, double* dst) const;
  bool coord(const double *src, double* dst) const;
  float value(const double *src, const float* fn) const;
  float value(const double *src, hid_t data, float (*fn)(float)) const;
  void dom(int bb[3]) const;
  geom_grid(const field_struct*);
  ~geom_grid();

private:
  class dim
  {
    double d, x1, x2;
  public:
    dim(int n, double s, double f);
    double lo() const;
    double hi() const;
    double coord(double n) const;
    double cell(double x) const;
    int size() const;
  };

  dim x, y;
  int num, *crd, nz;
  float *z;
  const float z1, z2;
};

/*
////////////////// S I T E ////////////////////

class CDaoDatabase;
class well_site_impl;
typedef std::list<long> well_list;
struct perf_data { long bed; float range[2]; };
typedef std::list<perf_data> perf_list;
struct seam_data { long bed; float range[2]; float value; };
typedef std::list<seam_data> seam_list;

class well_site
{
public:
  well_site();
  ~well_site();

  void init(CDaoDatabase* db, const geom_grid* gg);

  bool cell(double xx[2], coord& p) const;
  void range_query(const coord& s, double r, well_list &bb) const;

  bool well_cell(long b, vertex& data) const;
  int traj_coord(long b, f_array x[3]) const;
  double model_dist(CDaoDatabase* db, long b, long s) const;
  int well_perf(CDaoDatabase* db, long b, double d0, double dd, perf_list &g) const;
  int well_seam(CDaoDatabase* db, long b, double d0, double dd, int rec, seam_list &g) const;

private:
  well_site_impl* impl;
};
*/

//////////////////// W E L L G R I D ///////////////////////

typedef struct { char name[8]; float top_x, top_y; } well_type;
typedef double (__stdcall *wg_split_func)(const well_type*, const well_type*);

extern "C" int __stdcall wg_create_uniform(int cx, int cy, double x1, double x2, double y1, double y2);
extern "C" int __stdcall wg_create_nonuniform(int cx, int cy, double *init_x, double *init_y);
extern "C" void __stdcall wg_destroy(int h);
extern "C" void __stdcall wg_update(int h, well_type* w);
extern "C" void __stdcall wg_compute(int h, wg_split_func fn);
extern "C" double __stdcall wg_info(int h, well_type* w, int x, int y);

extern "C" int __stdcall pg_create_uniform(int cx, int cy, double x1, double x2, double y1, double y2);
extern "C" int __stdcall pg_create_nonuniform(int cx, int cy, double *init_x, double *init_y);
extern "C" void __stdcall pg_destroy(int h);
extern "C" void __stdcall pg_populate(int h, int n, double *x, double *y);
extern "C" double __stdcall pg_info(int h, int x, int y);

extern "C" int __stdcall wh_create();
extern "C" void __stdcall wh_destroy(int h);
extern "C" void __stdcall wh_update(int h, well_type* w);
extern "C" void __stdcall wh_remove(int h, well_type* w);
extern "C" void __stdcall wh_clear(int h);
extern "C" int __stdcall wh_chull(int h, double delta, int* n, double* coord_x, double* coord_y);

#endif