#include "stdafx.h"
#include <list>
#include <valarray>
#include <map>
#include <oledb.h>
#include <assert.h>

using std::min;
using std::max;

#include "Field.h"
#include "FieldDoc.h"
#include "CrossSection.h"
#include "stuff\ps_data.h"
#include "FindWell.h"
#include "LogtrackType.h"
#include "WellMatch.h"

WellMatch::WellMatch(): p_list(), p_iter(p_list.end())
{
}

void WellMatch::first_page()
{
  p_iter = p_list.begin();
}

void WellMatch::next_page()
{
  ++p_iter;
}

void WellMatch::plot_page(psstream& out)
{
  if (p_iter->w_list.empty())
    return;

  for (well_list::iterator w_iter = p_iter->w_list.begin(); \
		w_iter != p_iter->w_list.end(); ++w_iter)
  {
    Well& well = *w_iter;
    if (well.t_edge.empty())
      continue;

    if (well.name[0] != '\0')
    {
			out.setfont("Helvetica", 10.0);
			double bbox[8];
      coord_type& p = well.ver[well.t_edge.front().front()];
			out.setgray(1);
			out.rectfill(p.xcoord-6, p.ycoord, 12, 30);
			out.setgray(0);
			out.setlinewidth(1);
			out.rectstroke(p.xcoord-6, p.ycoord, 12, 30);
			out.annot(p.xcoord, p.ycoord+15, 1.0, 90.0, psstream::ANNOT_CENTER, true, well.name, bbox);
    }

    out.setgray(1);
    out.setlinewidth(2);
    plotpath(out, well.ver, well.t_edge);
    out.stroke();

    if (!well.p_closed.empty())
    {
      out.setgray(0.6);
      plotpath(out, well.ver, well.p_closed);
      out.stroke();
    }

    if (!well.p_opened.empty())
    {
      out.setgray(0);
      plotpath(out, well.ver, well.p_opened);
      out.stroke();
    }

		if (well.has_logtrack)
		{
			out.setgray(1);
			out.rectfill(well.x0, well.y0, well.dx, well.dy);
			out.setlinewidth(1);
			out.rectstroke(well.x0, well.y1, well.dx, well.y2 - well.y1);
			out.setgray(0);
			out.rectstroke(well.x0, well.y0, well.dx, well.dy);
			out.setlinewidth(0.5);
			out.rectstroke(well.x0, well.y1, well.dx, well.y2 - well.y1);
			out.setfont("Helvetica", 4.0);
			
			double bb[8];
			if (!well.sp_edge.empty())
			{
				plotpath(out, well.ver, well.sp_edge);
				out.gsave();
				out.setlinewidth(1);
				out.setgray(1);
				out.stroke();
				out.grestore();
				out.setlinewidth(0.5);
				out.setrgbcolor(1.0, 0.5, 0.5);
				double x1 = well.x1;
				double dx = well.x2 - well.x1;
				double x2 = well.x1 + 0.25 * dx;
				double x3 = well.x1 + 0.5 * dx;
				double x4 = well.x1 + 0.75 * dx;
				double x5 = well.x2;
				double y0 = well.y0 + 25;
				out.moveto(x1, y0);
				out.lineto(x5, y0);
				out.moveto(x1, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x2, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x3, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x4, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x5, y0-0.5);
				out.rlineto(0, 1);
				out.stroke();
				out.setgray(0);
				label(out, x3, y0, 0.5, psstream::LABEL_N, true, "SP", bb);

				char tick[16];
				double z0 = well.sp_range[0];
				double dz = well.sp_range[1] - z0;
				sprintf(tick, "%d", int(z0 + 0.5));
				label(out, x1, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.25 * dz + 0.5));
				label(out, x2, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.5 * dz + 0.5));
				label(out, x3, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.75 * dz + 0.5));
				label(out, x4, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + dz + 0.5));
				label(out, x5, y0, 0.5, psstream::LABEL_S, true, tick, bb);
			}
			if (!well.il_edge.empty())
			{
				plotpath(out, well.ver, well.il_edge);
				out.gsave();
				out.setlinewidth(1);
				out.setgray(1);
				out.stroke();
				out.grestore();
				out.setlinewidth(0.5);
				out.setrgbcolor(0.5, 1.0, 1.0);
				double x1 = well.x1;
				double dx = well.x2 - well.x1;
				double x2 = well.x1 + 0.25 * dx;
				double x3 = well.x1 + 0.5 * dx;
				double x4 = well.x1 + 0.75 * dx;
				double x5 = well.x2;
				double y0 = well.y0 + 15;
				out.moveto(x1, y0);
				out.lineto(x5, y0);
				out.moveto(x1, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x2, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x3, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x4, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x5, y0-0.5);
				out.rlineto(0, 1);
				out.stroke();
				out.setgray(0);
				label(out, x3, y0, 0.5, psstream::LABEL_N, true, "IL", bb);

				char tick[16];
				double z0 = well.il_range[0];
				double dz = well.il_range[1] - z0;
				sprintf(tick, "%d", int(z0 + 0.5));
				label(out, x1, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.25 * dz + 0.5));
				label(out, x2, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.5 * dz + 0.5));
				label(out, x3, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.75 * dz + 0.5));
				label(out, x4, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + dz + 0.5));
				label(out, x5, y0, 0.5, psstream::LABEL_S, true, tick, bb);
			}
			if (!well.pz_edge.empty())
			{
				plotpath(out, well.ver, well.pz_edge);
				out.gsave();
				out.setlinewidth(1);
				out.setgray(1);
				out.stroke();
				out.grestore();
				out.setlinewidth(0.5);
				out.setrgbcolor(1.0, 0.5, 1.0);
				double x1 = well.x1;
				double dx = well.x2 - well.x1;
				double x2 = well.x1 + 0.25 * dx;
				double x3 = well.x1 + 0.5 * dx;
				double x4 = well.x1 + 0.75 * dx;
				double x5 = well.x2;
				double y0 = well.y0 + 5;
				out.moveto(x1, y0);
				out.lineto(x5, y0);
				out.moveto(x1, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x2, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x3, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x4, y0-0.5);
				out.rlineto(0, 1);
				out.moveto(x5, y0-0.5);
				out.rlineto(0, 1);
				out.stroke();
				out.setgray(0);
				label(out, x3, y0, 0.5, psstream::LABEL_N, true, "PZ", bb);

				char tick[16];
				double z0 = well.pz_range[0];
				double dz = well.pz_range[1] - z0;
				sprintf(tick, "%d", int(z0 + 0.5));
				label(out, x1, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.25 * dz + 0.5));
				label(out, x2, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.5 * dz + 0.5));
				label(out, x3, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + 0.75 * dz + 0.5));
				label(out, x4, y0, 0.5, psstream::LABEL_S, true, tick, bb);
				sprintf(tick, "%d", int(z0 + dz + 0.5));
				label(out, x5, y0, 0.5, psstream::LABEL_S, true, tick, bb);
			}
		}
  }
}

void WellMatch::new_page(double x_range[2], double y_range[2], int first, int last, \
  const float *dist, const std::map<int, long>& bore, CActField* pDoc, CCrossSection* pSec)
{
  p_list.push_back(Page());
  Page* page = &p_list.back();

  DBPROP dbProp[2];
  dbProp[0].dwPropertyID = DBPROP_IRowsetIndex;
  dbProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
  dbProp[0].dwStatus = DBPROPSTATUS_OK;
  dbProp[0].colid = DB_NULLID;
  dbProp[0].vValue.vt = VT_BOOL;
  dbProp[0].vValue.boolVal = VARIANT_TRUE;

  dbProp[1].dwPropertyID = DBPROP_IRowsetCurrentIndex;
  dbProp[1].dwOptions = DBPROPOPTIONS_REQUIRED;
  dbProp[1].dwStatus = DBPROPSTATUS_OK;
  dbProp[1].colid = DB_NULLID;
  dbProp[1].vValue.vt = VT_BOOL;
  dbProp[1].vValue.boolVal = VARIANT_TRUE;

  DBPROPSET dbPropSet;
  dbPropSet.guidPropertySet = DBPROPSET_ROWSET;
  dbPropSet.cProperties = 2;
  dbPropSet.rgProperties = dbProp;

	CComQIPtr < IRowsetIndex > spRowsetIndex;
	CTable < CAccessor < CFindWell::Well > > tabl;
	HRESULT hr = tabl.Open(pDoc->session, "well", &dbPropSet);
	if (SUCCEEDED(hr))
	{
		CComQIPtr < IRowsetCurrentIndex > spRowsetCurrentIndex = tabl.m_spRowset;
		if (spRowsetCurrentIndex != NULL)
		{
			DBID IndexID;
			IndexID.eKind = DBKIND_NAME;
			IndexID.uName.pwszName = L"well_bore";
			hr = spRowsetCurrentIndex->SetIndex(&IndexID);
			if (SUCCEEDED(hr))
			{
				spRowsetIndex = tabl.m_spRowset;
			}
		}
	}

  for (int key = first; key <= last; ++key)
  {
    std::map<int, long>::const_iterator it = bore.find(key);
    if (it == bore.end())
      continue;

    long val = it->second;
    ASSERT(first <= key && key <= last);
    int n = pDoc->Traj(val, 0, 0);
    std::valarray<double> pp(3*n);
    double *q = &pp[0];
    pDoc->Traj(val, n, q);
    page->w_list.push_back(Well());
    Well* well = &page->w_list.back();
		well->bore = val;
		well->has_logtrack = false;
    std::valarray < double > xx(n);
    std::valarray < double > yy(n);
    std::valarray < double > md(n);
    double x = dist[key];
    double y = q[2];
    
    xx[0] = pSec->left_margin + (pSec->page_width - pSec->left_margin - pSec->right_margin)
      * (x - x_range[0]) / (x_range[1] - x_range[0]);
    yy[0] = pSec->bottom_margin + (pSec->page_height - pSec->top_margin - pSec->bottom_margin)
      * (y_range[1] - y) / (y_range[1] - y_range[0]);
    md[0] = 0.0;

    int np = 0;
    well->ver.push_back(coord_type());
    coord_type* p = &well->ver.back();
    well->t_edge.push_back(curve_type());
    curve_type* g = &well->t_edge.back();
    p->xcoord = xx[0];
    p->ycoord = yy[0];
    g->push_back(np++);

    int i;
    for (i = 1; i < n; ++i)
    {
      double dx = q[3] - q[0];
      double dy = q[4] - q[1];
      double dz = q[5] - q[2];

      x += hypot(dx, dy);
      y = q[5];
      q += 3;

      xx[i] = pSec->left_margin + (pSec->page_width - pSec->left_margin - pSec->right_margin)
        * (x - x_range[0]) / (x_range[1] - x_range[0]);
      yy[i] = pSec->bottom_margin + (pSec->page_height - pSec->top_margin - pSec->bottom_margin)
        * (y_range[1] - y) / (y_range[1] - y_range[0]);
      md[i] = md[i-1] + sqrt(dx*dx + dy*dy + dz*dz);

      well->ver.push_back(coord_type());
      p = &well->ver.back();
      p->xcoord = xx[i];
      p->ycoord = yy[i];
      g->push_back(np++);
    }

    // lookup the well name
		if (spRowsetIndex != NULL)
		{
			CFindWell::Well data;
			data.bore = val;
			if (S_OK == spRowsetIndex->Seek(tabl.GetHAccessor(0), 1, &data, DBSEEK_FIRSTEQ)
				&& S_OK == tabl.MoveNext())
			{
			  strcpy(well->name, (const char*) tabl.name);
			}
		}

    // P E R F O R A T I O N
    int n_per = pDoc->Perf(val, 0, 0);
    if (n_per > 0)
    {
      well->perf.resize(n_per);
      pDoc->Perf(val, n_per, &well->perf[0]);
      for (i = 0; i < n_per; ++i)
      {
        double *dd = &md[0];
        double d1 = well->perf[i]->perf_upper;
        double d2 = well->perf[i]->perf_lower;
				bool perf_done = well->perf[i]->perf_done;

        if (d1 < 0.0)
          d1 = 0.0;
        if (d1 > dd[n-1])
          d1 = dd[n-1];
        if (d2 < 0.0)
          d2 = 0.0;
        if (d2 > dd[n-1])
          d2 = dd[n-1];

        int n1 = std::lower_bound(dd, dd+n, d1) - dd;
        double x1 = xx[n1], y1 = yy[n1];
        if (d1 < dd[n1])
        {
          double t = (d1 - dd[n1-1]) / (dd[n1] - dd[n1-1]);
          ASSERT(0 <= t && t <= 1.0);
          x1 = xx[n1-1] + (xx[n1] - xx[n1-1]) * t;
          y1 = yy[n1-1] + (yy[n1] - yy[n1-1]) * t;
        }
        
        well->ver.push_back(coord_type());
        p = &well->ver.back();
				if (perf_done)
				{
					well->p_closed.push_back(curve_type());
					g = &well->p_closed.back();
				}
				else
				{
					well->p_opened.push_back(curve_type());
					g = &well->p_opened.back();
				}
        p->xcoord = x1;
        p->ycoord = y1;
        g->push_back(np++);

        int n2 = std::lower_bound(dd, dd+n, d2) - dd;
        double x2 = xx[n2], y2 = yy[n2];
        if (d2 < dd[n2])
        {
          double t = (d2 - dd[n2-1]) / (dd[n2] - dd[n2-1]);
          ASSERT(0 <= t && t <= 1.0);
          x2 = xx[n2-1] + (xx[n2] - xx[n2-1]) * t;
          y2 = yy[n2-1] + (yy[n2] - yy[n2-1]) * t;
        }

        for (int j = n1; j < n2; ++j)
        {
          well->ver.push_back(coord_type());
          p = &well->ver.back();
          p->xcoord = xx[j];
          p->ycoord = yy[j];
          g->push_back(np++);
        }
        
        well->ver.push_back(coord_type());
        p = &well->ver.back();
        p->xcoord = x2;
        p->ycoord = y2;
        g->push_back(np++);

        // F L O W  I N F O R M A T I O N
        int n_prod = pDoc->Prod(val, 0, 0);
        if (n_prod > 0)
        {
          well->prod.resize(n_prod);
          pDoc->Prod(val, n_prod, &well->prod[0]);
          #if 0
          // if we need flow information, uncomment these lines
          for (int j = 0; j < n_prod; ++j)
          {
            // look for flow record suitable to current perforation interval
            if (well->prod[j]->bed == well->perf[i]->perf_form)
            {
              // graphics output at point (x1, y1)
            }
          }
          #endif
        }
        int n_pump = pDoc->Pump(val, 0, 0);
        if (n_pump > 0)
        {
          well->pump.resize(n_pump);
          pDoc->Pump(val, n_pump, &well->pump[0]);
          #if 0
          // the same as for production records
          for (int j = 0; j < n_pump; ++j)
          {
            // look for flow record suitable to current perforation interval
            if (well->pump[j]->bed == well->perf[i]->perf_form)
            {
              // graphics output at point (x1, y1)
            }
          }
          #endif
        }
      }
    }

    CLogtrackType* pLogtrackType = GetLogtrackType();
    bool show_logtrack = (pLogtrackType->IsWindowVisible() == TRUE);
		if (show_logtrack)
		{
			CFieldDoc::LogtrackItem sp, il, pz;
			bool has_sp = (pDoc->Logtrack(val, "SP", 1, &sp) == 1);
			bool has_il = (pDoc->Logtrack(val, "IL", 1, &il) == 1);
			bool has_pz = (pDoc->Logtrack(val, "PZ", 1, &pz) == 1);
			if (has_sp || has_il || has_pz)
			{
				well->has_logtrack = true;
				well->dy = 30.0;
				well->dx = well->dy * GOLDEN_RATIO;
				well->x0 = xx[0]-6-30*GOLDEN_RATIO;
				well->y0 = yy[0];
				well->x1 = well->x0 + 6;
				well->x2 = well->x0 + well->dx - 6;
				well->y1 = yy.min();
				well->y2 = yy.max();

				int n_dist = md.size();
				double *dist = &md[0];
				double *offs = &yy[0];

				if (has_sp)
				{
					well->sp_range[0] = sp.minval;
					well->sp_range[1] = sp.maxval;
					curve_type *cur = 0;
					for (int n_point = 0; n_point < sp.num_data; ++n_point)
					{
						if (sp.value[n_point] != sp.null && sp.minval <= sp.value[n_point] \
							&& sp.value[n_point] <= sp.maxval)
						{
							double x = well->x1 + (well->x2 - well->x1) \
								* (sp.value[n_point] - well->sp_range[0]) \
								/ (well->sp_range[1] - well->sp_range[0]);
							double d = sp.depth_step * n_point;
							int s = std::lower_bound(dist, dist+n_dist, d) - dist;
							if (s != n_dist)
							{
								double y = yy[s];
								if (d < md[s])
								{
									double t = (d - dist[s-1]) / (dist[s] - dist[s-1]);
									assert(0.0 < t && t < 1.0);
									y = offs[s-1] + t * (offs[s] - offs[s-1]);
								}
								if (!cur)
								{
									well->sp_edge.push_back(curve_type());
									cur = &well->sp_edge.back();
								}
								cur->push_back(well->ver.size());
								well->ver.push_back(coord_type());
								coord_type* p = &well->ver.back();
								p->xcoord = x;
								p->ycoord = y;
							}
							else
							{
								cur = 0;
							}
						}
						else
						{
							cur = 0;
						}
					}
				}
				if (has_il)
				{
					well->il_range[0] = il.minval;
					well->il_range[1] = il.maxval;
					curve_type *cur = 0;
					for (int n_point = 0; n_point < il.num_data; ++n_point)
					{
						if (il.value[n_point] != il.null && il.minval <= il.value[n_point] \
							&& il.value[n_point] <= il.maxval)
						{
							double x = well->x1 + (well->x2 - well->x1) \
								* (il.value[n_point] - well->il_range[0]) \
								/ (well->il_range[1] - well->il_range[0]);
							double d = il.depth_step * n_point;
							int s = std::lower_bound(dist, dist+n_dist, d) - dist;
							if (s != n_dist)
							{
								double y = yy[s];
								if (d < md[s])
								{
									double t = (d - dist[s-1]) / (dist[s] - dist[s-1]);
									assert(0.0 < t && t < 1.0);
									y = offs[s-1] + t * (offs[s] - offs[s-1]);
								}
								if (!cur)
								{
									well->il_edge.push_back(curve_type());
									cur = &well->il_edge.back();
								}
								cur->push_back(well->ver.size());
								well->ver.push_back(coord_type());
								coord_type* p = &well->ver.back();
								p->xcoord = x;
								p->ycoord = y;
							}
							else
							{
								cur = 0;
							}
						}
						else
						{
							cur = 0;
						}
					}
				}
				if (has_pz)
				{
					well->pz_range[0] = pz.minval;
					well->pz_range[1] = pz.maxval;
					curve_type *cur = 0;
					for (int n_point = 0; n_point < il.num_data; ++n_point)
					{
						if (pz.value[n_point] != pz.null && pz.minval <= pz.value[n_point] \
							&& pz.value[n_point] <= pz.maxval)
						{
							double x = well->x1 + (well->x2 - well->x1) \
								* (pz.value[n_point] - well->pz_range[0]) \
								/ (well->pz_range[1] - well->pz_range[0]);
							double d = pz.depth_step * n_point;
							int s = std::lower_bound(dist, dist+n_dist, d) - dist;
							if (s != n_dist)
							{
								double y = yy[s];
								if (d < md[s])
								{
									double t = (d - dist[s-1]) / (dist[s] - dist[s-1]);
									assert(0.0 < t && t < 1.0);
									y = offs[s-1] + t * (offs[s] - offs[s-1]);
								}
								if (!cur)
								{
									well->pz_edge.push_back(curve_type());
									cur = &well->pz_edge.back();
								}
								cur->push_back(well->ver.size());
								well->ver.push_back(coord_type());
								coord_type* p = &well->ver.back();
								p->xcoord = x;
								p->ycoord = y;
							}
							else
							{
								cur = 0;
							}
						}
						else
						{
							cur = 0;
						}
					}
				}
			}
			if (has_sp)
			{
				FreeLogtrack(1, &sp);
			}
			if (has_il)
			{
				FreeLogtrack(1, &il);
			}
			if (has_pz)
			{
				FreeLogtrack(1, &pz);
			}
		}

		/*
		int n_inf = pDoc->Info(val, 0, 0);
		if (show_logtrack && n_inf > 0)
		{
			double dy = 30.0;
			double dx = dy * GOLDEN_RATIO;
			double x0 = xx[0]-6-30*GOLDEN_RATIO;
			double y0 = yy[0];

			// out.setgray(1);
			// out.rectfill(x0, y0, dx, dy);
			// out.setgray(0);
			// out.setlinewidth(1);
			// out.rectstroke(x0, y0, dx, dy);

			well->info.resize(n_inf);
			pDoc->Info(val, n_inf, &well->info[0]);
			bool has_currentpoint = false;
			// out.setlinewidth(0.5);
			// out.setgray(0);
			int n_dist = md.size();
			double *dist = &md[0];
			double *offs = &yy[0];
			double x, y;
			for (int i = 0; i < n_inf; ++i)
			{
				CFieldDoc::InfoItem* item = well->info[i];
				if (item->a_sp == -9999.0f)
				{
					if (has_currentpoint)
					{
						// out.stroke();
						has_currentpoint = false;
					}
					continue;
				}
				x = x0 + item->a_sp * dx;
				int s = std::lower_bound(dist, dist+n_dist, (double) item->top) - dist;
				if (s != n_dist)
				{
					y = yy[s];
					if (well->info[i]->top < md[s])
					{
						double t = (item->top - dist[s-1]) / (dist[s] - dist[s-1]);
						assert(0.0 < t && t < 1.0);
						y = offs[s-1] + t * (offs[s] - offs[s-1]);
					}
					if (!has_currentpoint)
					{
						out.moveto(x, y);
						has_currentpoint = true;
					}
					else
					{
						out.lineto(x, y);
					}
				}
				else
				{
					if (has_currentpoint)
					{
						out.stroke();
						has_currentpoint = false;
					}
				}
				s = std::lower_bound(dist, dist+n_dist, (double) item->bot) - dist;
				if (s != n_dist)
				{
					y = yy[s];
					if (well->info[i]->bot < md[s])
					{
						double t = (item->bot - dist[s-1]) / (dist[s] - dist[s-1]);
						assert(0.0 < t && t < 1.0);
						y = offs[s-1] + t * (offs[s] - offs[s-1]);
					}
					if (!has_currentpoint)
					{
						out.moveto(x, y);
						has_currentpoint = true;
					}
					else
					{
						out.lineto(x, y);
					}
				}
				else
				{
					if (has_currentpoint)
					{
						out.stroke();
						has_currentpoint = false;
					}
				}
			}
			if (has_currentpoint)
			{
				out.stroke();
			}
			out.setlinewidth(1);
			out.rectstroke(x0, y-1, dx, y0-y+1);
		}
		*/

		/*
    // L O G T R A C K   I N F O R M A T I O N
    CCumulativeS* pSat = GetCumulativeS();
    if (n_inf > 0 && !pSat->IsWindowVisible())
    {
      std::valarray<double> inf(3*n_inf);
      pDoc->FieldInfo(val, n_inf, &inf[0]);
      for (i = 0; i < n_inf; ++i)
      {
        double *dd = &md[0];
        double d1 = inf[3*i];
        double d2 = inf[3*i+1];
        if (d1 > dd[n-1] || d2 > dd[n-1])
          continue;

        int n1 = std::lower_bound(dd, dd+n, d1) - dd;
        double x1 = xx[n1], y1 = yy[n1];
        if (d1 < dd[n1])
        {
          double t = (d1 - dd[n1-1]) / (dd[n1] - dd[n1-1]);
          x1 = xx[n1-1] + (xx[n1] - xx[n1-1]) * t;
          y1 = yy[n1-1] + (yy[n1] - yy[n1-1]) * t;
        }
        out << "newpath\n" << x1 << ' ' << y1 << " moveto\n";

        int n2 = std::lower_bound(dd, dd+n, d2) - dd;
        double x2 = xx[n2], y2 = yy[n2];
        if (d2 < dd[n2])
        {
          double t = (d2 - dd[n2-1]) / (dd[n2] - dd[n2-1]);
          x2 = xx[n2-1] + (xx[n2] - xx[n2-1]) * t;
          y2 = yy[n2-1] + (yy[n2] - yy[n2-1]) * t;
        }

        for (int j = n1; j < n2; ++j)
        {
          out << xx[j] << ' ' << yy[j] << " lineto\n";
        }
        out << x2 << ' ' << y2 << " lineto\n";
        
        float z = pDoc->clut.fwd((float) inf[3*i+2]);
        CActField::Color* sh = pDoc->clut.sh;
        float* zz = pDoc->clut.zz;
        int nz = pDoc->clut.nz;
        int k = std::lower_bound(zz, zz + nz, z) - zz;
        if (z < zz[k])
        {
          ASSERT(zz[k-1] < z && z < zz[k]);
          double t = (z - zz[k-1]) / (zz[k] - zz[k-1]);
          double r = sh[k-1].r + (sh[k].r - sh[k-1].r) * t;
          double g = sh[k-1].g + (sh[k].g - sh[k-1].g) * t;
          double b = sh[k-1].b + (sh[k].b - sh[k-1].b) * t;
          out << r << ' ' << g << ' ' << b << " setrgbcolor\n";
        }
        else
        {
          double r = sh[k].r;
          double g = sh[k].g;
          double b = sh[k].b;
          out << r << ' ' << g << ' ' << b << " setrgbcolor\n";
        }
        out << "1 setlinewidth\nstroke\n";
      }
    }
		*/
  }
}

