#define LEDA_PREFIX

/*
#ifdef _DEBUG
#define _STLP_DEBUG
#elif !defined (NDEBUG)
#define NDEBUG
#endif
*/

#include <iostream>
#include <cassert>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <limits>
#include <string>

#include <LEDA/dictionary.h>
#include <LEDA/point.h>
#include <LEDA/point_set.h>
#include <LEDA/graph.h>
#include <LEDA/float_geo_alg.h>
#include <LEDA/face_array.h>
#include <LEDA/triangle.h>
#include <LEDA/rectangle.h>
#include <LEDA/polygon.h>
#include <LEDA/gen_polygon.h>
#include <LEDA/rat_point.h>
#include <LEDA/rat_gen_polygon.h>
#include <LEDA/param_types.h>

#include "wellgrid.h"

/*
#pragma comment(lib, "libL_md.lib")
#pragma comment(lib, "libP_md.lib")
#pragma comment(lib, "libG_md.lib")
*/

/*
#pragma comment(lib, "leda_md.lib")
*/

typedef leda::GRAPH<leda_point, int> well_graph;

class well_point_set
{
public:
  typedef wg_split_func split_func;

  void compute(well_graph&, split_func, int ghost_n, double ghost_r);
  void update(const well_type*);
  void remove(const well_type*);
  void clear();

  leda_list<leda_point> c_hull() const;
  well_point_set(): w_node(w_system, nil) {}

private:
  leda_point_set w_system;
  leda_node_map<const well_type*> w_node;
};

void well_point_set::update(const well_type* w)
{
  if (w->name[0] == 0)
  {
    std::cerr << "X::update() error: wellbore name is invalid\n";
    return;
  }

  leda_point p(w->top_x, w->top_y);
  leda_node n = w_system.lookup(p);
  if (n == nil)
  {
    n = w_system.insert(p);
  }
  else
  {
    std::cerr << "X::update() warning: duplicate points detected\n";
    std::cerr << "\tnode " << w->name << ": " << w->top_x << ", " << w->top_y << std::endl;

    const well_type* ww = w_node[n];
    std::cerr << "\tnode " << ww->name << ": " << ww->top_x << ", " << ww->top_y << std::endl;
  }

  w_node[n] = w;
}

void well_point_set::clear()
{
  w_system.clear();
}

void well_point_set::remove(const well_type* w)
{
  leda_point p(w->top_x, w->top_y);
  leda_node n = w_system.lookup(p);
  if (n != nil)
  {
    w_system.del(n);
  }
}

leda_list<leda_point> well_point_set::c_hull() const
{
  leda_edge ee = w_system.get_hull_edge(), e = ee;
  leda_list<leda_point> lp;
  do 
  {
    lp.append(w_system.pos_source(e));
    e = w_system.face_cycle_succ(e);
  }
  while (e != ee);
  return lp;
}

static leda_point center_of_icircle(const leda_point& p0, const leda_point& p1, const leda_point& p2) {
  // edges
  leda_vector e0 = p1 - p0;
  leda_vector e1 = p2 - p1;
  leda_vector e2 = p0 - p2;

  // normals
  leda_vector n0 = e0.rotate90().norm();
  leda_vector n1 = e1.rotate90().norm();
  leda_vector n2 = e2.rotate90().norm();

  double a0 = n1 * e0;
  double a1 = n2 * e1;
  double a2 = n0 * e2;
  double invA0 = 1.0 / a0;
  double invA1 = 1.0 / a1;
  double invA2 = 1.0 / a2;

  double R = 1.0 / (invA0 + invA1 + invA2);
  leda_vector v0(p0.xcoord(), p0.ycoord());
  leda_vector v1(p1.xcoord(), p1.ycoord());
  leda_vector v2(p2.xcoord(), p2.ycoord());
  return (v0 * invA0 + v1 * invA1 + v2 * invA2) * R;
}

class edge_cmp: public leda::leda_cmp_base<leda_edge>
{
  const well_graph& g;
public:
  edge_cmp(const well_graph& init): g(init) {}
  int operator() (const leda_edge& e1, const leda_edge& e2) const
  {
    leda_point s1 = g.inf(g.source(e1));
    leda_point t1 = g.inf(g.target(e1));
    leda_point s2 = g.inf(g.source(e2));
    leda_point t2 = g.inf(g.target(e2));
    return compare_by_angle(t1 - s1, t2 - s2);
  }
};

static void arrange_edges(well_graph& g)
{
  leda_list<leda_edge> e = g.all_edges();
  e.sort(edge_cmp(g));
  g.sort_edges(e);
  g.compute_faces();
}

void well_point_set::compute(well_graph& g, split_func split, int ghost_n, double ghost_r)
{
  g.clear();

  // STEP1 -- adding ghost wells around the system
  {
    leda_list<leda_point> pp = w_system.points();
    leda_circle c1 = SMALLEST_ENCLOSING_CIRCLE(pp), c2 = LARGEST_EMPTY_CIRCLE(pp);
    double d2 = c2.radius(), d1 = c1.radius() + 2 * d2;

    if (ghost_n < 12)
      ghost_n = 12;
    if (ghost_r < d1)
      ghost_r = d1;

    leda_circle c(c1.center(), ghost_r);
    for (int i = 0; i < ghost_n; i++)
    {
      leda_point p = c.point_on_circle(2 * LEDA_PI * i / ghost_n);
      leda_node n = w_system.insert(p);
      w_node[n] = 0;
    }
  }

  // STEP 2 -- calling external function to split triangles edges
  leda_point init;
  leda_edge_array<leda_point> e_split(w_system, init);
  {
    leda_edge e;
    forall_edges(e, w_system)
    {
      if (identical(e_split[e], init))
      {
        const well_type *w1 = w_node[w_system.source(e)];
        const well_type *w2 = w_node[w_system.target(e)];

        double t;
        leda_point p1 = w_system.pos_source(e), p2 = w_system.pos_target(e);
        if (w1 != 0 && w2 != 0)
        {
          t = split(w1, w2);
        }
        else if (w1 != 0)
        {
          well_type w2 = { "", (float) p2.xcoord(), (float) p2.ycoord() };
          t = split(w1, &w2);
        }
        else if (w2 != 0)
        {
          well_type w1 = { "", (float) p1.xcoord(), (float) p1.ycoord() };
          t = split(&w1, w2);
        }
        else
        {
          well_type w1 = { "", (float) p1.xcoord(), (float) p1.ycoord() };
          well_type w2 = { "", (float) p2.xcoord(), (float) p2.ycoord() };
          t = split(&w1, &w2);
        }
        assert(t > 0.0 && t < 1.0);
        e_split[e] = e_split[w_system.reversal(e)] = p1 + (p2 - p1) * t;
      }
    }
  }

  // STEP 3 -- generate nodes of result graph
  leda_edge_array<leda_node> f_node(w_system, nil);
  leda_edge_array<leda_node> e_node(w_system, nil);
  {
    leda_edge e;
    forall_edges(e, w_system)
    {   
      leda_edge e1, e2, e3;
      if (w_system.is_hull_edge(e1 = e))
        continue;

      if (!w_system.is_hull_edge(e) && f_node[e] == nil)
      {
        leda_point p1 = e_split[e1];
        leda_point p2 = e_split[e2 = w_system.face_cycle_succ(e1)];
        leda_point p3 = e_split[e3 = w_system.face_cycle_succ(e2)];
        leda_node n = g.new_node(center_of_icircle(p1, p2, p3));
        f_node[e1] = f_node[e2] = f_node[e3] = n;
      }

      e1 = e, e2 = w_system.reversal(e1);
      if (!w_system.is_hull_edge(e1) && !w_system.is_hull_edge(e2) && e_node[e] == nil)
      {
        e_node[e1] = e_node[e2] = g.new_node(e_split[e]);
      }
    }
  }
  // NB: here we can destroy e_split array

  // STEP 4 -- loop around wellbore nodes
  leda_node_array<leda_node> n_node(w_system, nil);
  {
    leda_node n;
    forall_nodes(n, w_system)
    {
      const well_type* w = w_node[n];
      if (w != 0)
      {
        leda_point p = w_system.pos(n);
        leda_node n2, n1 = g.new_node(p);
        n_node[n] = n1;

        leda_edge e, e1, e2;
        forall_adj_edges(e, n)
        {
          n2 = e_node[e];
          e1 = g.new_edge(n1, n2, (int) w);
          e2 = g.new_edge(n2, n1, (int) w);
          g.set_reversal(e1, e2);

          n2 = f_node[e];
          e1 = g.new_edge(n1, n2, (int) w);
          e2 = g.new_edge(n2, n1, (int) w);
          g.set_reversal(e1, e2);
        }
      }
    }
  }

  // STEP 5 -- connect edge nodes with face nodes
  {
    leda_edge e;
    forall_edges(e, w_system)
    {
      if (w_system.is_hull_edge(e))
        continue;

      leda_edge e1 = e, e2 = w_system.reversal(e);
      if (index(e1) > index(e2))
        continue;

      leda_node s = w_system.source(e), t = w_system.target(e);
      const well_type *w1 = w_node[s], *w2 = w_node[t];

      leda_node n0 = e_node[e];
      leda_node n1 = f_node[e1];
      leda_node n2 = f_node[e2];

      if (n0 == nil)
        continue;

      e1 = g.new_edge(n1, n0, (int) w2);
      e2 = g.new_edge(n0, n1, (int) w1);
      g.set_reversal(e1, e2);

      e1 = g.new_edge(n2, n0, (int) w1);
      e2 = g.new_edge(n0, n2, (int) w2);
      g.set_reversal(e1, e2);
    }
  }

  // STEP 6 -- rearrange edge list and make a map
  arrange_edges(g);
  w_system.clear();
}

class node_func
{
public:
  node_func(const well_graph& init);
  virtual double apply(leda_node n) const = 0;
  const well_graph& dom() const;

private:
  const well_graph& g;
};

class half_space: public node_func
{
public:
  half_space(const well_graph& g, double x1, double y1, double x2, double y2);
  double apply(leda_node n) const;

private:
  double a, b, c;
};

node_func::node_func(const well_graph& init): g(init)
{
}

const well_graph& node_func::dom() const
{
  return g;
}

half_space::half_space(const well_graph& init, double x1, double y1, double x2, double y2): 
  node_func(init)
{
  double dx = x2 - x1, dy = y2 - y1;
  a = dy, b = -dx, c = -a * x1 - b * y1;
}

double half_space::apply(leda_node n) const
{
  leda_point p = dom()[n];
  return (a * p.xcoord() + b * p.ycoord() + c);
}

static void classify(const node_func& fn, well_graph& g1, well_graph& g2)
{
  const well_graph& g = fn.dom();
  leda_node_array<double> fx(g);

  leda_node n;
  forall_nodes(n, g)
  {
    fx[n] = fn.apply(n);
  }

  g1.clear();
  g2.clear();

  leda_node_array<leda_node> n1(g, nil), n2(g, nil);
  leda_edge_array<leda_node> e1(g, nil), e2(g, nil);
	leda::face_array<bool> visit(g, false); // TODO: заменить на leda_edge_array<bool>

  double eps = 0.0000001;
  forall_nodes(n, g)
  {
    if (fx[n] < -eps)
      n1[n] = g1.new_node(g[n]);
    else if (fx[n] > eps)
      n2[n] = g2.new_node(g[n]);
    else
    {
      n1[n] = g1.new_node(g[n]);
      n2[n] = g2.new_node(g[n]);
    }
  }

  leda_edge h1, h2, e;
  forall_edges(e, g)
  {
    leda_node s = g.source(e), t = g.target(e);
    leda_edge r = g.reversal(e);
    if (index(e) > index(r))
      continue;

    if (n1[s] != nil && n2[s] == nil && n1[t] == nil && n2[t] != nil
    ||  n1[s] == nil && n2[s] != nil && n1[t] != nil && n2[t] == nil)
    {
      leda_point p = g[s] + (g[t] - g[s]) * fx[s] / (fx[s] - fx[t]);
      e1[e] = e1[r] = g1.new_node(p);
      e2[e] = e2[r] = g2.new_node(p);
    }
  }

  forall_edges(e, g)
  {
    leda_edge r = g.reversal(e);
    if (index(e) > index(r))
      continue;

    leda_node s = g.source(e), t = g.target(e);

    ////////////////////////////////////////////////////////////////////////////
    // случай 1 -- ребро в отрицательной полуплоскости
    if (n1[s] != nil && n1[t] != nil && (n2[s] == nil || n2[t] == nil))
    {
      h1 = g1.new_edge(n1[s], n1[t], g[e]);
      h2 = g1.new_edge(n1[t], n1[s], g[r]);
      g1.set_reversal(h1, h2);
    }
    // случай 1 -- ребро в отрицательной полуплоскости
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // случай 2 -- ребро в положительной полуплоскости
    else if ((n1[s] == nil || n1[t] == nil) && n2[s] != nil && n2[t] != nil)
    {
      h1 = g2.new_edge(n2[s], n2[t], g[e]);
      h2 = g2.new_edge(n2[t], n2[s], g[r]);
      g2.set_reversal(h1, h2);
    }
    // случай 2 -- ребро в положительной полуплоскости
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // случай 3 -- ребро направлено из отрицательной в положительную область
    else if (n1[s] != nil && n2[s] == nil && n1[t] == nil && n2[t] != nil)
    {
      h1 = g1.new_edge(n1[s], e1[e], g[e]);
      h2 = g1.new_edge(e1[r], n1[s], g[r]);
      g1.set_reversal(h1, h2);

      h1 = g2.new_edge(e2[e], n2[t], g[e]);
      h2 = g2.new_edge(n2[t], e2[r], g[r]);
      g2.set_reversal(h1, h2);

      // обработать грань, смежную с ребром e
      leda_face f = g.face_of(e);
      if (g[e] != 0 && !visit[f])
      {
        leda_edge succ_e = g.face_cycle_succ(e);
        leda_node opposite_n = g.target(succ_e);
        if (n1[opposite_n] == nil && n2[opposite_n] != nil)
        {
          leda_edge pred_e = g.face_cycle_pred(e);

          h1 = g1.new_edge(e1[e], e1[pred_e], g[e]);
          h2 = g1.new_edge(e1[pred_e], e1[e], 0);
          g1.set_reversal(h1, h2);

          leda_node a = e2[e], b = n2[t], c = n2[opposite_n], d = e2[pred_e];

          h1 = g2.new_edge(a, d, 0);
          h2 = g2.new_edge(d, a, g[e]);
          g2.set_reversal(h1, h2);

          // повести в четырёхугольнике диагональ с меньшей длиной
          if (g2[a].sqr_dist(g2[c]) <= g2[b].sqr_dist(g2[d]))
          {
            h1 = g2.new_edge(a, c, g[e]);
            h2 = g2.new_edge(c, a, g[e]);
            g2.set_reversal(h1, h2);
          }
          else
          {
            h1 = g2.new_edge(b, d, g[e]);
            h2 = g2.new_edge(d, b, g[e]);
            g2.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] == nil)
        {
          leda_node a = e1[e], b = e1[succ_e], c = n1[opposite_n], d = n1[s];

          h1 = g1.new_edge(a, b, g[e]);
          h2 = g1.new_edge(b, a, 0);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[e], e2[succ_e], 0);
          h2 = g2.new_edge(e2[succ_e], e2[e], g[e]);
          g2.set_reversal(h1, h2);

          // повести в четырёхугольнике диагональ с меньшей длиной
          if (g1[a].sqr_dist(g1[c]) <= g1[b].sqr_dist(g1[d]))
          {
            h1 = g1.new_edge(a, c, g[e]);
            h2 = g1.new_edge(c, a, g[e]);
            g1.set_reversal(h1, h2);
          }
          else
          {
            h1 = g1.new_edge(b, d, g[e]);
            h2 = g1.new_edge(d, b, g[e]);
            g1.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] != nil)
        {
          h1 = g1.new_edge(e1[e], n1[opposite_n], g[e]);
          h2 = g1.new_edge(n1[opposite_n], e1[e], 0);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[e], n2[opposite_n], 0);
          h2 = g2.new_edge(n2[opposite_n], e2[e], g[e]);
          g2.set_reversal(h1, h2);
        }
        else
          assert(0);

        visit[f] = true;
      }

      // обработать грань, смежную с ребром r
      f = g.face_of(r);
      if (g[r] != 0 && !visit[f])
      {
        leda_edge succ_r = g.face_cycle_succ(r);
        leda_node opposite_n = g.target(succ_r);
        if (n1[opposite_n] == nil && n2[opposite_n] != nil)
        {
          h1 = g1.new_edge(e1[r], e1[succ_r], 0);
          h2 = g1.new_edge(e1[succ_r], e1[r], g[r]);
          g1.set_reversal(h1, h2);

          leda_node a = e2[r], b = e2[succ_r], c = n2[opposite_n], d = n2[t];

          h1 = g2.new_edge(a, b, g[r]);
          h2 = g2.new_edge(b, a, 0);
          g2.set_reversal(h1, h2);

          if (g2[a].sqr_dist(g2[c]) <= g2[b].sqr_dist(g2[d]))
          {
            h1 = g2.new_edge(a, c, g[r]);
            h2 = g2.new_edge(c, a, g[r]);
            g2.set_reversal(h1, h2);
          }
          else
          {
            h1 = g2.new_edge(b, d, g[r]);
            h2 = g2.new_edge(d, b, g[r]);
            g2.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] == nil)
        {
          leda_edge pred_r = g.face_cycle_pred(r);

          leda_node a = e1[r], b = n1[s], c = n1[opposite_n], d = e1[pred_r];

          h1 = g1.new_edge(a, d, 0);
          h2 = g1.new_edge(d, a, g[r]);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[r], e2[pred_r], g[r]);
          h2 = g2.new_edge(e2[pred_r], e2[r], 0);
          g2.set_reversal(h1, h2);

          if (g1[a].sqr_dist(g1[c]) <= g1[b].sqr_dist(g1[d]))
          {
            h1 = g1.new_edge(a, c, g[r]);
            h2 = g1.new_edge(c, a, g[r]);
            g1.set_reversal(h1, h2);
          }
          else
          {
            h1 = g1.new_edge(b, d, g[r]);
            h2 = g1.new_edge(d, b, g[r]);
            g1.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] != nil)
        {
          h1 = g1.new_edge(e1[r], n1[opposite_n], 0);
          h2 = g1.new_edge(n1[opposite_n], e1[r], g[r]);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[r], n2[opposite_n], g[r]);
          h2 = g2.new_edge(n2[opposite_n], e2[r], 0);
          g2.set_reversal(h1, h2);
        }
        else
          assert(0);

        visit[f] = true;
      }
    }
    // случай 3 -- ребро направлено из отрицательной в положительную область
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // случай 4 -- ребро направлено из положительной в отрицательную область
    else if (n1[s] == nil && n2[s] != nil && n1[t] != nil && n2[t] == nil)
    {
      h1 = g1.new_edge(e1[e], n1[t], g[e]);
      h2 = g1.new_edge(n1[t], e1[r], g[r]);
      g1.set_reversal(h1, h2);

      h1 = g2.new_edge(n2[s], e2[e], g[e]);
      h2 = g2.new_edge(e2[r], n2[s], g[r]);
      g2.set_reversal(h1, h2);

      leda_face f = g.face_of(e);
      if (g[e] != 0 && !visit[f])
      {
        leda_edge succ_e = g.face_cycle_succ(e);
        leda_node opposite_n = g.target(succ_e);
        if (n1[opposite_n] == nil && n2[opposite_n] != nil)
        {
          h1 = g1.new_edge(e1[e], e1[succ_e], 0);
          h2 = g1.new_edge(e1[succ_e], e1[e], g[e]);
          g1.set_reversal(h1, h2);

          leda_node a = e2[e], b = e2[succ_e], c = n2[opposite_n], d = n2[s];

          h1 = g2.new_edge(a, b, g[e]);
          h2 = g2.new_edge(b, a, 0);
          g2.set_reversal(h1, h2);

          if (g2[a].sqr_dist(g2[c]) <= g2[b].sqr_dist(g2[d]))
          {
            h1 = g2.new_edge(a, c, g[e]);
            h2 = g2.new_edge(c, a, g[e]);
            g2.set_reversal(h1, h2);
          }
          else
          {
            h1 = g2.new_edge(b, d, g[e]);
            h2 = g2.new_edge(d, b, g[e]);
            g2.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] == nil)
        {
          leda_edge pred_e = g.face_cycle_pred(e);

          leda_node a = e1[e], b = n1[t], c = n1[opposite_n], d = e1[pred_e];

          h1 = g1.new_edge(a, d, 0);
          h2 = g1.new_edge(d, a, g[e]);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[e], e2[pred_e], g[e]);
          h2 = g2.new_edge(e2[pred_e], e2[e], 0);
          g2.set_reversal(h1, h2);

          if (g1[a].sqr_dist(g1[c]) <= g1[b].sqr_dist(g1[d]))
          {
            h1 = g1.new_edge(a, c, g[e]);
            h2 = g1.new_edge(c, a, g[e]);
            g1.set_reversal(h1, h2);
          }
          else
          {
            h1 = g1.new_edge(b, d, g[e]);
            h2 = g1.new_edge(d, b, g[e]);
            g1.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] != nil)
        {
          h1 = g1.new_edge(e1[e], n1[opposite_n], 0);
          h2 = g1.new_edge(n1[opposite_n], e1[e], g[e]);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[e], n2[opposite_n], g[e]);
          h2 = g2.new_edge(n2[opposite_n], e2[e], 0);
          g2.set_reversal(h1, h2);
        }
        else
          assert(0);

        visit[f] = true;
      }

      f = g.face_of(r);
      if (g[r] != 0 && !visit[f])
      {
        leda_edge succ_r = g.face_cycle_succ(r);
        leda_node opposite_n = g.target(succ_r);
        if (n1[opposite_n] == nil && n2[opposite_n] != nil)
        {
          leda_edge pred_r = g.face_cycle_pred(r);

          h1 = g1.new_edge(e1[r], e1[pred_r], g[r]);
          h2 = g1.new_edge(e1[pred_r], e1[r], 0);
          g1.set_reversal(h1, h2);

          leda_node a = e2[r], b = n2[s], c = n2[opposite_n], d = e2[pred_r];

          h1 = g2.new_edge(a, d, 0);
          h2 = g2.new_edge(d, a, g[r]);
          g2.set_reversal(h1, h2);

          if (g2[a].sqr_dist(g2[c]) <= g2[b].sqr_dist(g2[d]))
          {
            h1 = g2.new_edge(a, c, g[r]);
            h2 = g2.new_edge(c, a, g[r]);
            g2.set_reversal(h1, h2);
          }
          else
          {
            h1 = g2.new_edge(b, d, g[r]);
            h2 = g2.new_edge(d, b, g[r]);
            g2.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] == nil)
        {
          leda_node a = e1[r], b = e1[succ_r], c = n1[opposite_n], d = n1[t];

          h1 = g1.new_edge(a, b, g[r]);
          h2 = g1.new_edge(b, a, 0);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[r], e2[succ_r], 0);
          h2 = g2.new_edge(e2[succ_r], e2[r], g[r]);
          g2.set_reversal(h1, h2);

          if (g1[a].sqr_dist(g1[c]) <= g1[b].sqr_dist(g1[d]))
          {
            h1 = g1.new_edge(a, c, g[r]);
            h2 = g1.new_edge(c, a, g[r]);
            g1.set_reversal(h1, h2);
          }
          else
          {
            h1 = g1.new_edge(b, d, g[r]);
            h2 = g1.new_edge(d, b, g[r]);
            g1.set_reversal(h1, h2);
          }
        }
        else if (n1[opposite_n] != nil && n2[opposite_n] != nil)
        {
          h1 = g1.new_edge(e1[r], n1[opposite_n], g[r]);
          h2 = g1.new_edge(n1[opposite_n], e1[r], 0);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(e2[r], n2[opposite_n], 0);
          h2 = g2.new_edge(n2[opposite_n], e2[r], g[r]);
          g2.set_reversal(h1, h2);
        }
        else
          assert(0);

        visit[f] = true;
      }
    }
    // случай 4 -- ребро направлено из положительной в отрицательную область
    ////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////
    // случай 5 -- ребро на границе раздела
    else if (n1[s] != nil && n2[s] != nil && n1[t] != nil && n2[t] != nil)
    {
      if (g[e] == 0)
      {
        leda_edge succ_r = g.face_cycle_succ(r);
        leda_node opposite_n = g.target(succ_r);
        if (n1[opposite_n] != nil)
        {
          h1 = g1.new_edge(n1[s], n1[t], 0);
          h2 = g1.new_edge(n1[t], n1[s], g[r]);
          g1.set_reversal(h1, h2);
        }
        else if (n2[opposite_n] != nil)
        {
          h1 = g2.new_edge(n2[s], n2[t], 0);
          h2 = g2.new_edge(n2[t], n2[s], g[r]);
          g2.set_reversal(h1, h2);
        }
        else
          assert(0);
      }
      else if (g[r] == 0)
      {
        leda_edge succ_e = g.face_cycle_succ(e);
        leda_node opposite_n = g.target(succ_e);
        if (n1[opposite_n] != nil)
        {
          h1 = g1.new_edge(n1[s], n1[t], g[e]);
          h2 = g1.new_edge(n1[t], n1[s], 0);
          g1.set_reversal(h1, h2);
        }
        else if (n2[opposite_n] != nil)
        {
          h1 = g2.new_edge(n2[s], n2[t], g[e]);
          h2 = g2.new_edge(n2[t], n2[s], 0);
          g2.set_reversal(h1, h2);
        }
        else
          assert(0);
      }
      else
      {
        leda_edge succ_e = g.face_cycle_succ(e), succ_r = g.face_cycle_succ(r);
        leda_node opposite_e = g.target(succ_e), opposite_r = g.target(succ_r);

        if (n1[opposite_e] != nil && n1[opposite_r] == nil && n2[opposite_e] == nil && n2[opposite_r] != nil)
        {
          h1 = g1.new_edge(n1[s], n1[t], g[e]);
          h2 = g1.new_edge(n1[t], n1[s], 0);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(n2[s], n2[t], 0);
          h2 = g2.new_edge(n2[t], n2[s], g[r]);
          g2.set_reversal(h1, h2);
        }
        else if (n1[opposite_e] == nil && n1[opposite_r] != nil && n2[opposite_e] != nil && n2[opposite_r] == nil)
        {
          h1 = g1.new_edge(n1[s], n1[t], 0);
          h2 = g1.new_edge(n1[t], n1[s], g[r]);
          g1.set_reversal(h1, h2);

          h1 = g2.new_edge(n2[s], n2[t], g[e]);
          h2 = g2.new_edge(n2[t], n2[s], 0);
          g2.set_reversal(h1, h2);
        }
        else
          assert(0);
      }
    }
    // случай 5 -- ребро на границе раздела
    ////////////////////////////////////////////////////////////////////////////

    else
      assert(0);
  }

  arrange_edges(g1);
  arrange_edges(g2);
}

struct well_grid_key { int cell; const well_type* well; };

static int compare(const well_grid_key& k1, const well_grid_key& k2)
{
  if (k1.cell < k2.cell)
    return -1;

  if (k1.cell == k2.cell)
  {
    if (k1.well < k2.well)
      return -1;
  
    if (k1.well == k2.well)
      return 0;
  }

  return 1;
}

class well_grid
{
public:
	typedef well_grid_key key;

  double left(int i) const;
  double right(int i) const;
  double top(int j) const;
  double bottom(int j) const;
  double inf(const well_type* w, int i, int j) const;

  void populate(const well_graph& g);
  well_grid(int cx, int cy, double x1, double x2, double y1, double y2);
  well_grid(int cx, int cy, double *init_x, double *init_y);
  ~well_grid();

private:
  void traverse(const well_graph& g, int x1, int x2, int y1, int y2);

	leda_dictionary<key, double>* grid_fn;
  int countof_x, countof_y;
  double *coord_x, *coord_y;
};

well_grid::well_grid(int cx, int cy, double x1, double x2, double y1, double y2): 
  grid_fn(new leda_dictionary<key, double>(compare)), 
	countof_x(cx), 
	countof_y(cy)
{
  coord_x = new double[cx+1];
  coord_y = new double[cy+1];

  double dx = (x2 - x1) / cx;
  coord_x[0] = x1;
  for (int i = 0; i < cx; i++)
    coord_x[i+1] = coord_x[i] + dx;

  double dy = (y2 - y1) / cy;
  coord_y[0] = y1;
  for (int j = 0; j < cy; j++)
    coord_y[j+1] = coord_y[j] + dy;
}

well_grid::well_grid(int cx, int cy, double *init_x, double *init_y): countof_x(cx), countof_y(cy)
{
  coord_x = new double[cx+1];
  coord_y = new double[cy+1];

  for (int i = 0; i < cx+1; i++) 
    coord_x[i] = init_x[i];

  for (int j = 0; j < cy+1; j++)
    coord_y[j] = init_y[j];
}

well_grid::~well_grid()
{
	delete grid_fn;
  delete[] coord_x;
  delete[] coord_y;
}

inline double well_grid::left(int i) const
{
  return coord_x[i];
}

inline double well_grid::right(int i) const
{
  return coord_x[i+1];
}

inline double well_grid::top(int j) const
{
  return coord_y[j+1];
}

inline double well_grid::bottom(int j) const
{
  return coord_y[j];
}

double well_grid::inf(const well_type* w, int i, int j) const
{
  key k = { countof_x * j + i, w };
	leda::dic_item it = grid_fn->lookup(k);
  if (it != nil)
    return (*grid_fn)[it];
  return 0.0;
}

void well_grid::traverse(const well_graph& g0, int x1, int x2, int y1, int y2)
{
  int cx = x2 - x1, cy = y2 - y1;
  if (cx == 1 && cy == 1)
  {
    // std::cout << "cell(" << x1 << ", " << y1 << ") visited\n";
    
    double a;
    leda_face f;
    leda_map<const well_type*, double> acc;
    const well_type* w;
    forall_faces(f, g0)
    {
      leda_edge e = g0.first_face_edge(f);
      w = (const well_type*) g0[e];
      if (w == 0)
        continue;

      leda_node s = g0.source(e), t = g0.target(e);
      leda_node o = g0.target(g0.face_cycle_succ(e));

			a = leda::triangle(g0[s], g0[t], g0[o]).area();
      assert(a >= 0.0);

      if (acc.defined(w))
        acc[w] += a;
      else
        acc[w] = a;
    }

		a = leda::rectangle(left(x1), bottom(y1), right(x1), top(y1)).area();
    assert(a >= 0.0);

    key k = { countof_x * y1 + x1 };
    forall_defined(w, acc)
    {
      std::ofstream ff("area", std::ios::app);
      ff << w->name << ' ' << k.cell << ' ' << acc[w]/a << '\n';
      
      // k.well = w;
      // grid_fn.insert(k, acc[w] / a);
    }
  }
  else if (cx < cy)
  {
    int y0 = ((y1 + y2) >> 1);

    well_graph g1, g2;
    classify(half_space(g0, left(x2), bottom(y0), left(x1), bottom(y0)), g1, g2);
    
    traverse(g1, x1, x2, y1, y0);
    traverse(g2, x1, x2, y0, y2);
  }
  else
  {
    int x0 = ((x1 + x2) >> 1);

    well_graph g1, g2;
    classify(half_space(g0, left(x0), bottom(y1), left(x0), bottom(y2)), g1, g2);
    
    traverse(g1, x1, x0, y1, y2);
    traverse(g2, x0, x2, y1, y2);
  }
}

void well_grid::populate(const well_graph& g0)
{
  double x1 = coord_x[0], x2 = coord_x[countof_x];
  double y1 = coord_y[0], y2 = coord_y[countof_y];

  well_graph d1, g1, g2;

  classify(half_space(g0, x2, y1, x1, y1), d1, g2);
  classify(half_space(g2, x1, y1, x1, y2), d1, g1);
  classify(half_space(g1, x1, y2, x2, y2), d1, g2);
  classify(half_space(g2, x2, y2, x2, y1), d1, g1);

  d1.clear();
  g2.clear();
  
  grid_fn->clear();
  traverse(g1, 0, countof_x, 0, countof_y);
}

class wg_wrapper
{
public:
  typedef well_point_set::split_func split_func;

  wg_wrapper(int cx, int cy, double x1, double x2, double y1, double y2): g2(cx, cy, x1, x2, y1, y2) {}
  wg_wrapper(int cx, int cy, double *init_x, double *init_y): g2(cx, cy, init_x, init_y) {}

  void update(const well_type* w);
  void compute(split_func fn);

  double info(const well_type* w, int x, int y) const;

private:
  well_point_set g1;
  well_grid g2;
};

void wg_wrapper::update(const well_type* w)
{
  g1.update(w);
}

void wg_wrapper::compute(split_func fn)
{
  well_graph g;
  g1.compute(g, fn, 0, 0);
  g2.populate(g);
}

inline double wg_wrapper::info(const well_type* w, int x, int y) const
{
  return g2.inf(w, x-1, y-1);
}

extern "C" int __stdcall wg_create_uniform(int cx, int cy, double x1, double x2, double y1, double y2)
{
  return (int) (new wg_wrapper(cx, cy, x1, x2, y1, y2));
}

extern "C" int __stdcall wg_create_nonuniform(int cx, int cy, double *init_x, double *init_y)
{
  return (int) (new wg_wrapper(cx, cy, init_x, init_y));
}

extern "C" void __stdcall wg_destroy(int h)
{
  delete ((wg_wrapper*) h);
}

extern "C" void __stdcall wg_update(int h, well_type* w)
{
  ((wg_wrapper*) h)->update(w);
}

extern "C" void __stdcall wg_compute(int h, wg_wrapper::split_func fn)
{
  ((wg_wrapper*) h)->compute(fn);
}

extern "C" double __stdcall wg_info(int h, well_type* w, int x, int y)
{
  return ((wg_wrapper*) h)->info(w, x, y);
}

class polygon_grid
{
public:
  double left(int i) const;
  double right(int i) const;
  double top(int j) const;
  double bottom(int j) const;
  double inf(int i, int j) const;

  void populate(int n, double x[], double y[]);
  polygon_grid(int cx, int cy, double x1, double x2, double y1, double y2);
  polygon_grid(int cx, int cy, double *init_x, double *init_y);
  ~polygon_grid();

private:
  int countof_x, countof_y;
  double *coord_x, *coord_y;
  leda_dictionary<int, double> grid_fn;
};

int compare(int k1, int k2)
{
  if (k1 < k2) return -1;
  if (k1 > k2) return 1;
  return 0;
}

polygon_grid::polygon_grid(int cx, int cy, double x1, double x2, double y1, double y2): 
  countof_x(cx), countof_y(cy), grid_fn()
{
  coord_x = new double[cx+1];
  coord_y = new double[cy+1];

  double dx = (x2 - x1) / cx;
  coord_x[0] = x1;
  for (int i = 0; i < cx; i++)
    coord_x[i+1] = coord_x[i] + dx;

  double dy = (y2 - y1) / cy;
  coord_y[0] = y1;
  for (int j = 0; j < cy; j++)
    coord_y[j+1] = coord_y[j] + dy;
}

polygon_grid::polygon_grid(int cx, int cy, double *init_x, double *init_y): 
  countof_x(cx), countof_y(cy), grid_fn()
{
  coord_x = new double[cx+1];
  coord_y = new double[cy+1];

  for (int i = 0; i < cx+1; i++) 
    coord_x[i] = init_x[i];

  for (int j = 0; j < cy+1; j++)
    coord_y[j] = init_y[j];
}

polygon_grid::~polygon_grid()
{
  delete[] coord_x;
  delete[] coord_y;
}

inline double polygon_grid::left(int i) const
{
  return coord_x[i];
}

inline double polygon_grid::right(int i) const
{
  return coord_x[i+1];
}

inline double polygon_grid::top(int j) const
{
  return coord_y[j+1];
}

inline double polygon_grid::bottom(int j) const
{
  return coord_y[j];
}

double polygon_grid::inf(int i, int j) const
{
	leda::dic_item it = grid_fn.lookup(countof_x * j + i);
  if (it != nil)
    return grid_fn[it];
  return 0.0;
}

static double common_area1(leda_list<leda_point> numer, leda_rectangle& rect_c)
{
  leda_list<leda_point> denom = rect_c.vertices();
  leda_list<leda_rat_point> rat_numer, rat_denom;

  leda_point p;
  forall(p, numer) {
    leda_rational x(p.xcoord()), y(p.ycoord());
    rat_numer.push_back(leda_rat_point(x, y));
  }
  forall(p, denom) {
    leda_rational x(p.xcoord()), y(p.ycoord());
    rat_denom.push_back(leda_rat_point(x, y));
  }

	leda::rat_gen_polygon p1(rat_numer), p2(rat_denom);
  return p1.intersection(p2).area().to_float() / rect_c.area();
}

static double common_area(leda_list<leda_point> numer, leda_rectangle& rect_c)
{
  leda_list<leda_point> denom = rect_c.vertices();

	leda::gen_polygon p1(numer), p2(denom);
  return p1.intersection(p2).area() / rect_c.area();
}

void polygon_grid::populate(int n, double x[], double y[])
{
  int i, j;
  if (!grid_fn.empty())
    grid_fn.clear();

  leda_list<leda_point> lp;
  for (i = 0; i < n; ++i)
    lp.append(leda_point(*x++, *y++));

  leda_point pl, pb, pr, pt;
  Bounding_Box(lp, pl, pb, pr, pt);
  leda_rectangle rect_f(pl.xcoord(), pb.ycoord(), pr.xcoord(), pt.ycoord());

  int x1 = std::lower_bound(coord_x, coord_x + countof_x, rect_f.xmin()) - coord_x;
  int x2 = std::upper_bound(coord_x, coord_x + countof_x, rect_f.xmax()) - coord_x;
  int y1 = std::lower_bound(coord_y, coord_y + countof_y, rect_f.ymin()) - coord_y;
  int y2 = std::upper_bound(coord_y, coord_y + countof_y, rect_f.ymax()) - coord_y;
  if (x1 != 0) --x1;
  if (y1 != 0) --y1;

  for (j = y1; j < y2; ++j) for (i = x1; i < x2; ++i)
  {
    leda_rectangle rect_c(left(i), bottom(j), right(i), top(j));
    if (rect_f.intersection(rect_c).empty())
      continue;

    double w = common_area(lp, rect_c);
    if (w > 0.0)
      grid_fn.insert(countof_x * j + i, w);
  }
}

extern "C" int __stdcall pg_create_uniform(int cx, int cy, double x1, double x2, double y1, double y2)
{
  polygon_grid *p = new polygon_grid(cx, cy, x1, x2, y1, y2);
  return ((int) p);
}

extern "C" int __stdcall pg_create_nonuniform(int cx, int cy, double *init_x, double *init_y)
{
  polygon_grid* p = new polygon_grid(cx, cy, init_x, init_y);
  return ((int) p);
}

extern "C" void __stdcall pg_destroy(int h)
{
  polygon_grid* p = (polygon_grid*) h;
  delete p;
}

extern "C" void __stdcall pg_populate(int h, int n, double *x, double *y)
{
  polygon_grid* p = (polygon_grid*) h;
  p->populate(n, x, y);
}

extern "C" double __stdcall pg_info(int h, int x, int y)
{
  polygon_grid* p = (polygon_grid*) h;
  return p->inf(x, y);
}

extern "C" int __stdcall wh_create()
{
  well_point_set *ps = new well_point_set();
  return ((int) ps);
}

extern "C" void __stdcall wh_destroy(int h)
{
  well_point_set *ps = (well_point_set*) h;
  delete ps;
}

extern "C" void __stdcall wh_update(int h, well_type* w)
{
  well_point_set *ps = (well_point_set*) h;
  ps->update(w);
}

extern "C" void __stdcall wh_remove(int h, well_type* w)
{
  well_point_set *ps = (well_point_set*) h;
  ps->remove(w);
}

extern "C" void __stdcall wh_clear(int h)
{
  well_point_set* ps = (well_point_set*) h;
  ps->clear();
}

extern "C" int __stdcall wh_chull(int h, double delta, int* n, double* coord_x, double* coord_y)
{
  well_point_set* ps = (well_point_set*) h;
  leda_list<leda_point> lp = ps->c_hull();
  int m = lp.size();
  if (m < 3 || m > *n)
  {
    *n = m;
    return 0;
  }

  if (delta <= 0.0)
  {
    leda_point pt;
    forall(pt, lp)
    {
      *coord_x++ = pt.xcoord();
      *coord_y++ = pt.ycoord();
    }
    *n = m;
    return 1;
  }

  leda_polygon pg(lp, leda_polygon::NO_CHECK);
  pg.eliminate_colinear_vertices();
  m = (pg.size() << 1);
  if (m > *n)
  {
    *n = m;
    return 0;
  }

  leda_segment s;
  forall_segments(s, pg)
  {
    double dx = s.dx(), dy = s.dy();
    leda_segment ss = s + leda_vector(dy, -dx).norm() * delta;
    *coord_x++ = ss.xcoord1();
    *coord_y++ = ss.ycoord1();
    *coord_x++ = ss.xcoord2();
    *coord_y++ = ss.ycoord2();
  }
  *n = m;
  return 1;
}

#if 0

double __stdcall fn(const well_type* w1, const well_type* w2)
{
  return 0.5;
}

void main()
{
  double x1 = 1388105, x2 = 1428355, y1 = 706905, y2 = 784655;
  wg_wrapper g(350, 650, x1, x2, y1, y2);

  int n = 0;
  static well_type a[10000];
  while (std::cin >> a[n].name >> a[n].top_x >> a[n].top_y)
  {
    g.update(&a[n++]);
  }
  std::clog << n << " wells loaded\n";

  clock_t t = clock();
  g.compute(fn);
  std::cout << double(clock() - t) / CLOCKS_PER_SEC << " seconds elapsed\n";
}

#endif
