#include "ps_data.h"
#include "demo.h"
#include "math.h"
#include "stdlib.h"
#include "time.h"
#include "limits"

extern "C"
{
  void SRAND();
  double DRAND();
}

static void bcirc(int nr, int x1[1], int x2[1])
{
  int nd = 2*nr+1;
  for (int i = 0; i < nd; ++i)
  {
    x1[i] = x2[i] = 0;
  }

  // инициализируем вспомогательные массивы для
  // поиска в окрестности узла сетки,
  // применяем алгоритм Брозенхема для окружности 
  for (int x = 0, y = nr, dec = 3-2*nr; x <= y; x++)
  {
    if (x1[nr+y] == 0 || x1[nr+y] > x) x1[nr+y] = x;
    if (x2[nr+y] == 0 || x2[nr+y] < x) x2[nr+y] = x;

    if (x1[nr-y] == 0 || x1[nr-y] > x) x1[nr-y] = x;
    if (x2[nr-y] == 0 || x2[nr-y] < x) x2[nr-y] = x;

    if (x1[nr+y] == 0 || x1[nr+y] > -x) x1[nr+y] = -x;
    if (x2[nr+y] == 0 || x2[nr+y] < -x) x2[nr+y] = -x;

    if (x1[nr-y] == 0 || x1[nr-y] > -x) x1[nr-y] = -x;
    if (x2[nr-y] == 0 || x2[nr-y] < -x) x2[nr-y] = -x;

    if (x1[nr+x] == 0 || x1[nr+x] > y) x1[nr+x] = y;
    if (x2[nr+x] == 0 || x2[nr+x] < y) x2[nr+x] = y;

    if (x1[nr-x] == 0 || x1[nr-x] > y) x1[nr-x] = y;
    if (x2[nr-x] == 0 || x2[nr-x] < y) x2[nr-x] = y;

    if (x1[nr+x] == 0 || x1[nr+x] > -y) x1[nr+x] = -y;
    if (x2[nr+x] == 0 || x2[nr+x] < -y) x2[nr+x] = -y;

    if (x1[nr-x] == 0 || x1[nr-x] > -y) x1[nr-x] = -y;
    if (x2[nr-x] == 0 || x2[nr-x] < -y) x2[nr-x] = -y;

    if (dec >= 0) dec += -4*(y--)+4;

    dec += 4*x+6;
  }
}

void blank_disk(int i, int j, int r, grid_type* g)
{
  int n = 2 * r + 1;
  int* x1 = new int[n];
  int* x2 = new int[n];
  bcirc(r, x1, x2);
  for (int jj = j - r; jj <= j + r; ++jj)
  {
    if (jj  < 0 || jj >= g->cy)
      continue;
    
    n = jj - j + r;
    for (int ii = i + x1[n]; ii <= i + x2[n]; ++ii)
    {
      if (ii < 0 || ii >= g->cx)
        continue;

      g->zz[jj * g->cx + ii] = std::numeric_limits<float>::quiet_NaN();
    }
  }
  delete[] x2;
  delete[] x1;
}

void check_conrec_1(psstream& out, const char* file)
{
  int k;
  grid_type g;
  if (!read_grid(file, &g))
  {
    return;
  }

#if 0
  SRAND();
  blank_disk(int(g.cx - 1) * DRAND(), int(g.cy - 1) * DRAND(), 10, &g);
#else
  blank_disk(18, 19, 10, &g);
#endif

  double minor_dz = 10.0;
  double minor_width = 1.0;
  double z1 = ceil(g.z1 / minor_dz) * minor_dz;
  double z2 = floor(g.z2 / minor_dz) * minor_dz;
  int n = (int) ((z2 - z1) / minor_dz) + 1;
  
  float level[2];
  level[0] = (float) g.z1;
  level[1] = (float) g.z2;

  double color[5];
  color[0] = 0.3;
  color[1] = 0.8;
  color[2] = 0.1;
  color[3] = 0.9;
  color[4] = 1.0;

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


  out.newpage();

  vertex_type ver;
  edge_type pgn;
  blankimage(&g, 0, g.cx-1, 0, g.cy-1, ver, pgn);
  convert(x0, y0, sx / g.cx, sy / g.cy, ver);
  plotpath(out, ver, pgn);
  out.clip();

  out.setrgbcolor(0.0, 0.0, 1.0);
  out.rectfill(x0-10, y0-10, sx+20, sy+20);

  out.newpage();
  out.grayscaleimage(x0, y0, sx, sy, 2, level, color, g.cx, g.cy, g.zz);
  out.setgray(0.0);
  plotpath(out, ver, pgn);
  out.stroke();

  delete[] g.zz;
}

void check_conrec(psstream& out, const char* file)
{
  int k;
  grid_type g;
  if (!read_grid(file, &g))
  {
    return;
  }

  blank_disk(18, 19, 10, &g);

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

  out.newpage();

  double minor_dz = 10.0;
  double minor_width = 1.0;
  double z1 = ceil(g.z1 / minor_dz) * minor_dz;
  double z2 = floor(g.z2 / minor_dz) * minor_dz;
  int n = (int) ((z2 - z1) / minor_dz) + 1;
  float *level = new float[n];
  for (k = 0; k < n; ++k)
    level[k] = float(z1+k*(z2-z1)/(n-1));

  int major_step = 5;
  double major_width = 2.0;
  double major_dz = major_step * minor_dz;
  int first_major = (int) (ceil(z1 / major_dz) * major_dz - z1) / minor_dz;

  vertex_type ver;
  edge_type *me = new edge_type[n];
  edge_type *ce = new edge_type[n+2];
  marching_squares(g.cx, g.cy, 0, g.cx-1, 0, g.cy-1, g.zz, n, level, ver, me, ce);
  delete[] g.zz;

  convert(x0, y0, sx/(g.cx-1), sy/(g.cy-1), ver);

  for (k = 0; k <= n; ++k)
  {
    if (ce[k].empty())
      continue;

    out.setgray(0.5 + 0.5 * double(k+1) / (n + 2));
    out.newpath();
    plotpath(out, ver, ce[k]);
    out.fill();
  }

  out.setgray(0.0);
  out.setlinewidth(minor_width);
  for (k = 0; k < first_major; ++k)
  {
    plotpath(out, ver, me[k]);
    hatch_edge(out, minor_width, 20, ver, me[k]);
    out.stroke();
  }
  while (k < n)
  {
    out.setlinewidth(major_width);
    plotpath(out, ver, me[k]);
    hatch_edge(out, major_width, 20, ver, me[k]);
    out.stroke();
    out.setlinewidth(minor_width);
    for (int j = k+1; j < n && j < k+major_step; ++j)
    {
      plotpath(out, ver, me[j]);
      hatch_edge(out, minor_width, 20, ver, me[j]);
      out.stroke();
    }
    k = j;
  }

  // out.setlinewidth(1.0);
  // out.rectstroke(x0, y0, sx, sy); 

  delete[] ce;
  delete[] me;
  delete[] level;
}

void check_conrec_3(psstream& out, const char* file)
{
  grid_type g;
  if (read_grid(file, &g))
  {
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

		out.newpage();
    out.setfont("Helvetica", 4);
    out.setgray(0.0);
    auto_conrec(out, i1, i2, j1, j2, &g, x0, y0, sx, sy);

    out.setgray(0.0);
    out.setlinewidth(1.0);
    out.rectstroke(x0, y0, sx, sy); 

    delete[] g.zz;
  }
}

void check_bitmap(psstream& out, const char* file)
{
	grid_type grid;
	grid.zz = 0;
	if (read_grid(file, &grid))
	{
		out.newpage();

    double x1 = grid.x1, x2 = grid.x2, dx = x2 - x1;
    double y1 = grid.y1, y2 = grid.y2, dy = y2 - y1;
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

    const int n = 5;
    float level[n];
    color_type color[n+3] =
    {
      { 1.0, 0.5, 0.5 },
      { 1.0, 1.0, 0.5 },
      { 0.5, 1.0, 0.5 },
      { 0.5, 1.0, 1.0 },
      { 0.5, 0.5, 1.0 },
      { 1.0, 0.5, 0.5 },
      { 0.5, 0.5, 1.0 },
      { 0.8, 0.8, 0.8 }
    };

#if 0
    histeq(grid.cx*grid.cy, grid.zz, n, level);
#else
    for (int i = 0; i < n; ++i)
    {
      level[i] = grid.z1 + (grid.z2 - grid.z1) * (i+1) / (n+1);
      // level[i] = g.z1 + (g.z2 - g.z1) * i / (n-1);
    }
#endif
    
    out.colorimage(x0, y0, sx, sy, n, level, color, grid.cx, grid.cy, grid.zz);

    float func[10*(n-1)+1];
    for (int k = 0; k < n-1; ++k)
    {
    	for (int kk = 0; kk < 10; ++kk)
    	{
    		func[10*k+kk] = level[k]+(level[k+1]-level[k]) * kk/10;
    	}
    }
    func[10*k] = level[k];

    out.colorimage(x0+sx+2, y0+10.0, 10.0*GOLDEN_RATIO, 10.0*(n-1)+1, n, \
    	level, color, 1, countof(func), func);

		delete[] grid.zz;
	}
}
