#include <sstream>
#include <iostream>
#include <math.h>
#include <assert.h>
#include "ps_data.h"

#define USE_CMYK_COLORSPACE
#define PLOT_FILLED_TILES
// #define PLOT_ORDERING

double distance(double x, double y, double x1, double y1, double x2, double y2)
{
	double dx = x1 - x2;
	double dy = y1 - y2;
	double dd = hypot(dx, dy);
	return ((x - x1) * dy - (y - y1) * dx) / dd;
}

void rgb_to_cmyk(double r, double g, double b, double *c, double *m, double *y, double *k)
{
	*k = std::min(1.0 - r, std::min(1.0 - g, 1.0 - b));
	double d = 1.0 - *k;
	*c = (d - r) / d;
	*m = (d - g) / d;
	*y = (d - b) / d;
}

void cmyk_to_rgb(double c, double m, double y, double k, double *r, double *g, double *b)
{
	*r = 1.0 - std::min(1.0, c * (1.0 - k) + k);
	*g = 1.0 - std::min(1.0, m * (1.0 - k) + k);
	*b = 1.0 - std::min(1.0, y * (1.0 - k) + k);
}

void trilinear_to_cartesian(double x1, double y1, double x2, double y2, double x3, double y3, \
	double alpha, double betta, double gamma, double* x, double* y)
{
	double c1 = x1 - x2, c2 = y1 - y2, c = hypot(c1, c2);
	double a1 = x2 - x3, a2 = y2 - y3, a = hypot(a1, a2);
	double k = -(c2 * (x3 - x1) - c1 * (y3 - y1)) / (a * alpha + hypot(x3 - x1, y3 - y1) * betta + c * gamma);
	a1 /= a, a2 /= a;
	c1 /= c, c2 /= c;
	double s = (k * alpha - k * gamma * (a2 * c2 + a1 * c1) \
		+ a2 * (x1 - x2) + a1 * (y2 - y1)) / (c2 * a1 - c1 * a2);
	*x = x1 + s * c1 - k * gamma * c2;
	*y = y1 + s * c2 + k * gamma * c1;
}

void cartesian_to_trilinear(double x1, double y1, double x2, double y2, double x3, double y3, \
	double x, double y, double *alpha, double *betta, double *gamma)
{
	double a = hypot(x3 - x2, y3 - y2);
	double b = hypot(x1 - x3, y1 - y3);
	double c = hypot(x2 - x1, y2 - y1);
	double t1 = ((x - x2) * (y3 - y2) - (y - y2) * (x3 - x2)) / a;
	double t2 = ((x - x3) * (y1 - y3) - (y - y3) * (x1 - x3)) / b;
	double t3 = ((x - x1) * (y2 - y1) - (y - y1) * (x2 - x1)) / c;
	assert(t1 >= 0.0 && t2 >= 0.0 && t3 >= 0.0);
	double tt = t1 + t2 + t3;
	*alpha = t1 / tt;
	*betta = t2 / tt;
	*gamma = t3 / tt;
}

void cartesian_to_index(double x1, double y1, double x2, double y2, double x3, double y3, \
	double x, double y, int size, int *index)
{
	double e11 = x2 - x1, e12 = y2 - y1, e1 = hypot(e11, e12);
	double e21 = x3 - x1, e22 = y3 - y1, e2 = hypot(e21, e22);
	e11 /= e1, e12 /= e1;
	e21 /= e2, e22 /= e2;

	double sx = size / e1, sy = size / e2;
	double xx = x - x1;
	double yy = y - y1;
	double ee = e11 * e22 - e21 * e12;

	double row = (e22 * xx - e21 * yy) * sx / ee;
  double col = (e11 * yy - e12 * xx) * sy / ee;
	assert(0.0 <= col && 0.0 <= row && row+col <= size);

	int n_col = (int) floor(col);
	int n_row = (int) floor(row);
	*index = 2 * n_row * size + 2 * n_col - n_row * n_row + (int) floor(col - n_col + row - n_row);
}

void tridiv(psstream& out, int n, const coord_type* p1, const coord_type* p2, const coord_type* p3)
{
	out.newpage();
	out.setfont("Helvetica", 8.0);

	double r_oil = 1.0;
	double g_oil = 0.5;
	double b_oil = 0.0;

	double r_gas = 1.0;
	double g_gas = 1.0;
	double b_gas = 0.0;

	double r_wat = 0.0;
	double g_wat = 0.0;
	double b_wat = 1.0;

	double c_oil, m_oil, y_oil, k_oil;
	rgb_to_cmyk(r_oil, g_oil, b_oil, &c_oil, &m_oil, &y_oil, &k_oil);

	double c_wat, m_wat, y_wat, k_wat;
	rgb_to_cmyk(r_wat, g_wat, b_wat, &c_wat, &m_wat, &y_wat, &k_wat);

	double c_gas, m_gas, y_gas, k_gas;
	rgb_to_cmyk(r_gas, g_gas, b_gas, &c_gas, &m_gas, &y_gas, &k_gas);

	int k = 0;
	for (int j = 1; j <= n; ++j)
	{
		for (int i = 1; i <= n-j+1; ++i)
		{
			double x1 = p1->xcoord + (j-1) * (p2->xcoord - p1->xcoord) / n \
				+ (i-1) * (p3->xcoord - p1->xcoord) / n;
			double y1 = p1->ycoord + (j-1) * (p2->ycoord - p1->ycoord) / n \
				+ (i-1) * (p3->ycoord - p1->ycoord) / n;
			double x2 = p1->xcoord + (j-1) * (p2->xcoord - p1->xcoord) / n \
				+ i * (p3->xcoord - p1->xcoord) / n;
			double y2 = p1->ycoord + (j-1) * (p2->ycoord - p1->ycoord) / n \
				+ i * (p3->ycoord - p1->ycoord) / n;
			double x3 = p1->xcoord + j * (p2->xcoord - p1->xcoord) / n \
				+ (i-1) * (p3->xcoord - p1->xcoord) / n;
			double y3 = p1->ycoord + j * (p2->ycoord - p1->ycoord) / n \
				+ (i-1) * (p3->ycoord - p1->ycoord) / n;

			{
				double x = (x1 + x2 + x3) / 3.0;
				double y = (y1 + y2 + y3) / 3.0;

				double t1, t2, t3;
				cartesian_to_trilinear(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, x, y, &t1, &t2, &t3);

				double xx, yy;
				trilinear_to_cartesian(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, t1, t2, t3, &xx, &yy);

				int index;
				cartesian_to_index(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, xx, yy, n, &index);

#ifndef PLOT_FILLED_TILES
				out.newpath();
				out.arc(xx, yy, 1.0, 0.0, 360.0);
				out.fill();
#else
#ifdef USE_CMYK_COLORSPACE
				out.setcmykcolor(c_oil * t1 + c_gas * t2 + c_wat * t3, \
					m_oil * t1 + m_gas * t2 + m_wat * t3, y_oil * t1 + y_gas * t2 + y_wat * t3, \
					k_oil * t1 + k_gas * t2 + k_wat * t3);
#else
				out.setrgbcolor(r_oil * t1 + r_gas * t2 + r_wat * t3, \
					g_oil * t1 + g_gas * t2 + g_wat * t3, b_oil * t1 + b_gas * t2 + b_wat * t3);
#endif
  			out.moveto(x1, y1);
  			out.lineto(x2, y2);
  			out.lineto(x3, y3);
  			out.lineto(x1, y1);
  			out.fill();
#endif

  			double bb[8];
  			std::ostringstream ss;
#ifdef PLOT_ORDERING
  			ss << k++;
#else
  			ss << index;
#endif
				out.setgray(0.0);
  			out.annot(x, y, 0.0, 0.0, psstream::ANNOT_CENTER, true, ss.str().c_str(), bb);
			}

			if (i + j <= n)
			{
  			double x4 = p1->xcoord + j * (p2->xcoord - p1->xcoord) / n \
  				+ i * (p3->xcoord - p1->xcoord) / n;
  			double y4 = p1->ycoord + j * (p2->ycoord - p1->ycoord) / n \
  				+ i * (p3->ycoord - p1->ycoord) / n;

				double x = (x4 + x2 + x3) / 3.0;
				double y = (y4 + y2 + y3) / 3.0;

				double t1, t2, t3;
				cartesian_to_trilinear(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, x, y, &t1, &t2, &t3);

				double xx, yy;
				trilinear_to_cartesian(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, t1, t2, t3, &xx, &yy);

				int index;
				cartesian_to_index(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
					p3->xcoord, p3->ycoord, xx, yy, n, &index);

#ifndef PLOT_FILLED_TILES
				out.newpath();
				out.arc(xx, yy, 1.0, 0.0, 360.0);
				out.fill();
#else
#ifdef USE_CMYK_COLORSPACE
				out.setcmykcolor(c_oil * t1 + c_gas * t2 + c_wat * t3, \
					m_oil * t1 + m_gas * t2 + m_wat * t3, y_oil * t1 + y_gas * t2 + y_wat * t3, \
					k_oil * t1 + k_gas * t2 + k_wat * t3);
#else
				out.setrgbcolor(r_oil * t1 + r_gas * t2 + r_wat * t3, \
					g_oil * t1 + g_gas * t2 + g_wat * t3, b_oil * t1 + b_gas * t2 + b_wat * t3);
#endif
  			out.moveto(x3, y3);
  			out.lineto(x2, y2);
  			out.lineto(x4, y4);
  			out.lineto(x3, y3);
  			out.fill();
#endif

  			double bb[8];
  			std::ostringstream ss;
#ifdef PLOT_ORDERING
  			ss << k++;
#else
  			ss << index;
#endif
				out.setgray(0.0);
  			out.annot((x3 + x2 + x4) / 3.0, (y3 + y2 + y4) / 3.0, 0.0, 0.0, \
  				psstream::ANNOT_CENTER, true, ss.str().c_str(), bb);
			}
		}
	}
}

void check_trilin(psstream& out)
{
	coord_type p1 = { 100.0, 100.0 };
	coord_type p2 = { 250.0, 400.0 };
	coord_type p3 = { 400.0, 150.0 };
	tridiv(out, 10, &p1, &p2, &p3);
}
