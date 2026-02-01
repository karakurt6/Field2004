#define _USE_MATH_DEFINES
#ifndef __INTEL_COMPILER
  #include <math.h>
#else
  #include <mathimf.h>
#endif

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <list>

#include "ps_data.h"

typedef std::list<label_site*> label_list;
typedef std::vector<label_site*> label_vector;

/* 
 * Fast Circle-Rectangle Intersection Checking
 * by Clifford A. Shaffer
 * from "Graphics Gems", Academic Press, 1990
 */
bool rc_intersect(const double* bb, double x0, double y0, double r)
{
  double r2 = r * r;

  // Translate coordinates, placing the circle center at the origin
  double x1 = bb[0] - x0, x2 = bb[2] - x0;
  double y1 = bb[1] - y0, y2 = bb[3] - y0;

  if (x2 < 0.0)                                // R to left of circle center
  {
    if (y2 < 0.0)                              // R in left corner
    {
      return (x2 * x2 + y2 * y2 < r2);
    }
    else if (y1 > 0.0)                         // R in upper left corner
    {
      return (x2 * x2 + y1 * y1 < r2);
    }
    else                                       // R due West of circle
    {
      return (fabs(x2) < r);
    }
  }
  else if (x1 > 0.0)                           // R to right of circle center
  {
    if (y2 < 0.0)                              // R in lower right corner
    {
      return (x1 * x1 < r2);
    }
    else if (y1 > 0.0)                         // R in upper right corner
    {
      return (x1 * x1 + y1 * y1 < r2);
    }
    else                                       // R due East of circle
    {
      return (x1 < r);
    }
  }
  else
  {
    if (y2 < 0.0)                              // R due South of circle
    {
      return (fabs(y2) < r);
    }
    else if (y1 > 0.0)                         // R due North of circle
    {
      return (y1 < r);                 
    }
    else                                       // R contains circle centerpoint
    {
      return true;
    }
  }
}

bool rr_intersect(const double *b1, const double *b2)
{
  return !(b1[2] <= b2[0] || b2[2] <= b1[0] || b1[3] <= b2[1] || b2[3] <= b1[1]);
}

void compute_conflict_graph(int n, label_site** pflp)
{
  int j = 4 * n;
  double *bb = new double[j];
  for (int i = j = 0; i < n; ++i)
  {
    label_site* s = pflp[i];
    double dx = s->radius + s->bbox[2] - s->bbox[0];
    double dy = s->radius + s->bbox[3] - s->bbox[1];

    bb[j++] = s->xcoord - dx;
    bb[j++] = s->ycoord - dy;
    bb[j++] = s->xcoord + dx;
    bb[j++] = s->ycoord + dy;

    s->degree = 0;
  }

  for (i = 0; i < n; ++i)
  {
    j = i;
    label_site* p = pflp[i];
    while (++j < n)
    {
      if (!rr_intersect(&bb[i*4], &bb[j*4]))
        continue;
      label_site* q = pflp[j];
      ++p->degree;
      ++q->degree;
    }
    p->cfl = new label_site* [p->degree];
    p->degree = 0;
  }

  for (i = 0; i < n; ++i)
  {
    j = i;
    label_site* p = pflp[i];
    while (++j < n)
    {
      if (!rr_intersect(&bb[i*4], &bb[j*4]))
        continue;
      label_site* q = pflp[j];
      p->cfl[p->degree++] = q;
      q->cfl[q->degree++] = p;
    }
  }
  delete[] bb;
}

int compute_cost(label_site* p)
{
  int cost = 0;
  for (int i = 0; i < p->degree; ++i)
  {
    label_site* q = p->cfl[i];
    if (rc_intersect(p->state, q->xcoord, q->ycoord, q->radius) || rr_intersect(p->state, q->state))
      ++cost;
  }
  return cost;
}

void update_state(label_site* p)
{
  p->state[0] = p->bbox[0];
  p->state[1] = p->bbox[1];
  p->state[2] = p->bbox[2];
  p->state[3] = p->bbox[3];
  textsite(p->xcoord, p->ycoord, p->radius, p->param, p->state);
}

int setup_initial_state(int n, label_site** pflp)
{
  // int n = pflp->numSites;
  for (int i = 0; i < n; ++i)
  {
    label_site* p = pflp[i];
    if (p->degree == 0)
    {
      p->param = 0.0;
    }
    else
    {
      int m = 0;
      double sx = 0.0;
      double sy = 0.0;
      for (int j = 0; j < p->degree; ++j)
      {
        label_site* q = p->cfl[j];
        double dx = q->xcoord - p->xcoord;
        double dy = q->ycoord - p->ycoord;
        double dd = hypot(dx, dy);
        if (dd > 0.01)
        {
          sx += dx / dd;
          sy += dy / dd;
          ++m;
        }
      }
      if (m > 0)
      {
        double a = (atan2(sy, sx) + M_PI) / M_PI;
        double dx = p->bbox[2] - p->bbox[0];
        double dy = p->bbox[3] - p->bbox[1];
        double ss = 2.0 * M_PI * p->radius;
        double dd = 2.0 * (dx + dy) + ss;
        if (a < 0.125)
        {
          p->param = 0.0;
        }
        else if (a < 0.375)
        {
          p->param = 2.0 * (0.5 * dy + 0.125 * ss) / dd;
        }
        else if (a < 0.625)
        {
          p->param = 0.5;
        }
        else if (a < 0.875)
        {
          p->param =  2.0 * (0.5 * dy + dx + 0.375 * ss) / dd;
        }
        else if (a < 1.125)
        {
          p->param = 1.0;
        }
        else if (a < 1.375)
        {
          p->param = 2.0 * (1.5 * dy + dx + 0.625 * ss) / dd;
        }
        else if (a < 1.625)
        {
          p->param = 1.5;
        }
        else if (a < 1.875)
        {
          p->param = 2.0 * (1.5 * dy + 2.0 * dx + 0.875 * ss) / dd;
        }
        else
        {
          p->param = 0.0;
        }
      }
      else
      {
        p->param = 0.0;
      }
    }
    update_state(p);
  }

  int total = 0;
  for (i = 0; i < n; ++i)
  {
    label_site* p = pflp[i];
    p->cost = compute_cost(p);
    total += p->cost;
  }
  return total;
}

int resolve_conflict(label_site* p)
{
  double dx = p->bbox[2] - p->bbox[0];
  double dy = p->bbox[3] - p->bbox[1];
  double ss = 2.0 * M_PI * p->radius;
  double dd = 2.0 * (dx + dy) + ss;

  double param[8];
  param[0] = 0.0;
  param[1] = 1.0;
  param[2] = 0.5;
  param[3] = 1.5;
  param[4] = 2.0 * (0.5 * dy + 0.125 * ss) / dd;
  param[5] = 2.0 * (0.5 * dy + dx + 0.375 * ss) / dd,
  param[6] = 2.0 * (1.5 * dy + 2.0 * dx + 0.875 * ss) / dd;
  param[7] = 2.0 * (1.5 * dy + dx + 0.625 * ss) / dd;

  double initial_param = p->param, best_param = initial_param;
  int initial_cost = p->cost, best_cost = initial_cost;
  for (int i = 0; i < 8; ++i)
  {
    p->param = param[i];
    update_state(p);
    int cost = compute_cost(p);
    if (cost < best_cost)
    {
      best_cost = cost;
      best_param = p->param;
    }    
  }

  p->cost = best_cost;
  p->param = best_param;
  if (best_param == initial_param)
    return 0;

  p->param = best_param;
  update_state(p);
  int cost_diff = p->cost - initial_cost;
  for (i = 0; i < p->degree; ++i)
  {
    label_site* q = p->cfl[i];
    initial_cost = q->cost;
    q->cost = compute_cost(q);
    cost_diff += initial_cost - q->cost;
  }
  return cost_diff;
}

bool ss_compare(const label_site* p, const label_site* q)
{
  return (p->cost > q->cost);
}

bool cs_compare(const label_site* p, int cost)
{
  return (p->cost > cost);
}

void schedule_annealing(int n, label_site** pflp)
{
  // double t = -1.0 / log(1.0 / 3.0);
  srand(time(0));
  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "setting up an initial state\n");
  int total = setup_initial_state(n, pflp); // n = pflp->numSites;
  if (total <= 0)
    return;

  for (int i = 0; i < 20*n; ++i)
  {
    std::sort(pflp, pflp+n, ss_compare);
    int nn = std::lower_bound(pflp, pflp+n, 0, cs_compare) - pflp;
		if (nn == 0)
			break;
    // std::cout << total << ' ' << nn << '\n';
    total += resolve_conflict(pflp[rand() % nn]);
    if (total <= 0)
      break;
  }
}

void graphics_output(psstream& out, int n, label_site** pflp)
{
  // int n = pflp->numSites;
  for (int i = 0; i < n; ++i)
  {
    label_site* p = pflp[i];
    double bb[4];
    bb[0] = p->bbox[0];
    bb[1] = p->bbox[1];
    bb[2] = p->bbox[2];
    bb[3] = p->bbox[3];
    textsite(p->xcoord, p->ycoord, p->radius, p->param, bb);
    out.moveto(bb[0] - p->bbox[0], bb[1] - p->bbox[1]);
    out.show(p->text);
    out.newpath();
    out.arc(p->xcoord, p->ycoord, p->radius, 0.0, 360.0);
    out.moveto(bb[0], bb[1]);
    out.lineto(bb[2], bb[1]);
    out.lineto(bb[2], bb[3]);
    out.lineto(bb[0], bb[3]);
    out.lineto(bb[0], bb[1]);
    out.stroke();
  }
}

#if 0

#include "db_data.h"

int main()
{
  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "loading head table\n");
  HeadTable tabl;
  if (!InputHeadTable("head.1.bin", &tabl))
    return 1;

  typedef std::list<HeadRecord*> head_list;
  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "computing plot size\n");
  bool coord_initialized = false;
  double x1, y1, x2, y2;
  head_list hh;
  for (int i = 0; i < tabl.numData; ++i)
  {
    HeadRecord* h = tabl.data[i];
    if (has_coord(h) && (h->data_flags & E_HEAD_COORD) == 0)
    {
      if (coord_initialized)
      {
        if (x1 > h->head_xcoord) x1 = h->head_xcoord;
        if (x2 < h->head_xcoord) x2 = h->head_xcoord;
        if (y1 > h->head_ycoord) y1 = h->head_ycoord;
        if (y2 < h->head_ycoord) y2 = h->head_ycoord;
      }
      else
      {
        x1 = x2 = h->head_xcoord;
        y1 = y2 = h->head_ycoord;
        coord_initialized = true;
      }
      hh.push_back(h);
    }
  }
  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A0, psstream::PORTRAIT);
  out.newpage();
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

  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "output of well clusters\n");
  out.setgray(0.5);
  out.setlinewidth(1.0);
  for (head_list::iterator it = hh.begin(); it != hh.end(); ++it)
  {
    HeadRecord* h = *it;
    double x = x0 + sx * (h->head_xcoord - x1) / dx;
    double y = y0 + sy * (h->head_ycoord - y1) / dy;
    if (has_offset(h) && !is_vertical(h))
    {
      double tx = sx * h->hole_xdistance / dx;
      double ty = sy * h->hole_ydistance / dy;
      out.moveto(x, y);
      out.lineto(x+tx, y+ty);
      out.stroke();
      out.arc(x+tx, y+ty, 1.0, 0.0, 360.0);
      out.fill();
    }
    out.arc(x, y, 1.0, 0.0, 360.0);
    out.fill();
  }
  
  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "initializing PFLP\n");
  LabelProblem data;
  int numSites = hh.size();
  data.numSites = numSites;
  data.site = new label_site* [numSites];
  out.setgray(1.0);
  numSites = 0;
  out.setfont("Helvetica", 4.0);
  for (it = hh.begin(); it != hh.end(); ++it)
  {
    HeadRecord* h = *it;
    double x = x0 + sx * (h->head_xcoord - x1) / dx;
    double y = y0 + sy * (h->head_ycoord - y1) / dy;
    if (has_offset(h))
    {
      x += sx * h->hole_xdistance / dx;
      y += sy * h->hole_ydistance / dy;
      out.arc(x, y, 0.5, 0.0, 360.0);
      out.fill();
    }
    label_site* q = new label_site;
    data.site[numSites++] = q;
    strcpy(q->text, h->well_name);
    q->xcoord = x;
    q->ycoord = y;
    out.textbox(q->text, q->bbox);
    q->param = 0.0;
    q->radius = 2.0;
    q->degree = 0;
    q->cost = 0;
    q->cfl = 0;
  }
  FreeHeadTable(&tabl);

  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "building conflict graph\n");
  compute_conflict_graph(&data);
  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "solving labeling problem\n");
  schedule_annealing(&data);
  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "graphics output\n");
  out.setgray(0.0);
  out.setlinewidth(0.0);
  graphics_output(out, &data);

  // PrintMessage(LEVEL_VERBOSE, __FILE__, __LINE__, "cleanup\n");
  for (i = 0; i < data.numSites; ++i)
  {
    label_site* p = data.site[i];
    delete[] p->cfl;
    delete p;
  }
  delete[] data.site;
  
  return 0;
}

void driver()
{
  psstream out("zz.ps");
  out.selectmedia(psstream::FORMAT_A4, psstream::PORTRAIT);
  out.newpage();
  out.setfont("Helvetica", 10.0);
  out.setlinewidth(0.0);
  
  LabelProblem data;
  data.numSites = 3;
  data.site = new label_site* [3];

  label_site* s = new label_site;
  s->xcoord = 100;
  s->ycoord = 100;
  strcpy(s->text, "first");
  out.textbox(s->text, s->bbox);
  s->radius = 5;
  s->param = 0.0;
  s->degree = 0;
  s->cost = 0;
  data.site[0] = s;

  s = new label_site;
  s->xcoord = 120;
  s->ycoord = 100;
  strcpy(s->text, "second");
  out.textbox(s->text, s->bbox);
  s->radius = 5;
  s->param = 0.0;
  s->degree = 0;
  s->cost = 0;
  data.site[1] = s;

  s = new label_site;
  s->xcoord = 110;
  s->ycoord = 110;
  strcpy(s->text, "third");
  out.textbox(s->text, s->bbox);
  s->radius = 5;
  s->param = 0.0;
  s->degree = 0;
  s->cost = 0;
  data.site[2] = s;

  compute_conflict_graph(&data);
  setup_initial_state(&data);
  graphics_output(out, &data);
}

#endif
