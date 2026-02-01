#define NOMINMAX
#include <windows.h>
#include <atldbcli.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <valarray>
#include <vector>
#include <string>
#include <algorithm>
#include <list>
#include <limits>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <libh5/hdf5.h>

#include "surface_pick.h"

#pragma pack(push, 4)
struct Traj
{
  float dx;
  float dy;
  float dz;
  float md;
};
#pragma pack(pop)

struct Geophysics
{
  long wellbore;
  float startdist;
  float well_elev;
  float well_hx;
  float well_hy;
  Traj tail;
  short traj_ns;
  unsigned char traj_id;
	float well_md;
	char name[32];
	long cluster;
	DBSTATUS md_status;
  BEGIN_COLUMN_MAP(Geophysics)
    COLUMN_ENTRY(1, wellbore)
    COLUMN_ENTRY(2, startdist)
    COLUMN_ENTRY(3, well_elev)
    COLUMN_ENTRY(4, well_hx)
    COLUMN_ENTRY(5, well_hy)
    COLUMN_ENTRY(6, tail.dx)
    COLUMN_ENTRY(7, tail.dy)
    COLUMN_ENTRY(8, tail.dz)
    COLUMN_ENTRY(9, tail.md)
    COLUMN_ENTRY(10, traj_ns)
    COLUMN_ENTRY(11, traj_id)
		COLUMN_ENTRY_STATUS(12, well_md, md_status)
		COLUMN_ENTRY(13, name);
		COLUMN_ENTRY(14, cluster)
  END_COLUMN_MAP()
};

struct Cluster
{
  long number;
  char name[6];
  float xcoord;
  float ycoord;
  BEGIN_COLUMN_MAP(Cluster)
    COLUMN_ENTRY(1, number)
    COLUMN_ENTRY(2, name)
    COLUMN_ENTRY(3, xcoord)
    COLUMN_ENTRY(4, ycoord)
  END_COLUMN_MAP()
};

static bool md_compare(const Traj& t, float md)
{
  return (t.md < md);
}

bool surface_pick(const char* prefix, const char* surf, bool roof, Well_list& acc)
{
  char name[0x1000];
  strcat(strcpy(name, prefix), ".h5");
  hid_t file = H5Fopen(name, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (0 > file)
    return false;

  strcat(strcpy(name, prefix), ".mdb");
  CDataSource conn;
  HRESULT hr = conn.Open("Microsoft.Jet.OLEDB.4.0", name);
  if (FAILED(hr))
  {
    H5Fclose(file);
    return false;
  }
  else
  {
    CSession session;
    hr = session.Open(conn);
    if (FAILED(hr))
    {
      H5Fclose(file);
      return false;
    }
    else
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

    	CComQIPtr < IRowsetIndex > spRowsetIndex;
    	CTable < CAccessor < Cluster > > tabl;
    	HRESULT hr = tabl.Open(session, "cluster", &dbPropSet);
    	if (FAILED(hr))
    	{
        H5Fclose(file);
        return false;
    	}
    	else
    	{
    		CComQIPtr < IRowsetCurrentIndex > spRowsetCurrentIndex = tabl.m_spRowset;
    		if (!spRowsetCurrentIndex)
    		{
          H5Fclose(file);
          return false;
    		}
    		else
    		{
    			DBID IndexID;
    			IndexID.eKind = DBKIND_NAME;
    			IndexID.uName.pwszName = L"c_number";
    			hr = spRowsetCurrentIndex->SetIndex(&IndexID);
    			if (SUCCEEDED(hr))
    			{
    				spRowsetIndex = tabl.m_spRowset;
            CCommand < CAccessor < Geophysics > > cmd;

            sprintf(name, 
              "select "
                "g.wellbore, "
                "g.%s, "
                "w.well_elev, "
                "w.well_hx, "
                "w.well_hy, "
                "t.traj_dx, "
                "t.traj_dy, "
                "t.traj_dz, "
                "t.traj_md, "
                "t.traj_ns, "
                "t.traj_id, "
      					"w.well_md, "
      					"w.well_name, "
      					"w.cluster "
              "from "
                "geophysics g, "
                "dict d, "
                "well w, "
                "traj t "
              "where "
                "g.formation=d.dict_item and "
                "d.dict_group=1000101 and "
                "d.dict_mnemonics_ru='%s' and "
                "g.wellbore=w.well_bore and "
                "w.well_elev is not null and "
                "w.well_hx > 0 and "
                "w.well_hy > 0 and "
                "g.wellbore=t.traj_wb and "
                "t.traj_id=w.traj_id",
                (roof? "startdist": "finaldist"),
                surf);

            hr = cmd.Open(session, name);
            if (FAILED(hr))
            {
              H5Fclose(file);
              return false;
            }
            else
            {
            	HRESULT hr = cmd.MoveNext();
              while (S_OK == hr)
              {
            		int n = cmd.traj_ns + 2;
            		std::vector<Traj> traj(n);
            		Traj* tt = &traj[0];
            		tt->dx = tt->dy = tt->dz = tt->md = 0.0f;
            		traj[cmd.traj_ns + 1] = cmd.tail;
                if (cmd.traj_ns > 0)
                {
            			char name[128];
                  sprintf(name, "/traj/version_%d/%d", (int) cmd.traj_id, (int) cmd.wellbore);
                  hid_t dataset = H5Dopen(file, name);
                  if (0 > dataset)
                  {
                    H5Fclose(file);
            				return false;
            			}
                  herr_t err = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &tt[1]);
            			if (0 > err)
            			{
            				H5Dclose(dataset);
                    H5Fclose(file);
            				return false;
            			}
            			H5Dclose(dataset);
            		}
            		if (cmd.md_status == DBSTATUS_S_OK && cmd.tail.md < cmd.well_md)
            		{
            			traj.push_back(Traj());
            			float t = (cmd.well_md - traj[n-2].md) / (traj[n-1].md - traj[n-2].md);
            			traj[n].md = cmd.well_md;
            			traj[n].dx = traj[n-2].dx + t * (traj[n-1].dx - traj[n-2].dx);
            			traj[n].dy = traj[n-2].dy + t * (traj[n-1].dy - traj[n-2].dy);
            			traj[n++].dz = traj[n-2].dz + t * (traj[n-1].dz - traj[n-2].dz);
            			tt = &traj[0];
            		}

                int k = std::lower_bound(tt, tt+n, cmd.startdist, md_compare) - tt;
            		if (k < n && k > 0)
            		{
            			Sample org;
            			acc.push_back(Well());
            			Well* w = &acc.back();
            			w->bore = cmd.wellbore;
             		  tabl.number = cmd.cluster;
            			org.xcoord = cmd.well_hx;
            			org.ycoord = cmd.well_hy;
            			if (cmd.cluster != 0 
            			  && S_OK == spRowsetIndex->Seek(tabl.GetHAccessor(0), 1, &tabl, DBSEEK_FIRSTEQ)
            			  && S_OK == tabl.MoveNext())
            			{
            			  // org.xcoord = cluster.xcoord;
            			  // org.ycoord = cluster.ycoord;
            			  sprintf(w->name, "%s(%s)", cmd.name, tabl.name);
            			}
            			else
            			{
            				strcpy(w->name, cmd.name);
            			}

          				w->org.sparam = cmd.startdist;
            			if (tt[k].md == cmd.startdist)
            			{
            			  w->org.xcoord = org.xcoord + traj[k].dx;
            				w->org.ycoord = org.ycoord + traj[k].dy;
            			}
            			else
            			{
            				float t = (cmd.startdist - traj[k-1].md) / (traj[k].md - traj[k-1].md);
            				w->org.xcoord = org.xcoord + traj[k-1].dx + t * (traj[k].dx - traj[k-1].dx);
            				w->org.ycoord = org.ycoord + traj[k-1].dy + t * (traj[k].dy - traj[k-1].dy);
            			}
            			
            			if (cmd.traj_ns > 0)
            			{
            			  w->traj.reserve(n-1);
            				for (k = 1; k < n; ++k)
            				{
            				  w->traj.push_back(Sample());
            				  Sample* t = &w->traj.back();
            				  t->xcoord = org.xcoord + traj[k].dx - w->org.xcoord;
            				  t->ycoord = org.ycoord + traj[k].dy - w->org.ycoord;
            				  t->sparam = traj[k].md;
            				}
            			}
            		}
            		hr = cmd.MoveNext();
              } // while(S_OK == MoveNext()) 
            } // SUCCEEDED(command)
    			} // SUCCEEDED(SetIndex())
    		} // SUCCEEDED(QueryInterface())
    	} // SUCCEEDED(table)
    } // SUCCEEDED(session)
  } // SUCCEEDED(connection)
  H5Fclose(file);
  return (!acc.empty());
}

bool profile_pick(const char* prefix, const char* surf, bool roof, \
  int n, const long* bore, const double* xx, const double* yy, Well_list& acc)
{
  char name[0x1000];
  strcat(strcpy(name, prefix), ".h5");
  hid_t file = H5Fopen(name, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (0 > file)
    return false;

  double *dx = new double[n];
  double *dy = new double[n];
  dx[0] = xx[1] - xx[0];
  dy[0] = yy[1] - yy[0];
  for (int i = 1; i < n-1; ++i)
  {
    dx[i] = xx[i+1] - xx[i-1];
    dy[i] = yy[i+1] - yy[i-1];
  }
  dx[n-1] = xx[n-1] - xx[n-2];
  dy[n-1] = yy[n-1] - yy[n-2];
  for (int i = 0; i < n; ++i)
  {
    double dd = hypot(dx[i], dy[i]);
    assert(dd > 0.0);
    dx[i] /= dd;
    dy[i] /= dd;
  }

  strcat(strcpy(name, prefix), ".mdb");
  CDataSource conn;
  HRESULT hr = conn.Open("Microsoft.Jet.OLEDB.4.0", name);
  if (FAILED(hr))
  {
    delete[] dy;
    delete[] dx;
    H5Fclose(file);
    return false;
  }
  else
  {
    CSession session;
    hr = session.Open(conn);
    if (FAILED(hr))
    {
      delete[] dy;
      delete[] dx;
      H5Fclose(file);
      return false;
    }
    else
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

    	CComQIPtr < IRowsetIndex > spRowsetIndex;
    	CTable < CAccessor < Cluster > > tabl;
    	HRESULT hr = tabl.Open(session, "cluster", &dbPropSet);
    	if (FAILED(hr))
    	{
        delete[] dy;
        delete[] dx;
        H5Fclose(file);
        return false;
    	}
    	else
    	{
    		CComQIPtr < IRowsetCurrentIndex > spRowsetCurrentIndex = tabl.m_spRowset;
    		if (!spRowsetCurrentIndex)
    		{
          delete[] dy;
          delete[] dx;
          H5Fclose(file);
          return false;
    		}
    		else
    		{
    			DBID IndexID;
    			IndexID.eKind = DBKIND_NAME;
    			IndexID.uName.pwszName = L"c_number";
    			hr = spRowsetCurrentIndex->SetIndex(&IndexID);
    			if (FAILED(hr))
    			{
            delete[] dy;
            delete[] dx;
            H5Fclose(file);
            return false;
    			}
    			else
    			{
    				spRowsetIndex = tabl.m_spRowset;
						float acc_xcoord = 0.0f;
            for (int i = 0; i < n; ++i)
            {
              CCommand < CAccessor < Geophysics > > cmd;

              sprintf(name, 
                "select "
                  "g.wellbore, "
                  "g.%s, "
                  "w.well_elev, "
                  "w.well_hx, "
                  "w.well_hy, "
                  "t.traj_dx, "
                  "t.traj_dy, "
                  "t.traj_dz, "
                  "t.traj_md, "
                  "t.traj_ns, "
                  "t.traj_id, "
          				"w.well_md, "
          				"w.well_name, "
         					"w.cluster "
                "from "
                  "geophysics g, "
                  "dict d, "
                  "well w, "
                  "traj t "
                "where "
                  "g.formation=d.dict_item and "
                  "d.dict_group=1000101 and "
                  "d.dict_mnemonics_ru='%s' and "
                  "g.wellbore=w.well_bore and "
                  "w.well_bore=%d and "
                  "w.well_elev is not null and "
                  "w.well_hx > 0 and "
                  "w.well_hy > 0 and "
                  "g.wellbore=t.traj_wb and "
                  "t.traj_id=w.traj_id",
                  (roof? "startdist": "finaldist"),
                  surf, bore[i]);

              hr = cmd.Open(session, name);
              if (FAILED(hr))
              {
                delete[] dy;
                delete[] dx;
                H5Fclose(file);
        				return false;
              }
              else
              {
              	HRESULT hr = cmd.MoveNext();
                if (FAILED(hr))
                {
                  delete[] dy;
                  delete[] dx;
                  H5Fclose(file);
          				return false;
                }
                else
                {
              		int n = cmd.traj_ns + 2;
              		std::vector<Traj> traj(n);
              		Traj* tt = &traj[0];
              		tt->dx = tt->dy = tt->dz = tt->md = 0.0f;
              		traj[cmd.traj_ns + 1] = cmd.tail;
                  if (cmd.traj_ns > 0)
                  {
                    sprintf(name, "/traj/version_%d/%d", (int) cmd.traj_id, (int) cmd.wellbore);
                    hid_t dataset = H5Dopen(file, name);
                    if (0 > dataset)
                    {
                      delete[] dy;
                      delete[] dx;
                      H5Fclose(file);
              				return false;
              			}
                    herr_t err = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, &tt[1]);
              			if (0 > err)
              			{
              				H5Dclose(dataset);
                      delete[] dy;
                      delete[] dx;
                      H5Fclose(file);
              				return false;
              			}
              			H5Dclose(dataset);
              		}
               		if (cmd.md_status == DBSTATUS_S_OK && cmd.tail.md < cmd.well_md)
               		{
               			traj.push_back(Traj());
               			float t = (cmd.well_md - traj[n-2].md) / (traj[n-1].md - traj[n-2].md);
               			traj[n].md = cmd.well_md;
               			traj[n].dx = traj[n-2].dx + t * (traj[n-1].dx - traj[n-2].dx);
               			traj[n].dy = traj[n-2].dy + t * (traj[n-1].dy - traj[n-2].dy);
               			traj[n++].dz = traj[n-2].dz + t * (traj[n-1].dz - traj[n-2].dz);
               			tt = &traj[0];
               		}

                  int k = std::lower_bound(tt, tt+n, cmd.startdist, md_compare) - tt;
              		if (k < n && k > 0)
              		{
              			Sample org;
              			acc.push_back(Well());
              			Well* w = &acc.back();
              			w->bore = cmd.wellbore;
               		  tabl.number = cmd.cluster;
              			if (cmd.cluster != 0 
              			  && S_OK == spRowsetIndex->Seek(tabl.GetHAccessor(0), 1, &tabl, DBSEEK_FIRSTEQ)
              			  && S_OK == tabl.MoveNext())
              			{
              			  sprintf(w->name, "%s(%s)", cmd.name, tabl.name);
              			}
              			else
              			{
              				strcpy(w->name, cmd.name);
              			}

              			w->org.xcoord = acc_xcoord;
              			if (i > 0)
              			{
              			  w->org.xcoord += (float) hypot(xx[i] - xx[i-1], yy[i] - yy[i-1]);
              			}
										acc_xcoord = w->org.xcoord;

            				w->org.sparam = cmd.startdist;
              			if (tt[k].md == cmd.startdist)
              			{
              			  org.xcoord = traj[k].dx;
              			  org.ycoord = traj[k].dy;
              			  w->org.ycoord = traj[k].dz - cmd.well_elev;
              			}
              			else
              			{
              				float t = (cmd.startdist - traj[k-1].md) / (traj[k].md - traj[k-1].md);
              				org.xcoord = traj[k-1].dx + t * (traj[k].dx - traj[k-1].dx);
              				org.ycoord = traj[k-1].dy + t * (traj[k].dy - traj[k-1].dy);
              				w->org.ycoord = traj[k-1].dz \
              				  + t * (traj[k].dz - traj[k-1].dz) - cmd.well_elev;
              			}

              			// if (cmd.traj_ns > 0)
              			{
              			  w->traj.reserve(n);
              				for (k = 0; k < n; ++k)
              				{
              				  w->traj.push_back(Sample());
              				  Sample* t = &w->traj.back();
              				  t->xcoord = float((traj[k].dx - org.xcoord) * dx[i] 
              				    + (traj[k].dy - org.ycoord) * dy[i]);
              				  t->ycoord = traj[k].dz - cmd.well_elev - w->org.ycoord;
              				  t->sparam = traj[k].md;
              				}
              			}
              		}

                } // SUCCEEDED(record)
              } // SUCCEEDED(command)
            } // for (int i = 0; i < n; ++i)
    			} // SUCCEEDED(SetIndex())
    		} // SUCCEEDED(QueryInterface(IRowsetCurrentIndex))
    	} // SUCCEEDED(cluster)
    } // SUCCEEDED(session)
  } // SUCCEEDED(connection)
  delete[] dy;
  delete[] dx;
  H5Fclose(file);
  return (!acc.empty());
}
