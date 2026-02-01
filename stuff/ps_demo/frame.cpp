#include <math.h>
#include <iostream>
#include <double\mconf.h>
#include "ps_data.h"
#include "demo.h"

void check_frame_1(psstream& out)
{
  int page_width = (int) out.pagewidth();
  int page_height = (int) out.pageheight();
  int x_major_tick = 500;
  int x_minor_tick = 50;
  int y_major_tick = 10;
  int y_minor_tick = 1;
  int left_margin = 100;
  int right_margin = 100;
  int top_margin = 80;
  int bottom_margin = 80;
  
  out.newpage();
  
  out.rectstroke(left_margin - 5, bottom_margin - 5, \
    page_width - left_margin - right_margin + 10, \
    page_height - top_margin - bottom_margin + 10);

  out.rectstroke(left_margin - 7, bottom_margin - 7, \
    page_width - left_margin - right_margin + 14, \
    page_height - top_margin - bottom_margin + 14);

  double y_range[2] = { 2112.5, 2176.3 };
  
  double orig = y_range[1];
  double range = y_range[0] - y_range[1];
  double dy = page_height - top_margin - bottom_margin;
  double x0 = left_margin-7;
  double y0 = bottom_margin;
  out.vertical_scale((int) ceil(y_range[0] / y_minor_tick) * y_minor_tick,
    y_minor_tick, (int) floor(y_range[1] / y_minor_tick) * y_minor_tick,
    orig, range, dy, x0, y0, -1.5);

  out.setfont(FONT_27, 10);

  int start = (int) ceil(y_range[0] / y_major_tick) * y_major_tick;
  int step = y_major_tick;
  int final = (int) floor(y_range[1] / y_major_tick) * y_major_tick;
  int n = (final - start) / step;
  double sx = -3.0;

  for (int i = 0; i <= n; ++i)
  {
    double bb[4];
    char text[10];
    double tick = start + step * i;
    sprintf(text, "%g", tick);
    label(out, x0+sx, y0 + dy * (tick - orig) / range, 1.0, psstream::LABEL_W, true, text, bb);
  }

  out.vertical_scale(start, step, final, orig, range, dy, x0, y0, sx);
}

void check_frame_2(psstream& out)
{
  out.newpage();

  psstream::Scale scale;
  scale.type = psstream::Scale::ARITHMETIC;
  scale.label = psstream::LABEL_W;
  scale.flags = psstream::Scale::HAS_GRID_LINES \
  	| psstream::Scale::HAS_OUTER_TICK | psstream::Scale::HAS_INNER_TICK;
  scale.range[0] = 13860.0;
  scale.range[1] = 13900.0;
  scale.region[0] = 40.0;
  scale.region[1] = 40.0;
  scale.region[2] = out.pagewidth() - scale.region[0];
  scale.region[3] = out.pageheight() - scale.region[1];
  std::swap(scale.region[1], scale.region[3]);
  scale.margin = 2.0;
  scale.tick_length = 2.0;
  scale.line_width = 0.0;
  scale.font_name = "Helvetica";
  scale.font_size = 8.0;
  scale.color.r = 1.0;
  scale.color.g = 0.0;
  scale.color.b = 0.0;
  scale.units = 0;
  out.horizontal(scale);

  scale.flags = psstream::Scale::HAS_GRID_LINES | psstream::Scale::HAS_OUTER_FRAME \
  	| psstream::Scale::HAS_OUTER_TICK | psstream::Scale::HAS_INNER_TICK \
  	| psstream::Scale::HAS_LABEL_TEXT;
  scale.range[0] = 1386.0;
  scale.range[1] = 1390.0;
  scale.tick_length = 2.0;
  scale.line_width = 1.0;
  out.horizontal(scale);
}

void check_frame_3(psstream& out)
{
  double bbox[4];
	out.newpage();
	out.setfont("Helvetica", 10);
	out.setgray(0);
	out.setlinewidth(1);

	out.moveto(100, 100);
	out.lineto(400, 100);
	out.stroke();

	out.setgray(1);
	out.arc(200, 100, 2, 0, 360);
	out.fill();
	out.setgray(0);
	out.arc(200, 100, 2, 0, 360);
	out.stroke();
	hlabel(out, 200, 100, 3, psstream::LABEL_N, true, "LABEL_N", bbox);
	hlabel(out, 200, 100, 3, psstream::LABEL_S, true, "LABEL_S", bbox);
	
	out.setgray(1);
	out.arc(275, 100, 2, 0, 360);
	out.fill();
	out.setgray(0);
	out.arc(275, 100, 2, 0, 360);
	out.stroke();
	hlabel(out, 275, 100, 3, psstream::LABEL_E, true, "LABEL_E", bbox);
	hlabel(out, 275, 100, 3, psstream::LABEL_SE, true, "LABEL_SE", bbox);
	
	out.setgray(1);
	out.arc(325, 100, 2, 0, 360);
	out.fill();
	out.setgray(0);
	out.arc(325, 100, 2, 0, 360);
	out.stroke();
	hlabel(out, 325, 100, 3, psstream::LABEL_SW, true, "LABEL_SW", bbox);
	hlabel(out, 325, 100, 3, psstream::LABEL_W, true, "LABEL_W", bbox);

	out.moveto(100, 100);
	out.lineto(100, 400);
	out.stroke();

	out.setgray(1);
	out.arc(100, 200, 2, 0, 360);
	out.fill();
	out.setgray(0);
	out.arc(100, 200, 2, 0, 360);
	out.stroke();
	vlabel(out, 100, 200, 3, psstream::LABEL_E, true, "LABEL_E", bbox);
	vlabel(out, 100, 200, 3, psstream::LABEL_W, true, "LABEL_W", bbox);

	out.setgray(1);
	out.arc(100, 250, 2, 0, 360);
	out.fill();
	out.setgray(0);
	out.arc(100, 250, 2, 0, 360);
	out.stroke();
	vlabel(out, 100, 250, 3, psstream::LABEL_S, true, "LABEL_S", bbox);
	vlabel(out, 100, 250, 3, psstream::LABEL_SE, true, "LABEL_SE", bbox);

	out.setgray(1);
	out.arc(100, 325, 2, 0, 360);
	out.fill();
	out.setgray(0);
	out.arc(100, 325, 2, 0, 360);
	out.stroke();
	vlabel(out, 100, 325, 3, psstream::LABEL_N, true, "LABEL_N", bbox);
	vlabel(out, 100, 325, 3, psstream::LABEL_NE, true, "LABEL_NE", bbox);
}

void check_frame_arithmetic(psstream& out)
{
	double x1 = 1386.0, x2 = 1424.0;
	double y1 = 711.0, y2 = 782.0;

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

	sx /= dx;
	sy /= dy;
	x0 -= x1*sx;
	y0 -= y1*sy;

	out.newpage();
  {
    out.setlinewidth(0.0);
    int step = 1;
    int cx = (int) floor(x2 / step) * step;
    for (int i = (int) ceil(x1 / step) * step; i <= cx; i += step)
    {
    	out.moveto(x0+i*sx, y0+y1*sy);
    	out.lineto(x0+i*sx, y0+y2*sy);
    }
    int cy = (int) floor(y2 / step) * step;
    for (int i = (int) ceil(y1 / step) * step; i <= cy; i += step)
    {
    	out.moveto(x0+x1*sx, y0+i*sy);
    	out.lineto(x0+x2*sx, y0+i*sy);
    }
  	out.stroke();
  }

  {
  	int pattern[1] = { 3 };
    out.setlinewidth(1.0);
    out.setdash(1, pattern, 0);
    int step = 5;
    int cx = (int) floor(x2 / step) * step;
    for (int i = (int) ceil(x1 / step) * step; i <= cx; i += step)
    {
    	out.moveto(x0+i*sx, y0+y1*sy);
    	out.lineto(x0+i*sx, y0+y2*sy);
    }
    int cy = (int) floor(y2 / step) * step;
    for (int i = (int) ceil(y1 / step) * step; i <= cy; i += step)
    {
    	out.moveto(x0+x1*sx, y0+i*sy);
    	out.lineto(x0+x2*sx, y0+i*sy);
    }
  	out.stroke();
  	out.setdash(0, 0, 0);
  }

  {
    out.setlinewidth(1.0);
    int step = 10;
    int cx = (int) floor(x2 / step) * step;
    for (int i = (int) ceil(x1 / step) * step; i <= cx; i += step)
    {
    	out.moveto(x0+i*sx, y0+y1*sy);
    	out.lineto(x0+i*sx, y0+y2*sy);
    }
    int cy = (int) floor(y2 / step) * step;
    for (int i = (int) ceil(y1 / step) * step; i <= cy; i += step)
    {
    	out.moveto(x0+x1*sx, y0+i*sy);
    	out.lineto(x0+x2*sx, y0+i*sy);
    }
  	out.stroke();
  }
}

void check_frame(psstream& out)
{
	out.newpage();

	double k_min = -3;
	double k_max = 3;

  double pg_width = out.pagewidth();
  double pg_height = out.pageheight();
  double pg_margin = 40.0;
  double px = pg_width - 2.0 * pg_margin;
  double py = pg_height - 2.0 * pg_margin;
  double x0 = pg_margin;
  double y0 = pg_margin;

  out.setgray(0.0);
	double dx = px / (k_max - k_min);
	const char* font = "Helvetica";
	for (int k = k_min; k <= k_max; ++k)
	{
		double bbox[4];
		char text[8];
	  double x2 = x0 + px * (k - k_min) / (k_max - k_min);
	  if (k != k_min)
	  {
  	  out.setlinewidth(0.0);
  	  for (int kk = 1; kk < 9; ++kk)
  	  {
  	  	double x1 = x2 + dx * (log10(1.0 + kk) - 1.0);
  	  	out.moveto(x1, y0);
  	  	out.lineto(x1, y0+py);
  	  	out.stroke();
  	  }
	  }
	  out.setlinewidth(1.0);
	  out.moveto(x2, y0);
	  out.lineto(x2, y0+py);
	  out.stroke();
	  out.setfont(font, 10.0);
	  hlabel(out, x2, y0, 8.0, psstream::LABEL_S, true, "10", bbox);
	  sprintf(text, "%d", k);
	  out.setfont(font, 6.0);
	  label(out, bbox[2], bbox[3], 2.0, psstream::LABEL_NE, true, text, bbox);
	}

	double p1 = ndtri(0.01);
	double p2 = ndtri(0.99);
	out.setlinewidth(0.0);
	for (int k = 1; k < 100; ++k)
	{
		double y = y0 + py * (ndtri(0.01*k) - p1) / (p2 - p1);
		out.moveto(x0, y);
		out.lineto(x0+px, y);
	}
	out.stroke();

	out.setlinewidth(1.0);
	out.setfont(font, 10.0);
	for (int k = 10; k < 100; k+=10)
	{
		char text[8];
		double bbox[4];
		double y = y0 + py * (ndtri(0.01*k) - p1) / (p2 - p1);
		out.moveto(x0, y);
		out.lineto(x0+px, y);
  	out.stroke();
  	sprintf(text, "%d", k);
  	label(out, x0, y, 5, psstream::LABEL_W, true, text, bbox);
	}
}
