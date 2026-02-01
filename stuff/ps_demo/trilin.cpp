#include <sstream>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <assert.h>
#include "ps_data.h"
#include "leda\window.h"

#define USE_CMYK_COLORSPACE
#define PLOT_FILLED_TILES
#define PLOT_ORDERING

double distance(double x, double y, double x1, double y1, double x2, double y2)
{
	double dx = x1 - x2;
	double dy = y1 - y2;
	double dd = hypot(dx, dy);
	return ((x - x1) * dy - (y - y1) * dx) / dd;
}

double clamp(double x, double x1, double x2)
{
	if (x < x1) return x1;
	if (x > x2) return x2;
	return x;
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

bool cartesian_to_trilinear(double x1, double y1, double x2, double y2, double x3, double y3, \
	double x, double y, double *alpha, double *betta, double *gamma)
{
	double a = hypot(x3 - x2, y3 - y2);
	double b = hypot(x1 - x3, y1 - y3);
	double c = hypot(x2 - x1, y2 - y1);
	double t1 = ((x - x2) * (y3 - y2) - (y - y2) * (x3 - x2)) / a;
	double t2 = ((x - x3) * (y1 - y3) - (y - y3) * (x1 - x3)) / b;
	double t3 = ((x - x1) * (y2 - y1) - (y - y1) * (x2 - x1)) / c;
	double tt = ((x1 - x2) * (y3 - y2) - (y1 - y2) * (x3 - x2)) / a;
	*alpha = t1 / tt;
	*betta = t2 / tt;
	*gamma = t3 / tt;
	return (t1 >= 0.0 && t2 >= 0.0 && t3 >= 0.0);
}

bool cartesian_to_index(double x1, double y1, double x2, double y2, double x3, double y3, \
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

	double tol = 1.0e-5;
	double row = (e22 * xx - e21 * yy) * sx / ee;
  double col = (e11 * yy - e12 * xx) * sy / ee;
	if (-tol < row && row < 0.0) row = 0.0;
	if (-tol < col && col < 0.0) col = 0.0;
	if (size <= row && row < size+tol) row = size-1;
	if (size <= col && col < size+tol) col = size-1;
	if (row+col >= size)
	{
		row = floor(row);
		col = floor(col);
	}
	if (0.0 <= row && 0.0 <= col && row+col <= size)
	{
		int n_col = (int) col;
		int n_row = (int) row;
  	double delta = col - n_col + row - n_row;
  	*index = 2 * n_row * size + 2 * n_col - n_row * n_row + (int) delta;
  	return true;
	}
	{
  	double t1, t2, t3;
  	cartesian_to_trilinear(x1, y1, x2, y2, x3, y3, x, y, &t1, &t2, &t3);
  	if (t1 < 0.0) t1 = 0.0;
  	if (t2 < 0.0) t2 = 0.0;
  	if (t3 < 0.0) t3 = 0.0;
  	trilinear_to_cartesian(x1, y1, x2, y2, x3, y3, t1, t2, t3, &x, &y);

  	xx = x - x1;
  	yy = y - y1;
  	double row = (e22 * xx - e21 * yy) * sx / ee;
    double col = (e11 * yy - e12 * xx) * sy / ee;
  	if (0.0 <= row && 0.0 <= col && row+col <= size)
  	{
   		int n_col = (int) col;
   		int n_row = (int) row;
     	double delta = col - n_col + row - n_row;
    	if (row+col == size) delta = 0.0;
     	*index = 2 * n_row * size + 2 * n_col - n_row * n_row + (int) delta;
  	}
  	else
  	{
  		*index = -1;
  	}
	}
	return false;
}

void fill_vertex_colors(const coord_type* p1, const coord_type* p2, const coord_type* p3, \
	const color_type* s1, const color_type* s2, const color_type* s3, int n, color_type* s)
{
	std::ofstream out("trilin.txt");
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

				s->r = clamp(s1->r * t1 + s2->r * t2 + s3->r * t3, 0.0, 1.0);
				s->g = clamp(s1->g * t1 + s2->g * t2 + s3->g * t3, 0.0, 1.0);
				s->b = clamp(s1->b * t1 + s2->b * t2 + s3->b * t3, 0.0, 1.0);
				out << s->r << ' ' << s->g << ' ' << s->b << std::endl;
				++s;
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

				s->r = clamp(s1->r * t1 + s2->r * t2 + s3->r * t3, 0.0, 1.0);
				s->g = clamp(s1->g * t1 + s2->g * t2 + s3->g * t3, 0.0, 1.0);
				s->b = clamp(s1->b * t1 + s2->b * t2 + s3->b * t3, 0.0, 1.0);
				out << s->r << ' ' << s->g << ' ' << s->b << std::endl;
				++s;
			}
		}
	}
}

void quadfill_1(psstream& out, int cx, int cy, const double *xx, const double* yy, \
	const double* oo, const double* gg, double dx, double dy)
{
	color_type o_color = { 1.0, 0.5, 0.0 };
	color_type g_color = { 1.0, 1.0, 0.0 };
	color_type w_color = { 0.0, 0.0, 1.0 };

	int cx_refined = int((xx[cx-1]-xx[0])/dx);
	int cy_refined = int((yy[(cy-1)*cx]-yy[0])/dy);
	double* ff_refined = new double[cx_refined * cy_refined];

	for (int j = 1; j < cy; ++j)
	{
		for (int i = 1; i < cx; ++i)
		{
			double x1[4];
			x1[0] = xx[cx*(j-1)+i-1];
			x1[1] = xx[cx*(j-1)+i];
			x1[2] = xx[cx*j+i-1];
			x1[3] = xx[cx*j+i];

			double y1[4];
			y1[0] = yy[cx*(j-1)+i-1];
			y1[1] = yy[cx*(j-1)+i];
			y1[2] = yy[cx*j+i-1];
			y1[3] = yy[cx*j+i];

			double o1[4];
			o1[0] = oo[cx*(j-1)+i-1];
			o1[1] = oo[cx*(j-1)+i];
			o1[2] = oo[cx*j+i-1];
			o1[3] = oo[cx*j+i];

			double g1[4];
			g1[0] = gg[cx*(j-1)+i-1];
			g1[1] = gg[cx*(j-1)+i];
			g1[2] = gg[cx*j+i-1];
			g1[3] = gg[cx*j+i];

			int nx = (int) ceil(std::max(hypot(x1[1] - x1[0], y1[1] - y1[0]), \
				hypot(x1[3] - x1[2], y1[3] - y1[2])) / dx);
			int ny = (int) ceil(std::max(hypot(x1[2] - x1[0], y1[2] - y1[0]), \
				hypot(x1[3] - x1[1], y1[3] - y1[1])) / dy);
			for (int jj = 1; jj <= ny; ++jj)
			{
				double v1 = double(jj-1) / ny;
				double v2 = double(jj) / ny;
				double v = (jj-0.5) / ny;

				double x2[4];
				x2[0] = x1[0] + (x1[2] - x1[0]) * v1;
				x2[1] = x1[1] + (x1[3] - x1[1]) * v1;
				x2[2] = x1[0] + (x1[2] - x1[0]) * v2;
				x2[3] = x1[1] + (x1[3] - x1[1]) * v2;
				
				double y2[4];
				y2[0] = y1[0] + (y1[2] - y1[0]) * v1;
				y2[1] = y1[1] + (y1[3] - y1[1]) * v1;
				y2[2] = y1[0] + (y1[2] - y1[0]) * v2;
				y2[3] = y1[1] + (y1[3] - y1[1]) * v2;
				
				for (int ii = 1; ii <= nx; ++ii)
				{
					double u1 = double(ii-1) / nx;
					double u2 = double(ii) / nx;
					double u = (ii - 0.5) / nx;
					
  				double x3[4];
  				x3[0] = x2[0] + (x2[1] - x2[0]) * u1;
  				x3[1] = x2[0] + (x2[1] - x2[0]) * u2;
  				x3[2] = x2[2] + (x2[3] - x2[2]) * u1;
  				x3[3] = x2[2] + (x2[3] - x2[2]) * u2;
  				
  				double y3[4];
  				y3[0] = y2[0] + (y2[1] - y2[0]) * u1;
  				y3[1] = y2[0] + (y2[1] - y2[0]) * u2;
  				y3[2] = y2[2] + (y2[3] - y2[2]) * u1;
  				y3[3] = y2[2] + (y2[3] - y2[2]) * u2;
  				
  				double o = o1[0] * (1.0 - u) * (1.0 - v) + o1[1] * u * (1.0 - v) \
  					+ o1[2] * (1.0 - u) * v + o1[3] * u * v;
  				double g = g1[0] * (1.0 - u) * (1.0 - v) + g1[1] * u * (1.0 - v) \
  					+ g1[2] * (1.0 - u) * v + g1[3] * u * v;
  				assert(0.0 <= o && 0.0 <= g && o + g <= 1.0);
  				double w = 1.0 - o - g;

  				ff_refined[(i-1)*nx+ii-1 + ((j-1)*ny+jj-1) * cx_refined] = o;

  				out.newpath();
  				out.setrgbcolor(o_color.r * o + g_color.r * g + w_color.r * w,
  					o_color.g * o + g_color.g * g + w_color.g * w,
  					o_color.b * o + g_color.b * g + w_color.b * w);
  				out.moveto(x3[0], y3[0]);
  				out.lineto(x3[1], y3[1]);
  				out.lineto(x3[3], y3[3]);
  				out.lineto(x3[2], y3[2]);
  				out.lineto(x3[0], y3[0]);
  				out.fill();
				}
			}
		}
	}

#if 0
	std::cout << "DSAA\n" << cx_refined << ' ' << cy_refined << "\n0 1\n0 1\n0 1\n";
	for (int j = 0, k = 0; j < cy_refined; ++j)
	{
		for (int i = 0; i < cx_refined; ++i, ++k)
		{
			std::cout << std::setw(10) << ff_refined[k];
		}
		std::cout << std::endl;
	}
#endif
	delete[] ff_refined;
}

void quadfill(psstream& out, int cx, int cy, const double *xx, const double* yy, \
	const double* oo, const double* gg, double dx, double dy)
{
	color_type o_color = { 1.0, 0.5, 0.0 };
	color_type g_color = { 1.0, 1.0, 0.0 };
	color_type w_color = { 0.0, 0.0, 1.0 };

	double x_oil = 100.0, y_oil = 100.0;
	double x_gas = 250.0, y_gas = 400.0;
	double x_wat = 400.0, y_wat = 100.0;

	coord_type o_coord = { x_oil, y_oil };
	coord_type g_coord = { x_gas, y_gas };
	coord_type w_coord = { x_wat, y_wat };

	const int n_level = 10;
	const int n_shade = n_level * n_level;
	color_type shade[n_shade];
	fill_vertex_colors(&o_coord, &g_coord, &w_coord, &o_color, &g_color, &w_color, n_level, shade);

	for (int j = 1; j < cy; ++j)
	{
		for (int i = 1; i < cx; ++i)
		{
			double x1[4];
			x1[0] = xx[cx*(j-1)+i-1];
			x1[1] = xx[cx*(j-1)+i];
			x1[2] = xx[cx*j+i-1];
			x1[3] = xx[cx*j+i];

			double y1[4];
			y1[0] = yy[cx*(j-1)+i-1];
			y1[1] = yy[cx*(j-1)+i];
			y1[2] = yy[cx*j+i-1];
			y1[3] = yy[cx*j+i];

			double o1[4];
			o1[0] = oo[cx*(j-1)+i-1];
			o1[1] = oo[cx*(j-1)+i];
			o1[2] = oo[cx*j+i-1];
			o1[3] = oo[cx*j+i];

			double g1[4];
			g1[0] = gg[cx*(j-1)+i-1];
			g1[1] = gg[cx*(j-1)+i];
			g1[2] = gg[cx*j+i-1];
			g1[3] = gg[cx*j+i];

			int nx = (int) ceil(std::max(hypot(x1[1] - x1[0], y1[1] - y1[0]), \
				hypot(x1[3] - x1[2], y1[3] - y1[2])) / dx);
			int ny = (int) ceil(std::max(hypot(x1[2] - x1[0], y1[2] - y1[0]), \
				hypot(x1[3] - x1[1], y1[3] - y1[1])) / dy);
			for (int jj = 1; jj <= ny; ++jj)
			{
				double v1 = double(jj-1) / ny;
				double v2 = double(jj) / ny;
				double v = (jj-0.5) / ny;

				double x2[4];
				x2[0] = x1[0] + (x1[2] - x1[0]) * v1;
				x2[1] = x1[1] + (x1[3] - x1[1]) * v1;
				x2[2] = x1[0] + (x1[2] - x1[0]) * v2;
				x2[3] = x1[1] + (x1[3] - x1[1]) * v2;
				
				double y2[4];
				y2[0] = y1[0] + (y1[2] - y1[0]) * v1;
				y2[1] = y1[1] + (y1[3] - y1[1]) * v1;
				y2[2] = y1[0] + (y1[2] - y1[0]) * v2;
				y2[3] = y1[1] + (y1[3] - y1[1]) * v2;
				
				for (int ii = 1; ii <= nx; ++ii)
				{
					double u1 = double(ii-1) / nx;
					double u2 = double(ii) / nx;
					double u = (ii - 0.5) / nx;
					
  				double x3[4];
  				x3[0] = x2[0] + (x2[1] - x2[0]) * u1;
  				x3[1] = x2[0] + (x2[1] - x2[0]) * u2;
  				x3[2] = x2[2] + (x2[3] - x2[2]) * u1;
  				x3[3] = x2[2] + (x2[3] - x2[2]) * u2;
  				
  				double y3[4];
  				y3[0] = y2[0] + (y2[1] - y2[0]) * u1;
  				y3[1] = y2[0] + (y2[1] - y2[0]) * u2;
  				y3[2] = y2[2] + (y2[3] - y2[2]) * u1;
  				y3[3] = y2[2] + (y2[3] - y2[2]) * u2;
  				
  				double o = o1[0] * (1.0 - u) * (1.0 - v) + o1[1] * u * (1.0 - v) \
  					+ o1[2] * (1.0 - u) * v + o1[3] * u * v;
  				double g = g1[0] * (1.0 - u) * (1.0 - v) + g1[1] * u * (1.0 - v) \
  					+ g1[2] * (1.0 - u) * v + g1[3] * u * v;
  				assert(0.0 <= o && 0.0 <= g && o + g <= 1.0);
  				double w = 1.0 - o - g;

  				double xx, yy;
  				trilinear_to_cartesian(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, o, g, w, &xx, &yy);

    			int index;
    			if (cartesian_to_index(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, xx, yy, n_level, &index))
    			{
      			assert(0 <= index && index < n_shade);
    				out.newpath();
    				out.setrgbcolor(shade[index].r, shade[index].g, shade[index].b);
    				out.moveto(x3[0], y3[0]);
    				out.lineto(x3[1], y3[1]);
    				out.lineto(x3[3], y3[3]);
    				out.lineto(x3[2], y3[2]);
    				out.lineto(x3[0], y3[0]);
    				out.fill();
    			}
				}
			}
		}
	}

	{
		double o = 1.0;
		double g = 0.0;
		double w = 0.0;

 		double xx, yy;
 		trilinear_to_cartesian(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, o, g, w, &xx, &yy);

 		int index;
 		cartesian_to_index(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, xx, yy, n_level, &index);
 		assert(0 <= index && index < n_shade);

 		std::cout << index << std::endl;
	}

	{
		double o = 0.0;
		double g = 1.0;
		double w = 0.0;

 		double xx, yy;
 		trilinear_to_cartesian(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, o, g, w, &xx, &yy);

 		int index;
 		if (cartesian_to_index(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, xx, yy, n_level, &index))
 		{
   		std::cout << index << std::endl;
   		assert(0 <= index && index < n_shade);
 		}
 		else
 		{
   		std::cout << index << std::endl;
 			std::cout << "failed compute index for (" << o << ',' << g << ")\n";
 		}
	}

	{
		double o = 0.0;
		double g = 0.0;
		double w = 1.0;

 		double xx, yy;
 		trilinear_to_cartesian(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, o, g, w, &xx, &yy);

 		int index;
 		if (cartesian_to_index(x_oil, y_oil, x_gas, y_gas, x_wat, y_wat, xx, yy, n_level, &index))
 		{
   		std::cout << index << std::endl;
   		assert(0 <= index && index < n_shade);
 		}
 		else
 		{
   		std::cout << index << std::endl;
 			std::cout << "failed compute index for (" << o << ',' << g << ")\n";
 		}
	}
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

	{
	  double bb[4];
  	out.setgray(0.0);
  	label(out, p1->xcoord, p1->ycoord, 1.0, psstream::LABEL_SW, true, "So", bb);
  	label(out, p2->xcoord, p2->ycoord, 1.0, psstream::LABEL_N, true, "Sg", bb);
  	label(out, p3->xcoord, p3->ycoord, 1.0, psstream::LABEL_SE, true, "Sw", bb);
	}

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
  			out.annot(x, y, 0.0, 0.0, psstream::ANNOT_CENTER, true, ss.str().c_str(), bb);
			}
		}
	}
}

void leda_tridiv(leda::window& win, int n, const coord_type* p1, const coord_type* p2, const coord_type* p3)
{
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

				int r = int((r_oil * t1 + r_gas * t2 + r_wat * t3) * 255.0 + 0.5);
				int g = int((g_oil * t1 + g_gas * t2 + g_wat * t3) * 255.0 + 0.5);
				int b = int((b_oil * t1 + b_gas * t2 + b_wat * t3) * 255.0 + 0.5);

				win.draw_filled_triangle(leda::point(x1, y1), leda::point(x2, y2), leda::point(x3, y3), leda::color(r, g, b));
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

				int r = int((r_oil * t1 + r_gas * t2 + r_wat * t3) * 255.0 + 0.5);
				int g = int((g_oil * t1 + g_gas * t2 + g_wat * t3) * 255.0 + 0.5);
				int b = int((b_oil * t1 + b_gas * t2 + b_wat * t3) * 255.0 + 0.5);

				win.draw_filled_triangle(leda::point(x4, y4), leda::point(x2, y2), leda::point(x3, y3), leda::color(r, g, b));
			}
		}
	}

	leda::point p;
	while (win >> p)
	{
		int index;
		cartesian_to_index(p1->xcoord, p1->ycoord, p2->xcoord, p2->ycoord, \
			p3->xcoord, p3->ycoord, p.xcoord(), p.ycoord(), n, &index);

		std::cout << index << std::endl;
	}
}

void check_trilin(psstream& out)
{
	leda::window win(500, 500);
	win.init(0.0, 500.0, 0.0);
	win.display();

	coord_type p1 = { 100.0, 100.0 };
	coord_type p2 = { 250.0, 400.0 };
	coord_type p3 = { 400.0, 150.0 };
	tridiv(out, 10, &p1, &p2, &p3);
	leda_tridiv(win, 10, &p1, &p2, &p3);

	double xx[] =
  {
    100.0, 200.0, 300.0,
    100.0, 200.0, 300.0,
    100.0, 200.0, 300.0
  };

  double yy[] =
  {
    100.0, 100.0, 100.0,
    200.0, 200.0, 200.0,
    300.0, 300.0, 300.0
  };

  double gg[] =
  {
    0.51, 0.0, 0.0,
    0.0, 0.0, 0.0,
    0.0, 0.0, 0.0
  };

  double oo[] =
  {
    0.0, 0.0, 0.0,
    0.0, 0.0, 0.0,
    0.0, 0.0, 0.0
  };

  /*
  [
    100.0 50.0  10.0
    50.0  50.0  1.0
    10.0  1.0   0.1
  ]
  */

	out.newpage();
	out.setgray(0.0);
	out.setlinewidth(0.0);
	quadfill_1(out, 3, 3, xx, yy, oo, gg, 10.0, 10.0);

	out.newpage();
	out.setgray(0.0);
	out.setlinewidth(0.0);
	quadfill(out, 3, 3, xx, yy, oo, gg, 10.0, 10.0);
}
