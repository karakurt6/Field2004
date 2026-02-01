#include <iostream>
#include <iomanip>
#include "ps_data.h"
#include "demo.h"

void check_incurve()
{
  vertex_type v;
  coord_type p;
  curve_type c;
  edge_type e;

  p.xcoord = 0.0;
  p.ycoord = 0.0;
  v.push_back(p);
  c.push_back(0);

  p.xcoord = 3.0;
  p.ycoord = 0.0;
  v.push_back(p);
  c.push_back(1);

  p.xcoord = 3.0;
  p.ycoord = 3.0;
  v.push_back(p);
  c.push_back(2);

  p.xcoord = 0.0;
  p.ycoord = 3.0;
  v.push_back(p);
  c.push_back(3);

  c.push_back(0);
  e.push_back(c);
  c.clear();

  p.xcoord = 1.0;
  p.ycoord = 1.0;
  v.push_back(p);
  c.push_back(4);

  p.xcoord = 1.0;
  p.ycoord = 2.0;
  v.push_back(p);
  c.push_back(5);

  p.xcoord = 2.0;
  p.ycoord = 2.0;
  v.push_back(p);
  c.push_back(6);

  p.xcoord = 2.0;
  p.ycoord = 1.0;
  v.push_back(p);
  c.push_back(7);

  c.push_back(4);
  e.push_back(c);

  std::cout << inpoly_table[inside_edge(1.5, 1.5, v, e)] << std::endl;
  std::cout << inpoly_table[inside_edge(0.5, 0.5, v, e)] << std::endl;
  std::cout << inpoly_table[inside_edge(0.0, 0.5, v, e)] << std::endl;

  std::cout << inpoly_table[inside_edge(0.0, 0.0, v, e)] << std::endl;
  std::cout << inpoly_table[inside_edge(0.0, 3.0, v, e)] << std::endl;
  std::cout << inpoly_table[inside_edge(3.0, 3.0, v, e)] << std::endl;
  std::cout << inpoly_table[inside_edge(3.0, 0.0, v, e)] << std::endl;

  psstream out("ss_hull.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);
  out.newpage();

  double x1 = 0.0, x2 = 3.0;
  double y1 = 0.0, y2 = 3.0;
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

  convert(x0, y0, sx, sy, v);
  plotpath(out, v, e);
  out.setgray(0.8);
  out.fill();
}

