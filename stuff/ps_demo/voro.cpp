#include <string>
#include <sstream>
#include <fstream>
#include <limits>
#include <math.h>
#include <time.h>
#include "ps_data.h"
#include "demo.h"
#pragma comment(lib, "leda_md")

void check_voro(psstream& out, const char* file)
{
  std::ifstream in(file);
  std::vector<double> xx, yy;
  double x1 = std::numeric_limits<double>::max(), x2 = -x1, y1 = x1, y2 = x2;
  std::string line;
  while (std::getline(in, line))
  {
    std::istringstream ss(line);
    double x, y;
    if (ss >> x >> y)
    {
      xx.push_back(x);
      yy.push_back(y);
      if (x < x1) x1 = x;
      if (x > x2) x2 = x;
      if (y < y1) y1 = y;
      if (y > y2) y2 = y;
    }
  }

  x1 = floor(x1);
  x2 = ceil(x2);
  y1 = floor(y1);
  y2 = ceil(y2);

  vertex_type ver;
  edge_type pgn;
  int n = xx.size();
  std::vector<curve_type*> arr(n);
  voro(n, &xx[0], &yy[0], x1, x2, y1, y2, ver, pgn, &arr[0]);

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

  convert(x0, y0, sx, sy, ver);
  /*
  for (vertex_type::iterator v = ver.begin(); v != ver.end(); ++v)
  {
    v->xcoord = x0 + sx * v->xcoord;
    v->ycoord = y0 + sy * v->ycoord;
  }
  */

  out.setgray(0.9);
  for (int i = 0; i < n; ++i)
  {
    if (!arr[i])
      continue;

    curve_type::iterator it = arr[i]->begin();
    coord_type* p = &ver[*it];
    out.moveto(p->xcoord, p->ycoord);
    while (++it != arr[i]->end())
    {
      p = &ver[*it];
      out.lineto(p->xcoord, p->ycoord);
    }
    out.fill();
  }
  // plotpath(out, ver, pgn);

  out.setgray(0.0);
  out.setlinewidth(0.0);
  plotpath(out, ver, pgn);
  out.stroke();

  for (int i = 0; i < n; ++i)
  {
    if (arr[i])
    {
      out.arc(x0 + sx * xx[i], y0 + sy * yy[i], 0.5, 0, 360);
      out.fill();
    }
    else
    {
      out.setrgbcolor(1.0, 0.0, 0.0);
      out.arc(x0 + sx * xx[i], y0 + sy * yy[i], 0.5, 0, 360);
      out.fill();
      out.setgray(0.0);
    }
  }

#if 0
  for (int i = 0; i < n; ++i)
  {
    out.newpage();
    curve_type::iterator c = arr[i]->begin();
    out.moveto(ver[*c].xcoord, ver[*c].ycoord);
    while (++c != arr[i]->end())
      out.lineto(ver[*c].xcoord, ver[*c].ycoord);
    out.stroke();
  }
#endif
}
