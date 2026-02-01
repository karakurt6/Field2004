#define _USE_MATH_DEFINES
#ifndef __INTEL_COMPILER
  #include <math.h>
#else
  #include <math.h>
  // #include <mathimf.h>
#endif

#include <limits>
#include <numeric>
#include <algorithm>
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <assert.h>
#include "ps_data.h"

struct Segment
{
  double alpha;
  bool outgoing;
};

struct Coord_compare: public std::binary_function<const coord_type*, const coord_type*, bool>
{
  result_type operator() (first_argument_type p, second_argument_type q) const
  {
    if (p->ycoord < q->ycoord)
      return true;

    if (p->ycoord > q->ycoord)
      return false;

    return (p->xcoord < q->xcoord);
  }
};

typedef std::list<Segment> Segment_list;
typedef std::map<const coord_type*, Segment_list, Coord_compare> Segment_array;
typedef std::vector<const Segment*> Segment_vector;

bounding_box::bounding_box(const coord_type* p): ver(p)
{
}

double* bounding_box::operator()(double* bb, int n) const
{
  const coord_type* p = &ver[n];
  if (p->xcoord < bb[0]) bb[0] = p->xcoord;
  if (p->ycoord < bb[1]) bb[1] = p->ycoord;
  if (p->xcoord > bb[2]) bb[2] = p->xcoord;
  if (p->ycoord > bb[3]) bb[3] = p->ycoord;
  return bb;
}

double* bounding_box::max(double *bb)
{
  bb[0] = std::numeric_limits<double>::max();
  bb[1] = bb[0];
  bb[2] = -bb[0];
  bb[3] = bb[2];
  return bb;
}

double* bounding_box::init(const coord_type& p, double *bb)
{
  bb[0] = bb[2] = p.xcoord;
  bb[1] = bb[3] = p.ycoord;
  return bb;
}

double* curve_bbox(const vertex_type& ver, const curve_type& cur, double bb[4])
{
  return std::accumulate(++cur.begin(), cur.end(), \
    bounding_box::init(ver[cur.front()], bb), bounding_box(&ver[0]));
}

double* closed_curve_bbox(const vertex_type& ver, const curve_type& cur, double bb[4])
{
  assert(is_closed(cur));
  curve_type::const_iterator it = ++cur.begin();
  bounding_box::init(ver[*it++], bb);
  return std::accumulate(it, cur.end(), bb, bounding_box(&ver[0]));
}

bool is_empty(const curve_type& cur)
{
  return cur.empty();
}

bool is_point(const curve_type& cur)
{
  return (cur.size() == 1);
}

bool is_segment(const curve_type& cur)
{
  return (cur.size() == 2);
}

bool is_closed(const curve_type& cur)
{
  // assert(cur.size() > 2);
  return (cur.front() == cur.back());
}

// hint: is this precondition violated, try one of the following ways
// 1) apply std::unique(cur) method
// 2) build lexord dictionary from original data
bool has_duplicate_points(const vertex_type& ver, const curve_type& cur)
{
  assert(cur.size() > 1);
  curve_type::const_iterator c = cur.begin();
  const coord_type* p = &ver[*c];
  while (++c != cur.end())
  {
    const coord_type* q = &ver[*c];
    if (p->xcoord == q->xcoord && p->ycoord == q->ycoord)
    {
      return true;
    }
    p = q;
  }
  return false;
}

bool has_degenerate_segment(const vertex_type& ver, const curve_type& cur)
{
  assert(cur.size() > 2);
  curve_type::const_iterator c = cur.begin();
  const coord_type* p1 = &ver[*c++];
  const coord_type* p2 = &ver[*c];
  while (++c != cur.end())
  {
    const coord_type* p3 = &ver[*c];
    Classify c = classify(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, p3->xcoord, p3->ycoord);
    if (c == BEHIND || c == BETWEEN || c == ORIGIN || c == DESTINATION)
    {
      return true;
    }
    p1 = p2;
    p2 = p3;
  }
  return false;
}

// all segments aligned along common guide line
bool is_degenerate_curve(const vertex_type& ver, const curve_type& cur)
{
  if (!cur.empty())
  {
    curve_type::const_iterator c = cur.begin();
    const coord_type* p1 = &ver[*c++];
    const coord_type* p2 = &ver[*c];
    while (++c != cur.end())
    {
      const coord_type* p3 = &ver[*c];
      Classify c = classify(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, p3->xcoord, p3->ycoord);
      if (c == LEFT || c == RIGHT)
      {
        return false;
      }
      p1 = p2;
      p2 = p3;
    }
  }
  return true;
}

void plot_curve(const vertex_type& ver, const curve_type& cur)
{
  psstream out("curve.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);
  out.newpage();

  double bb[4];
  std::accumulate(++cur.begin(), cur.end(), \
    bounding_box::init(ver[cur.front()], bb), bounding_box(&ver[0]));

  double x0, y0, sx, sy;
  double dx = bb[2] - bb[0], dy = bb[3] - bb[1];
  double pg_width = out.pagewidth();
  double pg_height = out.pageheight();
  double pg_margin = 40.0;
  double px = pg_width - 2.0 * pg_margin;
  double py = pg_height - 2.0 * pg_margin;
  if (px * dy < py * dx)
  {
    x0 = pg_margin;
    sx = px;
    sy = px * dy / dx;
    y0 = 0.5 * (pg_height - sy);
  }
  else
  {
    y0 = pg_margin;
    sy = py;
    sx = py * dx / dy;
    x0 = 0.5 * (pg_width - sx);
  }
  if (dx > 0.0) sx /= dx;
  if (dy > 0.0) sy /= dy;
  x0 -= bb[0] * sx;
  y0 -= bb[1] * sy;

  curve_type::const_iterator s = cur.begin();
  const coord_type* p = &ver[*s];
  out.moveto(x0 + sx * p->xcoord, y0 + sy * p->ycoord);
  while (++s != cur.end())
  {
    p = &ver[*s];
    out.lineto(x0 + sx * p->xcoord, y0 + sy * p->ycoord);
  }
  out.stroke();

  /*
  for (s = cur.begin(); s != cur.end(); ++s)
  {
    p = &ver[*s];
    out.arc(x0 + sx * p->xcoord, y0 + sy * p->ycoord, 0.5, 0.0, 360.0);
    out.fill();
  }
  */
}

static bool find_crossing(const vertex_type& ver, const curve_type& cur, \
  Segment_array& arr)
{
  double a_max = 2.0 * M_PI;
  for (curve_type::const_iterator p = cur.begin(), q = p; ++q != cur.end(); p = q)
  {
    const coord_type* p1 = &ver[*p], *p2 = &ver[*q];
    curve_type::const_iterator t = q, s = t++;
    while (t != cur.end())
    {
      double u, v;
      const coord_type* q1 = &ver[*s], *q2 = &ver[*t];
      if (SKEW_CROSS == cross(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
        q1->xcoord, q1->ycoord, q2->xcoord, q2->ycoord, &u, &v) \
        && (0.0 != u && 1.0 != u || 0.0 != v && 1.0 != v))
      {
        return true;
      }
      s = t++;
    }

    double a = atan2(p1->ycoord - p2->ycoord, p1->xcoord - p2->xcoord);
    if (a < 0.0) a += a_max;

    arr[p2].push_back(Segment());
    Segment* g = &arr[p2].back();
    g->alpha = a;
    g->outgoing = false;

    a += M_PI;
    if (a >= a_max) a -= a_max;

    arr[p1].push_back(Segment());
    g = &arr[p1].back();
    g->alpha = a;
    g->outgoing = true;
  }
  return false;
}

bool is_self_crossed(const vertex_type& ver, const curve_type& cur)
{
  if (is_closed(cur) || has_duplicate_points(ver, cur))
    return true;

  Segment_array arr;
  if (find_crossing(ver, cur, arr))
  {
    return true;
  }

  for (Segment_array::iterator it = arr.begin(); it != arr.end(); ++it)
  {
    if (it->second.size() > 2)
    {
      return true;
    }
  }
  return false;
}

bool is_simple(const vertex_type& ver, const curve_type& cur)
{
  assert(is_closed(cur));
  assert(!has_duplicate_points(ver, cur));
  
  Segment_array arr;
  if (find_crossing(ver, cur, arr))
  {
    // plot_curve(ver, cur);
    return false;
  }
  for (Segment_array::iterator it = arr.begin(); it != arr.end(); ++it)
  {
    if (it->second.size() > 2)
    {
      return false;
    }
  }
  return true;
}

static inline const Segment* address(const Segment& z)
{
  return &z;
}

static inline bool alpha_compare(const Segment* p, const Segment* q)
{
  return (p->alpha < q->alpha);
}

bool is_weakly_simple(const vertex_type& ver, const curve_type& cur)
{
  Segment_array arr;
  if (find_crossing(ver, cur, arr))
  {
    return false;
  }
  for (Segment_array::iterator it = arr.begin(); it != arr.end(); ++it)
  {
    if (it->second.size() > 2)
    {
      Segment_vector adj(it->second.size());
      std::transform(it->second.begin(), it->second.end(), adj.begin(), address);
      std::sort(adj.begin(), adj.end(), alpha_compare);
      const Segment* prev = adj.back();
      for (Segment_vector::iterator s = adj.begin(); s != adj.end(); ++s)
      {
        const Segment* curr = *s;
        if (prev->outgoing == curr->outgoing)
        {
          return false;
        }
        prev = curr;
      }
    }
  }
  return true;
}

bool is_convex(const vertex_type& ver, const curve_type& cur)
{
  assert(is_simple(ver, cur));
  curve_type::const_iterator prev = cur.begin(), curr = prev;
  bool classified = false;
  Classify global;
  while (++curr != cur.end())
  {
    curve_type::const_iterator next = curr;
    if (++next == cur.end())
      next = ++cur.begin();

    Classify local = classify(ver[*prev].xcoord, ver[*prev].ycoord,
      ver[*curr].xcoord, ver[*curr].ycoord, ver[*next].xcoord, ver[*next].ycoord);
    if (local != BEYOND)
    {
      assert(local == LEFT || local == RIGHT);
      if (classified)
      {
        if (global != local)
        {
          return false;
        }
      }
      else
      {
        classified = true;
        global = local;
      }
    }
    prev = curr;
  }
  return true;
}

double area(const vertex_type& ver, const curve_type& cur)
{
  assert(is_weakly_simple(ver, cur));
  double a = 0.0;
  curve_type::const_iterator prev = cur.begin(), curr = prev;
  while (++curr != cur.end())
  {
    curve_type::const_iterator next = curr;
    if (++next == cur.end())
      next = ++cur.begin();
    a += ver[*curr].xcoord * (ver[*next].ycoord - ver[*prev].ycoord);
    prev = curr;
  }
  return (0.5 * a);
}

bool is_counter_clockwise(const vertex_type& ver, const curve_type& cur)
{
  return area(ver, cur) > 0.0;
}

bool is_generalized_polygon(const vertex_type& ver, const edge_type& pgn)
{
  Segment_array arr;
  for (edge_type::const_iterator e = pgn.begin(); e != pgn.end(); ++e)
  {
    if (find_crossing(ver, *e, arr))
    {
      return false;
    }
  }
  for (Segment_array::iterator it = arr.begin(); it != arr.end(); ++it)
  {
    if (it->second.size() > 2)
    {
      Segment_vector adj(it->second.size());
      std::transform(it->second.begin(), it->second.end(), adj.begin(), address);
      std::sort(adj.begin(), adj.end(), alpha_compare);
      const Segment* prev = adj.back();
      for (Segment_vector::iterator s = adj.begin(); s != adj.end(); ++s)
      {
        const Segment* curr = *s;
        if (prev->outgoing == curr->outgoing)
        {
          return false;
        }
        prev = curr;
      }
    }
  }
  return true;
}
