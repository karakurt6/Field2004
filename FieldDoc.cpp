#include <stdafx.h>
#include <libh5/hdf5.h>

#include <string>
#include <valarray>
#include <map>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <limits>

#define LEDA_PREFIX
#include <LEDA/point_set.h>
#include <LEDA/node_map.h>
#include <LEDA/list.h>
#include <LEDA/dictionary.h>

#include "Field.h"
#include "FieldDoc.h"
#include "CumulativeS.h"
#include "FindWell.h"
#include "Import\ImportWizard.h"

/*************** H I S T O R Y **************

2002.11.19 08:22 -- changed FieldRef type from leda_list<DataField*> to 
  std::map<std::string, DataField*> to support name lookup in Field() method.

2002.11.19 15:49 -- changed return type of Cell() and Coord() type from BOOL
  to the OutsideFlag bit combination

2002.11.19 22:36 -- parameter referenced to dataset in method Value() changed
  back to hid_t (from const DataField*) to support non-public datasets that
  doesn't resides in root group

2002.11.20 23:14 -- added CActField document for compatebility with old GUI stuff

*********************************************/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// #define TRAJ_CHECK

#ifndef countof
#define countof(ar) (sizeof(ar) / sizeof(*ar))
#endif

/*
#pragma comment(lib, "libG_md.lib")
#pragma comment(lib, "libL_md.lib")
#pragma comment(lib, "libP_md.lib")
*/

/*
#pragma comment(lib, "leda_md.lib")
*/

class FieldRef
{
public:
  FieldRef();
  ~FieldRef();

  typedef CActField::DataField DataField;
  typedef std::map<std::string, DataField*> FieldList;

  const DataField* Field(std::string s) const;
  int Enum(int nd, const DataField* dd[]) const;
  int Init(CFieldDoc* pDoc);
  void Done();

private:
  static herr_t Push(hid_t gr, const char* name, void* data);

  FieldList data;
};

class WellSite
{
	struct TrajRecord
	{
		long wb;
		unsigned char id;
		float dx;
		float dy;
		float dz;
		float md;
		short ns;
		float hx;
		float hy;
		float hz;
		float md_well;

		DBSTATUS md_status;

		BEGIN_COLUMN_MAP(TrajRecord)
			COLUMN_ENTRY(1, wb)
			COLUMN_ENTRY(2, id)
			COLUMN_ENTRY(3, dx)
			COLUMN_ENTRY(4, dy)
			COLUMN_ENTRY(5, dz)
			COLUMN_ENTRY(6, md)
			COLUMN_ENTRY(7, ns)
			COLUMN_ENTRY(8, hx)
			COLUMN_ENTRY(9, hy)
			COLUMN_ENTRY(10, hz)
			COLUMN_ENTRY_STATUS(11, md_well, md_status)
		END_COLUMN_MAP()
	};

	struct HistRecord
	{
		long wellbore;
		long formation;
		DATE startdate;
		DATE finaldate;
		float oil;
		float wat;
		float gas;
		float fluid;
		struct Status
		{
			DBSTATUS oil;
			DBSTATUS wat;
			DBSTATUS gas;
			DBSTATUS fluid;
		}
		status;
		BEGIN_COLUMN_MAP(HistRecord)
			COLUMN_ENTRY(1, formation)
			COLUMN_ENTRY(2, startdate)
			COLUMN_ENTRY(3, finaldate)
			COLUMN_ENTRY_STATUS(4, oil, status.oil)
			COLUMN_ENTRY_STATUS(5, wat, status.wat)
			COLUMN_ENTRY_STATUS(6, gas, status.gas)
			COLUMN_ENTRY_STATUS(7, fluid, status.fluid)
		END_COLUMN_MAP()
	};

	class GeophysDataset
	{
		hid_t file;
		hid_t data;
		hid_t space;
		int elem;
		char name[64];
	public:
		enum HasLogging
		{
			STARTDEPTH = 1,
			FINALDEPTH = 2,
			PERMEABILITY = 4,
			POROSITY = 8,
			DIFGLOGGING = 0x10,
			DIFNLOGGING = 0x20,
			RELGLOGGING = 0x40,
			RELNLOGGING = 0x80,
			SPOTENTIAL = 0x100,
			RESISTIVITY = 0x200,
			BACKGROUNDGAS = 0x400,
			HYDROCARBON = 0x800,
			CLAYINESS = 0x1000
		};

	GeophysDataset(hid_t ff, long b, const char* nn);
		~GeophysDataset();
		herr_t read(float* x);
	};

	struct GeophysRecord
	{
		long wellbore;
		long formation;
		float startdist;
		float finaldist;
		BEGIN_COLUMN_MAP(GeophysRecord)
			COLUMN_ENTRY(1, wellbore)
			COLUMN_ENTRY(2, formation)
			COLUMN_ENTRY(3, startdist)
			COLUMN_ENTRY(4, finaldist)
		END_COLUMN_MAP()
	};

	struct PerfRecord
	{
		long perf_bore;
		long perf_form;
		DATE perf_comp;
		DATE perf_close;
		float perf_upper;
		float perf_lower;
		struct Status
		{
			DBSTATUS perf_close;
		}
		status;
		BEGIN_COLUMN_MAP(PerfRecord)
			COLUMN_ENTRY(1, perf_bore)
			COLUMN_ENTRY(2, perf_form)
			COLUMN_ENTRY(3, perf_comp)
			COLUMN_ENTRY_STATUS(4, perf_close, status.perf_close)
			COLUMN_ENTRY(5, perf_upper)
			COLUMN_ENTRY(6, perf_lower)
		END_COLUMN_MAP()
	};

public:
  typedef CFieldDoc::TrajItem TrajItem;
  typedef CFieldDoc::PerfItem PerfItem;
  typedef CFieldDoc::InfoItem InfoItem;
  typedef CFieldDoc::ProdItem ProdItem;
  typedef CFieldDoc::PumpItem PumpItem;

  typedef leda_list<TrajItem*> TrajBlock;
  typedef leda_list<PerfItem*> WellPerf;
  typedef leda_list<InfoItem*> WellInfo;
  typedef leda_list<ProdItem*> WellProd;
  typedef leda_list<PumpItem*> WellPump;

  struct TrajEntry
  {
    TrajItem *item;
    double dist; 
  };

  struct WellTraj
  {
    TrajEntry start;
    TrajBlock block;
    TrajEntry final;
  };

  WellSite();
  ~WellSite();

  int Init(CFieldDoc* pDoc);
	void ResetPoints(CFieldDoc* pDoc);
  void Done();

  int WindowQuery(leda_point a, leda_point b, int nb, long* bb);
  int RangeQuery(leda_circle c, int nb, long* bb);
  int ProdQuery(const COleDateTime& dt, int nb, long* bb);
  int PumpQuery(const COleDateTime& dt, int nb, long* bb);

  WellProd Prod(long b);
  WellPump Pump(long b);
  WellInfo Info(long b);
  WellInfo Info2(long b, long s);
  WellPerf Perf(long b);
  WellPerf Perf2(long b, long s);
  WellTraj* Traj(long b);
  double Roof(long b);

#ifdef _DEBUG
  void Dump(CDumpContext &dc) const;
#endif

private:
  leda_dictionary<long, WellTraj*> traj;
  leda_dictionary<long, WellPerf> perf;
  leda_dictionary<long, WellInfo> info;
  leda_dictionary<long, WellProd> prod;
  leda_dictionary<long, WellPump> pump;
  leda_dictionary<long, double> roof;
  leda_point_set site;
  leda_node_map<long> node;
  CFieldDoc* data;

  void ClearProd();
  void ClearPump();
};

WellSite::GeophysDataset::GeophysDataset(hid_t ff, long b, const char* nn): 
  file(ff), data(-1), space(-1), elem(0)
{
	sprintf(name, "%s/%d", nn, b);
}

WellSite::GeophysDataset::~GeophysDataset()
{
	if (0 <= space)
	{
		H5Sclose(space);
	}
	if (0 <= data)
	{
		H5Dclose(data);
	}
}

herr_t WellSite::GeophysDataset::read(float* x)
{
	if (0 > data)
	{
		data = H5Dopen(file, name);
		if (0 > data)
		{
			return -1;
		}
		space = H5Dget_space(data);
		if (0 > space)
		{
			return -1;
		}
	}
	hsize_t count = 1;
	hid_t m_space = H5Screate_simple(1, &count, NULL);
	if (0 > m_space)
	{
		return -1;
	}
	hsize_t coord[1][1];
	coord[0][0] = elem++;
	herr_t err = H5Sselect_elements(space, H5S_SELECT_SET, 1, (const hsize_t**) coord);
	if (0 > err)
	{
		H5Sclose(m_space);
		return err;
	}
	err = H5Dread(data, H5T_NATIVE_FLOAT, m_space, space, H5P_DEFAULT, x);
	H5Sclose(m_space);
	return err;
}

//////////////// F I E L D   R E F /////////////////////

FieldRef::FieldRef()
{
}

FieldRef::~FieldRef()
{
  Done();
}

herr_t FieldRef::Push(hid_t gr, const char* name, void* data)
{
  FieldRef* p = (FieldRef*) data;

  H5E_auto_t e_func;
  void* e_data;
  herr_t er = H5Eget_auto(&e_func, &e_data);
  er = H5Eset_auto(0, 0);
  hid_t fd = H5Dopen(gr, name);
  er = H5Eset_auto(e_func, e_data);
  if (fd < 0) return 0;

  hid_t fs = H5Dget_space(fd);
  int nd = H5Sget_simple_extent_ndims(fs);
  // if (nd != 3 || strnicmp(name, "geom", 4) == 0)
	if ((nd != 3 && nd != 4) || strnicmp(name, "elev", 4) == 0)
  {
    er = H5Sclose(fs);
    er = H5Dclose(fd);
    return 0;
  }

  DataField* s = new DataField;
  strcpy(s->name, name);

  hid_t attr = H5Aopen_name(fd, "valid_min");
  H5Aread(attr, H5T_NATIVE_FLOAT, &s->valid_range[0]);
  H5Aclose(attr);

  attr = H5Aopen_name(fd, "valid_max");
  H5Aread(attr, H5T_NATIVE_FLOAT, &s->valid_range[1]);
  H5Aclose(attr);

  if (strcmp(name, "scom") != 0)
  {
		if (strcmp(name, "perm") != 0)
		{
			s->color_mapping = 0;
		}
		else
		{
			s->color_mapping = 1;
		}
    // attr = H5Aopen_name(fd, "color_mapping");
    // H5Aread(attr, H5T_NATIVE_UCHAR, &s->color_mapping);
    // H5Aclose(attr);
  }

  attr = H5Aopen_name(fd, "long_name");
  hid_t type = H5Tcopy(H5T_C_S1);
  H5Tset_size(type, countof(s->info));
  H5Aread(attr, type, s->info);
  H5Aclose(attr);

  attr = H5Aopen_name(fd, "units");
  type = H5Tcopy(H5T_C_S1);
  H5Tset_size(type, countof(s->units));
  H5Aread(attr, type, s->units);
  H5Aclose(attr);

  s->id = fd;
	p->data.insert(std::make_pair(name, s));
  H5Sclose(fs);

  return 0;
}

int FieldRef::Init(CFieldDoc* pDoc)
{
  herr_t err = H5Giterate(pDoc->hdf5_model, ".", NULL, Push, this);
  return data.size();
}

void FieldRef::Done()
{
  for (FieldList::iterator it = data.begin(); it != data.end(); ++it)
  {
    DataField* s = it->second;
    H5Dclose(s->id);
    delete s;
  }
}

int FieldRef::Enum(int nd, const DataField* dd[]) const
{
  ASSERT(nd >= 0);
  for (FieldList::const_iterator it = data.begin(); it != data.end(); ++it)
  {
    if (!nd--)
      break;
    *dd++ = it->second;
  }
  return data.size();
}

const FieldRef::DataField* FieldRef::Field(std::string s) const
{
  FieldList::const_iterator it = data.find(s);
  if (it != data.end())
    return it->second;
  return 0;
}

//////////////// W E L L   S I T E ///////////////

WellSite::WellSite(): traj(), site(), node(site), data(0)
{
}

WellSite::~WellSite()
{
  Done();
}

#ifdef TRAJ_CHECK

void TrajEntry_Check(WellSite::TrajItem* p1, WellSite::TrajItem* p2, double d, const CFieldDoc* g)
{
  double t = (d - p1->d) / (p2->d - p1->d);

  double p[3];
  p[0] = p1->x + t * (p2->x - p1->x);
  p[1] = p1->y + t * (p2->y - p1->y);
  p[2] = p1->z + t * (p2->z - p1->z);

  double q[3];
  if (!INSIDE_DOM(g->Cell(p, q)))
    std::cout << "false\n";
  else
  {
    std::cout << q[0] << ' ' << q[1] << ' ' << q[2] << '\n';
  }
}

#endif

void WellEntry_Init(WellSite::WellTraj* t, const CFieldDoc* g)
{
  WellSite::TrajItem* p1 = t->start.item;
  WellSite::TrajItem* p2 = (t->block.empty()? t->final.item: t->block.front());
  const double eps = 1.0e-15;

  double s1[3], d1;
  double s2[3], d2;
  double s[3], d;
  s1[0] = p1->x, s1[1] = p1->y, s1[2] = p1->z, d1 = p1->d;
  s2[0] = p2->x, s2[1] = p2->y, s2[2] = p2->z, d2 = p2->d;

  int cx = g->cx;
  int cy = g->cy;
  int cz = g->cz;
  double x1 = g->x1;
  double x2 = g->x2;
  double y1 = g->y1;
  double y2 = g->y2;

  while (1)
  {
    double dx = s1[0] - s2[0];
    double dy = s1[1] - s2[1];
    double dz = s1[2] - s2[2];
    if (dx*dx + dy*dy + dz*dz < eps)
      break;

    s[0] = 0.5 * (s1[0] + s2[0]);
    s[1] = 0.5 * (s1[1] + s2[1]);
    s[2] = 0.5 * (s1[2] + s2[2]);
    d = 0.5 * (d1 + d2);

    double p[3];
    p[0] = (cx - 1) * (s[0] - x1) / (x2 - x1);
    p[1] = (cy - 1) * (s[1] - y1) / (y2 - y1);
    p[2] = 0;

    double q[3];
    g->Coord(p, q);
    if (q[2] < s[2])
    {
      s2[0] = s[0];
      s2[1] = s[1];
      s2[2] = s[2];
      d2 = d;
    }
    else if (q[2] > s[2])
    {
      s1[0] = s[0];
      s1[1] = s[1];
      s1[2] = s[2];
      d1 = d;
    }
    else
    {
      break;
    }
  }
  t->start.dist = d2;

#ifdef TRAJ_CHECK
  TrajEntry_Check(p1, p2, d2, g);
#endif

  if (!t->final.item)
    return;

  p1 = (t->block.empty()? t->start.item: t->block.back());
  p2 = t->final.item;

  s1[0] = p1->x, s1[1] = p1->y, s1[2] = p1->z, d1 = p1->d;
  s2[0] = p2->x, s2[1] = p2->y, s2[2] = p2->z, d2 = p2->d;

  while (1)
  {
    double dx = s1[0] - s2[0];
    double dy = s1[1] - s2[1];
    double dz = s1[2] - s2[2];
    if (dx*dx + dy*dy + dz*dz < eps)
      break;

    s[0] = 0.5 * (s1[0] + s2[0]);
    s[1] = 0.5 * (s1[1] + s2[1]);
    s[2] = 0.5 * (s1[2] + s2[2]);
    d = 0.5 * (d1 + d2);

    double p[3];
    p[0] = (cx - 1) * (s[0] - x1) / (x2 - x1);
    p[1] = (cy - 1) * (s[1] - y1) / (y2 - y1);
    p[2] = (cz - 1);

    double q[3];
    g->Coord(p, q);
    if (q[2] < s[2])
    {
      s2[0] = s[0];
      s2[1] = s[1];
      s2[2] = s[2];
      d2 = d;
    }
    else if (q[2] > s[2])
    {
      s1[0] = s[0];
      s1[1] = s[1];
      s1[2] = s[2];
      d1 = d;
    }
    else
    {
      break;
    }
  }
  t->final.dist = d1;

#ifdef TRAJ_CHECK
  TrajEntry_Check(p1, p2, d1, g);
#endif
}

int WellSite::Init(CFieldDoc* pDoc)
{
  std::ostringstream ss;

  data = pDoc;
  int cx = pDoc->cx;
  int cy = pDoc->cy;
  int cz = pDoc->cz;
  double x1 = pDoc->x1;
  double x2 = pDoc->x2;
  double y1 = pDoc->y1;
  double y2 = pDoc->y2;

	CCommand < CAccessor < TrajRecord > > cmd;
	if (SUCCEEDED(cmd.Open(pDoc->session, "select t.traj_wb, t.traj_id, t.traj_dx, t.traj_dy, t.traj_dz, "
		"t.traj_md, t.traj_ns, w.well_hx, w.well_hy, w.well_elev, w.well_md from traj t, well w "
		"where t.traj_wb=w.well_bore and t.traj_id=w.traj_id and w.well_elev is not null")))
	{
		while (cmd.MoveNext() == S_OK)
		{
			int ns = cmd.ns;
			std::vector<TrajItem> tt(ns + 2);
			TrajItem *t = &tt.front();
			t->d = t->x = t->y = t->z = 0.0f;
			t = &tt.back();
			t->d = cmd.md;
			t->x = cmd.dx;
			t->y = cmd.dy;
			t->z = cmd.dz;
			if (ns > 0)
			{
				char name[0x100];
				sprintf(name, "/traj/version_%d/%d", cmd.id, cmd.wb);
				hid_t dataset = H5Dopen(pDoc->hdf5_file, name);
				if (0 > dataset)
				{
					continue;
				}
				std::vector<float> ff(ns*4);
				if (0 > H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &ff[0]))
				{
					H5Dclose(dataset);
					continue;
				}
				float *f = &ff[0];
				for (int i = 1; i <= ns; ++i)
				{
					tt[i].x = *f++;
					tt[i].y = *f++;
					tt[i].z = *f++;
					tt[i].d = *f++;
				}
				H5Dclose(dataset);
			}
			if (cmd.md_status == DBSTATUS_S_OK && cmd.md_well > cmd.md)
			{
				tt.push_back(TrajItem());
				t = &tt[ns+2];
				TrajItem* t1 = &tt[ns+1];
				TrajItem* t2 = &tt[ns];
				double d = (cmd.md_well - t2->d) / (t1->d - t2->d);
				t->d = cmd.md_well;
				t->x = t2->x + d * (t1->x - t2->x);
				t->y = t2->y + d * (t1->y - t2->y);
				t->z = t2->z + d * (t1->z - t2->z);
			}

			{
				TrajBlock block;
				TrajItem* start = 0;
				TrajItem* final = 0;

				int ns = tt.size() - 1;

				float rr[4];
				rr[0] = float(tt[ns].x + cmd.hx);
				rr[1] = float(tt[ns].y + cmd.hy);
				rr[2] = float(tt[ns].z - cmd.hz);
				rr[3] = float(tt[ns].d);

				/*
				hid_t dataset = -1;
				hid_t dataspace = -1;
				if (ns > 0)
				{
					char name[0x100];
					sprintf(name, "/traj/version_%d/%d", cmd.id, cmd.wb);
					dataset = H5Dopen(pDoc->hdf5_file, name);

					hsize_t dims[2];
					dims[0] = ns;
					dims[1] = 4;
					dataspace = H5Screate_simple(2, dims, NULL);

					if (cmd.md_status != DBSTATUS_S_OK && cmd.md_well > cmd.md)
					{
						hssize_t start[2];
						start[0] = ns-1;
						start[1] = 0;

						hsize_t count[2];
						count[0] = 1;
						count[1] = 4;
						if (0 > H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, start, NULL, count, NULL))
						{
							continue;
						}
						if (0 > H5Dread(dataset, H5T_NATIVE_FLOAT, m_space, dataspace, H5P_DEFAULT, rr))
						{
							break;
						}
					}
				}
				*/

				while (1)
				{
					if (rr[0] < x1 || x2 < rr[0] || rr[1] < y1 || y2 < rr[1])
					{
						// trajectory item is out of grid
						break;
					}

					double pp[3];
					pp[0] = (cx - 1) * (rr[0] - x1) / (x2 - x1);
					pp[1] = (cy - 1) * (rr[1] - y1) / (y2 - y1);
					pp[2] = (cz - 1);

					double zz[3];
					pDoc->Coord(pp, zz);
					double h2 = zz[2];
					pp[2] = 0;
					pDoc->Coord(pp, zz);
					double h1 = zz[2];

					if (rr[2] < h1)
					{
						// first record above the model
						start = new TrajItem;
						start->d = rr[3];
						start->x = rr[0];
						start->y = rr[1];
						start->z = rr[2];
						break;
					}

					if (h2 < rr[2])
					{
						if (!final) final = new TrajItem;
						final->d = rr[3];
						final->x = rr[0];
						final->y = rr[1];
						final->z = rr[2];
					}
					else
					{
						TrajItem* item = new TrajItem;
						item->d = rr[3];
						item->x = rr[0];
						item->y = rr[1];
						item->z = rr[2];
						block.push(item);
					}

					if (!ns--)
					{
						// there is no record to process
						_ASSERTE(start == 0);
						start = new TrajItem;
						start->d = 0.0;
						start->x = cmd.hx;
						start->y = cmd.hy;
						start->z = -cmd.hz;
						break;
					}

					rr[0] = float(tt[ns].x + cmd.hx);
					rr[1] = float(tt[ns].y + cmd.hy);
					rr[2] = float(tt[ns].z - cmd.hz);
					rr[3] = float(tt[ns].d);
				}

				if (start != 0 && (final != 0 || !block.empty()))
				{
					WellTraj* t = new WellTraj;
					t->start.item = start;
					t->final.item = final;
					t->block = block;
					traj.insert(cmd.wb, t);
					WellEntry_Init(t, pDoc);
				}
				else
				{
					// trajectory has no common points with the model
					delete start;
					delete final;
					TrajItem* it;
					forall(it, block)
					{
						delete it;
					}
				}
			}
		}
	}
	ResetPoints(pDoc);
  int n = traj.size();
  TRACE1(_T("%d wells loaded ok\n"), n);
  return n;
}

void WellSite::ResetPoints(CFieldDoc* pDoc)
{
  int cx = pDoc->cx;
  int cy = pDoc->cy;
  int cz = pDoc->cz;
  double x1 = pDoc->x1;
  double x2 = pDoc->x2;
  double y1 = pDoc->y1;
  double y2 = pDoc->y2;

	site.clear();

  long b;
  forall_defined(b, traj)
  {
    WellTraj* t = traj.access(b);
    TrajItem* p1 = t->start.item;
    TrajItem* p2 = (t->block.empty()? t->final.item: t->block.front());
    double s = (t->start.dist - p1->d) / (p2->d - p1->d);
    double x = p1->x + s * (p2->x - p1->x);
    double y = p1->y + s * (p2->y - p1->y);
    double i = (cx-1) * (x-x1) / (x2-x1);
    double j = (cy-1) * (y-y1) / (y2-y1);
   
		// TODO: I should somehow assure that there is no duplicate well sites
    leda_point p(i, j);
		if (site.lookup(p) != leda_nil)
		{
			leda_node n = site.lookup(p);
			long bb = node[n];
			if (b < bb)
			{
				node[n] = b;
			}
		}
		else
		{
			leda_node n = site.insert(p);
			node[n] = b;
		}
    // _ASSERTE(site.lookup(p) == leda_nil);
    // leda_node n = site.insert(p);
    // node[n] = b;
  }
}

void WellSite::Done()
{
  WellTraj* t;
  forall(t, traj)
  {
    delete t->start.item;
    delete t->final.item;
    TrajItem *it;
    forall(it, t->block)
    {
      delete it;
    }
    delete t;
  }

  WellPerf pp;
  forall(pp, perf)
  {
    PerfItem *p;
    forall(p, pp)
    {
      delete p;
    }
  }

  WellInfo qq;
  forall(qq, info)
  {
    InfoItem* q;
    forall(q, qq)
    {
      delete q;
    }
  }

  ClearProd();
  ClearPump();
}

int WellTraj_Coord(WellSite::WellTraj* t, int n, double *pp)
{
  _ASSERTE(n >= 0);

  if (t->block.empty())
  {
    int total = 2;
    
    if (!n--)
      return total;

    WellSite::TrajItem *p1 = t->start.item;
    WellSite::TrajItem *p2 = t->final.item;
    double s = (t->start.dist - p1->d) / (p2->d - p1->d);
    *pp++ = p1->x + s * (p2->x - p1->x);
    *pp++ = p1->y + s * (p2->y - p1->y);
    *pp++ = p1->z + s * (p2->z - p1->z);

    if (!n--)
      return total;

    s = (t->final.dist - p1->d) / (p2->d - p1->d);
    *pp++ = p1->x + s * (p2->x - p1->x);
    *pp++ = p1->y + s * (p2->y - p1->y);
    *pp++ = p1->z + s * (p2->z - p1->z);

    return total;
  }

  int total = 1 + t->block.size() + (t->final.item? 1: 0);

  if (!n--)
    return total;

  WellSite::TrajItem* p1 = t->start.item;
	leda::list_item it = t->block.first();
  WellSite::TrajItem* p2 = t->block[it];
  double s = (t->start.dist - p1->d) / (p2->d - p1->d);
  *pp++ = p1->x + s * (p2->x - p1->x);
  *pp++ = p1->y + s * (p2->y - p1->y);
  *pp++ = p1->z + s * (p2->z - p1->z);

  if (1.0 <= s)
  {
    it = t->block.succ(it);
    total -= 1;
  }

  while (it != leda_nil)
  {
    if (!n--)
      return total;

    p1 = t->block[it];
    *pp++ = p1->x;
    *pp++ = p1->y;
    *pp++ = p1->z;
    it = t->block.succ(it);
  }

  p2 = t->final.item;
  if (!p2 || !n--)
    return total;

  s = (t->final.dist - p1->d) / (p2->d - p1->d);
  if (s <= 0.0)
    return (total-1);

  *pp++ = p1->x + s * (p2->x - p1->x);
  *pp++ = p1->y + s * (p2->y - p1->y);
  *pp++ = p1->z + s * (p2->z - p1->z);
  return total;
}

double WellTraj_Length(WellSite::WellTraj* t)
{
  if (!t)
    return 0.0;

  int n = WellTraj_Coord(t, 0, 0);
  std::valarray<double> pp(3*n);
  double *p = &pp[0];
  WellTraj_Coord(t, n, p);
  double s = 0.0, *q = p+3;
  for (int i = 1; i < n; ++i)
  {
    double dx = p[0] - q[0], dy = p[1] - q[1], dz = p[2] - q[2];
    double dd = sqrt(dx * dx + dy * dy + dz * dz);
    s += dd;
    p = q;
    q += 3;
  }
  return s;
}

double CFieldDoc::TrajLength(long b)
{
	WellSite::WellTraj* t = ww->Traj(b);
	return (t? WellTraj_Length(t): 0.0);
}

#ifdef _DEBUG

void WellSite::Dump(CDumpContext& dc) const
{
  dc << "\nwell site: size = " << traj.size() << "\n";
  if (dc.GetDepth() & CFieldDoc::DUMP_WELLSITE)
  {
    long b;
    forall_defined(b, traj)
    {
      WellTraj* t = traj.access(b);
      int n = WellTraj_Coord(t, 0, 0);
			double* p = new double [3*n];
      int ii = WellTraj_Coord(t, n, p);
      dc << "\t" << b << "\n";
      for (int i = 0; i < ii; ++i)
      {
        double q[3];
        int flags = data->Cell(p+3*i, q);
        if (INSIDE_DOM(flags))
        {
          dc << "\t\t" << q[0] << " " << q[1] << " " << q[2] << "\n";
        }
        else
        {
          dc << "\t\t????????\n";
        }
      }
      if (ii < n)
      {
        dc << "\t\t........\n";
      }
			delete[] p;
    }
  }

  dc << "\nwell prod: size = " << prod.size() << "\n";
  if (dc.GetDepth() & CFieldDoc::DUMP_WELLPROD)
  {
    long b;
    forall_defined(b, prod)
    {
      dc << "\t" << b << "\n";
      WellProd pp = prod.access(b);
      ProdItem* it;
      forall(it, pp)
      {
        dc << "\t\t" << it->bed << " " << (int) it->days << " " \
          << it->oil << " " << it->wat << " " << it->gas << "\n";
      }
    }
  }

  dc << "\nwell pump: size = " << pump.size() << "\n";
  if (dc.GetDepth() & CFieldDoc::DUMP_WELLPUMP)
  {
    long b;
    forall_defined(b, pump)
    {
      dc << "\t" << b << "\n";
      WellPump pp = pump.access(b);
      PumpItem* it;
      forall(it, pp)
      {
        dc << "\t\t" << it->bed << " " << (int) it->days << " " << it->fluid << "\n";
      }
    }
  }
}

#endif

void WellSite::ClearProd()
{
  WellProd pp;
  forall(pp, prod)
  {
    ProdItem* p;
    forall(p, pp)
    {
      delete p;
    }
  }
  prod.clear();
}

void WellSite::ClearPump()
{
  WellPump pp;
  forall(pp, pump)
  {
    PumpItem* p;
    forall(p, pp)
    {
      delete p;
    }
  }
  pump.clear();
}

WellSite::WellInfo WellSite::Info(long b)
{
	leda::dic_item it = info.lookup(b);
  if (it != leda_nil)
    return info.inf(it);

	it = traj.lookup(b);
	if (it == leda_nil)
		return WellInfo();

	WellTraj* t = traj.inf(it);
	TrajItem* p1 = t->start.item;
	double d1 = t->start.dist;
	double d2 = d1 + WellTraj_Length(t);

	leda::dic_item di = roof.lookup(b);
	if (di != leda_nil)
	{
		roof.change_inf(di, d1);
	}
	else
	{
		roof.insert(b, d1);
	}

	char name[64];
	sprintf(name, "geophysics/%d", b);
	hid_t file = data->hdf5_file;
	hid_t data = H5Dopen(file, name);
	if (0 > data)
	{
		return WellInfo();
	}

	hid_t space = H5Dget_space(data);
	if (0 > space)
	{
		H5Dclose(data);
		return WellInfo();
	}

	hsize_t dim[2];
	if (0 > H5Sget_simple_extent_dims(space, dim, NULL))
	{
		H5Sclose(space);
		H5Dclose(data);
		return WellInfo();
	}

	int n = (int) dim[0];
	long (*rr)[5] = new long[n][5];
	if (0 > H5Dread(data, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &rr[0][0]))
	{
		delete[] rr;
		H5Sclose(space);
		H5Dclose(data);
		return WellInfo();
	}

	H5Sclose(space);
	H5Dclose(data);

  GeophysDataset backgroundgas(file, b, "/geophysics/backgroundgas");
  GeophysDataset clayiness(file, b, "/geophysics/clayiness");
  GeophysDataset difgrlog(file, b, "/geophysics/difgrlog");
  GeophysDataset difneulog(file, b, "/geophysics/difneulog");
  GeophysDataset finaldepth(file, b, "/geophysics/finaldepth");
  GeophysDataset finaldist(file, b, "/geophysics/finaldist");
  GeophysDataset hydrocarbon(file, b, "/geophysics/hydrocarbon");
  GeophysDataset permeability(file, b, "/geophysics/permeability");
  GeophysDataset porosity(file, b, "/geophysics/porosity");
  GeophysDataset relgrlog(file, b, "/geophysics/relgrlog");
  GeophysDataset relneulog(file, b, "/geophysics/relneulog");
  GeophysDataset relsplog(file, b, "/geophysics/relsplog");
  GeophysDataset resistivity(file, b, "/geophysics/resistivity");
  GeophysDataset startdepth(file, b, "/geophysics/startdepth");
  GeophysDataset startdist(file, b, "/geophysics/startdist");

	const float nodata = -9999.0;
	InfoItem* item = new InfoItem;
	WellInfo acc;
	for (int i = 0; i < n; ++i)
	{
		item->bed = rr[i][1];
		item->lit = rr[i][3];
		item->sat = rr[i][2];
		item->a_sp = nodata;
		if ((rr[i][0] & GeophysDataset::SPOTENTIAL) != 0 && 0 > relsplog.read(&item->a_sp))
			break;
		item->clay = nodata;
		if ((rr[i][0] & GeophysDataset::CLAYINESS) != 0 && 0 > clayiness.read(&item->clay))
			break;
		item->f_res = nodata;
		if ((rr[i][0] & GeophysDataset::RESISTIVITY) != 0 && 0 > resistivity.read(&item->f_res))
			break;
		item->kg = nodata;
		if ((rr[i][0] & GeophysDataset::BACKGROUNDGAS) != 0 && 0 > backgroundgas.read(&item->kg))
			break;
		item->kng = nodata;
		if ((rr[i][0] & GeophysDataset::HYDROCARBON) != 0 && 0 > hydrocarbon.read(&item->kng))
			break;
		if (0 > startdist.read(&item->top))
			break;
		item->perm = nodata;
		if ((rr[i][0] & GeophysDataset::PERMEABILITY) != 0 && 0 > permeability.read(&item->perm))
			break;
		item->poro = nodata;
		if ((rr[i][0] & GeophysDataset::POROSITY) != 0 && 0 > porosity.read(&item->poro))
			break;
		if (0 > finaldist.read(&item->bot))
			break;
		if (d1 <= item->top && item->bot <= d2)
		{
			item->top -= (float) d1;
			item->bot -= (float) d1;
			acc.push_back(item);
			item = new InfoItem;
		}
	}
	delete item;
	delete[] rr;
	info.insert(b, acc);
	return acc;
}

WellSite::WellInfo WellSite::Info2(long b, long s)
{
	leda::dic_item it = info.lookup(b);
  if (it != leda_nil)
    return info.inf(it);

	it = traj.lookup(b);
	if (it == leda_nil)
		return WellInfo();

	WellTraj* t = traj.inf(it);
	float d = (float) WellTraj_Length(t);

	char name[64];
	sprintf(name, "geophysics/%d", b);
	hid_t file = data->hdf5_file;
	hid_t data = H5Dopen(file, name);
	if (0 > data)
	{
		return WellInfo();
	}

	hid_t space = H5Dget_space(data);
	if (0 > space)
	{
		H5Dclose(data);
		return WellInfo();
	}

	hsize_t dim[2];
	if (0 > H5Sget_simple_extent_dims(space, dim, NULL))
	{
		H5Sclose(space);
		H5Dclose(data);
		return WellInfo();
	}

	int n = (int) dim[0];
	long (*rr)[5] = new long[n][5];
	if (0 > H5Dread(data, H5T_NATIVE_LONG, H5S_ALL, H5S_ALL, H5P_DEFAULT, &rr[0][0]))
	{
		H5Sclose(space);
		H5Dclose(data);
		delete[] rr;
		return WellInfo();
	}

	H5Sclose(space);
	H5Dclose(data);

  GeophysDataset backgroundgas(file, b, "/geophysics/backgroundgas");
  GeophysDataset clayiness(file, b, "/geophysics/clayiness");
  GeophysDataset difgrlog(file, b, "/geophysics/difgrlog");
  GeophysDataset difneulog(file, b, "/geophysics/difneulog");
  GeophysDataset finaldepth(file, b, "/geophysics/finaldepth");
  GeophysDataset finaldist(file, b, "/geophysics/finaldist");
  GeophysDataset hydrocarbon(file, b, "/geophysics/hydrocarbon");
  GeophysDataset permeability(file, b, "/geophysics/permeability");
  GeophysDataset porosity(file, b, "/geophysics/porosity");
  GeophysDataset relgrlog(file, b, "/geophysics/relgrlog");
  GeophysDataset relneulog(file, b, "/geophysics/relneulog");
  GeophysDataset relsplog(file, b, "/geophysics/relsplog");
  GeophysDataset resistivity(file, b, "/geophysics/resistivity");
  GeophysDataset startdepth(file, b, "/geophysics/startdepth");
  GeophysDataset startdist(file, b, "/geophysics/startdist");

	bool found = false;
	bool done = false;
	float top;
	const float nodata = -9999.0;
	InfoItem* item = new InfoItem;
	WellInfo acc;
	for (int i = 0; i < n; ++i)
	{
		item->bed = rr[i][1];
		item->lit = rr[i][3];
		item->sat = rr[i][2];
		item->a_sp = nodata;
		if ((rr[i][0] & GeophysDataset::SPOTENTIAL) != 0 && 0 > relsplog.read(&item->a_sp))
			break;
		item->clay = nodata;
		if ((rr[i][0] & GeophysDataset::CLAYINESS) != 0 && 0 > clayiness.read(&item->clay))
			break;
		item->f_res = nodata;
		if ((rr[i][0] & GeophysDataset::RESISTIVITY) != 0 && 0 > resistivity.read(&item->f_res))
			break;
		item->kg = nodata;
		if ((rr[i][0] & GeophysDataset::BACKGROUNDGAS) != 0 && 0 > backgroundgas.read(&item->kg))
			break;
		item->kng = nodata;
		if ((rr[i][0] & GeophysDataset::HYDROCARBON) != 0 && 0 > hydrocarbon.read(&item->kng))
			break;
		if (0 > startdist.read(&item->top))
			break;
		item->perm = nodata;
		if ((rr[i][0] & GeophysDataset::PERMEABILITY) != 0 && 0 > permeability.read(&item->perm))
			break;
		item->poro = nodata;
		if ((rr[i][0] & GeophysDataset::POROSITY) != 0 && 0 > porosity.read(&item->poro))
			break;
		if (0 > finaldist.read(&item->bot))
			break;
		if (found)
		{
			item->top -= (float) top;
			item->bot -= (float) top;
			if (item->top > d)
			{
				done = true;
			}
			else
			{
				if (item->bot > d)
				{
					item->bot = d;
					done = true;
				}
				acc.push_back(item);
				item = new InfoItem;
			}
		}
		else if (item->bed == s)
		{
			leda::dic_item di = roof.lookup(b);
			if (di != leda_nil)
			{
				roof.change_inf(di, item->top);
			}
			else
			{
				roof.insert(b, item->top);
			}

			top = item->top;
			item->top -= top;
			item->bot -= top;
			found = true;
			if (item->bot > d)
			{
				item->bot = d;
				done = true;
			}
			acc.push_back(item);
			item = new InfoItem;
		}
		if (done)
		{
			break;
		}
	}
	delete item;
	delete[] rr;
	if (!acc.empty())
	{
		info.insert(b, acc);
		return acc;
	}
	return Info(b);
}

double WellSite::Roof(long b)
{
	leda::dic_item di = roof.lookup(b);
  if (di == leda_nil)
  {
		return 0.0;
  }
  return roof.inf(di);
}

int WellSite::ProdQuery(const COleDateTime& dt, int nb, long* bb)
{
  ClearProd();

	CCommand < CAccessor < HistRecord > > cmd;

	char query[0x400];
	sprintf(query, "select wellbore, formation, startdate, finaldate, oil, wat, gas, fluid "
		"from hist where %g between startdate and finaldate and oil is not null or wat is not "
		"null or gas is not null", (DATE) dt);

	if (FAILED(cmd.Open(data->session, query)))
	{
		while (cmd.MoveNext() == S_OK)
		{
			ProdItem* p = new ProdItem;
			p->bed = cmd.formation;
			p->days = (unsigned char) (cmd.finaldate - cmd.startdate);
			p->oil = (cmd.status.oil == DBSTATUS_S_OK? cmd.oil: 0.0f);
			p->wat = (cmd.status.wat == DBSTATUS_S_OK? cmd.wat: 0.0f);
			p->gas = (cmd.status.gas == DBSTATUS_S_OK? cmd.gas: 0.0f);

			long b = cmd.wellbore;
			leda::dic_item it = prod.lookup(b);
			if (it == leda_nil)
			{
				it = prod.insert(b, WellProd());
			}
			prod[it].push_back(p);
		}

		long b;
		forall_defined(b, prod)
		{
			if (!nb--)
				break;
			*bb++ = b;
		}
		return prod.size();
	}
	return 0;

}

WellSite::WellProd WellSite::Prod(long b)
{
	leda::dic_item it = prod.lookup(b);
  if (it != leda_nil)
    return prod.inf(it);
  return WellProd();
}

int WellSite::PumpQuery(const COleDateTime& dt, int nb, long* bb)
{
  ClearPump();

	CCommand < CAccessor < HistRecord > > cmd;

	char query[0x400];
	sprintf(query, "select wellbore, formation, startdate, finaldate, oil, wat, gas, fluid "
		"from hist where %g between startdate and finaldate and fluid is not null", 
		(DATE) dt);

	if (FAILED(cmd.Open(data->session, query)))
	{
		while (cmd.MoveNext() == S_OK)
		{
			PumpItem* p = new PumpItem;
			p->bed = cmd.formation;
			p->days = (unsigned char) (cmd.finaldate - cmd.startdate);
			p->fluid = cmd.fluid;

			long b = cmd.wellbore;
			leda::dic_item it = pump.lookup(b);
			if (it == leda_nil)
			{
				it = pump.insert(b, WellPump());
			}
			pump[it].push_back(p);
		}

		long b;
		forall_defined(b, pump)
		{
			if (!nb--)
				break;
			*bb++ = b;
		}
		return pump.size();
	}
	return 0;
}

WellSite::WellPump WellSite::Pump(long b)
{
	leda::dic_item it = pump.lookup(b);
  if (it != leda_nil)
    return pump.inf(it);
  return WellPump();
}


int WellSite::WindowQuery(leda_point a, leda_point b, int nb, long* bb)
{
  ASSERT(nb >= 0);
  leda_list<leda_node> gg = site.range_search(a, b);
  leda_node n;
  forall(n, gg)
  {
    if (!nb--)
      break;
    *bb++ = node[n];
  }
  return gg.size();
}

int WellSite::RangeQuery(leda_circle c, int nb, long* bb)
{
  ASSERT(nb >= 0);
  leda_list<leda_node> gg = site.range_search(c);
  leda_node n;
  forall(n, gg)
  {
    if (!nb--)
      break;
    *bb++ = node[n];
  }
  return gg.size();
}

WellSite::WellPerf WellSite::Perf(long b)
{
	leda::dic_item it = perf.lookup(b);
  if (it != leda_nil)
    return perf.inf(it);

	WellPerf acc;
	it = traj.lookup(b);
	if (it == leda_nil)
		return acc;

	WellTraj* t = traj.inf(it);
	TrajItem* p1 = t->start.item;
	float d1 = (float) t->start.dist;
	float d2 = float(d1 + WellTraj_Length(t));

	char query[64];
	sprintf(query, "select * from perf where perf_bore=%d", b);
	CCommand< CAccessor < PerfRecord > > cmd;
	if (SUCCEEDED(cmd.Open(data->session, query)))
	{
		while (cmd.MoveNext() == S_OK)
		{
			if (cmd.perf_lower > d1 && cmd.perf_upper < d2)
			{
				PerfItem* item = new PerfItem;
				item->perf_form = cmd.perf_form;
				item->perf_comp = cmd.perf_comp;
				item->perf_done = false;
				if (cmd.status.perf_close == DBSTATUS_S_OK)
				{
					item->perf_done = true;
					item->perf_close = cmd.perf_close;
				}
				item->perf_upper = cmd.perf_upper - d1;
				item->perf_lower = cmd.perf_lower - d1;
				acc.push_back(item);
			}
		}
		perf.insert(b, acc);
	}
	return acc;
}

WellSite::WellPerf WellSite::Perf2(long b, long s)
{
	leda::dic_item it = perf.lookup(b);
  if (it != leda_nil)
    return perf.inf(it);

	WellPerf acc;
	it = traj.lookup(b);
	if (it == leda_nil)
		return acc;

	char query[128];
	sprintf(query, "select wellbore, formation, startdist, finaldist from geophysics "
		"where wellbore=%d and formation=%d", b, s);
	CCommand < CAccessor < GeophysRecord > > cmd1;
	if (FAILED(cmd1.Open(data->session, query)) || S_OK != cmd1.MoveNext())
		return Perf(b);
	
	WellTraj* t = traj.inf(it);
	float d1 = cmd1.startdist;
	float d2 = float(d1 + WellTraj_Length(t));

	sprintf(query, "select * from perf where perf_bore=%d", b);
	CCommand< CAccessor < PerfRecord > > cmd2;
	if (SUCCEEDED(cmd2.Open(data->session, query)))
	{
		while (cmd2.MoveNext() == S_OK)
		{
			if (cmd2.perf_lower < d1)
				continue;
			if (cmd2.perf_upper > d1+d2)
				continue;

			if (cmd2.perf_upper < d1)
				cmd2.perf_upper = 0.0;
			else
				cmd2.perf_upper -= d1;

			if (cmd2.perf_lower > d1+d2)
				cmd2.perf_lower = d2;
			else
				cmd2.perf_lower -= d1;

			PerfItem* item = new PerfItem;
			item->perf_form = cmd2.perf_form;
			item->perf_comp = cmd2.perf_comp;
			item->perf_done = false;
			if (cmd2.status.perf_close == DBSTATUS_S_OK)
			{
				item->perf_done = true;
				item->perf_close = cmd2.perf_close;
			}
			item->perf_upper = cmd2.perf_upper;
			item->perf_lower = cmd2.perf_lower;
			acc.push_back(item);
		}

		if (acc.empty())
			return Perf(b);
		perf.insert(b, acc);
	}
	return acc;
}

WellSite::WellTraj* WellSite::Traj(long b)
{
	leda::dic_item it = traj.lookup(b);
  if (it != leda_nil)
    return traj.inf(it);
  return 0;
}

//////////////// F I E L D   D O C ////////////////////

IMPLEMENT_DYNCREATE(CFieldDoc, CDocument)

BEGIN_MESSAGE_MAP(CFieldDoc, CDocument)
  //{{AFX_MSG_MAP(CFieldDoc)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

CFieldDoc::CFieldDoc(): /* hf(-1), */ cx(0), cy(0), cz(0), x1(0), x2(1), \
  y1(0), y2(1), z1(0), z2(1), sz(0), nz(0), zz(0), ww(0), ff(0)
{
	hdf5_model = -1;
	hdf5_file = -1;
	base_formation = -1;
}

CFieldDoc::~CFieldDoc()
{
  delete ff;
  delete ww;

	/*
  if (hf >= 0)
    H5Fclose(hf);
	*/

	if (hdf5_file >= 0)
		H5Fclose(hdf5_file);

	if (hdf5_model >= 0)
		H5Fclose(hdf5_model);

  delete[] nz;
  delete[] zz;
}

BOOL Dataset_ReadDimensions(CFieldDoc* pDoc)
{
  short cx, cy, cz;
  hid_t attr;
  herr_t err;

  attr = H5Aopen_name(pDoc->hdf5_model, "cx");
  if (attr < 0)
  {
    TRACE0(_T("Cannot get cx attribute\n"));
    return FALSE;
  }
  err = H5Aread(attr, H5T_NATIVE_SHORT, &cx);
  if (err < 0)
  {
    TRACE0(_T("Failed to read an attribute\n"));
    err = H5Aclose(attr);
    return FALSE;
  }
  err = H5Aclose(attr);

  attr = H5Aopen_name(pDoc->hdf5_model, "cy");
  if (attr < 0)
  {
    TRACE0(_T("Cannot get cy attribute\n"));
    return FALSE;
  }
  err = H5Aread(attr, H5T_NATIVE_SHORT, &cy);
  if (err < 0)
  {
    TRACE0(_T("Failed to read an attribute\n"));
    err = H5Aclose(attr);
    return FALSE;
  }
  err = H5Aclose(attr);

  attr = H5Aopen_name(pDoc->hdf5_model, "cz");
  if (attr < 0)
  {
    TRACE0(_T("Cannot get cz attribute\n"));
    return FALSE;
  }
  err = H5Aread(attr, H5T_NATIVE_SHORT, &cz);
  if (err < 0)
  {
    TRACE0(_T("Failed to read an attribute\n"));
    err = H5Aclose(attr);
    return FALSE;
  }
  err = H5Aclose(attr);

  if (cx < 2 || cy < 2 || cy < 2)
  {
    TRACE0(_T("Invalid dimension attribute\n"));
    return FALSE;
  }

  pDoc->cx = cx;
  pDoc->cy = cy;
  pDoc->cz = cz;
  return TRUE;
}

BOOL Dataset_ReadElevation(CFieldDoc* pDoc)
{
	herr_t err = 0;

  hid_t geom = H5Dopen(pDoc->hdf5_model, "elev");
  if (geom < 0)
  {
    TRACE0(_T("Failed to open a geometry dataset\n"));
    return FALSE;
  }
  
  hid_t space = H5Dget_space(geom);
  if (space < 0)
  {
    TRACE0(_T("Failed to get a geometry dataspace\n"));
    err = H5Dclose(geom);
    return FALSE;
  }

  int ndim = H5Sget_simple_extent_ndims(space);
  if (ndim != 2)
  {
    TRACE0(_T("Invalid dataspace extent\n"));
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

	hsize_t dim[2];
  if (H5Sget_simple_extent_dims(space, dim, NULL) < 0)
  {
    TRACE0(_T("Unable to get dataspace extent\n"));
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  if (dim[0] != pDoc->cy || dim[1] != pDoc->cx)
  {
    TRACE0(_T("Inconsistent size of geometry object\n"));
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  int nn = pDoc->cx * pDoc->cy;
  dim[0] = nn;
  hid_t mem = H5Screate_simple(1, dim, NULL);
  if (mem < 0)
  {
    TRACE0(_T("Failed to create a memory dataspace\n"));
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  float* zz = new float[2 * nn];
  err = H5Dread(geom, H5T_NATIVE_FLOAT, mem, space, H5P_DEFAULT, zz);
  if (err < 0)
  {
    TRACE0(_T("Failed to read elevation grids\n"));
    delete[] zz;
    err = H5Sclose(mem);
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  err = H5Sclose(mem);
  err = H5Sclose(space);
	err = H5Dclose(geom);

	hid_t attr = H5Aopen_name(pDoc->hdf5_model, "x1");
	if (0 > attr)
	{
		TRACE(_T("Failed to open x1 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	float x1;
	err = H5Aread(attr, H5T_NATIVE_FLOAT, &x1);
	if (0 > err)
	{
		TRACE(_T("Failed to load x1 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	err = H5Aclose(attr);

	attr = H5Aopen_name(pDoc->hdf5_model, "x2");
	if (0 > attr)
	{
		TRACE(_T("Failed to open x2 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	float x2;
	err = H5Aread(attr, H5T_NATIVE_FLOAT, &x2);
	if (0 > err)
	{
		TRACE(_T("Failed to load x1 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	err = H5Aclose(attr);

	attr = H5Aopen_name(pDoc->hdf5_model, "y1");
	if (0 > attr)
	{
		TRACE(_T("Failed to open y1 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	float y1;
	err = H5Aread(attr, H5T_NATIVE_FLOAT, &y1);
	if (0 > err)
	{
		TRACE(_T("Failed to load y1 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	err = H5Aclose(attr);

	attr = H5Aopen_name(pDoc->hdf5_model, "y2");
	if (0 > attr)
	{
		TRACE(_T("Failed to open y2 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	float y2;
	err = H5Aread(attr, H5T_NATIVE_FLOAT, &y2);
	if (0 > err)
	{
		TRACE(_T("Failed to load y2 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	err = H5Aclose(attr);

	attr = H5Aopen_name(pDoc->hdf5_model, "dz");
	if (0 > attr)
	{
		TRACE(_T("Failed to open y2 attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	float dz;
	err = H5Aread(attr, H5T_NATIVE_FLOAT, &dz);
	if (0 > err)
	{
		TRACE(_T("Failed to load dz attribute of the model\n"));
		delete[] zz;
		return FALSE;
	}
	err = H5Aclose(attr);

	dz *= pDoc->cz-1;
	pDoc->sz = 2;
	pDoc->nz = new short[2];
	pDoc->nz[0] = 0;
	pDoc->nz[1] = pDoc->cz-1;
	pDoc->zz = zz;
	pDoc->z1 = *std::min_element(zz, zz+nn);
	pDoc->z2 = *std::max_element(zz, zz+nn) + dz;
	std::transform(zz, zz+nn, zz+nn, std::bind2nd(std::plus<float>(), dz));
	pDoc->x1 = x1;
	pDoc->x2 = x2;
	pDoc->y1 = y1;
	pDoc->y2 = y2;
	return TRUE;

	/*
  attr = H5Aopen_name(geom, "card");
  if (attr < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Failed to open a card attribute\n"));
    delete[] zz;
    err = H5Dclose(geom);
    return FALSE;
  }

  space = H5Aget_space(attr);
  if (space < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Failed to get attribute dataspace\n"));
    delete[] zz;
    err = H5Aclose(attr);
    err = H5Dclose(geom);
    return FALSE;
  }

  ndim = H5Sget_simple_extent_ndims(space);
  if (ndim != 1)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Failed to get a dataspace rank\n"));
    delete[] zz;
    err = H5Aclose(attr);
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  if (H5Sget_simple_extent_dims(space, dim, NULL) < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Unable to get dataspace extent\n"));
    delete[] zz;
    err = H5Aclose(attr);
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  err = H5Sclose(space);
  if (dim[0] != sz-1)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Invalid dimension of card attribute\n"));
    delete[] zz;
    err = H5Aclose(attr);
    err = H5Dclose(geom);
    return FALSE;
  }

  dz = new short[sz-1];
  err = H5Aread(attr, H5T_NATIVE_SHORT, dz);
  if (err < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Cannot read the card attribute\n"));
    delete[] dz;
    delete[] zz;
    err = H5Aclose(attr);
    err = H5Dclose(geom);
    return FALSE;
  }

  err = H5Aclose(attr);
  nz = new short[sz];
  nz[0] = 0;
  for (int i = 1, n = 0; i < sz; ++i)
  {
    n += dz[i-1];
    nz[i] = n-1;
  }
  delete[] dz;

  if (nz[i-1] != pDoc->cz-1)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Invalid card attribute\n"));
    delete[] nz;
    delete[] zz;
    err = H5Dclose(geom);
    return FALSE;    
  }

  attr = H5Aopen_name(geom, "bbox");
  if (attr < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Failed to open a bbox attribute\n"));
    delete[] zz;
    delete[] nz;
    err = H5Dclose(geom);
    return FALSE;
  }

  space = H5Aget_space(attr);
  if (space < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Failed to get an attribute dataspace\n"));
    delete[] zz;
    delete[] nz;
    err = H5Aclose(attr);
    err = H5Dclose(geom);
    return FALSE;
  }

  ndim = H5Sget_simple_extent_ndims(space);
  if (ndim != 1)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Failed to get a dataspace rank\n"));
    delete[] zz;
    delete[] nz;
    err = H5Aclose(attr);
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  if (H5Sget_simple_extent_dims(space, dim, NULL) < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Unable to get dataspace extent\n"));
    delete[] zz;
    delete[] nz;
    err = H5Aclose(attr);
    err = H5Sclose(space);
    err = H5Dclose(geom);
    return FALSE;
  }

  err = H5Sclose(space);
  if (dim[0] != 6)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Invalid dimension of bbox attribute\n"));
    delete[] zz;
    delete[] nz;
    err = H5Aclose(attr);
    err = H5Dclose(geom);
    return FALSE;
  }

  err = H5Aread(attr, H5T_NATIVE_DOUBLE, bb);
  if (err < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Cannot read the bbox attribute\n"));
    delete[] dz;
    delete[] zz;
    err = H5Aclose(attr);
    err = H5Dclose(geom);
    return FALSE;
  }

  err = H5Aclose(attr);
  err = H5Dclose(geom);

  pDoc->sz = sz;
  pDoc->nz = nz;
  pDoc->zz = zz;
  pDoc->x1 = bb[0];
  pDoc->x2 = bb[1];
  pDoc->y1 = bb[2];
  pDoc->y2 = bb[3];
  pDoc->z1 = bb[4];
  pDoc->z2 = bb[5];
  return TRUE;
	*/
}

BOOL CFieldDoc::OpenDatasetFile(const char* name, const char* group)
{
	hdf5_file = H5Fopen(name, H5F_ACC_RDWR, H5P_DEFAULT);
	if (hdf5_file < 0)
	{
		TRACE("failed to open file fedorovskoe.h5\n");
		return FALSE;
	}

	hdf5_model = H5Gopen(hdf5_file, group);
	if (hdf5_model < 0)
	{
		TRACE("failed to open group\n");
		return FALSE;
	}

	/*
  hf = H5Fopen(name, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (hf < 0)
  {
    TRACE1(_T(__FILE__) _T("(%d): "), __LINE__);
    TRACE0(_T("Failed to open a HDF5 file\n"));
    return FALSE;
  }
	*/

  if (!Dataset_ReadDimensions(this))
  {
    H5Gclose(hdf5_model);
    hdf5_model = -1;
		H5Fclose(hdf5_file);
		hdf5_file = -1;
    return FALSE;
  }

  if (!Dataset_ReadElevation(this))
  {
    H5Gclose(hdf5_model);
    hdf5_model = -1;
		H5Fclose(hdf5_file);
		hdf5_file = -1;
    return FALSE;
  }

  ff = new FieldRef;
  if (!ff->Init(this))
  {
    delete ff;
    ff = 0;
    return FALSE;
  }

  return TRUE;
}

BOOL CFieldDoc::OpenDatabaseFile(LPCTSTR name)
{
	ATL::CDataSource conn;
	HRESULT hr = conn.Open("Microsoft.Jet.OLEDB.4.0", name);
	if (FAILED(hr))
		return FALSE;
	hr = session.Open(conn);
	if (FAILED(hr))
		return FALSE;

  ww = new WellSite;
  if (!ww->Init(this))
  {
    delete ww;
    ww = 0;

    return FALSE;
  }

  return TRUE;
}

BOOL CFieldDoc::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
    return FALSE;

#if 1
	CImportWizard* pDlg = CImportWizard::CreateInstance(AfxGetMainWnd());
	if (!pDlg)
		return FALSE;

	int nResponse = pDlg->DoModal();
	if (nResponse != ID_WIZFINISH)
	{
		delete pDlg;
		return FALSE;
	}

	cx = pDlg->cx;
	cy = pDlg->cy;
	cz = pDlg->cz;

	x1 = pDlg->bbox[0];
	x2 = pDlg->bbox[1];
	y1 = pDlg->bbox[2];
	y2 = pDlg->bbox[3];
	z1 = pDlg->bbox[4];
	z2 = pDlg->bbox[5];

	hdf5_file = pDlg->m_storageRoot;
	pDlg->m_storageRoot = -1;

	hdf5_model = pDlg->m_storageModel;
	pDlg->m_storageModel = -1;

	sz = pDlg->sz;
	nz = pDlg->nz;
	pDlg->nz = 0;
	zz = pDlg->zz;
	pDlg->zz = 0;

	char name[_MAX_PATH];	
	ATL::CDataSource conn;
	HRESULT hr = conn.Open("Microsoft.Jet.OLEDB.4.0", \
		strcat(strcpy(name, pDlg->m_dbaseName), ".mdb"));
	if (FAILED(hr))
	{
		delete pDlg;
		return FALSE;
	}

	hr = session.Open(conn);
	if (FAILED(hr))
	{
		delete pDlg;
		return FALSE;
	}

	if (pDlg->m_nBaseFormation != -1)
	{
		base_formation = pDlg->m_nBaseFormation;
	}
	else
	{
		const char* group = 0;
		if (0 == _tcsicmp(pDlg->m_fieldName, _T("Лянторское")) \
			&& 0 == _tcsicmp(pDlg->m_modelGroup, _T("model/350x650x250")))
		{
			base_formation = 1011001;
		}
		else if (0 == _tcsicmp(pDlg->m_fieldName, _T("Фёдоровское")) \
			&& 0 == _tcsicmp(pDlg->m_modelGroup, _T("model/500x500x200")))
		{
			base_formation = 1013601;
		}
		else
		{
			delete pDlg;
			return FALSE;
		}
	}

  ff = new FieldRef;
  if (!ff->Init(this))
  {
    delete ff;
    ff = 0;
		delete pDlg;
    return FALSE;
  }

  ww = new WellSite;
  if (!ww->Init(this))
  {
    delete ww;
    ww = 0;
		delete pDlg;
    return FALSE;
  }

	if (_tcslen(pDlg->m_updateCoords) > 0)
	{
		int b, x, y;
		char name[32];
		std::ifstream in(pDlg->m_updateCoords);
		std::ofstream out("coord-diff.txt");
		out << this->cx << ' ' << this->cy << std::endl
			<< std::setprecision(7) << this->x1 << ' ' << std::setprecision(7) << this->x2 << std::endl
			<< std::setprecision(6) << this->y1 << ' ' << std::setprecision(6) << this->y2 << std::endl;
		while (in >> b >> name >> x >> y)
		{
			WellSite::WellTraj* t = ww->Traj(b);
			if (!t || x == 0 && y == 0) continue;

			double p[3];
			WellTraj_Coord(t, 1, p);

			double q[3];
			q[0] = x - 0.5;
			q[1] = y - 0.5;
			q[2] = 0.0;

			double pp[3];
			Coord(q, pp);

			double dx = pp[0] - p[0];
			double dy = pp[1] - p[1];
			double dz = pp[2] - p[2];

			if (t->block.empty())
			{
				WellSite::TrajItem* q = t->start.item;
				q->x += dx;
				q->y += dy;
				q->z += dz;

				q = t->final.item;
				q->x += dx;
				q->y += dy;
				q->z += dz;
			}
			else
			{
				WellSite::TrajItem* q = t->start.item;
				q->x += dx;
				q->y += dy;
				q->z += dz;

				leda::list_item it = t->block.first();
				while (it != leda_nil)
				{
					q = t->block[it];
					q->x += dx;
					q->y += dy;
					q->z += dz;
					it = t->block.succ(it);
				}

				q = t->final.item;
				if (q)
				{
					q->x += dx;
					q->y += dy;
					q->z += dz;
				}
			}
		}
		ww->ResetPoints(this);
#if 0
		CFile file;
		CFileException e;
		if (file.Open(TEXT("wellsite.txt"), CFile::modeCreate | CFile::modeWrite, &e))
		{
			char name[32]
			CDumpContext dc(&file);
			dc.SetDepth(CFieldDoc::DUMP_WELLSITE);
			ww->Dump(dc);
		}
#endif
	}

	{
		CFieldApp* pApp = STATIC_DOWNCAST(CFieldApp, AfxGetApp());
		TCHAR text[128];
		_tcscpy(text, pDlg->m_fieldName);
		if (pApp->m_bDemoMode)
		{
			_tcscat(text, TEXT(" DEMO"));
		}
		SetPathName(text);
	}
	delete pDlg;

#else
	
	LPCTSTR szFilter = _T("Hierarchical Data Format (*.h5)|*.h5|")
		_T("Microsoft Jet Database (*.mdb)|*.mdb|All Files (*.*)|*.*||");
	CFileDialog dlg(TRUE, _T(".h5"), NULL, OFN_HIDEREADONLY, szFilter);
	if (IDOK != dlg.DoModal())
		return FALSE;

	TCHAR drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
	_tsplitpath(dlg.GetPathName(), drive, dir, fname, ext);

	const char* group = 0;
	if (0 == _tcsicmp(fname, _T("lyantorskoe")))
	{
		base_formation = 1011001;
		group = "model/350x650x250";
	}
	else if (0 == _tcsicmp(fname, _T("fedorovskoe")))
	{
		base_formation = 1013601;
		group = "model/500x500x200";
	}
	else
	{
		return FALSE;
	}

	TCHAR pathname[_MAX_PATH];
	_tcscpy(ext, _T(".h5"));
	_tmakepath(pathname, drive, dir, fname, ext);
  if (!OpenDatasetFile(pathname, group))
    return FALSE;

	_tcscpy(ext, _T(".mdb"));
	_tmakepath(pathname, drive, dir, fname, ext);
  if (!OpenDatabaseFile(pathname))
    return FALSE;
#endif

  return TRUE;
}

void CFieldDoc::DeleteContents()
{
  delete ff;
  ff = 0;

  delete ww;
  ww = 0;

  delete[] nz;
  nz = 0;

  delete[] zz;
  zz = 0;

	/*
  if (hf >= 0)
  {
    H5Fclose(hf);
    hf = -1;
  }
	*/

	if (hdf5_model >= 0)
	{
		H5Gclose(hdf5_model);
		hdf5_model = -1;
	}
	if (hdf5_file >= 0)
	{
		H5Fclose(hdf5_file);
		hdf5_file = -1;
	}

  x1 = 0, x2 = 1;
  y1 = 0, y2 = 1;
  z1 = 0, z2 = 1;

  cx = cy = cz = 0;
  sz = 0;

  CDocument::DeleteContents();
}

int CFieldDoc::Coord(const double pp[3], double xx[3]) const
{
  int flags = 0;

  if (pp[0] < 0)
  {
    flags |= LEFT_SIDE;
  }

  if (cx-1 < pp[0])
  {
    flags |= RIGHT_SIDE;
  }

  if (pp[1] < 0)
  {
    flags |= BACK_SIDE;
  }

  if (cy-1 < pp[1])
  {
    flags |= FRONT_SIDE;
  }

  if (pp[2] < 0)
  {
    flags |= BOTTOM_SIDE;
  }

  if (cz-1 < pp[2])
  {
    flags |= TOP_SIDE;
  }

  if (flags)
    return flags;

  xx[0] = x1 + (x2 - x1) * pp[0] / (cx - 1);
  xx[1] = y1 + (y2 - y1) * pp[1] / (cy - 1);

  int i = (int) floor(pp[0]);
  int j = (int) floor(pp[1]);
  int k = (int) floor(pp[2]);
  int n = std::lower_bound(nz, nz+sz, k) - nz;

  int dx = 1;
  int dy = cx;
  int dz = dy * cy;

  double u[2];
  u[1] = pp[0] - i;
  u[0] = 1.0 - u[1];
  if (u[1] == 0.0)
    dx = 0;
  
  double v[2];
  v[1] = pp[1] - j;
  v[0] = 1.0 - v[1];
  if (v[1] == 0.0)
    dy = 0;
  
  double w[2];
  if (nz[n] != k)
  {
    --n;
    w[1] = (pp[2] - nz[n]) / (nz[n+1] - nz[n]);
    w[0] = 1.0 - w[1];
  }
  else
  {
    w[1] = 0.0;
    w[0] = 1.0;
    dz = 0;
  }

  n = i + cx * (j + cy * n);
  xx[2] = 0.0;
  for (k = 0; k < 2; ++k)
  {
    for (j = 0; j < 2; ++j)
    {
      for (i = 0; i < 2; ++i)
      {
        xx[2] += zz[n + i * dx + j * dy + k * dz] * u[i] * v[j] * w[k];
      }
    }
  }
  return 0;
}

int CFieldDoc::Cell(const double xx[3], double pp[3]) const
{
  int flags = 0;
  if (xx[0] < x1)
  {
    flags |= LEFT_SIDE;
  }

  if (x2 < xx[0])
  {
    flags |= RIGHT_SIDE;
  }

  if (xx[1] < y1)
  {
    flags |= BACK_SIDE;
  }

  if (y2 < xx[1])
  {
    flags |= FRONT_SIDE;
  }

  if (flags)
    return flags;

  pp[0] = (cx - 1) * (xx[0] - x1) / (x2 - x1);
  pp[1] = (cy - 1) * (xx[1] - y1) / (y2 - y1);
  static const double tol = 1.0e-2;

  int i = int(pp[0] + 0.5);
  if (fabs(pp[0] - i) < tol)
  {
    pp[0] = i;
  }
  else
  {
    i = (int) floor(pp[0]);
  }

  int j = int(pp[1] + 0.5);
  if (fabs(pp[1] - j) < tol)
  {
    pp[1] = j;
  }
  else
  {
    j = (int) floor(pp[1]);
  }

  int dx = 1;
  int dy = cx;
  int dz = cx * cy;

  double u[2];
  u[1] = pp[0] - i;
  u[0] = 1.0 - u[1];
  if (u[1] == 0)
    dx = 0;

  double v[2];
  v[1] = pp[1] - j;
  v[0] = 1.0 - v[1];
  if (v[1] == 0)
    dy = 0;

  double top, bot;
  int d = cx * j + i;
  int lo = 0, hi = sz - 1;
  while (lo <= hi)
  {
    int k = ((lo + hi) >> 1);
    int dd = d + k * dz;
    double z = 0.0;
    for (j = 0; j < 2; ++j)
    {
      for (i = 0; i < 2; ++i)
      {
        z += zz[dd + dx * i + dy * j] * u[i] * v[j];
      }
    }
    if (xx[2] < z-tol)
    {
      hi = k - 1;
      bot = z;
    }
    else if (xx[2] > z+tol)
    {
      lo = k + 1;
      top = z;
    }
    else
    {
      pp[2] = nz[k];
      return 0;
    }
  }

  if (hi < 0)
  {
    flags |= BOTTOM_SIDE;
  }

  if (lo >= sz)
  {
    flags |= TOP_SIDE;
  }

  if (flags)
    return flags;

  double t = (xx[2] - top) / (bot - top);
  _ASSERTE(t > 0.0 && t < 1.0);

  pp[2] = nz[hi] * (1 - t) + nz[hi+1] * t;
  int k = int(pp[2] + 0.5);
  if (fabs(pp[2] - k) < tol)
  {
    pp[2] = k;
  }

  return 0;
}

float CFieldDoc::NodeValue(const double s[3], int id, float (*fn)(float)) const
{
  hssize_t index[3];
  index[2] = int(s[0]+0.5);
  index[1] = int(s[1]+0.5);
  index[0] = int(s[2]+0.5);

  hid_t fs = H5Dget_space(id);
  herr_t err = H5Sselect_elements(fs, H5S_SELECT_SET, 1, (const hsize_t**) &index);

  hsize_t count[1] = { 1 };
  hid_t ms = H5Screate_simple(1, count, NULL);

  float func;
  err = H5Dread(id, H5T_NATIVE_FLOAT, ms, fs, H5P_DEFAULT, &func);

  H5Sclose(ms);
  H5Sclose(fs);

  return fn(func);
}

float CFieldDoc::Value(const double s[3], int id, float (*fn)(float)) const
{
  static const double eps = 1.0e-7;
  double u[2], v[2], w[2];
  int i = (int) s[0], j = (int) s[1], k = (int) s[2];
  int ii, jj, kk;

  u[1] = s[0] - i;
  if (u[1] < eps)
  {
    u[1] = 0.0;
  }
  u[0] = 1 - u[1];
  if (u[0] < eps)
  {
    u[1] = 1.0;
    u[0] = 0.0;
  }

  v[1] = s[1] - j;
  if (v[1] < eps)
  {
    v[1] = 0.0;
  }
  v[0] = 1 - v[1];
  if (v[0] < eps)
  {
    v[1] = 1.0;
    v[0] = 0.0;
  }

  w[1] = s[2] - k;
  if (w[1] < eps)
  {
    w[1] = 0.0;
  }
  w[0] = 1 - w[1];
  if (w[0] < eps)
  {
    w[1] = 1.0;
    w[0] = 0.0;
  }

  hssize_t index[8][3];
  double term[8];
  int n = 0;
  for (kk = 0; kk < 2; ++kk)
  {
    for (jj = 0; jj < 2; ++jj)
    {
      for (ii = 0; ii < 2; ++ii)
      {
        term[n] = u[ii] * v[jj] * w[kk];
        if (term[n] != 0.0)
        {
          index[n][2] = i + ii;
          index[n][1] = j + jj;
          index[n][0] = k + kk;
          ++n;
        }
      }
    }
  }

  hid_t fs = H5Dget_space(id);
  herr_t err = H5Sselect_elements(fs, H5S_SELECT_SET, n, (const hsize_t**) index);

  hsize_t count[1] = { n };
  hid_t ms = H5Screate_simple(1, count, NULL);

  float func[8];
  err = H5Dread(id, H5T_NATIVE_FLOAT, ms, fs, H5P_DEFAULT, func);

  H5Sclose(ms);
  H5Sclose(fs);

  double f = fn(func[0]) * term[0];
  for (i = 1; i < n; ++i)
  {
    f += fn(func[i]) * term[i];
  }
  return float(f);
}

int CFieldDoc::FieldEnum(int nd, const DataField* dd[]) const
{
  return ff->Enum(nd, dd);
}

const CFieldDoc::DataField* CFieldDoc::Field(const char* name) const
{
  return ff->Field(name);
}

int CFieldDoc::WindowQuery(const Point& a, const Point& b, int nb, long* bb) const
{
  return ww->WindowQuery(leda_point(a.x, a.y), leda_point(b.x, b.y), nb, bb);
}

int CFieldDoc::RangeQuery(const Point& c, double r, int nb, long* bb) const
{
  ASSERT(r > 0.0);
  return ww->RangeQuery(leda_circle(leda_point(c.x, c.y), r), nb, bb);
}

int CFieldDoc::Perf(long b, int n, PerfItem* p[]) const
{
  ASSERT(n >= 0);
	WellSite::WellPerf q = (base_formation < 0? ww->Perf(b): ww->Perf2(b, base_formation));
  PerfItem* it;
  forall(it, q)
  {
    if (!n--)
      break;
    *p++ = it;
  }
  return q.size();
}

int CFieldDoc::Traj(long b, int n, double *p) const
{
  WellSite::WellTraj* t = ww->Traj(b);
  if (!t) return 0;
  return WellTraj_Coord(t, n, p);
}

int CFieldDoc::Info(long b, int n, InfoItem* p[]) const
{
  ASSERT(n >= 0);
	WellSite::WellInfo s = (base_formation < 0? ww->Info(b): ww->Info2(b, base_formation));
  InfoItem* it;
  forall(it, s)
  {
    if (!n--)
      break;
    *p++ = it;
  }
  return s.size();
}

int CFieldDoc::Prod(long b, int n, ProdItem* p[]) const
{
  ASSERT(n >= 0);
  WellSite::WellProd s = ww->Prod(b);
  ProdItem* it;
  forall(it, s)
  {
    if (!n--)
      break;
    *p++ = it;
  }
  return s.size();
}

int CFieldDoc::Pump(long b, int n, PumpItem* p[]) const
{
  ASSERT(n >= 0);
  WellSite::WellPump s = ww->Pump(b);
  PumpItem* it;
  forall(it, s)
  {
    if (!n--)
      break;
    *p++ = it;
  }
  return s.size();
}

int CFieldDoc::ProdQuery(COleDateTime dt, int nb, long *bb) const
{
  ASSERT(nb >= 0);
  return ww->ProdQuery(dt, nb, bb);
}

int CFieldDoc::PumpQuery(COleDateTime dt, int nb, long *bb) const
{
  ASSERT(nb >= 0);
  return ww->PumpQuery(dt, nb, bb);
}

#ifdef _DEBUG
void CFieldDoc::AssertValid() const
{
  CDocument::AssertValid();
}

void CFieldDoc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);

  dc << "\ngeometry grid:\n";
  dc << "volume = (" << cx << ", " << cy << ", " << cz << ")\n";
  dc << "x_range = [" << x1 << ", " << x2 << "]\n";
  dc << "y_range = [" << y1 << ", " << y2 << "]\n";
  dc << "z_range = [" << z1 << ", " << z2 << "]\n";

  ww->Dump(dc);
}
#endif

int CFieldDoc::Logtrack(long bore, const char* type, int np, LogtrackItem pp[])
{
  double length = TrajLength(bore);
  if (length == 0.0)
  {
    return 0;
  }

  double binding = ww->Roof(bore);
	if (binding == 0.0)
	{
		Info(bore, 0, 0);
		binding = ww->Roof(bore);
		if (binding == 0.0)
		  return 0;
	}

  char query[0x100];
  sprintf(query, "select * from logtrack where lt_bore=%d and lt_type='%s'", bore, type);
  CCommand < CAccessor < LogtrackItem > > item;
  if (FAILED(item.Open(session, query)))
  {
    return 0;
  }

  int n_item = 0;
  while (item.MoveNext() == S_OK)
  {
    if (binding + length < item.min_depth)
      continue;
    if (binding > item.max_depth)
      continue;
    if (n_item < np)
    {
			double start = std::max(binding, (double) item.min_depth);
			double final = std::min(binding+length, (double) item.max_depth);
			int start_elem = (int) floor((start - item.min_depth) / item.depth_step);
			int final_elem = (int) ceil((final - item.min_depth) / item.depth_step);
			if (start < 0)
				start_elem = 0;
			if (++final_elem > item.num_data)
				final_elem = item.num_data;

			LogtrackItem* track = pp + n_item;
      CopyMemory(track, (LogtrackItem*) &item, sizeof(LogtrackItem));
			track->num_data = final_elem - start_elem;
			track->value = new float [track->num_data];
			track->min_depth = item.min_depth + item.depth_step * start_elem;
			track->max_depth = item.min_depth + item.depth_step * (final_elem - 1);

			bool ok = false;
			sprintf(query, "/logtrack/%s/version_%d/%d", type, track->ver, bore);
			hid_t dataset = H5Dopen(hdf5_model, query);
			if (0 <= dataset)
			{
				hid_t f_space = H5Dget_space(dataset);
				if (0 <= f_space)
				{
					hsize_t first = start_elem;
					hsize_t count = track->num_data;
					herr_t err = H5Sselect_hyperslab(f_space, H5S_SELECT_SET, &first, NULL, &count, NULL);
					if (0 <= err)
					{
						hid_t m_space = H5Screate_simple(1, &count, NULL);
						if (0 <= m_space)
						{
							err = H5Dread(dataset, H5T_NATIVE_FLOAT, m_space, f_space, H5P_DEFAULT, track->value);
							if (0 <= err)
							{
								ok = true;
							}
							H5Sclose(m_space);
						}
					}
					H5Sclose(f_space);
				}
				H5Dclose(dataset);
			}

			if (ok)
			{
				++n_item;
			}
			else
			{
				delete[] track->value;
			}
    }
  }
  return n_item;
}

void FreeLogtrack(int np, CFieldDoc::LogtrackItem pp[])
{
	for (int i = 0; i < np; ++i)
	{
		delete[] pp->value;
	}
}

/////////////// N O N - M E M B E R   F U N C T I O N S /////////////////

long PointQuery(const CFieldDoc* pDoc, double x, double y)
{
  CFieldDoc::Point c;
  c.x = (pDoc->cx - 1) * (x - pDoc->x1) / (pDoc->x2 - pDoc->x1);
  c.y = (pDoc->cy - 1) * (y - pDoc->y1) / (pDoc->y2 - pDoc->y1);

  long b;
  int n = pDoc->RangeQuery(c, 0.1, 1, &b);
  if (!n) return 0L;

  ASSERT(n == 1);
  return b;
}

void CFieldDoc::DateRange(COleDateTime range[2]) const
{
	CTable < CAccessor < HistDates > > tbl;
	tbl.MoveFirst();
	range[0] = tbl.start;
	tbl.MoveLast();
	range[1] = tbl.final;
}

bool LookupWellName(CActField* pDoc, long bore, char* name)
{
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
				CComQIPtr < IRowsetIndex > spRowsetIndex = spRowsetCurrentIndex;
				if (spRowsetIndex != NULL)
				{
					CFindWell::Well data;
					data.bore = bore;
					HACCESSOR hAccessor = tabl.GetHAccessor(0);
					HRESULT hr = spRowsetIndex->Seek(hAccessor, 1, &data, DBSEEK_FIRSTEQ);
					if (S_OK == hr && S_OK == tabl.MoveNext())
					{
						strcpy(name, (const char*) tabl.name);
						return true;
					}
				}
      }
    }
  }
  return false;
}

void CFieldDoc::UpdateWellStock()
{
	delete ww;
	ww = new WellSite;
	VERIFY(ww->Init(this));
	GetFindWell()->PopulateList();
	UpdateAllViews(NULL);
}

