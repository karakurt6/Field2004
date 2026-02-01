#include <iostream>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "ps_data.h"
#include "demo.h"

void check_clip(psstream& out, const char* file)
{
  int k;
  grid_type g;
  if (read_grid(file, &g))
  {
    const int n = 15;
    float level[n];
    for (k = 0; k < n; ++k)
      level[k] = float(g.z1 + (k+1) * (g.z2-g.z1) / (n+1));

    vertex_type ver;
    edge_type me[n], ce[n+2];
    marching_squares(g.cx, g.cy, 0, g.cx-1, 0, g.cy-1, g.zz, n, level, ver, me, ce);
    delete[] g.zz;

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
  }
}

void plot_polygon_nodes(const edge_type& pgn)
{
  for (edge_type::const_iterator g = pgn.begin(); g != pgn.end(); ++g)
  {
    for (curve_type::const_iterator n = g->begin(); n != g->end(); ++n)
    {
      std::cout << *n << ' ';
    }
    std::cout << '\n';
  }
  std::cout << '\n';
}

void plot_polygon_vertex(const vertex_type& ver, const edge_type& pgn)
{
  std::cout << '<' << '\n';
  for (edge_type::const_iterator g = pgn.begin(); g != pgn.end(); ++g)
  {
    for (curve_type::const_iterator n = g->begin(); n != g->end(); ++n)
    {
      const coord_type* p = &ver[*n];
      std::cout << *n << ' ' << p->xcoord << ' ' << p->ycoord << '\n';
    }
    std::cout << '\n';
  }
}

void plot_clip_convex(const vertex_type& ver_convex, const curve_type& cur_convex, \
  const vertex_type& ver, const edge_type& pgn, vertex_type& ver_clipped, edge_type& pgn_clipped)
{
  psstream out("clip.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);

  assert(is_convex(ver_convex, cur_convex));
  assert(is_generalized_polygon(ver, pgn));

  ver_clipped = ver;
  pgn_clipped = pgn;
  
  curve_type::const_iterator iter = cur_convex.begin();
  const coord_type* prev = &ver_convex[*iter];
  int count = 0;
  while (++iter != cur_convex.end())
  {
    if (pgn_clipped.empty())
      break;

    out.newpage();
    out.setgray(0.0);
    out.setlinewidth(1.0);
    plotpath(out, ver_clipped, pgn_clipped);
    out.stroke();

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

    // plot_polygon_nodes(pos);
    plot_polygon_vertex(ver_clipped, neg);

    assert(is_generalized_polygon(ver_clipped, pos));
    assert(is_generalized_polygon(ver_clipped, neg));

    if (!pos.empty())
    {
      plotpath(out, ver_clipped, pos);
      out.setrgbcolor(1.0, 0.8, 0.8);
      out.fill();
    }
    if (!neg.empty())
    {
      plotpath(out, ver_clipped, neg);
      out.setrgbcolor(0.8, 0.8, 1.0);
      out.fill();
    }

    for (edge_type::iterator g = pgn_clipped.begin(); g != pgn_clipped.end(); ++g)
    {
      curve_type::iterator n = g->begin(); 
      while (++n != g->end())
      {
        coord_type* p = &ver_clipped[*n];
        double s = a * p->xcoord + b * p->ycoord + c;
        if (s < 0.0)
        {
          out.setrgbcolor(0.0, 0.0, 1.0);
        }
        else if (s > 0.0)
        {
          out.setrgbcolor(1.0, 0.0, 0.0);
        }
        else
        {
          out.setrgbcolor(0.0, 1.0, 0.0);
        }
        out.arc(p->xcoord, p->ycoord, 2.0, 0.0, 360.0);
        out.fill();
      }
    }

    out.setgray(0);
    out.setlinewidth(0);
    out.horizontal_scale(0, 10, 400, 0, 400, 400, 100, 100, 600);
    out.vertical_scale(0, 10, 600, 0, 600, 600, 100, 100, 400);

    out.setlinewidth(0.1);
    out.horizontal_scale(0, 50, 400, 0, 400, 400, 100, 100, 600);
    out.vertical_scale(0, 50, 600, 0, 600, 600, 100, 100, 400);

    out.setlinewidth(0.5);
    out.horizontal_scale(0, 100, 400, 0, 400, 400, 100, 100, 600);
    out.vertical_scale(0, 100, 600, 0, 600, 600, 100, 100, 400);

    out.setgray(0);
    out.setlinewidth(1.0);
    out.moveto(prev->xcoord, prev->ycoord);
    out.lineto(curr->xcoord, curr->ycoord);
    out.stroke();

    prev = curr;
    pgn_clipped = neg;

    /*
    if (++count == 5)
      break;
    */
  }
}

void check_clip_convex()
{
  coord_type pp[] = 
  {
    { 4, 4 },
    { 8, 6 },
    { 10, 10 },
    { 9, 16 },
    { 5, 15 },
    { 3, 10 },
    { 7, 8 },
    { 9, 9 },
    { 10, 13 },
    { 13, 16 },
    { 14, 19 },
    { 11, 19 },
    { 7, 16 },
    { 6, 11 },
    { 8, 12 },
    { 8, 14 },
    { 12, 18 },
    { 12, 16 }
  };

  vertex_type ver(pp, pp + countof(pp));
  convert(100.0, 100.0, 10.0, 10.0, ver);

  int g1[] =
  {
    0, 1, 2, 3, 4, 5, 0
  };

  curve_type cur(g1, g1 + countof(g1));


  int g2[] =
  {
    6, 7, 8, 9, 10, 11, 12, 13, 6
  };
  int g3[] =
  {
    14, 15, 16, 17, 14
  };

  edge_type pgn;
  pgn.push_back(curve_type(g2, g2 + countof(g2)));
  pgn.push_back(curve_type(g3, g3 + countof(g3)));

  assert(is_generalized_polygon(ver, pgn));

  vertex_type ver_clipped = ver;
  edge_type pgn_clipped = pgn;
  // plot_clip_convex(ver, cur, ver, pgn, ver_clipped, pgn_clipped);
  clip_convex(ver, cur, ver, pgn, ver_clipped, pgn_clipped);
  
  // edge_clip(5, 10, 10, 15, ver_clipped, pgn_clipped);
  // assert(is_generalized_polygon(ver_clipped, pgn_clipped));

  for (edge_type::iterator e = pgn_clipped.begin(); e != pgn_clipped.end(); ++e)
  {
    for (curve_type::iterator c = e->begin(); c != e->end(); ++c)
    {
      coord_type* p = &ver_clipped[*c];
      std::cout << p->xcoord << ' ' << p->ycoord << '\n';
    }
    std::cout << '\n';
  }
}
