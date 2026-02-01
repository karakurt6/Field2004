#include <functional>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <assert.h>


#define countof(arr) (sizeof(arr) / sizeof(*arr))

struct coord_type
{
  double x, y;
};

typedef std::list<int> curve_type;

double* max_bbox(double *bb)
{
  bb[0] = std::numeric_limits<double>::max();
  bb[1] = bb[0];
  bb[2] = -bb[0];
  bb[3] = bb[2];
  return bb;
}

double* init_bbox(const coord_type& p, double *bb)
{
  bb[0] = bb[2] = p.x;
  bb[1] = bb[3] = p.y;
  return bb;
}

class bbox
{
  const coord_type* ver;
public:
  bbox(const coord_type* p): ver(p)
  {
  }
  double* operator()(double* bb, int n) const
  {
    const coord_type* p = &ver[n];
    if (p->x < bb[0]) bb[0] = p->x;
    if (p->y < bb[1]) bb[1] = p->y;
    if (p->x > bb[2]) bb[2] = p->x;
    if (p->y > bb[3]) bb[3] = p->y;
    return bb;
  }
};

void main()
{
  coord_type zz[] = 
  { 
    { 1.0, 2.0 }, 
    { 3.0, 4.0 }, 
    { 5.0, 6.0 }
  };

  curve_type cur;
  cur.push_back(0);
  cur.push_back(1);
  cur.push_back(2);
  cur.push_back(0);

  double bb[4];
  // check if it is closed curve
  assert(cur.size() > 3 && cur.front() == cur.back());
  // std::accumulate(cur.begin(), cur.end(), max_bbox(bb), bbox(zz));

  curve_type::iterator it = ++cur.begin();
  init_bbox(zz[*it], bb);
  std::accumulate(++it, cur.end(), bb, bbox(zz));
  
  std::cout << bb[0] << ' ' << bb[1] << ' ' << bb[2] << ' ' << bb[3] << '\n';
}