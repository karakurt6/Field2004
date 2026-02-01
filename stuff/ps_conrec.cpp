#define _USE_MATH_DEFINES
#ifndef __INTEL_COMPILER
  #include <math.h>
#else
  #include <mathimf.h>
#endif

#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <limits>
#include <algorithm>
#include <stdlib.h>
#include <assert.h>
#include "ps_data.h"

static const int line_cases[16][5] =
{
  { -1, -1, -1, -1, -1 },
  {  0,  3, -1, -1, -1 },
  {  1,  0, -1, -1, -1 },
  {  1,  3, -1, -1, -1 },
  {  2,  1, -1, -1, -1 },
  {  0,  3,  2,  1, -1 },
  {  2,  0, -1, -1, -1 },
  {  2,  3, -1, -1, -1 },
  {  3,  2, -1, -1, -1 },
  {  0,  2, -1, -1, -1 },
  {  1,  0,  3,  2, -1 },
  {  1,  2, -1, -1, -1 },
  {  3,  1, -1, -1, -1 },
  {  0,  1, -1, -1, -1 },
  {  3,  0, -1, -1, -1 },
  { -1, -1, -1, -1, -1 }
};

static const int edge_table[16] =
{
  0, 9, 3, 10, 6, 15, 5, 12, 12, 5, 15, 6, 10, 3, 9, 0
};

static void edge_0(int i, int j, int s, float z, const float *zz, coord_type* p)
{
  p->xcoord = i + (z - zz[0]) / (zz[1] - zz[0]);
  p->ycoord = j;
}

static void edge_1(int i, int j, int s, float z, const float *zz, coord_type* p)
{
  p->xcoord = i + 1;
  p->ycoord = j + (z - zz[1]) / (zz[s+1] - zz[1]);
}

static void edge_2(int i, int j, int s, float z, const float *zz, coord_type* p)
{
  p->xcoord = i + (z - zz[s]) / (zz[s+1] - zz[s]);
  p->ycoord = j + 1;
}

static void edge_3(int i, int j, int s, float z, const float *zz, coord_type* p)
{
  p->xcoord = i;
  p->ycoord = j + (z - zz[0]) / (zz[s] - zz[0]);
}

typedef void (*edge_handler)(int, int, int, float, const float*, coord_type*);
static edge_handler edge[4] =
{
  edge_0, edge_1, edge_2, edge_3
};

struct hull_type
{
  int level, point;
  double distance;
};

static double hull_distance(int ilb, int iub, int jlb, int jub, const coord_type* p)
{
  if (p->ycoord == jlb)
    return (p->xcoord - ilb);
  if (p->xcoord == iub)
    return (iub - ilb) + (p->ycoord - jlb);
  if (p->ycoord == jub)
    return 2.0 * (iub - ilb) + (jub - jlb) - (p->xcoord - ilb);
  return 2.0 * ((iub - ilb) + (jub - jlb)) - (p->ycoord - jlb);
}

inline bool operator < (const hull_type& e1, const hull_type& e2)
{
  return (e1.distance < e2.distance);
}

inline bool hull_cmp(const hull_type& e, double d)
{
  return (e.distance < d);
}

bool merge_curves(edge_type& ee)
{
  while (true)
  {
    edge_type::iterator e = ee.begin();
    while (e != ee.end())
    {
      if (e->front() != e->back())
        break;
      ++e;
    }
    if (e == ee.end())
      break;
    edge_type::iterator e0 = e;
    curve_type& c1 = *e;
    while (++e != ee.end())
    {
      curve_type& c2 = *e;
      if (c2.front() == c2.back())
        continue;
      if (c2.front() == c1.back())
      {
        c1.pop_back();
        c1.splice(c1.end(), c2);
        break;
      }
    }
    if (e != ee.end())
    {
      ee.erase(e);
    }
    else
    {
      return false;
    }
  }

  return true;
}

static void split_blank(vertex_type& ver, int index, edge_type& blank)
{
  coord_type* p = &ver[index];
  for (edge_type::iterator f = blank.begin(); f != blank.end(); ++f)
  {
    curve_type::iterator c = f->begin();
    coord_type* p1 = &ver[*c];
    while (++c != f->end())
    {
      coord_type* p2 = &ver[*c];
      Classify s = classify(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, p->xcoord, p->ycoord);
      if (s == BETWEEN)
      {
        blank.push_back(curve_type());
        curve_type* g = &blank.back();
        g->splice(g->end(), *f, c, f->end());
        g->push_front(index);
        f->push_back(index);
        return;
      }
      else if (s == ORIGIN || s == DESTINATION)
      {
        if (index != *c)
        {
          for (curve_type::iterator g = f->begin(); g != f->end(); ++g)
          {
            if (*g == *c)
            {
              *g = index;
            }
          }
        }
        if (*c != f->front() && *c != f->back())
        {
          blank.push_back(curve_type());
          curve_type* g = &blank.back();
          g->splice(g->end(), *f, c, f->end());
          f->push_back(index);
        }
        return;
      }
      p1 = p2;
    }
  }
}

void marching_squares(int cx, int cy, int ilb, int iub, int jlb, int jub, \
  const float* elev, int num_levels, float* level, 
  vertex_type& m_ver, edge_type* m_edge, edge_type* c_edge)
{
  int i, j, k;
  const float* zz;
  const int* line;

  int n = 4 * (cx-1);
  int** m_cell = new int* [num_levels];
  for (k = 0; k < num_levels; ++k)
  {
    m_cell[k] = new int[n];
    std::fill_n(m_cell[k], n, -1);
  }

	edge_type blank;
	int* nn_b = new int [2 * cx], *n1_b = nn_b, *n2_b = nn_b + cx;
	std::fill(nn_b, nn_b + 2 * cx, -1);
  for (j = jlb; j < jub; ++j)
  {
    int num_facet = 0;
    for (i = ilb; i < iub; ++i)
    {
      zz = elev + j * cx + i;
			if (!_isnan(zz[0]) && !_isnan(zz[1]) && !_isnan(zz[cx]) && !_isnan(zz[cx+1]))
			{
				if (j == jlb || _isnan(zz[-cx]) || _isnan(zz[-cx+1]))
				{
				  if (n1_b[i] == -1)
				  {
				  	n1_b[i] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i;
				  	p->ycoord = j;
				  }
				  if (n1_b[i+1] == -1)
				  {
				    n1_b[i+1] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i+1;
				  	p->ycoord = j;
				  }
					blank.push_back(curve_type());
					curve_type* cur = &blank.back();
					cur->push_back(n1_b[i]);
					cur->push_back(n1_b[i+1]);
				}
				if (j == jub-1 || _isnan(zz[2*cx]) || _isnan(zz[2*cx+1]))
				{
				  if (n2_b[i+1] == -1)
				  {
				  	n2_b[i+1] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i+1;
				  	p->ycoord = j+1;
				  }
				  if (n2_b[i] == -1)
				  {
				  	n2_b[i] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i;
				  	p->ycoord = j+1;
				  }
					blank.push_back(curve_type());
					curve_type* cur = &blank.back();
					cur->push_back(n2_b[i+1]);
					cur->push_back(n2_b[i]);
				}
				if (i == ilb || _isnan(zz[-1]) || _isnan(zz[cx-1]))
				{
				  if (n2_b[i] == -1)
				  {
				  	n2_b[i] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i;
				  	p->ycoord = j+1;
				  }
				  if (n1_b[i] == -1)
				  {
				  	n1_b[i] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i;
				  	p->ycoord = j;
				  }
					blank.push_back(curve_type());
					curve_type* cur = &blank.back();
					cur->push_back(n2_b[i]);
					cur->push_back(n1_b[i]);
				}
				if (i == iub-1 || _isnan(zz[2]) || _isnan(zz[cx+2]))
				{
				  if (n1_b[i+1] == -1)
				  {
				  	n1_b[i+1] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i+1;
				  	p->ycoord = j;
				  }
				  if (n2_b[i+1] == -1)
				  {
				  	n2_b[i+1] = m_ver.size();
				  	m_ver.push_back(coord_type());
				  	coord_type* p = &m_ver.back();
				  	p->xcoord = i+1;
				  	p->ycoord = j+1;
				  }
					blank.push_back(curve_type());
					curve_type* cur = &blank.back();
					cur->push_back(n1_b[i+1]);
					cur->push_back(n2_b[i+1]);
				}

        for (k = 0; k < num_levels; ++k)
        {
          int* f = m_cell[k] + num_facet;
          float z = level[k];
          int cell_index = 0;

          if (zz[0] >= z)
            cell_index |= 1;
          if (zz[1] >= z)
            cell_index |= 2;
          if (zz[cx] >= z)
            cell_index |= 8;
          if (zz[cx+1] >= z)
            cell_index |= 4;

          for (int e = 0; e < 4; ++e)
          {
            if (edge_table[cell_index] & (1 << e))
            {
              if (f[e] == -1)
              {
                n = m_ver.size();
                m_ver.push_back(coord_type());
                edge[e](i, j, cx, z, zz, &m_ver.back());
                f[e] = n;
              }
            }
            else
            {
              f[e] = -1;
            }
          }

          line = line_cases[cell_index];
          if (cell_index == 5)
          {
            float midpoint = 0.25f * (zz[0]+zz[1]+zz[cx]+zz[cx+1]);
            if (midpoint < z)
            {
              static const int special[] = { 0, 1, 2, 3, -1 };
              line = special;
            }
          }
          else if (cell_index == 10)
          {
            float midpoint = 0.25f * (zz[0]+zz[1]+zz[cx]+zz[cx+1]);
            if (midpoint < z)
            {
              static const int special[] = { 3, 0, 1, 2, -1 };
              line = special;
            }
          }

          while (*line != -1)
          {
            int s = f[*line++];
            int t = f[*line++];

            edge_type::iterator e = m_edge[k].begin();
            while (e != m_edge[k].end())
            {
              curve_type& c = *e;
              if (c.front() != c.back())
              {
                if (t == c.front())
                {
                  c.push_front(s);
                  edge_type::iterator ee = e;
                  while (++ee != m_edge[k].end())
                  {
                    curve_type& cc = *ee;
                    if (s == cc.back())
                    {
                      cc.pop_back();
                      c.splice(c.begin(), cc);
                      m_edge[k].erase(ee);
                      break;
                    }
                  }
                  break;
                }

                // insert after final
                if (s == c.back())
                {
                  c.push_back(t);
                  edge_type::iterator ee = e;
                  while (++ee != m_edge[k].end())
                  {
                    curve_type& cc = *ee;
                    if (t == cc.front())
                    {
                      c.pop_back();
                      c.splice(c.end(), cc);
                      m_edge[k].erase(ee);
                      break;
                    }
                  }
                  break;
                }
              }
              ++e;
            }

            if (e == m_edge[k].end())
            {
              m_edge[k].push_back(curve_type());
              curve_type* c = &m_edge[k].back();
              c->push_back(s);
              c->push_back(t);
            }
          }

          if (i < cx-2)
          {
            f[7] = f[1];
          }
          f[0] = f[2];
          f[3] = f[2] = f[1] = -1;
        }
			}
      num_facet += 4;
    }
    std::swap(n1_b, n2_b);
    std::fill(n2_b, n2_b+cx, -1);
  }
  merge_curves(blank);

  if (c_edge)
  {
  	{
      for (edge_type::iterator e = blank.begin(); e != blank.end(); ++e)
      {
        c_edge[num_levels+1].push_back(curve_type());
        std::copy(e->rbegin(), e->rend(), std::back_inserter(c_edge[num_levels+1].back()));
      }
  	}

    for (int k = 0; k < num_levels; ++k)
    {
      edge_type::iterator e = blank.begin();
      while (e != blank.end())
      {
      	curve_type::iterator c = e->begin(); 
      	while (c != e->end())
      	{
      	  coord_type* p = &m_ver[*c];
      	  int x1 = (int) floor(p->xcoord);
      	  int x2 = (int) ceil(p->xcoord);
      	  int y1 = (int) floor(p->ycoord);
      	  int y2 = (int) ceil(p->ycoord);
      	  if (x1 == x2 && y1 == y2 && elev[cx * y1 + x1] > level[k])
      	  {
        	  break;
      	  }
      	  ++c;
      	}
      	if (c != e->end())
      	{
      	  ++e;
      	}
      	else
      	{
      	  c_edge[k].push_back(*e);
      	  e = blank.erase(e);
      	}
      }
      for (e = m_edge[k].begin(); e != m_edge[k].end(); ++e)
      {
        c_edge[k].push_back(curve_type());
        std::copy(e->rbegin(), e->rend(), std::back_inserter(c_edge[k].back()));
        if (e->front() != e->back())
        {
        	split_blank(m_ver, e->front(), blank);
        	split_blank(m_ver, e->back(), blank);
        }
      }
      for (e = c_edge[k].begin(); e != c_edge[k].end(); ++e)
      {
      	if (e->front() != e->back())
      	{
      	  for (edge_type::iterator f = blank.begin(); f != blank.end(); ++f)
      	  {
      	    if (e->back() == f->front())
      	    {
      	    	f->pop_front();
      	      e->splice(e->end(), *f, f->begin(), f->end());
      	      blank.erase(f);
      	      break;
      	    }
      	  }
      	  for (edge_type::iterator f = blank.begin(); f != blank.end(); ++f)
      	  {
      	    if (e->front() == f->back())
      	    {
      	      f->pop_back();
      	      e->splice(e->begin(), *f, f->begin(), f->end());
      	      blank.erase(f);
      	      break;
      	    }
      	  }
      	}
      }
      merge_curves(c_edge[k]);
      for (e = m_edge[k].begin(); e != m_edge[k].end(); ++e)
      {
        blank.push_back(*e);
      }
      merge_curves(blank);
    }
    c_edge[num_levels] = blank;
  }

  delete[] nn_b;
  for (k = 0; k < num_levels; ++k)
    delete[] m_cell[k];
  delete[] m_cell;
}

void convert(double tx, double ty, double sx, double sy, vertex_type& ver)
{
  for (vertex_type::iterator p = ver.begin(); p != ver.end(); ++p)
  {
    p->xcoord = tx + sx * p->xcoord;
    p->ycoord = ty + sy * p->ycoord;
  }
}

#if 0

#define DATA_FOLDER "d:\\dat\\"

int main()
{
  int k;
  grid_type g;
  if (!read_grid(DATA_FOLDER "depth-to-top.grd", &g))
    return 1;

  double minor_dz = 5.0;
  double minor_width = 0.3;
  double z1 = ceil(g.z1 / minor_dz) * minor_dz;
  double z2 = floor(g.z2 / minor_dz) * minor_dz;
  int n = (int) ((z2 - z1) / minor_dz) + 1;
  float *level = new float[n];
  for (k = 0; k < n; ++k)
    level[k] = float(z1+k*(z2-z1)/(n-1));

  int major_step = 5;
  double major_width = 0.6;
  double major_dz = major_step * minor_dz;
  int first_major = (int) (ceil(z1 / major_dz) * major_dz - z1) / minor_dz;

  vertex_type ver;
  edge_type *me = new edge_type[n];
  edge_type *ce = new edge_type[n+1];
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

  out.setlinewidth(1.0);
  out.rectstroke(x0, y0, sx, sy); 

  delete[] ce;
  delete[] me;
  delete[] level;

  return 0;
}

#endif
