#define _USE_MATH_DEFINES
#ifndef __INTEL_COMPILER
  #include <math.h>
#else
  #include <mathimf.h>
#endif

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#include "ps_data.h"
#include "ps_priv.h"

inline int sign(double x)
{
  if (x > 0.0)
    return 1;
  if (x < 0.0)
    return -1;
  return 0;
}

static void intersect(double a, double b, double c, \
  const coord_type* s, const coord_type* t, coord_type* p)
{
  long double a11 = a;
  long double a12 = b;
  long double b1 = -c;

  long double a21 = t->ycoord - s->ycoord;
  long double a22 = s->xcoord - t->xcoord;
  long double b2 = a21 * s->xcoord + a22 * s->ycoord;
  
  long double d = a11 * a22 - a12 * a21;

  long double x1 = (b1 * a22 - b2 * a12) / d;
  long double x2 = (a11 * b2 - a21 * b1) / d;

#if 1
  // ref: Numerical Methods and Fortran Programming, D.McCracken, W.Dorn, pp.305-308
  b1 -= a11 * x1 + a12 * x2;
  b2 -= a21 * x1 + a22 * x2;

  int max_iter = 10; 
  double eps = (long double) std::numeric_limits<double>::epsilon();
  while (std::max(fabs(b1), fabs(b2)) > eps)
  {
    if (--max_iter == 0)
      break;

    long double e1 = (b1 * a22 - b2 * a12) / d;
    long double e2 = (a11 * b2 - a21 * b1) / d;

    b1 -= a11 * e1 + a12 * e2;
    b2 -= a21 * e1 + a22 * e2;

    x1 += e1;
    x2 += e2;
  }
#endif

  p->xcoord = (double) x1;
  p->ycoord = (double) x2;
}

/*
static int ray_cast(double x, double y, const vertex_type& ver, const curve_type& cc)
{
  int n = 0;
  for (curve_type::const_iterator prev = cc.begin(), curr = prev; ++curr != cc.end(); prev = curr)
  {
    const coord_type *s = &ver[*prev], *t = &ver[*curr];
    if (s->ycoord == t->ycoord || s->ycoord > y && t->ycoord > y || s->ycoord < y && t->ycoord < y)
      continue;
    if (s->ycoord > t->ycoord && s->ycoord == y)
    {
      if (s->xcoord <= x)
        ++n;
    }
    else if (std::min(s->ycoord, t->ycoord) != y)
    {
      double h = (y - s->ycoord) / (t->ycoord - s->ycoord);
      assert(0.0 < h && h <= 1.0);
      if (s->xcoord + h * (t->xcoord - s->xcoord) <= x)
        ++n;
    }
  }
  return n;
}
*/

/*
bool inside_curve(double x, double y, const vertex_type& ver, const curve_type& cc)
{
  return ((ray_cast(x, y, ver, cc) & 1) != 0);
}
*/

bool inside_bbox(double x, double y, const double *bb)
{
  return (bb[0] <= x && x <= bb[2] && bb[1] <= y && y <= bb[3]);
}

/*
bool inside_shape(double x, double y, const vertex_type& ver, const shape_type& ss)
{
  int num_cross = 0;
  for (shape_type::const_iterator s = ss.begin(); s != ss.end(); ++s)
  {
    if (inside_bbox(x, y, s->bbox))
      num_cross += ray_cast(x, y, ver, *s->curve);
  }
  return ((num_cross & 1) != 0);
}
*/

/*
bool inside_edge(double x, double y, const vertex_type& ver, const edge_type& ee)
{
  int num_cross = 0;
  for (edge_type::const_iterator e = ee.begin(); e != ee.end(); ++e)
  {
    num_cross += ray_cast(x, y, ver, *e);
  }
  return ((num_cross & 1) != 0);
}
*/

/*
void edge_shape(const vertex_type& ver, const edge_type& ee, shape_type& ss)
{
  for (edge_type::const_iterator e = ee.begin(); e != ee.end(); ++e)
  {
    const curve_type& cc = *e;
    ss.push_back(shape_curve());
    shape_curve* g = &ss.back();
    g->curve = &cc;
    double* bb = g->bbox;
    bb[1] = bb[0] = std::numeric_limits<double>::max();
    bb[3] = bb[2] = -bb[0];
    for (curve_type::const_iterator c = ++cc.begin(); c != cc.end(); ++c)
    {
      const coord_type* p = &ver[*c];
      if (bb[0] > p->xcoord) bb[0] = p->xcoord;
      if (bb[2] < p->xcoord) bb[2] = p->xcoord;
      if (bb[1] > p->ycoord) bb[1] = p->ycoord;
      if (bb[3] < p->ycoord) bb[3] = p->ycoord;
    }
  }
}
*/

class vertex_compare
{
  const coord_type* point;
public:
  vertex_compare(const coord_type* p): point(p)
  {
  }
  bool operator() (int i, int j) const
  {
    return (point[i].xcoord == point[j].xcoord && point[i].ycoord == point[j].ycoord);
  }
};

// remove possible duplicate points from contour chains
void merge_nodes(const vertex_type& ver, edge_type& pgn)
{
  for (edge_type::iterator e = pgn.begin(); e != pgn.end(); ++e)
  {
    e->unique(vertex_compare(&ver[0]));
  }
}

#if 1

struct Zero_type
{
  int kind;
  int start;
  int final;
  bool enter;
};

typedef std::list<Zero_type> Zero_list;
typedef std::vector<Zero_type*> Zero_ptr_vector;

class SH_zero_compare
{
  const coord_type* vertex;
  const coord_type* origin;
  double nx, ny;

public:
  
  SH_zero_compare(const coord_type* p, Zero_type* z, double a, double b):
    vertex(p), origin(&vertex[z->kind != 0 && !z->enter? z->final: z->start]), nx(a), ny(b)
  {
  }
  
  const coord_type* zero(const Zero_type* z) const
  {
    const coord_type* p = &vertex[z->kind != 0 && !z->enter? z->final: z->start];
#ifdef _DEBUG
    double dx = p->xcoord - origin->xcoord;
    double dy = p->ycoord - origin->ycoord;
    double dp = nx * dx + ny * dy;
    assert(fabs(dp) < 1.0e-7);
#endif
    return p;
  }

  double distance(const Zero_type* z) const
  {
    const coord_type* point = zero(z);
    double dx = point->xcoord - origin->xcoord;
    double dy = point->ycoord - origin->ycoord;
    return (nx * dy - ny * dx);
  }

  double slope(const Zero_type* z) const
  {
    const coord_type* prev = &vertex[z->start];
    const coord_type* next = &vertex[z->final];
    double dx = next->xcoord - prev->xcoord;
    double dy = next->ycoord - prev->ycoord;
    double dd = hypot(dx, dy);
    assert(dd > 0.0);

    dx /= dd;
    dy /= dd;

    if (z->kind != 0 && z->enter == false)
    {
      dx = -dx;
      dy = -dy;
    }

    return (nx * dy - ny * dx);
  }

  bool operator() (const Zero_type* z1, const Zero_type* z2) const
  {
    double d1 = distance(z1), d2 = distance(z2);
    if (d1 == d2)
    {
      double s1 = slope(z1), s2 = slope(z2);
      assert(s1 != s2);
      return (s1 < s2);
    }
    return d1 < d2;
  }
};

void sutherland_hodgman(double a, double b, double c, vertex_type& ver, \
  const edge_type& orig, edge_type& pos, edge_type& neg)
{
  Zero_list pos_border;
  Zero_list neg_border;

  pos.clear();
  neg.clear();
  for (edge_type::const_iterator e = orig.begin(); e != orig.end(); ++e)
  {
    assert(is_closed(*e));
    curve_type* negative = 0;
    curve_type* positive = 0;

    curve_type::const_iterator iter_prev = e->begin();
    coord_type* node_prev = &ver[*iter_prev];
    int sign_prev = sign(a * node_prev->xcoord + b * node_prev->ycoord + c);

    curve_type::const_iterator iter_curr = iter_prev; 
    while (++iter_curr != e->end())
    {
      coord_type* node_curr = &ver[*iter_curr];
      int sign_curr = sign(a * node_curr->xcoord + b * node_curr->ycoord + c);

      if (sign_prev < 0 && sign_curr < 0)
      {
        if (0 == negative)
        {
          neg.push_back(curve_type());
          negative = &neg.back();
          negative->push_back(*iter_prev);
        }
        negative->push_back(*iter_curr);
      }
      else if (sign_prev > 0 && sign_curr > 0)
      {
        if (0 == positive)
        {
          pos.push_back(curve_type());
          positive = &pos.back();
          positive->push_back(*iter_prev);
        }
        positive->push_back(*iter_curr);
      }
      else if (sign_prev < 0 && sign_curr > 0)
      {
        int n = ver.size();
        ver.push_back(coord_type());
        coord_type* node = &ver.back();
        intersect(a, b, c, &ver[*iter_prev], &ver[*iter_curr], node);

        if (0 == negative)
        {
          neg.push_back(curve_type());
          negative = &neg.back();
          negative->push_back(*iter_prev);
        }
        negative->push_back(n);
        negative = 0;

        neg_border.push_back(Zero_type());
        Zero_type* zero = &neg_border.back();
        zero->kind = -1;
        zero->start = *iter_prev;
        zero->final = n;
        zero->enter = false;

        pos_border.push_back(Zero_type());
        zero = &pos_border.back();
        zero->kind = 1;
        zero->start = n;
        zero->final = *iter_curr;
        zero->enter = true;

        pos.push_back(curve_type());
        positive = &pos.back();
        positive->push_back(n);
        positive->push_back(*iter_curr);        
      }
      else if (sign_prev > 0 && sign_curr < 0)
      {
        int n = ver.size();
        ver.push_back(coord_type());
        coord_type* node = &ver.back();
        intersect(a, b, c, &ver[*iter_prev], &ver[*iter_curr], node);

        if (0 == positive)
        {
          pos.push_back(curve_type());
          positive = &pos.back();
          positive->push_back(*iter_prev);
        }
        positive->push_back(n);
        positive = 0;

        pos_border.push_back(Zero_type());
        Zero_type* zero = &pos_border.back();
        zero->kind = 1;
        zero->start = *iter_prev;
        zero->final = n;
        zero->enter = false;

        neg_border.push_back(Zero_type());
        zero = &neg_border.back();
        zero->kind = -1;
        zero->start = n;
        zero->final = *iter_curr;
        zero->enter = true;

        neg.push_back(curve_type());
        negative = &neg.back();
        negative->push_back(n);
        negative->push_back(*iter_curr);        
      }
      else if (sign_prev == 0 && sign_curr > 0)
      {
        if (0 == positive)
        {
          pos.push_back(curve_type());
          positive = &pos.back();
          positive->push_back(*iter_prev);
        }
        positive->push_back(*iter_curr);

        pos_border.push_back(Zero_type());
        Zero_type* zero = &pos_border.back();
        zero->kind = 1;
        zero->start = *iter_prev;
        zero->final = *iter_curr;
        zero->enter = true;
      }
      else if (sign_prev > 0 && sign_curr == 0)
      {
        positive->push_back(*iter_prev);
        positive->push_back(*iter_curr);
        positive = 0;

        pos_border.push_back(Zero_type());
        Zero_type* zero = &pos_border.back();
        zero->kind = 1;
        zero->start = *iter_prev;
        zero->final = *iter_curr;
        zero->enter = false;
      }
      else if (sign_prev == 0 && sign_curr < 0)
      {
        if (0 == negative)
        {
          neg.push_back(curve_type());
          negative = &neg.back();
          negative->push_back(*iter_prev);
        }
        negative->push_back(*iter_curr);

        neg_border.push_back(Zero_type());
        Zero_type* zero = &neg_border.back();
        zero->kind = -1;
        zero->start = *iter_prev;
        zero->final = *iter_curr;
        zero->enter = true;
      }
      else if (sign_prev < 0 && sign_curr == 0)
      {
        negative->push_back(*iter_prev);
        negative->push_back(*iter_curr);
        negative = 0;

        neg_border.push_back(Zero_type());
        Zero_type* zero = &neg_border.back();
        zero->kind = -1;
        zero->start = *iter_prev;
        zero->final = *iter_curr;
        zero->enter = false;
      }
      else
      {
        coord_type* source = &ver[*iter_prev];
        coord_type* target = &ver[*iter_curr];
        double dx = target->xcoord - source->xcoord;
        double dy = target->ycoord - source->ycoord;
#ifdef _DEBUG
        double dp = a * dx + b * dy;
        assert(fabs(dp) < 1.0e-7);
#endif
        // true if segment has inorder direction
        bool inorder = (a * dy > b * dx);

        Zero_type* zero = 0;
        if (inorder)
        {
          pos_border.push_back(Zero_type());
          zero = &pos_border.back();
        }
        else
        {
          neg_border.push_back(Zero_type());
          zero = &neg_border.back();
        }
        zero->kind = 0;
        zero->start = *iter_prev;
        zero->final = *iter_curr;
        zero->enter = inorder;
      }

      sign_prev = sign_curr;
      iter_prev = iter_curr;
    }
  }
  
  if (!pos_border.empty())
  {
    Zero_ptr_vector border(pos_border.size());
    Zero_ptr_vector::iterator iter_order = border.begin();
    for (Zero_list::iterator iter_store = pos_border.begin(); \
      iter_store != pos_border.end(); ++iter_store)
    {
      Zero_type& zero = *iter_store;
      *iter_order++ = &zero;
    }

    std::sort(border.begin(), border.end(), SH_zero_compare(&ver[0], border.front(), -a, -b));

#if 0
    for (iter_order = border.begin(); iter_order != border.end(); ++iter_order)
    {
      Zero_type* zero = *iter_order;
      coord_type* p = &ver[zero->start];
      coord_type* q = &ver[zero->final];
      std::cout << zero->kind << ' ' << std::boolalpha << zero->enter << ' '
        << p->xcoord << ' ' << p->ycoord <<  ' ' << q->xcoord << ' ' << q->ycoord << '\n';
    }
    std::cout << '\n';
#endif

    for (iter_order = border.begin(); iter_order != border.end(); ++iter_order)
    {
      Zero_type* zero = *iter_order;
      assert(zero->kind == 1 && zero->enter == false);
      pos.push_back(curve_type());
      curve_type *positive = &pos.back();
      positive->push_back(zero->final);
      while (++iter_order != border.end())
      {
        // must be the same directin as guide line
        zero = *iter_order;
        if (zero->kind == 0)
        {
          assert(zero->enter == true);
          positive->push_back(zero->final);
        }
        else
        {
          // must be entering positive segment
          assert(zero->kind == 1 && zero->enter == true);
          positive->push_back(zero->start);
          break;
        }
      }
      if (positive->size() == 2 && positive->front() == positive->back())
      {
        neg.pop_back();
      }
      assert(iter_order != border.end());
    }
  }
  
  if (!neg_border.empty())
  {
    Zero_ptr_vector border(neg_border.size());
    Zero_ptr_vector::iterator iter_order = border.begin();
    for (Zero_list::iterator iter_store = neg_border.begin(); \
      iter_store != neg_border.end(); ++iter_store)
    {
      Zero_type& zero = *iter_store;
      *iter_order++ = &zero;
    }

    std::sort(border.begin(), border.end(), SH_zero_compare(&ver[0], border.front(), a, b));

#if 0
    for (iter_order = border.begin(); iter_order != border.end(); ++iter_order)
    {
      Zero_type* zero = *iter_order;
      coord_type* p = &ver[zero->start];
      coord_type* q = &ver[zero->final];
      std::cout << zero->kind << ' ' << std::boolalpha << zero->enter << ' '
        << p->xcoord << ' ' << p->ycoord <<  ' ' << q->xcoord << ' ' << q->ycoord << '\n';
    }
    std::cout << '\n';
#endif

    for (iter_order = border.begin(); iter_order != border.end(); ++iter_order)
    {
      Zero_type* zero = *iter_order;
      assert(zero->kind == -1 && zero->enter == false);
      neg.push_back(curve_type());
      curve_type* negative = &neg.back();
      negative->push_back(zero->final);
      while (++iter_order != border.end())
      {
        // must be the same directin as guide line
        zero = *iter_order;
        if (zero->kind == 0)
        {
          assert(zero->enter == true);
          negative->push_back(zero->final);
        }
        else
        {
          // must be entering positive segment
          assert(zero->kind == -1 && zero->enter == true);
          negative->push_back(zero->start);
          break;
        }
      }
      if (negative->size() == 2 && negative->front() == negative->back())
      {
        neg.pop_back();
      }
      assert(iter_order != border.end());
    }
  }

  merge_curves(pos);
  merge_curves(neg);

  merge_nodes(ver, pos);
  merge_nodes(ver, neg);
}

#else

class sh_border_compare
{
  double nx, ny;
  const vertex_type& w;
public:
  sh_border_compare(double a, double b, const vertex_type& ver): nx(a), ny(b), w(ver) 
  {
  }
  bool operator() (int n1, int n2) const
  {
    const coord_type* p1 = &w[n1];
    const coord_type* p2 = &w[n2];
    double dx = p2->xcoord - p1->xcoord;
    double dy = p2->ycoord - p1->ycoord;
    double s = nx * dy - ny * dx;
    return (s < 0);
  }
};

void sutherland_hodgman(double a, double b, double c, vertex_type& ver, \
  const edge_type& org, edge_type& pos, edge_type& neg)
{
  typedef std::vector<int> border_type;
  border_type border;
  shape_type pgn;
  make_shape_list(ver, org, &pgn);
  bbox_shape_list(ver, &pgn);
  pos.clear();
  neg.clear();

  for (shape_list::iterator sh = pgn.shape.begin(); sh != pgn.shape.end(); ++sh)
  {
    const curve_type& curve = *sh->curve;
    // assert(curve.front() == curve.back());
    {
      double x1 = sh->bbox[0], x2 = sh->bbox[2];
      double y1 = sh->bbox[1], y2 = sh->bbox[3];
      double s1 = sign(a * x1 + b * y1 + c);
      double s2 = sign(a * x2 + b * y1 + c);
      double s3 = sign(a * x2 + b * y2 + c);
      double s4 = sign(a * x1 + b * y1 + c);
      if (s1 <= 0.0 && s2 <= 0.0 && s3 <= 0.0 && s4 <= 0.0)
      {
        neg.push_back(curve);
        continue;
      }
      if (s1 >= 0.0 && s2 >= 0.0 && s3 >= 0.0 && s4 >= 0.0)
      {
        pos.push_back(curve);
        continue;
      }
    }

    curve_type::const_iterator c_iter = ++curve.begin();
    const coord_type* p = &ver[*c_iter];
    int s = sign(a * p->xcoord + b * p->ycoord + c);
    while (s == 0)
    {
      ++c_iter;
      assert(c_iter != curve.end());
      p = &ver[*c_iter];
      s = sign(a * p->xcoord + b * p->ycoord + c);
    }
    
    curve_type *p_curve = 0, *n_curve = 0;
    if (s > 0)
    {
      pos.push_back(curve_type());
      p_curve = &pos.back();
    }
    else
    {
      neg.push_back(curve_type());
      n_curve = &neg.back();
    }

    curve_type::const_iterator c_circ = c_iter;
    do
    {
      int prev = *c_circ;
      if (++c_circ == curve.end())
        c_circ = ++curve.begin();

      int curr = *c_circ;
      p = &ver[curr];
      int ss = sign(a * p->xcoord + b * p->ycoord + c);
      if (p_curve)
      {
        p_curve->push_back(prev);
        if (ss < 0)
        {
          if (s >= 0)
          {
            int n = ver.size();
            ver.push_back(coord_type());
            intersect(a, b, c, &ver[prev], &ver[curr], &ver[n]);
            p_curve->push_back(n);
            prev = n;
          }
          neg.push_back(curve_type());
          n_curve = &neg.back();
          n_curve->push_back(prev);
          border.push_back(prev);
          p_curve = 0;
        }
      }
      else
      {
        n_curve->push_back(prev);
        if (ss >= 0)
        {
          if (s < 0)
          {
            int n = ver.size();
            ver.push_back(coord_type());
            intersect(a, b, c, &ver[prev], &ver[curr], &ver[n]);
            n_curve->push_back(n);
            prev = n;
          }
          pos.push_back(curve_type());
          p_curve = &pos.back();
          p_curve->push_back(prev);
          border.push_back(prev);
          n_curve = 0;
        }
      }
      s = ss;
    }
    while (c_circ != c_iter);

    if (p_curve)
    {
      p_curve->push_back(*c_circ);
    }
    else
    {
      n_curve->push_back(*c_circ);
    }
  }

  std::stable_sort(border.begin(), border.end(), sh_border_compare(a, b, ver));
  {
    for (int i = 0; i < (int) border.size(); i += 2)
    {
      pos.push_back(curve_type());
      curve_type *curve = &pos.back();
      curve->push_back(border[i]);
      curve->push_back(border[i+1]);
      neg.push_back(curve_type());
      curve = &neg.back();
      curve->push_back(border[i+1]);
      curve->push_back(border[i]);
    }
  }

  // merge curves
  merge_curves(pos);
  merge_curves(neg);

  merge_nodes(ver, pos);
  merge_nodes(ver, neg);
}

#endif

////////////////////////////////////////////////////////////////////////////////

clip_polygon::clip_polygon(vertex_type& v, edge_type& e): ver(v), pgn(e)
{
}

int clip_polygon::new_vertex(double x, double y)
{
  int n = ver.size();
  ver.push_back(coord_type());
  coord_type* p = &ver.back();
  p->xcoord = x;
  p->ycoord = y;
  return n;
}

void clip_polygon::begin_curve()
{
  pgn.push_back(curve_type());
  cur = &pgn.back();
}

void clip_polygon::add_vertex(int n)
{
  cur->push_back(n);
}

void clip_polygon::end_curve()
{
  cur = 0;
}

struct cb_intersection
{
  int index;
  bool entering;
  double param;
};

inline bool cb_compare(cb_intersection* p1, cb_intersection* p2)
{
  return (p1->param < p2->param);
}

void cyrus_beck(double* bbox, const vertex_type& clip_ver, \
  const curve_type& clip_cur, vertex_type& ver, edge_type& ee)
{
  edge_type ff;
  edge_type::iterator e = ee.begin();
  while (e != ee.end())
  {
    curve_type& cc = *e;
    curve_type::iterator c = cc.begin();
    {
      double bb[4];
      coord_type* p = &ver[*c];
      bb[0] = bb[2] = p->xcoord;
      bb[1] = bb[3] = p->ycoord;
      while (++c != cc.end())
      {
        p = &ver[*c];
        double x = p->xcoord;
        double y = p->ycoord;
        if (bb[0] > x) bb[0] = x;
        if (bb[1] > y) bb[1] = y;
        if (bb[2] < x) bb[2] = x;
        if (bb[3] < y) bb[3] = y;
      }
      if (!rr_intersect(bbox, bb))
      {
        ++e;
        continue;
      }
      c = cc.begin();
    }

    coord_type* p1 = &ver[*c];
		double p1_xcoord = p1->xcoord;
		double p1_ycoord = p1->ycoord;
    int code1 = 0;
    if (p1_xcoord < bbox[0]) code1 |= 1;
    if (p1_ycoord < bbox[1]) code1 |= 2;
    if (p1_xcoord > bbox[2]) code1 |= 4;
    if (p1_ycoord > bbox[3]) code1 |= 8;
    bool has_cross = false;
    bool entering = false;
    while (++c != cc.end())
    {
      coord_type* p2 = &ver[*c];
			double p2_xcoord = p2->xcoord;
			double p2_ycoord = p2->ycoord;
      int code2 = 0;
      if (p2_xcoord < bbox[0]) code2 |= 1;
      if (p2_ycoord < bbox[1]) code2 |= 2;
      if (p2_xcoord > bbox[2]) code2 |= 4;
      if (p2_ycoord > bbox[3]) code2 |= 8;
      if ((code1 & code2) == 0)
      {
        // line segment has common points with bounding box
        // as in trivial case of Sutherland-Cohen algorithm
        curve_type::const_iterator d1 = clip_cur.begin(), d2 = d1;
        const coord_type* q1 = &clip_ver[*d1];
        double dx = p2_xcoord - p1_xcoord;
        double dy = p2_ycoord - p1_ycoord;
        typedef std::vector<cb_intersection*> cb_vector;
        cb_vector cross;
        while (++d2 != clip_cur.end())
        {
          const coord_type* q2 = &clip_ver[*d2];
          double sx = q2->xcoord - q1->xcoord;
          double sy = q2->ycoord - q1->ycoord;
          double ss = dx * sy - dy * sx;
          if (ss != 0.0)
          {
            double qx = p1_xcoord - q1->xcoord;
            double qy = p1_ycoord - q1->ycoord;
            double r = (qy * sx - qx * sy) / ss;
            double s = (qy * dx - qx * dy) / ss;
            if (0.0 <= r && r <= 1.0 && 0.0 <= s && s <= 1.0)
            {
              cb_intersection* g = new cb_intersection;
              g->index = ver.size();
              g->entering = (ss < 0.0);
              g->param = r;
              ver.push_back(coord_type());
              coord_type* p = &ver[g->index];
              p->xcoord = p1_xcoord + r * dx;
              p->ycoord = p1_ycoord + r * dy;
              cross.push_back(g);
            }
          }
          q1 = q2;
          d1 = d2;
        }
        if (!cross.empty())
        {
          std::sort(cross.begin(), cross.end(), cb_compare);
          for (cb_vector::iterator g = cross.begin(); g != cross.end(); ++g)
          {
            cb_intersection* s = *g;
            // int curr = s->index;
            if (s->entering)
            {
              curve_type& g = *ee.insert(e, curve_type());
              g.splice(g.end(), cc, cc.begin(), c);
              g.push_back(s->index);
              entering = true;
            }
            else
            {
              cc.erase(cc.begin(), c);
              cc.push_front(s->index);
              entering = false;
            }
            delete s;
          }
          has_cross = true;
        }
      }
      code1 = code2;
      p1 = p2;
			p1_xcoord = p2_xcoord;
			p1_ycoord = p2_ycoord;
    }

    if (has_cross)
    {
      if (entering)
      {
        e = ee.erase(e);
      }
      else
      {
        ++e;
      }
    } 
    else
    {
      if (INSIDE == inside_curve(p1->xcoord, p1->ycoord, clip_ver, clip_cur))
      // if ( 1 & ray_cast(p1->xcoord, p1->ycoord, clip_ver, clip_cur))
      {
        e = ee.erase(e);
      }
      else
      {
        ++e;
      }
    }
  }
}

void clippath(vertex_type& ver, int n, edge_type* ee, \
  const vertex_type& clip_ver, const edge_type& clip_pgn)
{
  shape_type clip;
  // edge_shape(clip_ver, clip_pgn, &clip);
  make_shape_list(clip_ver, clip_pgn, &clip);
  bbox_shape_list(clip_ver, &clip);
  for (int i = 0; i < n; ++i)
  {
    if (ee[i].empty())
      continue;

    for (shape_list::iterator s = clip.shape.begin(); s != clip.shape.end(); ++s)
    {
      cyrus_beck(s->bbox, clip_ver, *s->curve, ver, ee[i]);
    }
  }
}

#if 1

void auto_conrec(psstream& out, int i1, int i2, int j1, int j2, grid_type* g, \
  double x0, double y0, double sx, double sy)
{
  double minor_dz = 1.0;
  double minor_width = 0.0;
  double z1 = ceil(g->z1 / minor_dz) * minor_dz;
  double z2 = floor(g->z2 / minor_dz) * minor_dz;
  const int n = (int) ((z2 - z1) / minor_dz) + 1;
  float *level = new float[n];
  for (int k = 0; k < n; ++k)
    level[k] = float(z1+k*(z2-z1)/(n-1));

  int major_step = 10;
  double major_width = 1.0;
  double major_dz = major_step * minor_dz;
  int first_major = (int) ((ceil(z1 / major_dz) * major_dz - z1) / minor_dz);

  vertex_type ver;
  edge_type *me = new edge_type[n];
  edge_type *ce = new edge_type[n+2];
  marching_squares(g->cx, g->cy, i1, i2-1, j1, j2-1, g->zz, n, level, ver, me, ce);

  convert(x0-sx*i1/(i2-i1-1), y0-sy*j1/(j2-j1-1), sx/(i2-i1-1), sy/(j2-j1-1), ver);

  vertex_type clip_ver;
  edge_type clip_pgn;
  clip_polygon acc(clip_ver, clip_pgn);
  for (k = first_major; k < n; k += major_step)
  {
    char text[16];
    sprintf(text, "%.0f", level[k]);
    annotpath(out, ver, me[k], text, 200.0, major_width, &acc);
  }
  clippath(ver, n, me, clip_ver, clip_pgn);

  out.setlinewidth(minor_width);
  for (k = 0; k < first_major; ++k)
  {
    if (me[k].empty())
      continue;
    out.newpath();
    plotpath(out, ver, me[k]);
    if (minor_width > 0.0)
      hatch_edge(out, minor_width, 20, ver, me[k]);
    out.stroke();
  }
  while (k < n)
  {
    if (!me[k].empty())
    {
      out.setlinewidth(major_width);
      out.newpath();
      plotpath(out, ver, me[k]);
      hatch_edge(out, major_width, 20, ver, me[k]);
      out.stroke();
    }

    out.setlinewidth(minor_width);
    for (int j = k+1; j < n && j < k+major_step; ++j)
    {
      if (me[j].empty())
        continue;
      out.newpath();
      plotpath(out, ver, me[j]);
      if (minor_width > 0.0)
        hatch_edge(out, minor_width, 20, ver, me[j]);
      out.stroke();
    }
    k = j;
  }

  delete[] ce;
  delete[] me;
  delete[] level;
}

#endif

#if 0

#define DATA_FOLDER "d:\\dat\\"

int driver_1()
{
  int k;
  grid_type g;
  if (!read_grid(DATA_FOLDER "depth-to-top.grd", &g))
    return 1;

  const int n = 15;
  float level[n];
  for (k = 0; k < n; ++k)
    level[k] = float(g.z1 + (k+1) * (g.z2-g.z1) / (n+1));

  vertex_type ver;
  edge_type me[n], ce[n+1];
  marching_squares(g.cx, g.cy, 0, g.cx-1, 0, g.cy-1, g.zz, n, level, ver, me, ce);
  delete[] g.zz;

  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);
  out.newpage();

  double dx = g.x2 - g.x1, dy = g.y2 - g.y1;
  double pg_width = out.pagewidth();
  double pg_height = out.pageheight();
  double pg_margin = 40.0;
  double px = pg_width - 2.0 * pg_margin;
  double py = pg_height - 2.0 * pg_margin;
  double x0, y0, sx, sy;
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

  for (vertex_type::iterator v = ver.begin(); v != ver.end(); ++v)
  {
    v->xcoord = x0 + sx * v->xcoord / (g.cx-1);
    v->ycoord = y0 + sy * v->ycoord / (g.cy-1);
  }

  clock_t t = clock();
  edge_type pos, neg;
  sutherland_hodgman(0.0, 1.0, -(y0 + 0.5 * sy), ver, ce[4], pos, neg);
  std::clog << double(clock() - t) / CLOCKS_PER_SEC << " sec\n";

  if (!pos.empty())
  {
    out.setrgbcolor(1, 0, 0);
    plotpath(out, ver, pos);
    out.fill();
  }
  if (!neg.empty())
  {
    out.setrgbcolor(0, 0, 1);
    plotpath(out, ver, neg);
    out.fill();
  }

  return 0;
}

int main()
{
  grid_type g;
  if (!read_grid(DATA_FOLDER "depth-to-top.grd", &g))
    return 1;

  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);
  out.newpage();

  double i1 = 150, i2 = 250;
  double j1 = 300, j2 = 500;
  double x1 = g.x1 + i1 * (g.x2 - g.x1) / g.cx;
  double x2 = g.x1 + i2 * (g.x2 - g.x1) / g.cx;
  double y1 = g.y1 + j1 * (g.y2 - g.y1) / g.cy;
  double y2 = g.y1 + j2 * (g.y2 - g.y1) / g.cy;

  double dx = x2 - x1, dy = y2 - y1;
  double pg_width = out.pagewidth();
  double pg_height = out.pageheight();
  double pg_margin = 40.0;
  double px = pg_width - 2.0 * pg_margin;
  double py = pg_height - 2.0 * pg_margin;
  double x0, y0, sx, sy;
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

  out.setfont("Helvetica", 4);
  out.setgray(0.0);
  auto_conrec(out, i1, i2, j1, j2, &g, x0, y0, sx, sy);
  delete[] g.zz;

  out.setgray(0.0);
  out.setlinewidth(1.0);
  out.rectstroke(x0, y0, sx, sy); 

  return 0;
}

#endif
