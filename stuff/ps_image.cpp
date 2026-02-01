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
#include <float.h>
#include "ps_data.h"
#include "ps_priv.h"

inline void lerp(double t, const double* start, const double* final, double* value)
{
  *value = *start * (1 - t) + *final * t;
}

inline void lerp(double t, const color_type* start, const color_type* final, color_type* value)
{
  lerp(t, &start->r, &final->r, &value->r);
  lerp(t, &start->g, &final->g, &value->g);
  lerp(t, &start->b, &final->b, &value->b);
}

inline int cast_color(double value)
{
	assert(0.0 <= value && value <= 1.0);
  return int(255.0 * value + 0.5);
}

template <typename T>
const T* lookup_color(double value, int num_levels, const float* level, \
  const T* color, T* temp)
{
  if (_isnan(value))
  {
    return color+num_levels+2;
  }
  else if (value > level[num_levels-1])
  {
    return color+num_levels+1;
  }
  else if (value < level[0])
  {
    return color+num_levels;
  }
  num_levels = std::lower_bound(level, level + num_levels, value) - level;
  if (value == level[num_levels])
  {
    return color+num_levels;
  }
  lerp((value - level[num_levels-1]) / (level[num_levels] - level[num_levels-1]), \
  	color+num_levels-1, color+num_levels, temp);
  return temp;
}

void psstream_impl::colorimage(double x0, double y0, double sx, double sy, \
  int num_levels, const float* level, const color_type* color, \
  int cx, int cy, const float* elev)
{
  int k = 0, nn = cx * cy;
  double gx = cx / sx, gy = cy / sy;
  out << cx << ' ' << cy << " 8 [ " << gx << " 0 0 " << gy << ' ' 
    << -x0*gx << ' ' << -y0*gy << " ]\n{ currentfile " << 3*cx 
    << " string readhexstring pop } false 3 colorimage" 
    << std::hex << std::setfill('0');
  while (nn--)
  {
    color_type temp;
    const color_type* shade = lookup_color<color_type>(*elev++, num_levels, level, color, &temp);
    if (k++ % 40 == 0) out << '\n'; out << std::setw(2) << cast_color(shade->r);
    if (k++ % 40 == 0) out << '\n'; out << std::setw(2) << cast_color(shade->g);
    if (k++ % 40 == 0) out << '\n'; out << std::setw(2) << cast_color(shade->b);
  }
  out << std::setfill(' ') << std::dec << '\n';
  updatebb(x0, x0+sx, y0, y0+sy);
}

void psstream_impl::grayscaleimage(double x0, double y0, double sx, double sy, \
  int num_levels, const float* level, const double* color, \
  int cx, int cy, const float* elev)
{
  int k = 0, nn = cx * cy;
  double gx = cx / sx, gy = cy / sy;
  out << cx << ' ' << cy << " 8 [ " << gx << " 0 0 " << gy << ' ' 
    << -x0*gx << ' ' << -y0*gy << " ]\n{ currentfile " << cx 
    << " string readhexstring pop } image" 
    << std::hex << std::setfill('0');
  while (nn--)
  {
    double temp;
    const double* shade = lookup_color<double>(*elev++, num_levels, level, color, &temp);
    if (k++ % 40 == 0) out << '\n'; out << std::setw(2) << cast_color(*shade);
  }
  out << std::setfill(' ') << std::dec << '\n';
  updatebb(x0, x0+sx, y0, y0+sy);
}

void psstream_impl::bilevelimage(double x0, double y0, double sx, double sy, \
  int cx, int cy, const vertex_type& ver, const edge_type& pgn)
{
  shape_type ss;
  // edge_shape(ver, pgn, &ss);
  make_shape_list(ver, pgn, &ss);
  bbox_shape_list(ver, &ss);

  int s = (cx + 7) / 8;
  int nc = s * cy;
  unsigned char *ch = new unsigned char [nc];
  std::fill_n(ch, nc, (unsigned char) 0xff);
  double gx = cx / sx, gy = cy / sy;
  for (int j = 0; j < cy; ++j)
  {
    for (int i = 0; i < cx; ++i)
    {
      double x = x0+(i+0.5)/gx, y = y0+(j+0.5)/gy;
      if (inside_shape(x, y, ss))
      {
        ch[s*j+i/8] &= ~(1 << (7-i%8));
      }
    }
  }
  out << cx << ' ' << cy << " 1 [ " << gx << " 0 0 " << gy << ' ' 
    << -x0*gx << ' ' << -y0*gy << "]\n{<" << std::hex << std::setfill('0');
  for (j = 0; j < nc; ++j)
  {
    if ((j%40) == 0) out << '\n';
    out << std::setw(2) << (int) ch[j];
  }
  out << std::setfill(' ') << std::dec << "\n>}\nimage\n";
  delete[] ch;
  updatebb(x0, x0+sx, y0, y0+sy);
}

void psstream_impl::grayscaleimage(double x0, double y0, double sx, double sy, \
  int cx, int cy, const vertex_type& ver, const edge_type& pgn)
{
  int nc = cx*cy;
  unsigned char *ch = new unsigned char[nc];
  double gx = cx / sx, gy = cy / sy, gg = gx * gy;

  int i, j, k = 0;
  vertex_type w(ver);
  const edge_type* org_y = &pgn;
  edge_type strip1, strip2, *pos_y = &strip1;
  for (j = 1; j < cy; ++j)
  {
    edge_type neg_y;
    sutherland_hodgman(0.0, 1.0, -(y0 + j / gy), w, *org_y, *pos_y, neg_y);
    const edge_type* org_x = &neg_y;
    edge_type strip3, strip4, *pos_x = &strip3;
    for (i = 1; i < cx; ++i)
    {
      edge_type neg_x;
      sutherland_hodgman(1.0, 0.0, -(x0 + i / gx), w, *org_x, *pos_x, neg_x);
      ch[k++] = (unsigned char) (0xff * (1.0 - edge_area(w, neg_x) * gg) + 0.5);
      org_x = pos_x;
      pos_x = (pos_x == &strip3? &strip4: &strip3);
    }
    ch[k++] = (unsigned char) (0xff * (1.0 - edge_area(w, *pos_x) * gg) + 0.5);
    org_y = pos_y;
    pos_y = (pos_y == &strip1? &strip2: &strip1);
  }
  {
    const edge_type* org_x = org_y;
    edge_type strip3, strip4, *pos_x = &strip3;
    for (i = 1; i < cx; ++i)
    {
      edge_type neg_x;
      sutherland_hodgman(1.0, 0.0, -(x0 + i / gx), w, *org_x, *pos_x, neg_x);
      ch[k++] = (unsigned char) (0xff * (1.0 - edge_area(w, neg_x) * gg) + 0.5);
      org_x = pos_x;
      pos_x = (pos_x == &strip3? &strip4: &strip3);
    }
    ch[k++] = (unsigned char) (0xff * (1.0 - edge_area(w, *pos_x) * gg) + 0.5);
  }

  out << cx << ' ' << cy << " 8 [ " << gx << " 0 0 " << gy << ' ' 
    << -x0*gx << ' ' << -y0*gy << " ]\n{ currentfile " << cx 
    << " string readhexstring pop } image" << std::hex << std::setfill('0');
  for (k = 0; k < nc; ++k)
  {
    if ((k%40) == 0) out << '\n';
    out << std::setw(2) << (int) ch[k];
  }
  out << std::setfill(' ') << std::dec << '\n';
  delete[] ch;
  updatebb(x0, x0+sx, y0, y0+sy);
}

/////////////////////////////////////////////////////////////////////

void psstream::colorimage(double x0, double y0, double sx, double sy, \
  int num_levels, const float* level, const color_type* color, \
  int cx, int cy, const float* elev)
{
  impl->colorimage(x0, y0, sx, sy, num_levels, level, color, cx, cy, elev);
}

void psstream::grayscaleimage(double x0, double y0, double sx, double sy, \
  int num_levels, const float* level, const double* color, \
  int cx, int cy, const float* elev)
{
  impl->grayscaleimage(x0, y0, sx, sy, num_levels, level, color, cx, cy, elev);
}

void psstream::bilevelimage(double x0, double y0, double sx, double sy, \
  int cx, int cy, const vertex_type& ver, const edge_type& pgn)
{
  impl->bilevelimage(x0, y0, sx, sy, cx, cy, ver, pgn);
}

void psstream::grayscaleimage(double x0, double y0, double sx, double sy, \
  int cx, int cy, const vertex_type& ver, const edge_type& pgn)
{
  impl->grayscaleimage(x0, y0, sx, sy, cx, cy, ver, pgn);
}

/////////////////////////////////////////////////////////////////////

void edge_clip(double x1, double y1, double x2, double y2, vertex_type& ver, edge_type& pgn)
{
  edge_type pos, neg;
  sutherland_hodgman(0.0, 1.0, -y1, ver, pgn, pos, neg);
  sutherland_hodgman(-1.0, 0.0, x2, ver, pos, pgn, neg);
  sutherland_hodgman(0.0, -1.0, y2, ver, pgn, pos, neg);
  sutherland_hodgman(1.0, 0.0, -x1, ver, pos, pgn, neg);
}

// implementetion of subject 2.01 from graphics algorithms FAQ
double edge_area(const vertex_type& ver, const edge_type& ee)
{
  double a = 0.0;
  for (edge_type::const_iterator e = ee.begin(); e != ee.end(); ++e)
  {
    const curve_type& cc = *e;
    curve_type::const_iterator prev = cc.begin(), curr = prev;
    while (++curr != cc.end())
    {
      curve_type::const_iterator next = curr;
      if (++next == cc.end())
        next = ++cc.begin();
      a += ver[*curr].xcoord * (ver[*next].ycoord - ver[*prev].ycoord);
      prev = curr;
    }
  }
  return (0.5 * a);
}

void edge_mask(const vertex_type& ver, const edge_type& pgn, grid_type* g)
{
  int cx = g->cx;
  int cy = g->cy;
  int nz = cx * cy;
  float *zz = new float[nz];
  double x0 = g->x1;
  double sx = g->x2 - x0;
  double y0 = g->y1;
  double sy = g->y2 - y0;
  double gx = cx / sx;
  double gy = cy / sy;
  double gg = gx * gy;
  g->z1 = 0.0f;
  g->z2 = 1.0f;
  g->zz = zz;

  int i, j;
  vertex_type w(ver);
  const edge_type* org_y = &pgn;
  edge_type strip1, strip2, *pos_y = &strip1;
  for (j = 1; j < cy; ++j)
  {
    edge_type neg_y;
    sutherland_hodgman(0.0, 1.0, -(y0 + j / gy), w, *org_y, *pos_y, neg_y);
    const edge_type* org_x = &neg_y;
    edge_type strip3, strip4, *pos_x = &strip3;
    for (i = 1; i < cx; ++i)
    {
      edge_type neg_x;
      sutherland_hodgman(1.0, 0.0, -(x0 + i / gx), w, *org_x, *pos_x, neg_x);
      *zz++ = float(edge_area(w, neg_x) * gg);
      org_x = pos_x;
      pos_x = (pos_x == &strip3? &strip4: &strip3);
    }
    *zz++ = float(edge_area(w, *pos_x) * gg);
    org_y = pos_y;
    pos_y = (pos_y == &strip1? &strip2: &strip1);
  }
  {
    const edge_type* org_x = org_y;
    edge_type strip3, strip4, *pos_x = &strip3;
    for (i = 1; i < cx; ++i)
    {
      edge_type neg_x;
      sutherland_hodgman(1.0, 0.0, -(x0 + i / gx), w, *org_x, *pos_x, neg_x);
      *zz++ = float(edge_area(w, neg_x) * gg);
      org_x = pos_x;
      pos_x = (pos_x == &strip3? &strip4: &strip3);
    }
    *zz = float(edge_area(w, *pos_x) * gg);
  }
}

void blankimage(const grid_type* g, int ilb, int iub, int jlb, int jub, \
	vertex_type& ver, edge_type& pgn)
{
  int cx = g->cx;
  int cy = g->cy;
  int *nn = new int [2 * (cx+1)], *n1 = nn, *n2 = nn + (cx+1);
  std::fill(nn, nn+2*(cx+1), -1);
  for (int j = jlb; j <= jub; ++j)
  {
    for (int i = ilb; i <= iub; ++i)
    {
      int k = j*cx+i;
			if (!_isnan(g->zz[k]))
			{
  			if (j == jlb || _isnan(g->zz[k-cx]))
  			{
  			  if (n1[i] == -1)
  			  {
  			    n1[i] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i;
  			    p->ycoord = j;
  			  }
  			  if (n1[i+1] == -1)
  			  {
  			    n1[i+1] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i+1;
  			    p->ycoord = j;
  			  }
  			  pgn.push_back(curve_type());
  			  curve_type* cur = &pgn.back();
  			  cur->push_back(n1[i]);
  			  cur->push_back(n1[i+1]);
  			}
  			if (j == jub || _isnan(g->zz[k+cx]))
  			{
  			  if (n2[i+1] == -1)
  			  {
  			    n2[i+1] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i+1;
  			    p->ycoord = j+1;
  			  }
  			  if (n2[i] == -1)
  			  {
  			    n2[i] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i;
  			    p->ycoord = j+1;
  			  }
  			  pgn.push_back(curve_type());
  			  curve_type* cur = &pgn.back();
  			  cur->push_back(n2[i+1]);
  			  cur->push_back(n2[i]);
  			}
  			if (i == ilb || _isnan(g->zz[k-1]))
  			{
  			  if (n2[i] == -1)
  			  {
  			    n2[i] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i;
  			    p->ycoord = j+1;
  			  }
  			  if (n1[i] == -1)
  			  {
  			    n1[i] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i;
  			    p->ycoord = j;
  			  }
  			  pgn.push_back(curve_type());
  			  curve_type* cur = &pgn.back();
  			  cur->push_back(n2[i]);
  			  cur->push_back(n1[i]);
  			}
  			if (i == iub || _isnan(g->zz[k+1]))
  			{
  			  if (n1[i+1] == -1)
  			  {
  			    n1[i+1] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i+1;
  			    p->ycoord = j;
  			  }
  			  if (n2[i+1] == -1)
  			  {
  			    n2[i+1] = ver.size();
  			    ver.push_back(coord_type());
  			    coord_type* p = &ver.back();
  			    p->xcoord = i+1;
  			    p->ycoord = j+1;
  			  }
  			  pgn.push_back(curve_type());
  			  curve_type* cur = &pgn.back();
  			  cur->push_back(n1[i+1]);
  			  cur->push_back(n2[i+1]);
  			}
			}
    }
    std::swap(n1, n2);
    std::fill(n2, n2+cx+1, -1);
  }
  merge_curves(pgn);
  delete[] nn;
}

#if 0

// noise type 2, flood land area
void main()
{
  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);

  double x1 = 1388105.0, x2 = 1428355.0, dx = x2 - x1;
  double y1 = 706905.0, y2 = 784655.0, dy = y2 - y1;
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

  clock_t t = clock();
  noise_type nod;
  vertex_type ver;
  edge_type pgn;
  noise_type *org[1] = { &nod };
  double ext1[1] = { 2.2 }, ext2[1] = { 6.667 };

  std::clog << "noise generation\n";
  make_noise(x1, y1, x2, y2, 750.0, 75.0, 10.0, 39.79, 5.0, ver, pgn, 1, org, ext1, ext2);

  std::clog << "coordinate conversion\n";
  convert(x0 - sx * x1 / dx, y0 - sy * y1 / dy, sx / dx, sy / dy, ver);

  std::clog << "clipping at image boundaries\n";
  edge_clip(x0, y0, x0+sx, y0+sy, ver, pgn);

  std::clog << "image output\n";
  out.newpage();
  out.setlinewidth(0);
  for (edge_type::iterator it = pgn.begin(); it != pgn.end(); ++it)
  {
    curve_type::iterator g = it->begin();
    coord_type* p = &ver[*g];
    out.moveto(p->xcoord, p->ycoord);
    while (++g != it->end())
    {
      p = &ver[*g];
      out.lineto(p->xcoord, p->ycoord);
    }
    out.stroke();
  }
  out.setgray(0);
  out.rectstroke(x0, y0, sx, sy);

  out.newpage();
  out.grayscaleimage(x0, y0, sx, sy, 350, 650, ver, pgn);
  out.setgray(0);
  out.rectstroke(x0, y0, sx, sy);

  out.newpage();
  out.bilevelimage(x0, y0, sx, sy, 350, 650, ver, pgn);
  out.setgray(0);
  out.rectstroke(x0, y0, sx, sy);

  std::clog << "done, " << double(clock() - t) / CLOCKS_PER_SEC << " sec elapsed\n";
}

#endif
