#define USE_MATH_DEFINES
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cassert>
#include "ps_data.h"

struct correlation
{
  int source;
  int target;
};

bool source_order(const correlation& r1, const correlation& r2)
{
  return (r1.source < r2.source || (r1.source == r2.source && r1.target < r2.target));
}

bool target_order(const correlation& r1, const correlation& r2)
{
  return (r1.target < r2.target || (r1.target == r2.target && r1.source < r2.source));
}

double clamp(double z1, double z2, double z)
{
	if (z < z1)
		return z1;
	if (z > z2)
		return z2;
	return z;
}

void compute(int n1, double* x1, double* y1, double* f1, 
  int n2, double* x2, double* y2, double* f2,
  int m, double d_max, int *t, double* r, double* d)
{
  const double r_min = 0.95;
  assert(2*m+1 < n1);
  assert(2*m+1 < n2);
  for (int i = 0; i < m; ++i)
  {
    t[i] = -1;
  }
  for (;i < n1-m; ++i)
  {
    double d_corr, r_corr, d_dist, r_dist;
    int j_corr = -1, j_dist = -1;
    for (int j = m; j < n2-m; ++j)
    {
      double d = hypot(x1[i] - x2[j], y1[i] - y2[j]);
      if (d > d_max)
        continue;

      double f1_avg = 0.0, f2_avg = 0.0;
      for (int k = -m; k <= m; ++k)
      {
        f1_avg += f1[i+k];
        f2_avg += f2[j+k];
      }
      f1_avg /= 2*m;
      f2_avg /= 2*m;

      double cov = 0.0, var1 = 0.0, var2 = 0.0;
      for (int k = -m; k <= m; ++k)
      {
        double d1 = f1[i+k] - f1_avg;
        double d2 = f2[j+k] - f2_avg;
        cov += d1 * d2;
        var1 += d1 * d1;
        var2 += d2 * d2;
      }

      double r = clamp(-1.0, 1.0, cov / sqrt(var1 * var2));
      if (j_corr == -1 || (r > r_corr || r == r_corr && d < d_corr))
      {
        j_corr = j;
        r_corr = r;
        d_corr = d;
      }
      if (j_dist == -1 || d < d_dist)
      {
        j_dist = j;
        r_dist = r;
        d_dist = d;
      }
    }
    if (r_corr > r_min)
    {
      r[i] = r_corr;
      d[i] = d_corr;
      t[i] = j_corr;
    }
    else
    {
      t[i] = -1;
    }
  }
  for (; i < n1; ++i)
  {
    t[i] = -1;
  }
}

int combine(int n1, int* t1, double* r1, double* d1, 
  int n2, int* t2, double* r2, double* d2, 
  correlation *corr)
{
  int total = 0;
  for (int i = 0; i < n1; ++i)
  {
    int j = t1[i];
    if (j != -1)
    {
      corr[total].source = i;
      corr[total].target = j;
      ++total;
    }
  }
  for (int i = 0; i < n2; ++i)
  {
    int j = t2[i];
    if (j != -1)
    {
      if (t1[j] != i)
      {
        corr[total].source = j;
        corr[total].target = i;
        ++total;
      }
    }
  }

  std::sort(corr, corr+total, target_order);
  correlation* append = corr + total;
  for (int i = 1; i < total; ++i)
  {
    int ds = corr[i].source - corr[i-1].source;
    if (ds > 1)
    {
      int dt = corr[i].target - corr[i-1].target;
      for (int j = 1; j < ds; ++j)
      {
        append->source = corr[i-1].source + j;
        append->target = corr[i-1].target + int(0.5 + dt * j / ds);
        ++append;
      }
    }
  }
  total = append - corr;
  
  std::sort(corr, corr+total, source_order);
  append = corr + total;
  for (int i = 1; i < total; ++i)
  {
    int dt = corr[i].target - corr[i-1].target;
    if (dt > 1)
    {
      int ds = corr[i].source - corr[i-1].source;
      for (int j = 1; j < dt; ++j)
      {
        append->target = corr[i-1].target + j;
        append->source = corr[i-1].source + int(0.5 + ds * j / dt);
        ++append;
      }
    }
  }
  total = append - corr;
  return total;
}

void iter(psstream& out, int m)
{
  out.newpage();
  out.setfont(FONT_33, 6.0);

  out.setlinewidth(1.0);
  out.setgray(0.0);

  const int n = 201;
  double x1[n], y1[n], f1[n];
  double x0 = 20.0; // абсцисса начальной точки
  double dx = 4.0; // шаг вдоль профиля
  double T = 40.0; // период функции
  double y0 = 100.0; // ордината начальной точки
  double df = 40.0; // амплитуда сигнала
  double phi = 0.0; // начальная фаза
  for (int i = 0; i < n; ++i)
  {
    x1[i] = x0 + dx * i;
    y1[i] = y0;
    f1[i] = 0.5 * df * (1.0 + sin(phi + 2.0 * M_PI * i / T));
    out.moveto(x1[i], y1[i]);
    out.lineto(x1[i], y1[i] - f1[i]);
    out.stroke();
    if (i % 10 == 0)
    {
      double bb[4];
      char text[20];
      sprintf(text, "%d", i);
      label(out, x1[i], y1[i], 1.0, LABEL_N, true, text, bb);
    }
  }

  double x2[n], y2[n], f2[n];
  T = 32.0; // период функции
  y0 = 200.0; // ордината начальной точки
  df = 30.0; // амплитуда сигнала
  phi = 2 * M_PI * 3 / T; // начальная фаза
  for (int i = 0; i < n; ++i)
  {
    x2[i] = x0 + dx * i;
    y2[i] = y0;
    f2[i] = 0.5 * df * (1.0 + sin(phi + 2.0 * M_PI * i / T));
    out.moveto(x2[i], y2[i]);
    out.lineto(x2[i], y2[i] + f2[i]);
    out.stroke();
    if (i % 10 == 0)
    {
      double bb[4];
      char text[20];
      sprintf(text, "%d", i);
      label(out, x2[i], y2[i], 1.0, LABEL_S, true, text, bb);
    }
  }

  int t1[n], t2[n];
  correlation corr[n+n];
  double r1[n], d1[n], r2[n], d2[n];
  compute(n, x1, y1, f1, n, x2, y2, f2, m, 110.0, t1, r1, d1);
  compute(n, x2, y2, f2, n, x1, y1, f1, m, 110.0, t2, r2, d2);
  int nn = combine(n, t1, r1, d1, n, t2, r2, d2, corr);

  out.setlinewidth(0.0);
  for (int i = 0; i < nn; ++i)
  {
    int s = corr[i].source;
    int t = corr[i].target;
    out.moveto(x1[s], y1[s]);
    out.lineto(x2[t], y2[t]);
  }
  out.stroke();

  /*
  for (int i = 0; i < n; ++i)
  {
    if (t1[i] == -1)
      continue;

    int j = t1[i];
    out.moveto(x1[i], y1[i]);
    out.lineto(x2[j], y2[j]);
    out.stroke();
  }
  */

  out.setrgbcolor(1.0, 0.0, 0.0);
  int num_points = 0;
  int num_segments = 0;
  for (int i = 0; i < n; ++i)
  {
    if (t1[i] == -1)
    {
      if (num_points == 1)
      {
        out.lineto(x1[i-1], y1[i-1] * 100.0 * r1[i-1]);
      }
      num_points = 0;
      continue;
    }

    if (num_points > 0)
    {
      out.lineto(x1[i], y1[i] + 100.0 * r1[i]);
    }
    else
    {
      out.moveto(x1[i], y1[i] + 100.0 * r1[i]);
      ++num_segments;
    }
    ++num_points;
  }
  if (num_segments > 0)
  {
    out.stroke();
  }
  
  out.setlinewidth(1.0);
  out.setgray(0.0);

  y0 = 300.0; // ордината начальной точки
  for (int i = 0; i < n; ++i)
  {
    y1[i] = y0;
    out.moveto(x1[i], y1[i]);
    out.lineto(x1[i], y1[i] - f1[i]);
    out.stroke();
    if (i % 10 == 0)
    {
      double bb[4];
      char text[20];
      sprintf(text, "%d", i);
      label(out, x1[i], y1[i], 1.0, LABEL_N, true, text, bb);
    }
  }

  y0 = 400.0; // ордината начальной точки
  for (int i = 0; i < n; ++i)
  {
    y2[i] = y0;
    out.moveto(x2[i], y2[i]);
    out.lineto(x2[i], y2[i] + f2[i]);
    out.stroke();
    if (i % 10 == 0)
    {
      double bb[4];
      char text[20];
      sprintf(text, "%d", i);
      label(out, x2[i], y2[i], 1.0, LABEL_S, true, text, bb);
    }
  }

  out.setlinewidth(0.0);
  for (int i = 0; i < n; ++i)
  {
    if (t2[i] == -1)
      continue;

    int j = t2[i];
    out.moveto(x2[i], y2[i]);
    out.lineto(x1[j], y1[j]);
    out.stroke();
  }

  out.setrgbcolor(1.0, 0.0, 0.0);
  num_points = 0;
  num_segments = 0;
  for (int i = 0; i < n; ++i)
  {
    if (t2[i] == -1)
    {
      if (num_points == 1)
      {
        out.lineto(x1[i-1], y1[i-1] * 100.0 * r2[i-1]);
      }
      num_points = 0;
      continue;
    }

    if (num_points > 0)
    {
      out.lineto(x2[i], y1[i] + 100.0 * r2[i]);
    }
    else
    {
      out.moveto(x2[i], y1[i] + 100.0 * r2[i]);
      ++num_segments;
    }
    ++num_points;
  }
  if (num_segments > 0)
  {
    out.stroke();
  }

  {
    double bb[4];
    char text[20];
    sprintf(text, "%d", m);
    out.setfont(FONT_33, 32.0);
    out.setgray(0.0);
    label(out, out.pagewidth()-50.0, out.pageheight()-50.0, \
      1.0, LABEL_SE, true, text, bb);
  }
}

void main(int argc, char** argv)
{
  psstream out("CON:");
  out.selectmedia(psstream::FORMAT_A4, psstream::LANDSCAPE);

  int m = 5;
  if (argc > 1)
  {
    m = atoi(argv[1]);
  }
  iter(out, m);
}
