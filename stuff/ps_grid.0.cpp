#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <assert.h>
#include <float.h>

#include "ps_data.h"

bool read_grid(const char* name, grid_type* data)
{
  char ch[4];
  std::ifstream in(name, std::ios::binary);
  if (!in.read(ch, sizeof(ch)))
    return false;

  if (strncmp(ch, "DSBB", 4) == 0)
  {
    if (!in.read((char*) &data->cx, sizeof(data->cx)))
      return false;
    if (!in.read((char*) &data->cy, sizeof(data->cy)))
      return false;
    if (!in.read((char*) &data->x1, sizeof(data->x1)))
      return false;
    if (!in.read((char*) &data->x2, sizeof(data->x2)))
      return false;
    if (!in.read((char*) &data->y1, sizeof(data->y1)))
      return false;
    if (!in.read((char*) &data->y2, sizeof(data->y2)))
      return false;
    if (!in.read((char*) &data->z1, sizeof(data->z1)))
      return false;
    if (!in.read((char*) &data->z2, sizeof(data->z2)))
      return false;

    int nn = data->cx * data->cy;
    data->zz = new float[nn];
    in.read((char*) data->zz, nn * sizeof(float));
  }
  else if (strncmp(ch, "DSAA", 4) == 0)
  {
    std::ifstream in(name);
    std::string ch;
    if (in >> ch >> data->cx >> data->cy >> data->x1 >> data->x2 >> \
      data->y1 >> data->y2 >> data->z1 >> data->z2)
    {
      int nn = data->cx * data->cy;
      data->zz = new float[nn];
      int n = std::copy(std::istream_iterator<float>(in), \
          std::istream_iterator<float>(), data->zz) - data->zz;
      return (n == nn);
    }
    return false;
  }
  else if (strncmp(ch, "DSRB", 4) == 0)
  {
    long nn;
    if (!in.read((char*) &nn, sizeof(nn)) || nn != 4)
      return false;

    long ver;
    if (!in.read((char*) &ver, sizeof(ver)) || ver != 1)
      return false;

    if (!in.read((char*) ch, sizeof(ch)) || strncmp(ch, "GRID", 4) != 0)
      return false;

    if (!in.read((char*) &nn, sizeof(nn)) || nn != 9*8)
      return false;

    long cy;
    if (!in.read((char*) &cy, sizeof(cy)))
      return false;
    data->cy = (short) cy;

    long cx;
    if (!in.read((char*) &cx, sizeof(cx)))
      return false;
    data->cx = (short) cx;

    if (!in.read((char*) &data->x1, sizeof(data->x1)))
      return false;

    if (!in.read((char*) &data->y1, sizeof(data->y1)))
      return false;

    double dx;
    if (!in.read((char*) &dx, sizeof(dx)))
      return false;
    data->x2 = data->x1 + dx * (data->cx-1);

    double dy;
    if (!in.read((char*) &dy, sizeof(dy)))
      return false;
    data->y2 = data->y1 + dy * (data->cy-1);

    if (!in.read((char*) &data->z1, sizeof(data->z1)))
      return false;

    if (!in.read((char*) &data->z2, sizeof(data->z2)))
      return false;

    double rot;
    if (!in.read((char*) &rot, sizeof(rot)))
      return false;

    double bln;
    if (!in.read((char*) &bln, sizeof(bln)))
      return false;

    if (!in.read(ch, sizeof(ch)) || strncmp(ch, "DATA", 4) != 0)
      return false;

    if (!in.read((char*) &nn, sizeof(nn)) || nn != (long) (cx * cy * sizeof(double)))
      return false;

    nn = cx * cy;
    data->zz = new float[nn];
    for (int i = 0; i < nn; ++i)
    {
      double z;
      if (!in.read((char*) &z, sizeof(z)))
        return false;
      data->zz[i] = (float) z;
    }
  }
  else
  {
    return false;
  }
  return true;
}

void histeq(int n_data, const float* data, int n_level, float* level)
{
  int i, *pp = new int[n_data];
  float* zz = new float[n_data];
  float *z = zz;
  for (i = 0; i < n_data; ++i)
  {
    float d = data[i];
    if (!_isnan(d))
      *z++ = d;
  }
  n_data = (z - zz);
  // std::copy(data, data+n_data, zz);
  std::sort(zz, zz+n_data);
  int np = 0;
  pp[np] = 1;
  for (i = 1; i < n_data; ++i)
  {
    if (zz[i] != zz[np])
    {
      ++np;
      zz[np] = zz[i];
      pp[np] = 1;
    }
    else
    {
      ++pp[np];
    }
  }
  ++np;
  std::partial_sum(pp, pp+np, pp);
  float *ff = new float[np-1];
  for (i = 0; i < np-1; ++i)
  {
    // ff[i] = (float) pp[i] / n_data;
    ff[i] = (float) pp[i] / n_data;
  }
	level[0] = zz[0];
  for (i = 1; i < n_level-1; ++i)
  {
    float p = (float) i / n_level;
    assert(ff[0] <= p && p <= ff[np-2]);
    int n = std::lower_bound(ff, ff+np-1, p) - ff;
    if (p < ff[n])
    {
			float f0 = (n > 0? ff[n-1]: 0.0f);
      level[i] = zz[n] + (zz[n+1] - zz[n]) * (p - f0) / (ff[n] - f0);
    }
    else
    {
      level[i] = zz[n];
    }
  }
	level[i] = zz[np-1];
  delete[] ff;
  delete[] zz;
  delete[] pp;
}

bool sub(grid_type* g1, grid_type* g2, grid_type* g3)
{
  if (g1->cx != g2->cx || g1->cy != g2->cy || g1->x1 != g2->x1 || g1->y1 != g2->y1
    || g1->x2 != g2->x2 || g1->y2 != g2->y2)
    return false;

  g3->cx = g1->cx;
  g3->cy = g1->cy;
  g3->x1 = g1->x1;
  g3->y1 = g1->y1;
  g3->x2 = g1->x2;
  g3->y2 = g1->y2;
  
  int nn = g1->cx * g1->cy;
  g3->zz = new float[nn];
  std::transform(g2->zz, g2->zz+nn, g1->zz, g3->zz, std::minus<float>());
  g3->z1 = *std::min_element(g3->zz, g3->zz+nn);
  g3->z2 = *std::max_element(g3->zz, g3->zz+nn);
  return true;
}

#if 0

void main()
{
  grid_type g;
  g.zz = 0;
  if (!read_grid("d:\\dat\\depth-to-top.grd", &g))
  {
    std::cerr << "failed to load data\n";
    delete[] g.zz;
    return;
  }

  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);
  double pg_width = out.pagewidth();
  double pg_height = out.pageheight();
  double pg_margin = 40.0;
  double px = pg_width - 2.0 * pg_margin;
  double py = pg_height - 2.0 * pg_margin;
  double dx = g.x2 - g.x1;
  double dy = g.y2 - g.y1;
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
  color_type color[n] =
  {
    { 1.0, 0.5, 0.5 },
    { 1.0, 1.0, 0.5 },
    { 0.5, 1.0, 0.5 },
    { 0.5, 1.0, 1.0 },
    { 0.5, 0.5, 1.0 }
  };

#if 0
  histeq(g.cx*g.cy, g.zz, n, level);
#else
  for (int i = 0; i < n; ++i)
  {
    level[i] = g.z1 + (g.z2 - g.z1) * (i+1) / (n+1);
    // level[i] = g.z1 + (g.z2 - g.z1) * i / (n-1);
  }
#endif

  out.newpage();
  out.colorimage(x0, y0, sx, sy, n, level, color, g.cx, g.cy, g.zz);
  delete[] g.zz;
}

#endif
