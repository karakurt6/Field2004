#define NOMINMAX
#include <iostream>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <valarray>
#include <vector>
#include <algorithm>
#include <list>
#include <map>
#include <assert.h>
#include <math.h>
#include "ps_data.h"

#include <LEDA/point_set.h>

// compute ray-segment intersection
// choose between [(x1, y1), (x2, y1)], [(x2, y1), (x2, y2)],
//   [(x2, y2), (x1, y2)] and [(x1, y2), (x1, y2)] segments
// solve
//   bx  + ax * t = bx  + ax * s
//     1     1        2     2 
//   by  + ay * t = by  + ay * s 
//     1     1        2     2
// simultameous equations and check if t > 0 && 0 <= s <= 1

static bool ray_segment_intersection(double x1, double y1, double x2, double y2, \
  double bx1, double by1, double ax1, double ay1, double* t, double* s)
{
  double bx2 = x1;
  double by2 = y1;
  double ax2 = x2 - x1;
  double ay2 = y2 - y1;
  double cx = bx2 - bx1;
  double cy = by2 - by1;
  double dd = ay1 * ax2 - ax1 * ay2;
  /*
  if (fabs(dd) < 1.0e-12)
  {
    return false;
  }
  */
  *t = (ax2 * cy - ay2 * cx) / dd;
  *s = (ax1 * cy - ay1 * cx) / dd;
  return true;
}
static bool ray_rectangle_intersection(double x1, double x2, double y1, double y2, \
  double bx, double by, double ax, double ay, double* x, double* y)
{
  double t, s;
  if (ray_segment_intersection(x1, y1, x2, y1, bx, by, ax, ay, &t, &s) \
    && t > 0 && 0.0 <= s && s <= 1.0)
  {
    *x = bx + ax * t;
    *y = by + ay * t;
    return true;
  }
  if (ray_segment_intersection(x2, y1, x2, y2, bx, by, ax, ay, &t, &s) \
    && t > 0 && 0.0 <= s && s <= 1.0)
  {
    *x = bx + ax * t;
    *y = by + ay * t;
    return true;
  }
  if (ray_segment_intersection(x2, y2, x1, y2, bx, by, ax, ay, &t, &s) \
    && t > 0 && 0.0 <= s && s <= 1.0)
  {
    *x = bx + ax * t;
    *y = by + ay * t;
    return true;
  }
  if (ray_segment_intersection(x1, y2, x1, y1, bx, by, ax, ay, &t, &s) \
    && t > 0 && 0.0 <= s && s <= 1.0)
  {
    *x = bx + ax * t;
    *y = by + ay * t;
    return true;
  }
  return false;
}

void voro(int np, const double xx[1], const double yy[1], \
  double x1, double x2, double y1, double y2, \
  vertex_type& ver, edge_type& pgn, curve_type* arr[1])
{
  leda::point_set p_data;
  leda::node_map<int> s_data(p_data);
  for (int i = 0; i < np; ++i)
  {
    double x = xx[i];
    double y = yy[i];
    arr[i] = 0;
    if (x1 <= x && x <= x2 && y1 <= y && y <= y2)
    {
      leda_point p(x, y);
      leda_node n = p_data.lookup(p);
      if (n == leda_nil)
      {
        n = p_data.insert(p);
        s_data[n] = i;
			}
			else
			{
				std::clog << "point number " << i << " is duplicate\n";
			}
    }
		else
		{
			std::clog << "point number " << i << " is situated beyond of region of interest\n";
		}
  }

  leda::GRAPH<leda_circle, leda_point> v_data;
  p_data.compute_voronoi(v_data);

  // check
  /*
  leda::edge_map<int> e_sign(v_data, 1);
  {
    leda::edge e;
    forall_edges(e, v_data)
    {
      leda::point s_point, t_point;
      bool has_s_point = false;
      bool has_t_point = false;

      leda::node s = v_data.source(e);
      leda::circle s_circle = v_data[s];
      if (!s_circle.is_degenerate())
      {
        s_point = s_circle.center();
        has_s_point = true;
      }

      leda::node t = v_data.target(e);
      leda::circle t_circle = v_data[t];
      if (!t_circle.is_degenerate())
      {
        t_point = t_circle.center();
        has_t_point = true;
      }

      if (has_s_point && has_t_point)
      {
        double x1 = s_point.xcoord();
        double y1 = s_point.ycoord();
        double x2 = t_point.xcoord();
        double y2 = t_point.ycoord();
        double a = y2 - y1;
        double b = x1 - x2;
        double c = 1.0 / hypot(a, b);
        a *= c;
        b *= c;
        c = -(a * x1 + b * y1);

        leda::point p = v_data[e];
        double x = p.xcoord();
        double y = p.ycoord();
        double s = a * x + b * y + c;
        if (s < 0.0)
        {
          e_sign[e] = -1;
        }
      }
    }
  }
  */

  leda_node n;
  leda::node_map<int> v_index(v_data, -1);
  forall_nodes(n, v_data)
  {
    leda::circle c = v_data[n];
    if (!c.is_degenerate())
    {
      leda::point p = c.center();
      if (x1 <= p.xcoord() && p.xcoord() <= x2 && y1 <= p.ycoord() && p.ycoord() <= y2)
      {
        v_index[n] = ver.size();
        ver.push_back(coord_type());
        coord_type* q = &ver.back();
        q->xcoord = p.xcoord();
        q->ycoord = p.ycoord();
      }
    }
  }

  leda::edge e;
  leda::edge_map<int> v_outer(v_data, -1);
  forall_edges(e, v_data)
  {
    leda::node s = v_data.source(e);
    if (v_index[s] == -1)
      continue;

    leda::node t = v_data.target(e);
    if (v_index[t] != -1)
      continue;

    leda::circle s_circ = v_data[s];
    // if (s_circ.is_degenerate())
    //   continue;

    leda::point p = s_circ.center();
    double bx = p.xcoord();
    double by = p.ycoord();
    // if (x1 == bx || x2 == bx || y1 == by || y2 == by)
    //   continue;

    leda::circle t_circ = v_data[t];
    double ax, ay;
    if (t_circ.is_degenerate())
    {
      // clip ray
      leda::edge e_succ = v_data.cyclic_adj_succ(e);
      leda::node n_succ = v_data.target(e_succ);
      leda::circle c_succ = v_data[n_succ];
      assert(!c_succ.is_degenerate());
      leda::point p_succ = c_succ.center();
      double dx_succ = p_succ.xcoord() - bx;
      double dy_succ = p_succ.ycoord() - by;
      double dd_succ = hypot(dx_succ, dy_succ);

      leda::edge e_pred = v_data.cyclic_adj_pred(e);
      leda::node n_pred = v_data.target(e_pred);
      leda::circle c_pred = v_data[n_pred];
      assert(!c_pred.is_degenerate());
      leda::point p_pred = c_pred.center();
      double dx_pred = p_pred.xcoord() - bx;
      double dy_pred = p_pred.ycoord() - by;
      double dd_pred = hypot(dx_pred, dy_pred);

      ax = dx_succ / dd_succ + dx_pred / dd_pred;
      ay = dy_succ / dd_succ + dy_pred / dd_pred;
      double aa = -hypot(ax, ay);
      ax /= aa;
      ay /= aa;
    }
    else
    {
      // clip edge
      leda::point q = t_circ.center();
      ax = q.xcoord() - bx;
      ay = q.ycoord() - by;

      double aa = hypot(ax, ay);
      ax /= aa;
      ay /= aa;

      double gx = 0.0, gy = 0.0;
      leda::edge e_succ = v_data.cyclic_adj_succ(e);
      leda::node n_succ = v_data.target(e_succ);
      leda::circle c_succ = v_data[n_succ];
      if (!c_succ.is_degenerate())
      {
        leda::point p_succ = c_succ.center();
        double dx_succ = p_succ.xcoord() - bx;
        double dy_succ = p_succ.ycoord() - by;
        double dd_succ = hypot(dx_succ, dy_succ);
        gx += dx_succ / dd_succ;
        gy += dy_succ / dd_succ;
      }
      leda::edge e_pred = v_data.cyclic_adj_pred(e);
      leda::node n_pred = v_data.target(e_pred);
      leda::circle c_pred = v_data[n_pred];
      if (!c_pred.is_degenerate())
      {
        leda::point p_pred = c_pred.center();
        double dx_pred = p_pred.xcoord() - bx;
        double dy_pred = p_pred.ycoord() - by;
        double dd_pred = hypot(dx_pred, dy_pred);
        gx += dx_pred / dd_pred;
        gy += dy_pred / dd_pred;
      }
      double gg = -hypot(ax, ay);
      gx /= gg;
      gy /= gg;

      if (gx * ax + gy * ay < 0.0)
      {
        ax = -ax;
        ay = -ay;
      }
    }

    double x, y;
    if (!ray_rectangle_intersection(x1, x2, y1, y2, bx, by, ax, ay, &x, &y))
    {
      assert(false);
    }
    
    v_outer[e] = v_outer[v_data.reversal(e)] = ver.size();
    ver.push_back(coord_type());
    coord_type* q = &ver.back();
    q->xcoord = x;
    q->ycoord = y;
  }

  leda::edge_map<bool> visit(v_data, false);
  forall_edges(e, v_data)
  {
    if (visit[e])
      continue;

    leda::list<leda::point> p_label;
    edge_type temp;
    curve_type* c = 0;
    leda::edge ee = e;
    do
    {
      leda::node s = v_data.source(ee);
      if (v_index[s] != -1)
      {
        leda::node t = v_data.target(ee);
        if (v_index[t] != -1)
        {
          if (c == 0 || c->back() != v_index[s])
          {
            temp.push_back(curve_type());
            c = &temp.back();
            c->push_back(v_index[s]);
            c->push_back(v_index[t]);
          }
          else
          {
            c->push_back(v_index[t]);
          }
        }
        else if (v_outer[ee] != -1)
        {
          if (c == 0 || c->back() != v_index[s])
          {
            temp.push_back(curve_type());
            c = &temp.back();
            c->push_back(v_index[s]);
            c->push_back(v_outer[ee]);
          }
          else
          {
            c->push_back(v_outer[ee]);
          }
          c = 0;
        }
        else
        {
          c = 0;
        }
      }
      else
      {
        leda::node t = v_data.target(ee);
        if (v_index[t] != -1 && v_outer[ee] != -1)
        {
          temp.push_back(curve_type());
          c = &temp.back();
          c->push_back(v_outer[ee]);
          c->push_back(v_index[t]);
        }
        else
        {
          c = 0;
        }
      }
      visit[ee] = true;
      p_label.push_back(v_data[ee]);
      leda::edge rr = v_data.reversal(ee);
      s = v_data.source(rr);
      ee = v_data.cyclic_adj_pred(rr);
      int s_degree = v_data.degree(s);
      if (s_degree < 3)
      {
        // back propagation for outer edges
        ee = e;
        c = 0;
        s = v_data.source(ee);
        s_degree = v_data.degree(s);
        while (s_degree >= 3)
        {
          leda::edge rr = v_data.cyclic_adj_succ(ee);
          ee = v_data.reversal(rr);
          visit[ee] = true;
          p_label.push_back(v_data[ee]);
          s = v_data.source(ee);
          s_degree = v_data.degree(s);
          leda::node t = v_data.target(ee);
          if (v_index[t] != -1)
          {
            leda::node s = v_data.source(ee);
            if (v_index[s] != -1)
            {
              if (c == 0 || c->front() != v_index[t])
              {
                temp.push_back(curve_type());
                c = &temp.back();
                c->push_front(v_index[t]);
                c->push_front(v_index[s]);
              }
              else
              {
                c->push_front(v_index[s]);
              }
            }
            else if (v_outer[ee] != -1)
            {
              if (c == 0 || c->front() != v_index[t])
              {
                temp.push_back(curve_type());
                c = &temp.back();
                c->push_front(v_index[t]);
                c->push_front(v_outer[ee]);
              }
              else
              {
                c->push_front(v_outer[ee]);
              }
              c = 0;
            }
            else
            {
              c = 0;
            }
          }
          else
          {
            leda::node s = v_data.source(ee);
            if (v_index[s] != -1 && v_outer[ee] != -1)
            {
              temp.push_back(curve_type());
              c = &temp.back();
              c->push_front(v_outer[ee]);
              c->push_front(v_index[s]);
            }
            else
            {
              c = 0;
            }
          }
        }
        break;
      }
    }
    while (ee != e);

    // merge curves segment and close path as needed
    // if (temp.size() > 1)
    {
      int p_kind;
      coord_type* p = 0;
      edge_type::iterator leave = temp.end();
      for (edge_type::iterator it = temp.begin(); it != temp.end(); ++it)
      {
        p = &ver[it->back()];
        if (p->xcoord == x1)
        {
          p_kind = 0;
          leave = it;
          break;
        }
        if (p->xcoord == x2)
        {
          p_kind = 2;
          leave = it;
          break;
        }
        if (p->ycoord == y1)
        {
          p_kind = 1;
          leave = it;
          break;
        }
        if (p->ycoord == y2)
        {
          p_kind = 3;
          leave = it;
          break;
        }
      }
      if (leave != temp.end())
      {
        temp.push_back(curve_type());
        curve_type* c = &temp.back();
        c->push_back(leave->back());
        int q_kind;
        coord_type* q = 0;
        edge_type::iterator enter = temp.end();
        for (edge_type::iterator it = temp.begin(); it != temp.end(); ++it)
        {
          q = &ver[it->front()];
          if (q->xcoord == x1)
          {
            q_kind = 0;
            enter = it;
            break;
          }
          if (q->xcoord == x2)
          {
            q_kind = 2;
            enter = it;
            break;
          }
          if (q->ycoord == y1)
          {
            q_kind = 1;
            enter = it;
            break;
          }
          if (q->ycoord == y2)
          {
            q_kind = 3;
            enter = it;
            break;
          }
        }
        assert(enter != temp.end());

        coord_type gg[4] =
        {
          { x1, y1 },
          { x2, y1 },
          { x2, y2 },
          { x1, y2 }
        };
        while (p_kind != q_kind)
        {
          if (gg[p_kind].xcoord != q->xcoord || gg[p_kind].ycoord != q->ycoord)
          {
            c->push_back(ver.size());
            ver.push_back(coord_type());
            ver.back() = gg[p_kind];
          }
          p_kind = (p_kind + 1) % 4;
        }
        c->push_back(enter->front());
      }
      merge_curves(temp);
    }

#if 0
    {
      psstream out("curves.ps");
      out.selectmedia(psstream::FORMAT_A3, psstream::PORTRAIT);
      out.newpage();

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
      x0 -= sx * x1 / dx;
      y0 -= sy * y1 / dy;
      sx /= dx;
      sy /= dy;

      // edge_clip(x1, y1, x2, y2, ver, pgn);

      out.setlinewidth(0);
      for (edge_type::iterator e = temp.begin(); e != temp.end(); ++e)
      {
        curve_type::iterator c = e->begin(); 
        if (c != e->end())
        {
          out.moveto(x0 + sx * ver[*c].xcoord, y0 + sy * ver[*c].ycoord);
          while (++c != e->end())
          {
            out.lineto(x0 + sx * ver[*c].xcoord, y0 + sy * ver[*c].ycoord);
          }
        }
        out.stroke();
      }
    }
    return;
#endif

    // move the result in the output container
    assert(temp.size() == 1);
    for (edge_type::iterator e_iter = temp.begin(); e_iter != temp.end(); ++e_iter)
    {
      pgn.push_back(curve_type());
      c = &pgn.back();
      c->splice(c->end(), *e_iter);
    }

    // find Sample record for curve c
    leda::node n_site = leda_nil;
    leda::list_item it = p_label.first();
    while (it != leda_nil)
    {
      leda::point p = p_label.contents(it);
      Inpoly s = inside_curve(p.xcoord(), p.ycoord(), ver, *c);
			if (s == INSIDE || s == BOUNDARY)
      {
        n_site = p_data.lookup(p);
        break;
      }
      it = p_label.succ(it);
    }
    if (it == leda_nil)
    {
      leda::node n;
      forall_nodes (n, p_data)
      {
        leda::point p = p_data[n];
        Inpoly s = inside_curve(p.xcoord(), p.ycoord(), ver, *c);
				if (s == INSIDE || s == BOUNDARY)
        {
          n_site = n;
          break;
        }
      }
    }
    if (n_site != leda_nil)
    {
      int i = s_data[n_site];
      arr[i] = c;
      assert(is_simple(ver, *c));
    }
  }
}

#if 0

int main()
{
  std::vector<double> xx, yy;
  double x, y, z;
  while (std::cin >> x >> y >> z)
  {
    xx.push_back(x);
    yy.push_back(y);
  }

  double x1 = 1380.0, x2 = 1429.0;
  double y1 = 706.0, y2 = 785.0;
  vertex_type ver;
  edge_type pgn;
  int n = xx.size();
  std::vector<curve_type*> arr(n);
  voro(n, &xx[0], &yy[0], x1, x2, y1, y2, ver, pgn, &arr[0]);

  psstream out("voronoi.ps");
  out.selectmedia(psstream::FORMAT_A3, psstream::PORTRAIT);
  out.newpage();

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
  x0 -= sx * x1 / dx;
  y0 -= sy * y1 / dy;
  sx /= dx;
  sy /= dy;

  // edge_clip(x1, y1, x2, y2, ver, pgn);

  for (vertex_type::iterator v = ver.begin(); v != ver.end(); ++v)
  {
    v->xcoord = x0 + sx * v->xcoord;
    v->ycoord = y0 + sy * v->ycoord;
  }

  out.setlinewidth(0);
  plotpath(out, ver, pgn);
  out.stroke();

  for (int i = 0; i < n; ++i)
  {
    out.arc(x0 + sx * xx[i], y0 + sy * yy[i], 0.5, 0, 360);
    out.fill();
  }

  return 0;
}

#endif
