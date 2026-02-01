#define _USE_MATH_DEFINES
#ifndef __INTEL_COMPILER
  #include <math.h>
  #define hypotl hypot
#else
  #include <mathimf.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <algorithm>
#include <functional>
#include <numeric>
#include <limits>
#include "ps_data.h"

#define DEBUG_OUTPUT

const char* classify_table[] =
{
  "LEFT", "RIGHT", "BEHIND", "BEYOND", "BETWEEN", "ORIGIN", "DESTINATION"
};

const char* intersect_table[] =
{
  "COLLINEAR", "PARALLEL", "SKEW", "SKEW_CROSS", "SKEW_NO_CROSS"
};

const char* raycast_table[] =
{
  "TOUCHING", "CROSSING", "INESSENTIAL"
};

const char* inpoly_table[] =
{
  "INSIDE", "OUTSIDE", "BOUNDARY"
};

/*
 * decomposition of vector (x, y) on components in coordinate system
 * with origin at (x1, y1) and abscissa collinear to vector (x2-x1, y2-y1)
 * 
 * this can be applied do find:
 * 
 * 1) distance between line and point
 *     double d = hypot(x2 - x1, y2 - y1) * perpendicular(x1, y1, x2, y2, x, y);
 *     d > 0.0 => classify(x, y) == LEFT
 *     d < 0.0 => classify(x, y) == RIGHT
 * 2) parametrization along line
 *     double s = parallel(x1, y1, x2, y2, x, y),
 *     s == 0.0 => classify(x, y) == ORIGIN
 *     s == 1.0 => classify(x, y) == DESTINATION
 *     s > 1.0  => classify(x, y) == BEYOND
 *     s < 0.0  => classify(x, y) == BEHIND
 *     0.0 < s && s < 1.0 => classify(x, y) == BETWEEN
 */
double perpendicular(double x1, double y1, double x2, double y2, double x, double y)
{
  double bx = x2 - x1;
  double by = y2 - y1;
  return ((x - x1) * by - (y - y1) * bx) / (bx * bx + by * by);
}

double parallel(double x1, double y1, double x2, double y2, double x, double y)
{
  double bx = x2 - x1;
  double by = y2 - y1;
  return ((x - x1) * bx + (y - y1) * by) / (bx * bx + by * by);
}

Classify classify(double x1, double y1, double x2, double y2, double x, double y)
{
  double ax = x2 - x1;
  double ay = y2 - y1;
  double bx = x - x1;
  double by = y - y1;
  double sa = ax * by - bx * ay;
  if (sa > 0.0)
  {
    return LEFT;
  }
  if (sa < 0.0)
  {
    return RIGHT;
  }
  if ((ax * bx < 0.0) || (ay * by < 0.0))
  {
    return BEHIND;
  }
  if (ax * ax + ay * ay < bx * bx + by * by)
  {
    return BEYOND;
  }
  if (x1 == x && y1 == y)
  {
    return ORIGIN;
  }
  if (x2 == x && y2 == y)
  {
    return DESTINATION;
  }
  return BETWEEN;
}

Intersect intersect(double x1, double y1, double x2, double y2, \
  double x3, double y3, double x4, double y4, double* t)
{
  long double a11 = x1 - x2;
  long double a12 = x4 - x3;
  long double a21 = y1 - y2;
  long double a22 = y4 - y3;
  long double b1 = x1 - x3;
  long double b2 = y1 - y3;
  long double d = a11 * a22 - a21 * a12;
  if (d == 0.0)
  {
    Classify c = classify(x3, y3, x4, y4, x1, y1);
    if ((c == LEFT) || (c == RIGHT))
    {
      return PARALLEL;
    }
    else
    {
      return COLLINEAR;
    }
  }

  long double x0 = (b1 * a22 - b2 * a12) / d;
  long double y0 = (a11 * b2 - a21 * b1) / d;
  long double e1 = 0.0, e2 = 0.0;
  b1 -= a11 * x0 + a12 * y0;
  b2 -= a21 * x0 + a22 * y0;
  do
  {
    long double e1 = (b1 * a22 - b2 * a12) / d;
    long double e2 = (a11 * b2 - a21 * b1) / d;
    b1 -= a11 * e1 + a12 * e2;
    b2 -= a21 * e1 + a22 * e2;
    x0 += e1;
    y0 += e2;
  }
  while (e1 != 0.0 && e2 != 0.0);

  *t = (double) x0;
  return SKEW;
}

Intersect cross(double x1, double y1, double x2, double y2, \
  double x3, double y3, double x4, double y4, double *s, double* t)
{
  Intersect g = intersect(x3, y3, x4, y4, x1, y1, x2, y2, t);
  if ((g == COLLINEAR) || (g == PARALLEL))
  {
    return g;
  }
  if ((*t < 0.0) || (1.0 < *t))
  {
    return SKEW_NO_CROSS;
  }
  intersect(x1, y1, x2, y2, x3, y3, x4, y4, s);
  if ((*s < 0.0) || (1.0 < *s))
  {
    return SKEW_NO_CROSS;
  }
  return SKEW_CROSS;
}

Raycast raycast(const coord_type& p, const coord_type& q, double x, double y)
{
  switch (classify(p.xcoord, p.ycoord, q.xcoord, q.ycoord, x, y))
  {
  case LEFT:
    return (p.ycoord < y && y <= q.ycoord? CROSSING: INESSENTIAL);
  case RIGHT:
    return (q.ycoord < y && y <= p.ycoord? CROSSING: INESSENTIAL);
  case BETWEEN:
  case ORIGIN:
  case DESTINATION:
    return TOUCHING;
  }
  return INESSENTIAL;
}

Inpoly inside_curve(double x, double y, const vertex_type& ver, const curve_type& cur)
{
  bool parity = false;
  curve_type::const_iterator prev = cur.begin();
  for (curve_type::const_iterator curr = prev; ++curr != cur.end(); prev = curr)
  {
    switch (raycast(ver[*prev], ver[*curr], x, y))
    {
    case TOUCHING:
      return BOUNDARY;
    case CROSSING:
      parity = !parity;
    }
  }
  return (parity? INSIDE: OUTSIDE);
}

Inpoly inside_edge(double x, double y, const vertex_type& ver, const edge_type& pgn)
{
  bool parity = false;
  for (edge_type::const_iterator e = pgn.begin(); e != pgn.end(); ++e)
  {
    curve_type::const_iterator prev = e->begin();
    for (curve_type::const_iterator curr = prev; ++curr != e->end(); prev = curr)
    {
      switch (raycast(ver[*prev], ver[*curr], x, y))
      {
      case TOUCHING:
        return BOUNDARY;
      case CROSSING:
        parity = !parity;
      }
    }
  }
  return (parity? INSIDE: OUTSIDE);
}

bool inside_convex(double x, double y, const vertex_type& ver, const curve_type& cur)
{
  if (cur.empty())
  {
    return false;
  }
  if (cur.size() == 1)
  {
    const coord_type* p = &ver[cur.front()];
    return (x == p->xcoord && y == p->ycoord);
  }
  if (cur.size() == 2)
  {
    const coord_type* p = &ver[cur.front()];
    const coord_type* q = &ver[cur.back()];
    Classify c = classify(p->xcoord, p->ycoord, q->xcoord, q->ycoord, x, y);
    return (c == BETWEEN || c == ORIGIN || c == DESTINATION);
  }
  curve_type::const_iterator p = cur.begin();
  for (curve_type::const_iterator q = p; ++q != cur.end(); p = q)
  {
    if (RIGHT == classify(ver[*p].xcoord, ver[*p].ycoord, ver[*q].xcoord, ver[*q].ycoord, x, y))
      return false;
  }
  return true;
}

bool iscrossed(double bbox[4], const vertex_type& ver, const curve_type& cur)
{
  for (curve_type::const_iterator c = cur.begin(); c != cur.end(); ++c)
  {
    const coord_type* p = &ver[*c];
    if (inside_bbox(p->xcoord, p->ycoord, bbox))
      return true;
  }
  return false;
}

void clip_convex(const vertex_type& ver_convex, const curve_type& cur_convex, \
  const vertex_type& ver, const edge_type& pgn, vertex_type& ver_clipped, edge_type& pgn_clipped)
{
  ver_clipped = ver;
  pgn_clipped = pgn;
  assert(is_convex(ver_convex, cur_convex));
  assert(is_generalized_polygon(ver, pgn));
  curve_type::const_iterator iter = cur_convex.begin();
  const coord_type* prev = &ver_convex[*iter];
  while (++iter != cur_convex.end())
  {
    if (pgn_clipped.empty())
      break;

    const coord_type* curr = &ver_convex[*iter];
    double a = (curr->ycoord - prev->ycoord);
    double b = (prev->xcoord - curr->xcoord);
    double c = hypot(a, b);
    assert(c > 0.0);
    a /= c;
    b /= c;
    c = -a * prev->xcoord - b * prev->ycoord;

    edge_type pos, neg;
    sutherland_hodgman(a, b, c, ver_clipped, pgn_clipped, pos, neg);

    assert(is_generalized_polygon(ver_clipped, pos));
    assert(is_generalized_polygon(ver_clipped, neg));

    pgn_clipped = neg;
    prev = curr;
  }
}

class gs_lexord // lexographical ordering
{
  const coord_type* ver;
public:
  gs_lexord(const coord_type* v): ver(v)
  {
  }
  bool operator()(int n1, int n2) const
  {
    const coord_type *p = &ver[n1], *q = &ver[n2];
    return (p->ycoord < q->ycoord || p->ycoord == q->ycoord && p->xcoord < q->xcoord);
  }
};

class gs_polord // polar ordering
{
  const coord_type* ver;
  const coord_type* org;
public:
  gs_polord(const coord_type* v, const coord_type* o): ver(v), org(o)
  {
  }
  bool operator()(int n1, int n2) const;
};

bool gs_polord::operator()(int n1, int n2) const
{
  const coord_type* p1 = &ver[n1];
  const coord_type* p2 = &ver[n2];
  long double x1 = p1->xcoord - org->xcoord;
  long double y1 = p1->ycoord - org->ycoord;
  long double x2 = p2->xcoord - org->xcoord;
  long double y2 = p2->ycoord - org->ycoord;
  long double a1 = atan2l(y1, x1); if (a1 < 0.0) a1 += 2.0 * M_PI;
  long double a2 = atan2l(y2, x2); if (a2 < 0.0) a2 += 2.0 * M_PI;
  return (a1 < a2 || a1 == a2 && hypotl(x1, y1) < hypotl(x2, y2));
}

#if 0

static void plot_polar_ordering(const coord_type* ver, int n, const int* data)
{
  int i;

  double x1 = std::numeric_limits<double>::max(), x2 = -x1, y1 = x1, y2 = x2;
  for (i = 0; i < n; ++i)
  {
    int s = data[i];
    double x = ver[s].xcoord;
    double y = ver[s].ycoord;
    if (x < x1) x1 = x;
    if (x > x2) x2 = x;
    if (y < y1) y1 = y;
    if (y > y2) y2 = y;
  }

  psstream out("polar.ps");
  out.selectmedia(psstream::FORMAT_A3, psstream::PORTRAIT);
  out.newpage();

  double x0, y0, sx, sy;
  double dx = x2 - x1, dy = y2 - y1;
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
  sx /= dx;
  sy /= dy;
  x0 -= x1 * sx;
  y0 -= y1 * sy;

  for (i = 0; i < n; ++i)
  {
    int s = data[i];
    double x = x0 + sx * ver[s].xcoord;
    double y = y0 + sy * ver[s].ycoord;
    out.arc(x, y, 3, 0, 360);
    out.fill();
  }

  for (i = 1; i < n; ++i)
  {
    out.newpage();
    int s = data[0];
    double x1 = x0 + sx * ver[s].xcoord;
    double y1 = y0 + sy * ver[s].ycoord;
    out.moveto(x1, y1);
    int t = data[i];
    double x2 = x0 + sx * ver[t].xcoord;
    double y2 = y0 + sy * ver[t].ycoord;
    out.lineto(x2, y2);
    out.stroke();
  }
}

#endif

// PRECONDITION: ver must be lexicographically ordered
void graham_scan(const vertex_type& ver, const curve_type& cur, curve_type& hull)
{
	assert(is_lexord(ver));
  hull.clear();
  if (!cur.empty())
  {
    if (cur.size() == 1)
    {
      hull.push_back(cur.front());
    }
    else if (cur.size() == 2)
    {
      hull.push_back(cur.front());
      if (cur.front() != cur.back())
      {
        hull.push_back(cur.back());
        hull.push_back(cur.front());
      }
    }
    else
    {
      typedef std::vector<int> int_vector;
      int_vector dict(cur.begin(), cur.end());
      // int_vector dict(cur.size());
      // std::copy(cur.begin(), cur.end(), dict.begin());
      std::swap(dict.front(), *std::min_element(dict.begin(), dict.end()));
      std::sort(++dict.begin(), dict.end(), gs_polord(&ver.at(0), &ver.at(dict[0])));

      // plot_polar_ordering(&ver[0], dict.size(), &dict[0]);

      size_t i = 1; 
      const coord_type* o = &ver[dict.at(0)];
      const coord_type* p = &ver[dict.at(i)];
      while (i+1 < dict.size())
      {
        const coord_type* q = &ver[dict.at(i+1)];
        if (classify(o->xcoord, o->ycoord, p->xcoord, p->ycoord, q->xcoord, q->ycoord) != BEYOND)
        {
          break;
        }
        p = q;
        ++i;
      }

      hull.push_front(dict[0]);
      hull.push_front(dict[i]);
      while (++i < dict.size())
      {
        int top = hull.front(); 
        hull.pop_front();
        int next_to_top = hull.front();
        int curr = dict[i];
        o = &ver[next_to_top];
        p = &ver[top];
        const coord_type* q = &ver[curr];
        while (classify(o->xcoord, o->ycoord, p->xcoord, p->ycoord, q->xcoord, q->ycoord) != LEFT)
        {
          top = next_to_top; 
          hull.pop_front();
          if (hull.empty()) break;
          next_to_top = hull.front();
          p = o;
          o = &ver[next_to_top];
        }
        hull.push_front(top);
        hull.push_front(curr);
      }
      hull.push_front(hull.back());
    }
  }

  // assert(is_convex(ver, hull));
  if (!is_simple(ver, hull))
  {
    plot_curve(ver, cur);
    assert(false);
    exit(1);
  }
}

/*
double* curve_bbox(const vertex_type& ver, const curve_type& cur, double bb[4])
{
  curve_type::const_iterator c = cur.begin();
  const coord_type* p = &ver[*c];
  bb[0] = bb[2] = p->xcoord;
  bb[1] = bb[3] = p->ycoord;
  while (++c != cur.end())
  {
    p = &ver[*c];
    if (bb[0] > p->xcoord) bb[0] = p->xcoord;
    if (bb[2] < p->xcoord) bb[2] = p->xcoord;
    if (bb[1] > p->ycoord) bb[1] = p->ycoord;
    if (bb[3] < p->ycoord) bb[3] = p->ycoord;
  }
  return bb;
}
*/

void make_shape_list(const vertex_type& ver, const edge_type& pgn, shape_type* init)
{
  init->shape.clear();
  init->hull.clear();
  bounding_box::max(init->bbox);
  for (edge_type::const_iterator e = pgn.begin(); e != pgn.end(); ++e)
  {
    const curve_type& item = *e;

    init->shape.push_back(shape_curve());
    shape_curve* shape = &init->shape.back();
    shape->curve = &item;
    bounding_box::max(shape->bbox);
  }
}

void bbox_shape_list(const vertex_type& ver, shape_type* init)
{
  bounding_box::max(init->bbox);
  for (shape_list::iterator shape = init->shape.begin(); shape != init->shape.end(); ++shape)
  {
    curve_bbox(ver, *shape->curve, shape->bbox);
    if (shape->bbox[0] < init->bbox[0]) init->bbox[0] = shape->bbox[0];
    if (shape->bbox[1] < init->bbox[1]) init->bbox[1] = shape->bbox[1];
    if (shape->bbox[2] > init->bbox[2]) init->bbox[2] = shape->bbox[2];
    if (shape->bbox[3] > init->bbox[3]) init->bbox[3] = shape->bbox[3];
  }
}

void hull_shape_list(const vertex_type& ver, shape_type* init)
{
  curve_type temp;
  for (shape_list::iterator shape = init->shape.begin(); shape != init->shape.end(); ++shape)
  {
    graham_scan(ver, *shape->curve, shape->hull);
    std::copy(shape->hull.begin(), shape->hull.end(), std::back_inserter(temp));
  }
  graham_scan(ver, temp, init->hull);
}

Inpoly inside_shape(double x, double y, const shape_type& pgn)
{
  bool parity = false;
  for (shape_list::const_iterator s = pgn.shape.begin(); s != pgn.shape.end(); ++s)
  {
    if (inside_bbox(x, y, s->bbox))
    {
      Inpoly e = BOUNDARY; // = inside_curve(x, y, *pgn.ver, *s->curve);
      if (e == BOUNDARY)
      {
        return BOUNDARY;
      }
      if (e == INSIDE)
      {
        parity = !parity;
      }
    }
  }
  return (parity? INSIDE: OUTSIDE);
}

// PRECONDITION: cur defines convex polygon
void min_rect(const vertex_type& ver, const curve_type& cur, double rect[8])
{
  curve_type::const_iterator prev = cur.begin();
  bool has_area = false;
  double area;
  for (curve_type::const_iterator curr = prev; ++curr != cur.end(); prev = curr)
  {
    double h1 = 0.0;
    double h2 = 0.0;
    double w1 = 0.0;
    double w2 = 0.0;
    const coord_type* p = &ver.at(*curr);
    const coord_type* q = &ver.at(*prev);
    double ax = q->xcoord - p->xcoord;
    double ay = q->ycoord - p->ycoord;
    double aa = hypot(ax, ay);
    ax /= aa;
    ay /= aa;

    for (curve_type::const_iterator next = cur.begin(); next != cur.end(); ++next)
    {
      const coord_type* r = &ver.at(*next);
      double bx = r->xcoord - p->xcoord;
      double by = r->ycoord - p->ycoord;

      double hh = -(ax * by - ay * bx);
      double ww = ax * bx + ay * by;
      if (h1 > hh) h1 = hh;
      if (h2 < hh) h2 = hh;
      if (w1 > ww) w1 = ww;
      if (w2 < ww) w2 = ww;
    }

    double a = (h2 - h1) * (w2 - w1);
    if (!has_area || a < area)
    {
      has_area = true;
      area = a;
      rect[0] = p->xcoord + w1 * ax + h1 * ay;
      rect[1] = p->ycoord + w1 * ay - h1 * ax;
      rect[2] = p->xcoord + w2 * ax + h1 * ay;
      rect[3] = p->ycoord + w2 * ay - h1 * ax;
      rect[4] = p->xcoord + w2 * ax + h2 * ay;
      rect[5] = p->ycoord + w2 * ay - h2 * ax;
      rect[6] = p->xcoord + w1 * ax + h2 * ay;
      rect[7] = p->ycoord + w1 * ay - h2 * ax;
    }
  }
}

class lexord_cmp // lexicographical ordering
{
  const coord_type* ver;
public:
  lexord_cmp(const coord_type* v): ver(v) { }
  bool operator()(int n1, int n2) const
  {
    const coord_type *p = &ver[n1], *q = &ver[n2];
    return (p->ycoord < q->ycoord || p->ycoord == q->ycoord && p->xcoord < q->xcoord);
  }
};

// NOTES: 
// 1) on input data array consist of element list which will be included in dictionary
// 2) size of data array must be at least ver.size() elements
void lexord(const vertex_type& ver, int n, int* data, vertex_type& dict)
{
  typedef std::vector<int> int_vector;
  lexord_cmp cmp(&ver[0]);
  int_vector temp(data, data+n);
  std::sort(temp.begin(), temp.end());
  std::unique(temp.begin(), temp.end());
  n = temp.size();
  std::sort(temp.begin(), temp.end(), cmp);
  if (!temp.empty())
  {
    dict.clear();
    dict.reserve(n);
    int_vector::iterator curr = temp.begin();
    data[*curr] = dict.size();
    for (int_vector::iterator prev = curr; ++curr != temp.end(); prev = curr)
    {
      if (cmp(*prev, *curr))
      {
        dict.push_back(ver[*prev]);
      }
      data[*curr] = dict.size();
    }
    dict.push_back(ver[*prev]);
  }
}

bool coord_cmp(const coord_type& p, const coord_type& q)
{
  return (p.ycoord < q.ycoord || p.ycoord == q.ycoord && p.xcoord < q.xcoord);
}

bool is_lexord(const vertex_type& ver)
{
  for (vertex_type::const_iterator curr = ver.begin(), prev = curr; ++curr != ver.end(); prev = curr)
  {
    if (!coord_cmp(*prev, *curr))
    {
      return false;
    }
  }
  return true;
}

int find(const vertex_type& ver, const coord_type& p)
{
  int k = std::lower_bound(ver.begin(), ver.end(), p, coord_cmp) - ver.begin();
  if (k < ver.size() && ver[k].xcoord == p.xcoord && ver[k].ycoord == p.ycoord)
  {
    return k;
  }
  return -1;
}

void clone(const vertex_type& in, const curve_type& src, int *xlat, \
  vertex_type& ver, edge_type& pgn)
{
  ver.clear();
  pgn.clear();
  pgn.push_back(curve_type());
  curve_type* dst = &pgn.back();
  for (curve_type::const_iterator s1 = src.begin(); s1 != src.end(); ++s1)
  {
    xlat[*s1] = -1;
  }
  for (curve_type::const_iterator s2 = src.begin(); s2 != src.end(); ++s2)
  {
    if (xlat[*s2] == -1)
    {
      xlat[*s2] = ver.size();
      ver.push_back(in[*s2]);
    }
    dst->push_back(xlat[*s2]);
  }
}

void boxedge(const double *bbox, vertex_type& ver, edge_type& pgn)
{
  ver.clear();
  pgn.clear();
  pgn.push_back(curve_type());
  curve_type* dst = &pgn.back();
  coord_type p;
  p.xcoord = bbox[0];
  p.ycoord = bbox[1];
  ver.push_back(p);
  dst->push_back(0);
  p.xcoord = bbox[2];
  p.ycoord = bbox[1];
  ver.push_back(p);
  dst->push_back(1);
  p.xcoord = bbox[2];
  p.ycoord = bbox[3];
  ver.push_back(p);
  dst->push_back(2);
  p.xcoord = bbox[0];
  p.ycoord = bbox[3];
  ver.push_back(p);
  dst->push_back(3);
  dst->push_back(0);
}

