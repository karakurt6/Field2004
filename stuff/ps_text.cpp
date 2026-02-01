#define _USE_MATH_DEFINES
#ifndef __INTEL_COMPILER
  #include <math.h>
#else
  #include <mathimf.h>
#endif

#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include "ps_data.h"
#include "ps_priv.h"

#define eps 0.0001

back_polygon::back_polygon(psstream& o): out(o)
{
}

int back_polygon::new_vertex(double x, double y)
{
  int n = ver.size();
  ver.push_back(coord_type());
  coord_type* p = &ver.back();
  p->xcoord = x;
  p->ycoord = y;
  return n;
}

void back_polygon::begin_curve()
{
  ver.clear();
  cur.clear();
}

void back_polygon::add_vertex(int n)
{
  cur.push_back(n);
}

void back_polygon::end_curve()
{
  out.setrgbcolor(1.0, 1.0, 0.5);
  out.newpath();
  curve_type::iterator c = cur.begin();
  const coord_type* p = &ver[*c];
  out.moveto(p->xcoord, p->ycoord);
  while (++c != cur.end())
  {
    p = &ver[*c];
    out.lineto(p->xcoord, p->ycoord);
  }
  out.fill();
  out.setgray(0.0);
}

void plotpath(psstream& out, const vertex_type& ver, const edge_type& ee)
{
  assert(!ee.empty());
  for (edge_type::const_iterator e = ee.begin(); e != ee.end(); ++e)
  {
    const curve_type& c = *e;
    curve_type::const_iterator q = c.begin();
    const coord_type* p = &ver[*q];
    out.moveto(p->xcoord, p->ycoord);
    while (++q != c.end())
    {
      p = &ver[*q];
      out.lineto(p->xcoord, p->ycoord);
    }
    if (is_closed(c))
    {
      out.closepath();
    }
  }
}

class s_curve
{
  const int n;
  bool closed;
  double *xx;
  double *yy;
  double *ss;

public:
  s_curve(const vertex_type& ver, const curve_type& cur);
  ~s_curve();
  
  double length() const;
  void position(double s, double *x, double *y) const;
  bool forward(double s, double w, double *g, double *dg) const;
  bool backward(double s, double w, double *g, double *dg) const;
  bool is_closed() const;
};

s_curve::s_curve(const vertex_type& ver, const curve_type& cur): 
  n(cur.size()), closed(cur.front() == cur.back()), 
  xx(new double[n]), yy(new double[n]), ss(new double[n])
{
  int i = 0;
  for (curve_type::const_iterator c = cur.begin(); c != cur.end(); ++c, ++i)
  {
    int j = *c;
    xx[i] = ver[j].xcoord;
    yy[i] = ver[j].ycoord;
  }
  ss[0] = 0.0;
  for (i = 1; i < n; ++i)
  {
    double dx = xx[i] - xx[i-1];
    double dy = yy[i] - yy[i-1];
    double dd = hypot(dx, dy);
    ss[i] = ss[i-1] + dd;
  }
}

s_curve::~s_curve()
{
  delete[] ss;
  delete[] yy;
  delete[] xx;
}

double s_curve::length() const
{
  return ss[n-1];
}

bool  s_curve::is_closed() const
{
  return closed;
}

void s_curve::position(double s, double *x, double *y) const
{
  if (is_closed()) s = modulo_range(s, length());
  assert(s >= 0.0 && s <= length());
  int i = std::lower_bound(ss, ss+n-1, s) - ss;
  if (s == ss[i])
  {
    *x = xx[i];
    *y = yy[i];
  }
  else
  {
    double t = (s - ss[i-1]) / (ss[i] - ss[i-1]);
    *x = xx[i-1] + t * (xx[i] - xx[i-1]);
    *y = yy[i-1] + t * (yy[i] - yy[i-1]);
  }
}

bool s_curve::forward(double s, double w, double *s0, double* w0) const
{
  double p[2], p1[2], p2[2];
  double d1, d2, s1, s2, t;

  if (s < 0.0 || s > length())
    return false;

  assert(0.0 <= s && s <= length());
  assert(0.0 < w && w < length());

  int i = std::lower_bound(ss, ss+n-1, s) - ss;
  if (s == ss[i])
  {
    p[0] = xx[i];
    p[1] = yy[i];
    if (++i == n)
    {
      if (!is_closed())
        return false;
      i = 1;
    }
  }
  else
  {
    t = (s - ss[i-1]) / (ss[i] - ss[i-1]);
    p[0] = xx[i-1] + t * (xx[i] - xx[i-1]);
    p[1] = yy[i-1] + t * (yy[i] - yy[i-1]);
  }

  p1[0] = p[0];
  p1[1] = p[1];
  s1 = s;
  d1 = 0.0;
  *w0 = 0.0;
  if (is_closed())
  {
    int ii = i;
    do
    {
      p2[0] = xx[i];
      p2[1] = yy[i];
      s2 = ss[i];
      d2 = hypot(p2[0]-p[0], p2[1]-p[1]);
      if (d2 > w)
      {
        t = (w - d1) / (d2 - d1);
        assert (0.0 <= t && t <= 1.0);
        *s0 = s1 + t * (s2 - s1);
        *w0 = (*w0 + *s0) - s;
        return true;
      }
      if (++i == n)
      {
        *w0 += length();
        i = 1;
      }
      p1[0] = p2[0];
      p1[1] = p2[1];
      s1 = s2;
    }
    while (ii != i);
    return false;
  }

  while(true)
  {
    p2[0] = xx[i];
    p2[1] = yy[i];
    s2 = ss[i];
    d2 = hypot(p2[0]-p[0], p2[1]-p[1]);
    if (d2 > w)
    {
      t = (w - d1) / (d2 - d1);
      assert (0.0 <= t && t <= 1.0);
      *s0 = s1 + t * (s2 - s1);
      *w0 = (*w0 + *s0) - s;
      return true;
    }
    if (++i == n)
      break;
    p1[0] = p2[0];
    p1[1] = p2[1];
    s1 = s2;
  }
  return false;
}

bool s_curve::backward(double s, double w, double *s0, double* w0) const
{
  double p[2], p1[2], p2[2];
  double d1, d2, s1, s2, t;

  if (s < 0.0 || s > length())
    return false;

  assert(0.0 <= s && s <= length());
  assert(0.0 < w && w < length());

  int i = std::lower_bound(ss, ss+n-1, s) - ss;
  if (s == ss[i])
  {
    p[0] = xx[i];
    p[1] = yy[i];
  }
  else
  {
    t = (s - ss[i-1]) / (ss[i] - ss[i-1]);
    p[0] = xx[i-1] + t * (xx[i] - xx[i-1]);
    p[1] = yy[i-1] + t * (yy[i] - yy[i-1]);
  }
  if (i-- == 0)
  {
    if (!is_closed())
      return false;
    i = n-2;
  }

  p1[0] = p[0];
  p1[1] = p[1];
  s1 = s;
  d1 = 0.0;
  *w0 = 0.0;
  if (is_closed())
  {
    int ii = i;
    do
    {
      p2[0] = xx[i];
      p2[1] = yy[i];
      s2 = ss[i];
      d2 = hypot(p2[0]-p[0], p2[1]-p[1]);
      if (d2 > w)
      {
        t = (w - d1) / (d2 - d1);
        *s0 = s1 + t * (s2 - s1);
        *w0 = (*w0 + *s0) - s;
        return true;
      }
      if (i-- == 0)
      {
        *w0 -= length();
        i = n-1;
      }
      p1[0] = p2[0];
      p1[1] = p2[1];
      s1 = s2;
    }
    while (ii != i);
    return false;
  }

  while(true)
  {
    p2[0] = xx[i];
    p2[1] = yy[i];
    s2 = ss[i];
    d2 = hypot(p2[0]-p[0], p2[1]-p[1]);
    if (d2 > w)
      break;
    if (i-- == 0)
      return false;
    p1[0] = p2[0];
    p1[1] = p2[1];
    s1 = s2;
  }
  t = (w - d1) / (d2 - d1);
  *s0 = s1 + t * (s2 - s1);
  *w0 = s - *s0;
  return true;
}

static void hatch_curve(psstream& out, double line_width, int hatch_step, \
  const vertex_type& ver, const curve_type& g)
{
  s_curve cur(ver, g);

  double curve_len = cur.length();
  double hatch_dist = line_width * hatch_step;
  if (curve_len > 2.0 * hatch_dist)
  {
    double dist = 0.0;
    int n_hatch = (int) floor(curve_len / hatch_dist);
    if (cur.is_closed())
    {
      hatch_dist = curve_len / n_hatch;
    }
    else
    {
      dist = 0.5 * (curve_len - n_hatch * hatch_dist);
    }

    double dg, s2, s1 = dist, g = 0.0;
    while (cur.forward(s1, line_width, &s2, &dg))
    {
      double x, y, x1, y1, x2, y2;
      cur.position(s1, &x1, &y1);
      cur.position(s2, &x2, &y2);
      double dx = x2 - x1;
      double dy = y2 - y1;
      double dd = hypot(dx, dy);
      double nx = dy / dd;
      double ny = -dx / dd;
      double s = 0.5 * (s1 + s2);
      cur.position(s, &x, &y);
      out.moveto(x, y);
      x += 2.0 * line_width * nx;
      y += 2.0 * line_width * ny;
      out.lineto(x, y);
      s1 += hatch_dist;
      g += hatch_dist;
      if (g > (curve_len - dist - line_width))
        break;
    }
  }
}

void hatch_edge(psstream& out, double line_width, int hatch_step, \
  const vertex_type& ver, const edge_type& ee)
{
  for (edge_type::const_iterator e = ee.begin(); e != ee.end(); ++e)
  {
    if (e->empty())
      continue;
    hatch_curve(out, line_width, hatch_step, ver, *e);
  }
}


struct glyph_type
{
  char ch[2];
  double ww;
  double x1, y1;
  double x2, y2;
  double nx, ny;
  double x0, y0;
  double a;
};

static void output_bounding_polygon(int n_glyph, const glyph_type* glyph, double gx, double gy,
  pgn_builder* pgn)
{
  pgn->begin_curve();

  // left edge
  int clip_first = pgn->new_vertex(glyph[0].x1 - gy * glyph[0].nx + gx * glyph[0].ny, \
    glyph[0].y1 - gy * glyph[0].ny - gx * glyph[0].nx);
  int clip_index = clip_first;
  pgn->add_vertex(clip_index++);
  pgn->new_vertex(glyph[0].x1 + gy * glyph[0].nx + gx * glyph[0].ny, \
    glyph[0].y1 + gy * glyph[0].ny - gx * glyph[0].nx);
  pgn->add_vertex(clip_index++);

  // top edge
  for (int i = 1; i < n_glyph; ++i)
  {
    if (glyph[i-1].nx * glyph[i].ny > glyph[i].nx * glyph[i-1].ny)
    {
      pgn->new_vertex(glyph[i-1].x2 + gy * glyph[i-1].nx, glyph[i-1].y2 + gy * glyph[i-1].ny);
      pgn->add_vertex(clip_index++);
      pgn->new_vertex(glyph[i].x1 + gy * glyph[i].nx, glyph[i].y1 + gy * glyph[i].ny);
      pgn->add_vertex(clip_index++);          
    }
    else
    {
      double x1 = glyph[i-1].x2 + gy * glyph[i-1].nx;
      double y1 = glyph[i-1].y2 + gy * glyph[i-1].ny;
      double x2 = glyph[i].x1 + gy * glyph[i].nx;
      double y2 = glyph[i].y1 + gy * glyph[i].ny;
      double nx = 0.5 * (glyph[i-1].nx + glyph[i].nx);
      double ny = 0.5 * (glyph[i-1].ny + glyph[i].ny);
      double nn = hypot(nx, ny);
      double dx = 0.5 * (x1 + x2) - glyph[i].x1;
      double dy = 0.5 * (y1 + y2) - glyph[i].y1;
      double dd = gy * gy / hypot(dx, dy);
      pgn->new_vertex(glyph[i].x1 + nx * dd / nn, glyph[i].y1 + ny * dd / nn);
      pgn->add_vertex(clip_index++);          
    }
  }

  // right edge
  pgn->new_vertex(glyph[i-1].x2 + gy * glyph[i-1].nx - gx * glyph[i-1].ny, \
    glyph[i-1].y2 + gy * glyph[i-1].ny + gx * glyph[i-1].nx);
  pgn->add_vertex(clip_index++);
  pgn->new_vertex(glyph[i-1].x2 - gy * glyph[i-1].nx - gx * glyph[i-1].ny, \
    glyph[i-1].y2 - gy * glyph[i-1].ny + gx * glyph[i-1].nx);
  pgn->add_vertex(clip_index++);

  // bottom edge
  while (--i)
  {
    if (glyph[i-1].nx * glyph[i].ny < glyph[i].nx * glyph[i-1].ny)
    {
      pgn->new_vertex(glyph[i].x1 - gy * glyph[i].nx, glyph[i].y1 - gy * glyph[i].ny);
      pgn->add_vertex(clip_index++);          
      pgn->new_vertex(glyph[i-1].x2 - gy * glyph[i-1].nx, glyph[i-1].y2 - gy * glyph[i-1].ny);
      pgn->add_vertex(clip_index++);          
    }
    else
    {
      double x1 = glyph[i-1].x2 - gy * glyph[i-1].nx;
      double y1 = glyph[i-1].y2 - gy * glyph[i-1].ny;
      double x2 = glyph[i].x1 - gy * glyph[i].nx;
      double y2 = glyph[i].y1 - gy * glyph[i].ny;
      double nx = -0.5 * (glyph[i-1].nx + glyph[i].nx);
      double ny = -0.5 * (glyph[i-1].ny + glyph[i].ny);
      double nn = hypot(nx, ny);
      double dx = 0.5 * (x1 + x2) - glyph[i].x1;
      double dy = 0.5 * (y1 + y2) - glyph[i].y1;
      double dd = gy * gy / hypot(dx, dy);
      pgn->new_vertex(glyph[i].x1 + nx * dd / nn, glyph[i].y1 + ny * dd / nn);
      pgn->add_vertex(clip_index++);          
    }
  }
  pgn->add_vertex(clip_first);
  pgn->end_curve();
}

static bool place_text_curve(int ng, glyph_type* g, const s_curve& c, \
  double c_pos, double text_width, double text_height, double text_gap, \
  pgn_builder* pgn)
{
  double x1, y1, x2, y2;
  c.position(c_pos, &x1, &y1);
  c.position(c_pos + text_width, &x2, &y2);
  
  bool (s_curve::*dir)(double, double, double*, double*) const = &s_curve::forward;
	double a = atan2(y2-y1, x2-x1);
	if (a < 0.0) a += 2.0 * M_PI;
	a = 360.0 * a / (2.0 * M_PI);

  double pos_curr = c_pos;
  if (a >= 90.0 && a < 270.0)
  {
    pos_curr += text_width;
    dir = &s_curve::backward;
  }

  for (int i = 0; i < ng; ++i)
  {
    double pos_next, cum_delta;
    bool ok = (c.*dir)(pos_curr, g[i].ww, &pos_next, &cum_delta);
    assert(ok);
    c.position(pos_curr, &x1, &y1);
    c.position(pos_next, &x2, &y2);
    double dx = x2 - x1;
    double dy = y2 - y1;
    double dd = hypot(dx, dy);
    double x0 = 0.5 * (x1 + x2);
    double y0 = 0.5 * (y1 + y2);
    double a = 360.0 * atan2(dy, dx) / (2.0 * M_PI);
    g[i].x1 = x1;
    g[i].x2 = x2;
    g[i].y1 = y1;
    g[i].y2 = y2;
    g[i].nx = dy / dd;
    g[i].ny = -dx / dd;
    g[i].x0 = x0;
    g[i].y0 = y0;
    g[i].a = a;
    pos_curr = pos_next;
  }

  // select tolerance of 10 degrees between two sequential segment
  for (i = 1; i < ng; ++i)
  {
    if (fabs(g[i-1].nx * g[i].ny - g[i-1].ny * g[i].nx) > 0.17)
      break;
  }

  if (i != ng)
    return false;

  output_bounding_polygon(ng, g, text_gap, text_gap + 0.5 * text_height, pgn);
  return true;
}

void annotpath(psstream& out, const vertex_type& path_ver, const edge_type& path_edge, \
  const char* text, double step, double gap, pgn_builder* pgn)
{
  int n_glyph = strlen(text);
  glyph_type *glyph = new glyph_type[n_glyph];
  double text_width = 0.0, text_height = 0.0;
  for (int i = 0; i < n_glyph; ++i)
  {
    double bb[4];
    glyph[i].ch[0] = text[i];
    glyph[i].ch[1] = '\0';
    out.textbox(glyph[i].ch, bb);
    
    double glyph_width = bb[2] - bb[0];
    glyph[i].ww = glyph_width;
    text_width += glyph_width;

    double glyph_height = bb[3] - bb[1];
    if (text_height < glyph_height) 
      text_height = glyph_height;
  }

	for (edge_type::const_iterator e = path_edge.begin(); e != path_edge.end(); ++e)
	{
		if (e->empty())
			continue;

		s_curve cur(path_ver, *e);
		double curve_len = cur.length();
		double annot_dist = text_width + step;
		if (curve_len < annot_dist)
			continue;

		double curve_dist = 0.0;
		double annot_gap = step;
		int n_annot = (int) floor(curve_len / annot_dist);
		if (cur.is_closed())
		{
			annot_gap = (curve_len - n_annot * text_width) / n_annot;
			annot_dist = annot_gap + text_width;
		}
		else
		{
			curve_dist = 0.5 * (curve_len - ((n_annot-1) * annot_dist + text_width));
		}

		for (int j = 0; j < n_annot; ++j)
		{
			if (place_text_curve(n_glyph, glyph, cur, curve_dist, \
				text_width, text_height, gap, pgn))
			{
				for (i = 0; i < n_glyph; ++i)
				{
					double bb[8];
					out.annot(glyph[i].x0, glyph[i].y0, 1.0, glyph[i].a, \
						psstream::ANNOT_CENTER, true, glyph[i].ch, bb);
				}
			}
			curve_dist += annot_dist;
		}
	}
  delete[] glyph;
}

#if 0

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
  double major_width = 0.5;
  double major_dz = major_step * minor_dz;
  int first_major = (int) (ceil(z1 / major_dz) * major_dz - z1) / minor_dz;

  vertex_type ver;
  edge_type *me = new edge_type[n];
  edge_type *ce = new edge_type[n+1];
  marching_squares(g->cx, g->cy, i1, i2-1, j1, j2-1, g->zz, n, level, ver, me, ce);

  convert(x0-sx*i1/(i2-i1-1), y0-sy*j1/(j2-j1-1), sx/(i2-i1-1), sy/(j2-j1-1), ver);

  out.setgray(0.0);
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

  back_polygon bgr(out);
  for (k = first_major; k < n; k += major_step)
  {
    char text[16];
    sprintf(text, "%.0f", level[k]);
    annotpath(out, ver, me[k], text, 60.0, major_width * 0.5, &bgr);
  }

  delete[] ce;
  delete[] me;
  delete[] level;
}

#endif

#if 0

#define DATA_FOLDER "d:\\dat\\"

int main()
{
  int k;
  grid_type g;
  if (!read_grid(DATA_FOLDER "depth-to-top.grd", &g))
    return 1;

  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A3, psstream::PORTRAIT);
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

  out.setfont("Helvetica", 4.0);
  auto_conrec(out, 0, g.cx, 0, g.cy, &g, x0, y0, sx, sy);
  out.setgray(0.0);
  out.setlinewidth(1.0);
  out.rectstroke(x0, y0, sx, sy); 
  delete[] g.zz;

  return 0;
}

#endif
