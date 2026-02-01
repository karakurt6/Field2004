#include <cstdio>
#include <algorithm>

char ps_init[] =
  "%%!PS-Adobe-3.0\n"
  "%%%%DocumentMedia: a4 595 842 80 white ( )\n"
  "%%%%Orientation: Portrait\n"
  "%%%%Pages: 1\n"
  "%%%%Page: 1 1\n\n"

  "%% s x0 y0 tsize x1 y1 x2 y2\n"
  "%% returns a bounding box\n"
  "%% (x1 x2) x (y1 y2) around\n"
  "%% text string 's'\n"
  "/tsize\n"
  "{\n"
  "  gsave\n"
  "  newpath\n"
  "  moveto\n"
  "  true charpath\n"
  "  flattenpath\n"
  "  pathbbox\n"
  "  grestore\n"
  "}\n"
  "def\n\n"


  "%% x y a s ctext -\n"
  "%% plots a text string 's'\n"
  "%% centered at point (x y)\n"
  "%% and rotated by angle 'a'\n"
  "/ctext\n"
  "{\n"
  "  matrix currentmatrix\n"
  "  4 index 4 index translate\n"
  "  1 index 0 0 tsize\n"
  "  3 -1 roll pop\n"
  "  3 -1 roll pop\n"
  "  -2 div exch\n"
  "  -2 div exch\n"
  "  0 0 moveto\n"
  "  rmoveto\n"
  "  currentpoint\n"
  "  4 index rotate\n"
  "  moveto\n"
  "  1 index show\n"
  "  setmatrix\n"
  "  pop pop pop pop\n"
  "}\n"
  "def\n\n"

  "%% x y a s crect -\n"
  "%% strokes a rectangle\n"
  "%% around text string 's'\n"
  "%% centered at point (x y)\n"
  "%% and rotated by angle 'a'\n"
  "/crect\n"
  "{\n"
  "  matrix currentmatrix\n"
  "  4 index 4 index translate\n"
  "  1 index 0 0 tsize\n"
  "  3 -1 roll pop\n"
  "  3 -1 roll pop\n"
  "  -2 div exch\n"
  "  -2 div exch\n"
  "  0 0 moveto\n"
  "  rmoveto\n"
  "  1 index currentpoint tsize\n"
  "  2 index sub exch\n"
  "  3 index sub exch\n"
  "  6 index rotate\n"
  "  3 index 3 index moveto\n"
  "  1 index 0 rlineto\n"
  "  0 1 index rlineto\n"
  "  1 index neg 0 rlineto\n"
  "  0 1 index neg rlineto\n"
  "  pop pop pop pop\n"
  "  setmatrix\n"
  "  pop pop pop pop\n"
  "}\n"
  "def\n\n"

  "%% x y a s ltext -\n"
  "%% plots a text string 's' with left side\n"
  "%% adjusted to point (x, y)\n"
  "%% and rotated by angle 'a'\n"
  "/ltext\n"
  "{\n"
  "  matrix currentmatrix\n"
  "  4 index 4 index translate\n"
  "  1 index 0 0 tsize\n"
  "  3 -1 roll pop\n"
  "  3 -1 roll pop\n"
  "  -2 div exch\n"
  "  pop dup neg exch\n"
  "  0 0 moveto\n"
  "  rmoveto\n"
  "  currentpoint\n"
  "  4 index rotate\n"
  "  moveto\n"
  "  1 index show\n"
  "  setmatrix\n"
  "  pop pop pop pop\n"
  "}\n"
  "def\n\n"

  "%% x y a s rtext -\n"
  "%% plots a text string 's' with right side\n"
  "%% adjusted to point (x, y)\n"
  "%% and rotated by angle 'a'\n"
  "/rtext\n"
  "{\n"
  "  matrix currentmatrix\n"
  "  4 index 4 index translate\n"
  "  1 index 0 0 tsize\n"
  "  3 -1 roll pop\n"
  "  3 -1 roll pop\n"
  "  -2 div exch\n"
  "  neg 1 index add exch\n"
  "  0 0 moveto\n"
  "  rmoveto\n"
  "  currentpoint\n"
  "  4 index rotate\n"
  "  moveto\n"
  "  1 index show\n"
  "  setmatrix\n"
  "  pop pop pop pop\n"
  "}\n"
  "def\n\n";

extern "C" {
#include "double\mconf.h"
}

double pval[] =
{
  0.0001, 0.001, 0.002, 0.01, 0.02, 0.05, 0.10, 0.20, 0.30,
  0.40, 0.50, 0.60, 0.70, 0.80, 0.90, 0.95, 0.98, 0.99, 0.998,
  0.999, 0.9999
};

void main()
{
  double u = 595.0 * 0.1, w = 595.0 * 0.8;
  double v = 842.0 * 0.1, h = 842.0 * 0.8;

  double x1 = 100.0, y1 = 1.0e-5;
  double x2 = 200.0, y2 = 1.0e3;

  FILE* ps_file = stdout;
  fprintf(ps_file, ps_init);
  fprintf(ps_file, "/Times-Roman findfont 6 scalefont setfont\n");

  int cx = 100; // arithmetic scaling
  {
    fprintf(ps_file, "0.2 setgray 0.5 setlinewidth\n");

    for (int i = 0; i <= cx; i += 10)
    {
      double x = x1 + (x2 - x1) * i / cx, t = u + w * (x - x1) / (x2 - x1);
      fprintf(ps_file, "%g %g 270 (%g) ltext\n", t, v, x);
      fprintf(ps_file, "newpath %g %g moveto 0 %g rlineto stroke\n", t, v, h);
      if (i != cx)
      {
        fprintf(ps_file, "0.5 setgray 0.1 setlinewidth\n");
        for (int j = 1; j < 10; ++j)
        {
          x = x1 + (x2 - x1) * (i + j) / cx, t = u + w * (x - x1) / (x2 - x1);
          fprintf(ps_file, "newpath %g %g moveto 0 %g rlineto stroke\n", t, v, h);
        }
        fprintf(ps_file, "0.2 setgray 0.5 setlinewidth\n");
      }
    }
  }

  /*
  int cy = 80; // logarithmic scaling
  {
    fprintf(ps_file, "0.2 setgray 0.5 setlinewidth\n");

    double d1 = log10(y1), d2 = log10(y2), s = y1;
    for (int i = 0; i <= cy; i += 10)
    {
      double t = v + h * (log10(s) - d1) / (d2 - d1);
      fprintf(ps_file, "%g %g 0 (%g) rtext\n", u, t, s);
      fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);
      if (i != cy)
      {
        fprintf(ps_file, "0.5 setgray 0.01 setlinewidth\n");
        for (int j = 2; j < 10; ++j)
        {
          t = v + h * (log10(s*j) - d1) / (d2 - d1);
          fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);
        }
        fprintf(ps_file, "0.2 setgray 0.05 setlinewidth\n");
      }
      s *= 10;
    }
  }
  */

  /*
  {
    int n = sizeof(pval) / sizeof(*pval);
    double d1 = ndtri(pval[0]), d2 = ndtri(pval[n-1]);
    for (int i = 0; i < n; ++i)
    {
      double s = pval[i];
      double t = v + h * (ndtri(s) - d1) / (d2 - d1);
      fprintf(ps_file, "%g %g 0 (%g) rtext\n", u, t, s);
      fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);
    }
  }
  */

  {
    fprintf(ps_file, "0.2 setgray 0.5 setlinewidth\n");

    double s = 0.0001;
    double d1 = ndtri(0.0001), d2 = ndtri(0.9999);
    double t = v + h * (ndtri(s) - d1) / (d2 - d1);
    fprintf(ps_file, "%g %g 0 (%g) rtext\n", u, t, s);
    fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);

    for (int i = 1; i < 10; ++i)
    {
      s = 0.01 * i;
      t = v + h * (ndtri(s) - d1) / (d2 - d1);
      fprintf(ps_file, "%g %g 0 (%g) rtext\n", u, t, s);
      fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);
    }

    for (i = 0; i < 80 / 5; ++i)
    {
      s = 0.1 + 0.05 * i;
      t = v + h * (ndtri(s) - d1) / (d2 - d1);
      fprintf(ps_file, "%g %g 0 (%g) rtext\n", u, t, s);
      fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);

      fprintf(ps_file, "0.5 setgray 0.1 setlinewidth\n");
      for (int j = 1; j < 5; j++)
      {
        t = v + h * (ndtri(s + 0.01 * j) - d1) / (d2 - d1);
        fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);        
      }
      fprintf(ps_file, "0.2 setgray 0.5 setlinewidth\n");
    }

    for (i = 0; i < 10; ++i)
    {
      s = 0.9 + 0.01 * i;
      t = v + h * (ndtri(s) - d1) / (d2 - d1);
      fprintf(ps_file, "%g %g 0 (%g) rtext\n", u, t, s);
      fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);
    }

    s = 0.9999;
    t = v + h * (ndtri(s) - d1) / (d2 - d1);
    fprintf(ps_file, "%g %g 0 (%g) rtext\n", u, t, s);
    fprintf(ps_file, "newpath %g %g moveto %g 0 rlineto stroke\n", u, t, w);
  }

  fprintf(ps_file, "showpage\n");
}
