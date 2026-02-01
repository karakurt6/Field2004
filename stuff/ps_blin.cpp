#define _USE_MATH_DEFINES
#ifndef __INTEL_COMPILER
  #include <math.h>
#else
  #include <mathimf.h>
#endif

#include "ps_data.h"

// sqaured distance
static double sd(const coord_type& p, const coord_type& q)
{
  double dx = q.xcoord - p.xcoord, dy = q.ycoord - p.ycoord;
  return dx * dx + dy * dy;
}

void blin(const coord_type& p, const coord_type& q, vertex_type& ver)
{
  double b = q.xcoord - p.xcoord, a = p.ycoord - q.ycoord, c = p.xcoord * a + p.ycoord * b;
  double i, ii, di, j, jj, dj;
  if (p.xcoord < q.xcoord)
  {
    i = ceil(p.xcoord), ii = floor(q.xcoord), di = 1.0;
  }
  else if (p.xcoord > q.xcoord)
  {
    i = floor(p.xcoord), ii = ceil(q.xcoord), di = -1.0;
  }
  else
  {
    i = p.xcoord, ii = q.xcoord, di = 0.0;
  }
  if (p.ycoord < q.ycoord)
  {
    j = ceil(p.ycoord), jj = floor(q.ycoord), dj = 1.0;
  }
  else if (p.ycoord > q.ycoord)
  {
    j = floor(p.ycoord), jj = ceil(q.ycoord), dj = -1.0;
  }
  else
  {
    j = p.ycoord, jj = q.ycoord, dj = 0.0;
  }
  if (i != p.xcoord || j != p.ycoord)
  {
    ver.push_back(p);
  }
  coord_type p1, p2;
  if (di != 0.0 && dj != 0.0)
  {
    p1.xcoord = i, p1.ycoord = (c - a * i) / b;
    p2.xcoord = (c - b * j) / a, p2.ycoord = j;
    while (i-di != ii || j-dj != jj)
    {
      double t, d1 = sd(p1, p), d2 = sd(p2, p);
      if (d1 < d2)
      {
        t = (p1.xcoord - p.xcoord) / (q.xcoord - p.xcoord);
        i += di;
        p1.xcoord = i;
        p1.ycoord = (c - a * i) / b;
      }
      else if (d1 > d2)
      {
        t = (p2.ycoord - p.ycoord) / (q.ycoord - p.ycoord);
        j += dj;
        p2.xcoord = (c - b * j) / a;
        p2.ycoord = j;
      }
      else
      {
        t = (p1.xcoord - p.xcoord) / (q.xcoord - p.xcoord);
        i += di;
        p1.xcoord = i;
        p1.ycoord = (c - a * i) / b;
        j += dj;
        p2.xcoord = (c - b * j) / a;
        p2.ycoord = j;
      }
      coord_type pp;
      pp.xcoord = p.xcoord + t * (q.xcoord - p.xcoord);
      pp.ycoord = p.ycoord + t * (q.ycoord - p.ycoord);
      ver.push_back(pp);
    }
  }
  else if (di != 0.0)
  {
    p1.xcoord = i, p1.ycoord = (c - a * i) / b;
    while (i-di != ii)
    {
      double t = (p1.xcoord - p.xcoord) / (q.xcoord - p.xcoord);
      i += di;
      p1.xcoord = i;
      p1.ycoord = (c - a * i) / b;
      coord_type pp;
      pp.xcoord = p.xcoord + t * (q.xcoord - p.xcoord);
      pp.ycoord = p.ycoord + t * (q.ycoord - p.ycoord);
      ver.push_back(pp);
    }
  }
  else if (dj != 0)
  {
    p2.xcoord = (c - b * j) / a, p2.ycoord = j;
    while (j-dj != jj)
    {
      double t = (p2.ycoord - p.ycoord) / (q.ycoord - p.ycoord);
      j += dj;
      p2.xcoord = (c - b * j) / a;
      p2.ycoord = j;
      coord_type pp;
      pp.xcoord = p.xcoord + t * (q.xcoord - p.xcoord);
      pp.ycoord = p.ycoord + t * (q.ycoord - p.ycoord);
      ver.push_back(pp);
    }
  }
  if (ver.back().xcoord != q.xcoord || ver.back().ycoord != q.ycoord)
  {
    ver.push_back(q);
  }
}

