// CrossSection.cpp : implementation file
//

#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <strstream>
#include <valarray>
#include <assert.h>
#include "field.h"
#include "CrossSection.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "FieldForm.h"
#include "PlotDialog.h"
#include "CumulativeS.h"
#include "FindWell.h"
#include "stuff\ps_font.h"
#include "WellMatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

typedef std::valarray<float> fltarray;

static void tridiv(psstream& out, int n, const color_type* s, const coord_type* p1, \
	const coord_type* p2, const coord_type* p3)
{
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
				out.setrgbcolor(s[k].r, s[k].g, s[k].b);
  			out.moveto(x1, y1);
  			out.lineto(x2, y2);
  			out.lineto(x3, y3);
  			out.lineto(x1, y1);
  			out.fill();
				++k;
			}

			if (i + j <= n)
			{
  			double x4 = p1->xcoord + j * (p2->xcoord - p1->xcoord) / n \
  				+ i * (p3->xcoord - p1->xcoord) / n;
  			double y4 = p1->ycoord + j * (p2->ycoord - p1->ycoord) / n \
  				+ i * (p3->ycoord - p1->ycoord) / n;

				out.setrgbcolor(s[k].r, s[k].g, s[k].b);
  			out.moveto(x3, y3);
  			out.lineto(x2, y2);
  			out.lineto(x4, y4);
  			out.lineto(x3, y3);
  			out.fill();
				++k;
			}
		}
	}
	out.setgray(0.0);
	out.setlinewidth(1.0);
	out.moveto(p1->xcoord, p1->ycoord);
	out.lineto(p2->xcoord, p2->ycoord);
	out.lineto(p3->xcoord, p3->ycoord);
	out.lineto(p1->xcoord, p1->ycoord);
	out.closepath();
	out.stroke();
}

/////////////////////////////////////////////////////////////////////////////
// CCrossSection dialog

CCrossSection::CCrossSection(): CDialog(CCrossSection::IDD, NULL)
{
  data_mapping = LOGARITHMIC;
  x_major_tick = 500;
  x_minor_tick = 50;
  y_major_tick = 10;
  y_minor_tick = 1;
  show_grid = 1;
  show_legend = 1;
  left_margin = 100;
  right_margin = 100;
  top_margin = 80;
  bottom_margin = 80;
  num_pages = 1;
  strcpy(font_family, FONT_05);
  font_size = 10;
  format = FORMAT_A4;
  orientation = LANDSCAPE;
  InitPageSizes();
  m_bVisible = FALSE;
  m_bRun = FALSE;
	page_numbering = true;
	show_formation = false;
	//{{AFX_DATA_INIT(CCrossSection)
	//}}AFX_DATA_INIT
}

void CCrossSection::InitPageSizes()
{
  switch (format)
  {
  case FORMAT_A4:
    page_width = 595;
    page_height = 842;
    break;

  case FORMAT_A3:
    page_width = 842;
    page_height = 1191;
    break;

  case FORMAT_A0:
    page_width = 2384;
    page_height = 3370;
    break;
  }

  if (orientation == LANDSCAPE)
  {
    int tmp = page_width;
    page_width = page_height;
    page_height = tmp;
  }
}

void CCrossSection::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCrossSection)
	// DDX_Control(pDX, IDC_EDIT1, m_ctrlApp);
	// DDX_Control(pDX, IDC_EDIT2, m_ctrlPlot);
	DDX_Control(pDX, IDC_LIST1, m_ctrlPoints);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCrossSection, CDialog)
	//{{AFX_MSG_MAP(CCrossSection)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(ID_PLOT_CROSS_SECTION, OnPlotCrossSection)
	ON_WM_SHOWWINDOW()
	ON_WM_CLOSE()
	ON_COMMAND(ID_PLOT_PROPERTIES, OnPlotProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCrossSection message handlers

BOOL CCrossSection::OnInitDialog() 
{
	CDialog::OnInitDialog();

  // m_ctrlPoints.SetExtendedStyle(LVS_EX_TRACKSELECT|LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
  m_ctrlPoints.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
  m_ctrlPoints.InsertColumn(0, "well number");
  m_ctrlPoints.InsertColumn(1, "x coordinate");
  m_ctrlPoints.InsertColumn(2, "y coordinate");

  m_ctrlPoints.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
  int ww = m_ctrlPoints.GetColumnWidth(0);

  CRect rc;
  m_ctrlPoints.GetClientRect(&rc);
  ww = rc.Width() - ww;

  int w1 = ww / 2, w2 = ww - w1;
  m_ctrlPoints.SetColumnWidth(1, w1);
  m_ctrlPoints.SetColumnWidth(2, w2);

  HINSTANCE hh = GetModuleHandle(NULL);

  CButton* pWnd = (CButton*) GetDlgItem(IDC_BUTTON1);
  pWnd->SetBitmap(::LoadBitmap(hh, MAKEINTRESOURCE(IDB_EDIT_DEL)));

  pWnd = (CButton*) GetDlgItem(IDC_BUTTON2);
  pWnd->SetBitmap(::LoadBitmap(hh, MAKEINTRESOURCE(IDB_EDIT_DN)));

  pWnd = (CButton*) GetDlgItem(IDC_BUTTON3);
  pWnd->SetBitmap(::LoadBitmap(hh, MAKEINTRESOURCE(IDB_EDIT_UP)));

	m_ctrlPlot.Initialize(IDC_EDIT2, this);
	m_ctrlApp.Initialize(IDC_EDIT1, this);
	m_ctrlApp.EnableWindow(FALSE);

  m_ctrlPlot.SetWindowText("plot1.ps");
  // m_ctrlApp.SetBkColor(GetSysColor(COLOR_3DFACE));

  m_ctrlApp.SetWindowText(AfxGetApp()->GetProfileString("Run", "App"));

  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCrossSection::OnDestroy() 
{
	CDialog::OnDestroy();

  int n = m_ctrlPoints.GetItemCount();
  for (int i = 0; i < n; ++i)
  {
    coord_type* p = (coord_type*) m_ctrlPoints.GetItemData(i);
    delete p;
  }
  m_ctrlPoints.DeleteAllItems();
}

void CCrossSection::OnCheck1() 
{
  m_bRun ^= 1;
  if (m_bRun)
  {
    m_ctrlApp.EnableWindow(TRUE);
    // m_ctrlApp.SetBkColor(GetSysColor(COLOR_WINDOW));
  }
  else
  {
    m_ctrlApp.EnableWindow(FALSE);
    // m_ctrlApp.SetBkColor(GetSysColor(COLOR_3DFACE));
  }
}

void CCrossSection::OnButton1() 
{
  int n = m_ctrlPoints.GetNextItem(-1, LVNI_SELECTED);
  if (n != -1)
  {
    m_ctrlPoints.SetRedraw(FALSE);
    do
    {
      delete (coord_type*) m_ctrlPoints.GetItemData(n);
      m_ctrlPoints.DeleteItem(n);
      GetFieldView()->GetDocument()->UpdateAllViews(0);
      n = m_ctrlPoints.GetNextItem(-1, LVNI_SELECTED);
    }
    while (n != -1);
    m_ctrlPoints.SetRedraw(TRUE);
    GetFieldView()->GetDocument()->UpdateAllViews(0);
  }

  /*
  if (n != -1)
  {
    delete (coord*) m_ctrlPoints.GetItemData(n);
    m_ctrlPoints.DeleteItem(n);
    GetFieldView()->GetDocument()->UpdateAllViews(0);
  }
  */
}

void CCrossSection::OnButton2() 
{
  int n = m_ctrlPoints.GetNextItem(-1, LVNI_SELECTED);
  if (n != -1 && n < m_ctrlPoints.GetItemCount()-1)
  {
    coord_type* p = (coord_type*) m_ctrlPoints.GetItemData(n);

    char f1[16], f2[16], f3[16];
    m_ctrlPoints.GetItemText(n, 0, f1, 16);
    m_ctrlPoints.GetItemText(n, 1, f2, 16);
    m_ctrlPoints.GetItemText(n, 2, f3, 16);

    m_ctrlPoints.DeleteItem(n);

    m_ctrlPoints.InsertItem(n+1, f1);
    m_ctrlPoints.SetItemText(n+1, 1, f2);
    m_ctrlPoints.SetItemText(n+1, 2, f3);
    m_ctrlPoints.SetItemData(n+1, (DWORD) p);

    m_ctrlPoints.SetItemState(n+1, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
    GetFieldView()->GetDocument()->UpdateAllViews(0);
  }
}

void CCrossSection::OnButton3() 
{
  int n = m_ctrlPoints.GetNextItem(-1, LVNI_SELECTED);
  if (n != -1 && n > 0)
  {
    coord_type* p = (coord_type*) m_ctrlPoints.GetItemData(n);

    char f1[16], f2[16], f3[16];
    m_ctrlPoints.GetItemText(n, 0, f1, 16);
    m_ctrlPoints.GetItemText(n, 1, f2, 16);
    m_ctrlPoints.GetItemText(n, 2, f3, 16);

    m_ctrlPoints.DeleteItem(n);

    m_ctrlPoints.InsertItem(n-1, f1);
    m_ctrlPoints.SetItemText(n-1, 1, f2);
    m_ctrlPoints.SetItemText(n-1, 2, f3);
    m_ctrlPoints.SetItemData(n-1, (DWORD) p);

    m_ctrlPoints.SetItemState(n-1, LVIS_FOCUSED|LVIS_SELECTED, LVIS_FOCUSED|LVIS_SELECTED);
    GetFieldView()->GetDocument()->UpdateAllViews(0);
  }
}

void CCrossSection::OnFileOpen() 
{
  CFileDialog dlg(TRUE);
  if (dlg.DoModal() == IDOK)
  {
    CWaitCursor wait;

    int n = m_ctrlPoints.GetItemCount();
    for (int i = 0; i < n; ++i)
      delete (coord_type*) m_ctrlPoints.GetItemData(i);
    m_ctrlPoints.DeleteAllItems();

    CFieldView* pFV = GetFieldView();
    CActField* pDoc = pFV->GetDocument();
    std::ifstream f(dlg.GetPathName());
    n = 0;
    double x, y;
    while (f >> x >> y)
    {
      if (pDoc->x1 <= x && x <= pDoc->x2 && pDoc->y1 <= y && y <= pDoc->y2)
      {
        coord_type* p = new coord_type;
        p->xcoord = (pDoc->cx-1) * (x - pDoc->x1) / (pDoc->x2 - pDoc->x1);
        p->ycoord = (pDoc->cy-1) * (y - pDoc->y1) / (pDoc->y2 - pDoc->y1);

        pFV->AppendCtrlPoint(p);
        pFV->UpdateCtrlPoint(n++);
      }
    }
    GetFieldView()->GetDocument()->UpdateAllViews(0);
  }
}

void CCrossSection::OnFileSave() 
{
  CWaitCursor wait;
  CFileDialog dlg(FALSE);
  if (dlg.DoModal() == IDOK)
  {
    std::ofstream f(dlg.GetPathName());
    f.precision(10);

    CActField* pDoc = GetFieldView()->GetDocument();
    int n = m_ctrlPoints.GetItemCount();
    for (int i = 0; i < n; ++i)
    {
      coord_type* p = (coord_type*) m_ctrlPoints.GetItemData(i);
      f << pDoc->x1 + (pDoc->x2 - pDoc->x1) * p->xcoord / (pDoc->cx-1) << ' '
        << pDoc->y1 + (pDoc->y2 - pDoc->y1) * p->ycoord / (pDoc->cy-1) << '\n';
    }
  }
}

void CCrossSection::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
  int n = m_ctrlPoints.GetNextItem(-1, LVNI_SELECTED);
  if (n != -1)
  {
    coord_type* p = (coord_type*) m_ctrlPoints.GetItemData(n);
    CFieldView* pFV = GetFieldView();
    CSize sizeTot = pFV->GetTotalSize();
    CActField* pDoc = pFV->GetDocument();

    int x = int(sizeTot.cx * p->xcoord / (pDoc->cx-1) + 0.5);
    int y = int(sizeTot.cy - sizeTot.cy * p->ycoord / (pDoc->cy-1) + 0.5);

    CRect rc;
    pFV->GetClientRect(&rc);
    CClientDC dc(pFV);
    pFV->OnPrepareDC(&dc);
    dc.DPtoLP(&rc);
    CPoint pt = rc.CenterPoint();
    CSize ss = pt - CPoint(x, -y);
    dc.LPtoDP(&ss);
    ss.cx = -ss.cx;
    pFV->OnScrollBy(ss, TRUE);
  }
	
	*pResult = 0;
}

float Value3(hid_t id, int t, const double s[3], float (*fn)(float))
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

float Value4(hid_t id, int t, const double s[3], float (*fn)(float))
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

  hssize_t index[8][4];
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
          index[n][3] = i + ii;
          index[n][2] = j + jj;
          index[n][1] = k + kk;
          index[n][0] = t;
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

typedef float (ValueFunc)(hid_t id, int t, const double s[3], float (*fn)(float));
static int SelectFunc(short year, hid_t id, ValueFunc** func)
{
  int nt = 0;
  hsize_t count[4];
  hid_t fs = H5Dget_space(id);
  int nd = H5Sget_simple_extent_dims(fs, count, NULL);
  if (nd == 3)
  {
    *func = &Value3;
  }
  else if (nd == 4)
  {
    nt = (int) count[0];
    hid_t t_attr = H5Aopen_name(id, "time");
    short* tt = new short[nt];
    herr_t err = H5Aread(t_attr, H5T_NATIVE_SHORT, tt);
    err = H5Aclose(t_attr);
    
    if (year < tt[0])
    {
      nt = 0;
    }
    else if (year > tt[nt-1])
    {
      --nt;
    }
    else
    {
      nt = std::lower_bound(tt, tt+nt, year) - tt;
    }
    delete[] tt;
    *func = &Value4;
  }
  else
  {
    *func = 0;
  }
  herr_t er = H5Sclose(fs);
  return nt;
}

void make_cross_grid(CActField* pDoc, const vertex_type& sect, \
  fltarray& dist, fltarray& elev, fltarray& data)
{
  int cx = sect.size(), cy = pDoc->cz, nn = cx*cy;

  data.resize(nn);
  elev.resize(nn);
  dist.resize(cx);

  ValueFunc* Value = 0;
  int nt = SelectFunc(pDoc->year, pDoc->field->id, &Value);

  {
    double x0, y0, dd, pp[3];
    pp[2] = 0;
    dd = 0.0;
    for (int i = 0; i < cx; ++i)
    {
      pp[0] = sect[i].xcoord;
      pp[1] = sect[i].ycoord;

      double xx[3];
      VERIFY(INSIDE_DOM(pDoc->Coord(pp, xx)));
      if (i == 0)
      {
        x0 = xx[0];
        y0 = xx[1];
      }

      data[i] = Value(pDoc->field->id, nt, pp, pDoc->clut.fwd);
      elev[i] = (float) xx[2];
      dd += hypot(x0-xx[0], y0-xx[1]);
      dist[i] = (float) dd;

      x0 = xx[0];
      y0 = xx[1];
    }
  }

  for (int j = 1; j < cy; ++j)
  {
    double pp[3];
    pp[2] = j;
    for (int i = 0; i < cx; ++i)
    {
      pp[0] = sect[i].xcoord;
      pp[1] = sect[i].ycoord;

      double xx[3];
      VERIFY(INSIDE_DOM(pDoc->Coord(pp, xx)));

      data[j*cx+i] = Value(pDoc->field->id, nt, pp, pDoc->clut.fwd);
      elev[j*cx+i] = (float) xx[2];
    }
  }
}

void make_cross_formation(CActField* pDoc, const vertex_type& sect, \
	const float* form, float* elev)
{
	int cx = sect.size(), cy = 2, nn = cx*cy;

	double x0, y0, dd, pp[3];
	pp[2] = 0;
	dd = 0;
	for (int i = 0; i < cx; ++i)
	{
		pp[0] = sect[i].xcoord;
		pp[1] = sect[i].ycoord;

		double xx[3];
		VERIFY(INSIDE_DOM(pDoc->Coord(pp, xx)));
		if (i == 0)
		{
			x0 = xx[0];
			y0 = xx[1];
		}

		int u1 = (int) floor(pp[0]);
		int v1 = (int) floor(pp[1]);
		int u2 = (int) ceil(pp[0]);
		int v2 = (int) ceil(pp[1]);
		double s1 = pp[0] - u1;
		double t1 = pp[1] - v1;
		double s2 = 1.0 - s1;
		double t2 = 1.0 - t1;
		int nu = pDoc->cx;
		int nuv = nu * pDoc->cy;
		double top = form[nu*v1+u1]*s2*t2 + form[nu*v1+u2]*s1*t2 \
			+ form[nu*v2+u2]*s1*t1 + form[nu*v2+u1]*s2*t1;
		double thk = form[nu*v1+u1+nuv]*s2*t2 + form[nu*v1+u2+nuv]*s1*t2 \
			+ form[nu*v2+u2+nuv]*s1*t1 + form[nu*v2+u1+nuv]*s2*t1;
		elev[i] = (float) (xx[2] + top);
		elev[cx+i] = (float) (elev[i] + (thk > 0.0? thk: 0.0));
		dd += hypot(x0-xx[0], y0-xx[1]);
		elev[2*cx+i] = (float) dd;
		x0 = xx[0];
		y0 = xx[1];
	}
}

static float fwd(float x)
{
  return x;
}

// 2003.12.01 -- parameter that denotes collector/noncollector changed ftom a_ps to perm 

void make_combined_cross_grid(CActField* pDoc, const vertex_type& sect, \
  fltarray& dist, fltarray& elev, fltarray& sgas, fltarray& soil, fltarray& coef)
{
  CCumulativeS* pSat = GetCumulativeS();
  int cx = sect.size(), cy = pDoc->cz, nn = cx*cy;
  const CFieldDoc::DataField* pCoef = pDoc->Field("perm");

  coef.resize(nn);
  soil.resize(nn);
  sgas.resize(nn);
  elev.resize(nn);
  dist.resize(cx);

  {
    double x0, y0, dd, pp[3];
    pp[2] = 0;
    dd = 0.0;
    for (int i = 0; i < cx; ++i)
    {
      pp[0] = sect[i].xcoord;
      pp[1] = sect[i].ycoord;

      double xx[3];
      VERIFY(INSIDE_DOM(pDoc->Coord(pp, xx)));
      if (i == 0)
      {
        x0 = xx[0];
        y0 = xx[1];
      }

      coef[i] = pDoc->NodeValue(pp, pCoef->id, fwd);
      soil[i] = pDoc->NodeValue(pp, pSat->id_o, fwd) * 0.01f;
      sgas[i] = pDoc->NodeValue(pp, pSat->id_g, fwd) * 0.01f;
      elev[i] = (float) xx[2];
      dd += hypot(x0-xx[0], y0-xx[1]);
      dist[i] = (float) dd;

      x0 = xx[0];
      y0 = xx[1];
    }
  }

  for (int j = 1; j < cy; ++j)
  {
    double pp[3];
    pp[2] = j;
    for (int i = 0; i < cx; ++i)
    {
      pp[0] = sect[i].xcoord;
      pp[1] = sect[i].ycoord;

      double xx[3];
      VERIFY(INSIDE_DOM(pDoc->Coord(pp, xx)));

      coef[j*cx+i] = pDoc->NodeValue(pp, pCoef->id, fwd);
      soil[j*cx+i] = pDoc->NodeValue(pp, pSat->id_o, fwd) * 0.01f;
      sgas[j*cx+i] = pDoc->NodeValue(pp, pSat->id_g, fwd) * 0.01f;
      elev[j*cx+i] = (float) xx[2];
    }
  }
}

/*
void compute_depth_range(const f_array& ff, const well_grid::node_list& ss, double yy[2])
{
  yy[0] = std::numeric_limits<double>::max();
  yy[1] = -yy[0];
  int n_ff = ff.size();
  for (int i = 0; i < n_ff; ++i)
  {
    if (yy[0] > ff[i]) yy[0] = ff[i];
    if (yy[1] < ff[i]) yy[1] = ff[i];
  }
  for (well_grid::node_list::const_iterator s = ss.begin(); s != ss.end(); ++s)
  {
    if (s->bore && !s->bore->traj.empty())
    {
      double z = s->bore->traj.back().z;
      if (yy[1] < z) yy[1] = z;
    }
  }

  yy[0] = floor(yy[0]);
  yy[1] = ceil(yy[1]);
}
*/

void print_prolog(psstream& out, CCrossSection* sect)
{
	psstream::Format format = psstream::FORMAT_NUM;
	psstream::Orientation orientation = psstream::PORTRAIT;
	switch (sect->format)
	{
	case CCrossSection::FORMAT_A4:
		format = psstream::FORMAT_A4;
		break;
	case CCrossSection::FORMAT_A3:
		format = psstream::FORMAT_A3;
		break;
	case CCrossSection::FORMAT_A0:
		format = psstream::FORMAT_A0;
		break;
	};
	if (sect->orientation == CCrossSection::LANDSCAPE)
		orientation = psstream::LANDSCAPE;
	out.selectmedia(format, orientation);
}

class profile_cell_to_point
{
  int cx, cy;
  const fltarray& dist;
  const fltarray& elev;
public:
  profile_cell_to_point(const fltarray& dist, const fltarray& elev);
  coord_type operator() (const coord_type& p) const;
};

profile_cell_to_point::profile_cell_to_point(const fltarray& d, const fltarray& e): 
  dist(d), elev(e)
{
  cx = dist.size();
  cy = elev.size() / cx;
}

coord_type profile_cell_to_point::operator() (const coord_type& p) const
{
  int i1 = (int) floor(p.xcoord), i2 = (int) ceil(p.xcoord);
  int j1 = (int) floor(p.ycoord), j2 = (int) ceil(p.ycoord);
  double sx = p.xcoord-i1, sy = p.ycoord-j1;

  coord_type pp;
  pp.xcoord = dist[i1]*(1-sx)+dist[i2]*sx;
  pp.ycoord = elev[j1*cx+i1]*(1-sx)*(1-sy)
       + elev[j2*cx+i1]*(1-sx)*sy
       + elev[j2*cx+i2]*sx*sy
       + elev[j1*cx+i2]*sx*(1-sy);

 return pp;
}

void update_profile_border(int n_pgn, vertex_type& ver, edge_type* pgn, double bottom)
{
	for (int i = 0; i < n_pgn; ++i)
	{
		if (pgn[i].empty())
			continue;

		for (edge_type::iterator e = pgn[i].begin(); e != pgn[i].end(); ++e)
		{
			curve_type& cc = *e;
			assert(cc.front() == cc.back());
			curve_type::iterator b = cc.begin(), c = b;
			while (++c != cc.end())
			{
				double ycoord;
				bool update = false;
				coord_type *p1 = &ver[*b], *p2 = &ver[*c];
				if (p1->ycoord == 0.0 && p2->ycoord == 0)
				{
					ycoord = 0.0;
					update = true;
				}
				else if (p1->ycoord == bottom && p2->ycoord == bottom)
				{
					ycoord = bottom;
					update = true;
				}
				if (update)
				{
					if (p1->xcoord < p2->xcoord)
					{
						double x1 = ceil(p1->xcoord);
						double x2 = floor(p2->xcoord);
						int n1 = (x1 != p1->xcoord? int(x1): int(x1)+1);
						int n2 = (x2 != p2->xcoord? int(x2): int(x2)-1);
						for (int xcoord = n1; xcoord <= n2; ++xcoord)
						{
							coord_type p;
							p.xcoord = xcoord;
							p.ycoord = ycoord;
							int n = ver.size();
							ver.push_back(p);
							cc.insert(c, n);
						}
					}
					else
					{
						double x1 = floor(p1->xcoord);
						double x2 = ceil(p2->xcoord);
						int n1 = (x1 != p1->xcoord? int(x1): int(x1)-1);
						int n2 = (x2 != p2->xcoord? int(x2): int(x2)+1);
						for (int xcoord = n1; xcoord >= n2; --xcoord)
						{
							coord_type p;
							p.xcoord = xcoord;
							p.ycoord = ycoord;
							int n = ver.size();
							ver.push_back(p);
							cc.insert(c, n);
						}
					}
				}
				b = c;
			}
		}
	}
}

void print_legend(psstream& out, CActField* pDoc, CCrossSection* pSec)
{
  // good idea: plot a continuous legend with colorimage operator
  // sample: c:/pkg/math/geology/gslib/2.0/locmap/locmap.ps
  CCumulativeS* pSat = GetCumulativeS();
  if (!pSat->IsWindowVisible())
  {
    double x0 = pSec->page_width - pSec->right_margin + 2, y0 = pSec->bottom_margin + 2;
    double dx = pSec->font_size * GOLDEN_RATIO, \
      dy = (pSec->page_height - pSec->top_margin - pSec->bottom_margin)  / GOLDEN_RATIO;
    dy = ceil(dy / pSec->font_size) * pSec->font_size;

		out.setgray(0);
		out.setlinewidth(1);

    int n = (int) dy / pSec->font_size;
    for (int i = 1; i <= n; ++i)
    {
      double t = (double) (pDoc->clut.nz+1) * (i-0.5) / n;
      int s1 = (int) floor(t), s2 = (int) ceil(t);
      t -= s1;

      double r = pDoc->clut.sh[s1].r * (1.0 - t) + pDoc->clut.sh[s2].r * t;
      double g = pDoc->clut.sh[s1].g * (1.0 - t) + pDoc->clut.sh[s2].g * t;
      double b = pDoc->clut.sh[s1].b * (1.0 - t) + pDoc->clut.sh[s2].b * t;
      
      double y = y0 + dy * (i-1) / n;
			out.setrgbcolor(r, g, b);
			out.rectfill(x0, y, dx, dy / n);
			out.setgray(0);
			out.moveto(x0+dx, y);
			out.rlineto(3, 0);
			out.stroke();

      float f1 = pDoc->clut.fwd(pDoc->field->valid_range[0]);
      float f2 = pDoc->clut.fwd(pDoc->field->valid_range[1]);
      double z = f1 + (f2 - f1) * (i-1) / n;
      if (pSec->data_mapping == CCrossSection::LOGARITHMIC)
        z = pDoc->clut.bck((float) z);

      CString str;
      if (z < 10.0f)
        str.Format("%4.4f", z);
      else if (z < 100.0f)
        str.Format("%4.3f", z);
      else if (z < 1000.0f)
        str.Format("%4.2f", z);
      else if (z < 10000.0f)
        str.Format("%4.1f", z);
      else
        str.Format("%4.0f", z);

			double bb[4];
			label(out, x0+dx, y, pSec->font_size*0.5, psstream::LABEL_E, true, str, bb);
    }

    double z = pDoc->clut.fwd(pDoc->field->valid_range[1]);
    if (pSec->data_mapping == CCrossSection::LOGARITHMIC)
      z = pDoc->clut.bck((float) z);

    CString str;
    if (z < 10.0f)
      str.Format("%4.4f", z);
    else if (z < 100.0f)
      str.Format("%4.3f", z);
    else if (z < 1000.0f)
      str.Format("%4.2f", z);
    else if (z < 10000.0f)
      str.Format("%4.1f", z);
    else
      str.Format("%4.0f", z);

		double bb[4];
		out.setgray(0);
		out.moveto(x0+dx, y0+dy);
		out.rlineto(3, 0);
		out.stroke();
		label(out, x0+dx, y0+dy, pSec->font_size*0.5, psstream::LABEL_E, true, str, bb);
		out.rectstroke(x0, y0, dx, dy);
  }
	else
	{
    double x0 = pSec->page_width - pSec->right_margin + 2, y0 = pSec->bottom_margin + 2;
		double dy = 10.0;
    double dx = dy * GOLDEN_RATIO;

		COLORREF cr = pSat->m_minColor.GetColor();
		color_type color[5];
		color[0].r = GetRValue(cr) / 255.0;
		color[0].g = GetGValue(cr) / 255.0;
		color[0].b = GetBValue(cr) / 255.0;
		cr = pSat->m_maxColor.GetColor();
		color[1].r = GetRValue(cr) / 255.0;
		color[1].g = GetGValue(cr) / 255.0;
		color[1].b = GetBValue(cr) / 255.0;
		color[2] = color[0];
		color[3] = color[1];
		color[4].r = color[4].g = color[4].b = 1.0;
		float level[2];
		level[0] = log10(pSat->m_minCollector);
		level[1] = log10(pSat->m_maxCollector);
		int k_min = (int) floor(level[0]);
		int k_max = (int) ceil(level[1]);
		int cy = 10 * (k_max - k_min) + 1;
		float *func = new float [cy], *f = func;
		for (int k = k_min; k < k_max; ++k)
		{
			for (int kk = 0; kk < 10; ++kk)
			{
				*f++ = k + 0.1f * kk;
			}
		}
		*f = (float) k_max;
		out.colorimage(x0, y0, dx, dy * (k_max - k_min), 2, level, color, 1, cy, func);
		delete[] func;

		out.setgray(0.0);
		out.setlinewidth(1.0);
		out.rectstroke(x0, y0, dx, dy * (k_max - k_min));
		for (int k = k_min; k <= k_max; ++k)
		{
			double bb[4];
			std::ostringstream ss;
			ss << k;
			out.setfont("Helvetica", dy*0.6);
			label(out, x0 + dx, y0 + (k-k_min) * dy, 1.0, psstream::LABEL_E, true, "10", bb);
			out.setfont("Helvetica", dy*0.4);
			label(out, bb[2], bb[3], 0.0, psstream::LABEL_NE, true, ss.str().c_str(), bb);
		}

		double h = dy * (k_max - k_min);
		coord_type p1, p2, p3;
		p3.xcoord = x0 - dx, p3.ycoord = y0;
		p2.xcoord = p3.xcoord - h * tan(M_PI / 6.0), p2.ycoord = p3.ycoord + h;
		p1.xcoord = p2.xcoord - (p3.xcoord - p2.xcoord), p1.ycoord = y0;
		tridiv(out, pSat->m_nSize, pSat->m_pColorData, &p1, &p2, &p3);

		{
			double b1[4], b2[4];
			out.setfont("Helvetica", dy*0.6);
			label(out, p1.xcoord, p1.ycoord, 1.0, psstream::LABEL_SW, false, "S", b1);
			out.setgray(1.0);
			out.rectfill(b1[0], b1[1], b1[2]-b1[0], b1[3]-b1[1]);
			out.setgray(0.0);
			label(out, p1.xcoord, p1.ycoord, 1.0, psstream::LABEL_SW, true, "S", b1);
			out.setfont("Helvetica", dy*0.4);
			label(out, b1[2], b1[1], 0.0, psstream::LABEL_SE, false, "o", b2);
			out.setgray(1.0);
			out.rectfill(b2[0], b2[1], b2[2]-b2[0], b2[3]-b2[1]);
			out.setgray(0.0);
			label(out, b1[2], b1[1], 0.0, psstream::LABEL_SE, true, "o", b2);

			out.setfont("Helvetica", dy*0.6);
			label(out, p2.xcoord, p2.ycoord, 1.0, psstream::LABEL_N, false, "S", b1);
			out.setgray(1.0);
			out.rectfill(b1[0], b1[1], b1[2]-b1[0], b1[3]-b1[1]);
			out.setgray(0.0);
			label(out, p2.xcoord, p2.ycoord, 1.0, psstream::LABEL_N, true, "S", b1);
			out.setfont("Helvetica", dy*0.4);
			label(out, b1[2], b1[1], 0.0, psstream::LABEL_SE, false, "g", b2);
			out.setgray(1.0);
			out.rectfill(b2[0], b2[1], b2[2]-b2[0], b2[3]-b2[1]);
			out.setgray(0.0);
			label(out, b1[2], b1[1], 0.0, psstream::LABEL_SE, true, "g", b2);

			out.setfont("Helvetica", dy*0.6);
			label(out, p3.xcoord, p3.ycoord, 1.0, psstream::LABEL_SE, false, "S", b1);
			out.setgray(1.0);
			out.rectfill(b1[0], b1[1], b1[2]-b1[0], b1[3]-b1[1]);
			out.setgray(0.0);
			label(out, p3.xcoord, p3.ycoord, 1.0, psstream::LABEL_SE, true, "S", b1);
			out.setfont("Helvetica", dy*0.4);
			label(out, b1[2], b1[1], 0.0, psstream::LABEL_SE, false, "w", b2);
			out.setgray(1.0);
			out.rectfill(b2[0], b2[1], b2[2]-b2[0], b2[3]-b2[1]);
			out.setgray(0.0);
			label(out, b1[2], b1[1], 0.0, psstream::LABEL_SE, true, "w", b2);
		}
	}
}

void print_frame_grid(psstream& out, double x_range[2], double y_range[2], \
  CActField* pDoc, CCrossSection* pSec)
{
	out.setgray(0);
	out.setlinewidth(1);
	out.rectstroke(pSec->left_margin - 5, pSec->bottom_margin - 5, \
		pSec->page_width - pSec->left_margin - pSec->right_margin + 10, \
		pSec->page_height - pSec->top_margin - pSec->bottom_margin + 10);
	out.rectstroke(pSec->left_margin - 7, pSec->bottom_margin - 7, \
		pSec->page_width - pSec->left_margin - pSec->right_margin + 14, \
		pSec->page_height - pSec->top_margin - pSec->bottom_margin + 14);
	out.setlinewidth(0.5);

	out.setfont(FONT_05, 10);

	double orig = y_range[1];
	double range = y_range[0] - y_range[1];
	double dy = pSec->page_height - pSec->top_margin - pSec->bottom_margin;
	double x0 = pSec->left_margin - 7;
	double y0 = pSec->bottom_margin;
	out.setlinewidth(0.5);
	out.vertical_scale((int) ceil(y_range[0] / pSec->y_minor_tick) * pSec->y_minor_tick,
		pSec->y_minor_tick, (int) floor(y_range[1] / pSec->y_minor_tick) * pSec->y_minor_tick,
		orig, range, dy, x0, y0, -1.5);
	if (pSec->show_grid)
	{
		out.vertical_scale((int) ceil(y_range[0] / pSec->y_minor_tick) * pSec->y_minor_tick,
			pSec->y_minor_tick, (int) floor(y_range[1] / pSec->y_minor_tick) * pSec->y_minor_tick,
			orig, range, dy, pSec->left_margin-5, y0, 
			pSec->page_width - pSec->right_margin - pSec->left_margin + 10);
	}

	int step = pSec->y_major_tick;
	int start = (int) ceil(y_range[0] / step) * step;
	int final = (int) floor(y_range[1] / step) * step;
	int n = (final - start) / step;
	double dx = -3.0;

	for (int i = 0; i <= n; ++i)
	{
		double bb[4];
		char text[10];
		double tick = start + step * i;
		sprintf(text, "%g", tick);
		label(out, x0+dx, y0+dy*(tick-orig) / range, 1.0, psstream::LABEL_W, true, text, bb);
	}
	out.setlinewidth(1.0);
	out.vertical_scale(start, step, final, orig, range, dy, x0, y0, dx);
	if (pSec->show_grid)
	{
		out.vertical_scale(start, step, final, orig, range, dy, pSec->left_margin-5, y0, 
			pSec->page_width - pSec->right_margin - pSec->left_margin + 10);
	}

	orig = x_range[0];
	range = x_range[1] - x_range[0];
	dx = pSec->page_width - pSec->left_margin - pSec->right_margin;
	x0 = pSec->left_margin;
	y0 = pSec->bottom_margin - 7;
	out.setlinewidth(0.5);
	out.horizontal_scale((int) ceil(x_range[0] / pSec->x_minor_tick) * pSec->x_minor_tick, 
		pSec->x_minor_tick, (int) floor(x_range[1] / pSec->x_minor_tick) * pSec->x_minor_tick,
		orig, range, dx, x0, y0, -1.5);
	if (pSec->show_grid)
	{
		out.horizontal_scale((int) ceil(x_range[0] / pSec->x_minor_tick) * pSec->x_minor_tick, 
			pSec->x_minor_tick, (int) floor(x_range[1] / pSec->x_minor_tick) * pSec->x_minor_tick,
			orig, range, dx, x0, pSec->bottom_margin - 5, 
			pSec->page_height - pSec->top_margin - pSec->bottom_margin + 10);
	}

	step = pSec->x_major_tick;
	start = (int) ceil(x_range[0] / step) * step;
	final = (int) floor(x_range[1] / step) * step;
	n = (final - start) / step;
	dy = -3.0;

	for (int i = 0; i <= n; ++i)
	{
		double bb[4];
		char text[10];
		double tick = start + step * i;
		sprintf(text, "%g", tick);
		label(out, x0+dx*(tick-orig)/range, y0+dy, 1.0, psstream::LABEL_S, true, text, bb);
	}
	out.setlinewidth(1.0);
	out.horizontal_scale(start, step, final, orig, range, dx, x0, y0, dy);
	if (pSec->show_grid)
	{
		out.horizontal_scale(start, step, final, orig, range, dx, x0, pSec->bottom_margin - 5,
			pSec->page_height - pSec->top_margin - pSec->bottom_margin + 10);
	}
}

void print_well_bores(psstream& out, double x_range[2], double y_range[2], int first, int last, \
  float *dist, const std::map<int, long>& bore, CActField* pDoc, CCrossSection* pSec)
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

    std::valarray<double> xx(n);
    std::valarray<double> yy(n);
    std::valarray<double> md(n);
    double x = dist[key];
    double y = q[2];
    
    xx[0] = pSec->left_margin + (pSec->page_width - pSec->left_margin - pSec->right_margin)
      * (x - x_range[0]) / (x_range[1] - x_range[0]);
    yy[0] = pSec->bottom_margin + (pSec->page_height - pSec->top_margin - pSec->bottom_margin)
      * (y_range[1] - y) / (y_range[1] - y_range[0]);
    md[0] = 0.0;

		out.setfont(pSec->font_family, pSec->font_size);
		out.newpath();
		out.moveto(xx[0], yy[0]);

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

			out.lineto(xx[i], yy[i]);
    }
		out.setgray(1);
		out.setlinewidth(2);
		out.stroke();

		if (spRowsetIndex != NULL)
		{
			CFindWell::Well data;
			data.bore = val;
			HRESULT hr = spRowsetIndex->Seek(tabl.GetHAccessor(0), 1, &data, DBSEEK_FIRSTEQ);
			if (hr == S_OK && S_OK == tabl.MoveNext())
			{
				double bbox[8];
				const char* name = (const char*) tabl.name;
				out.setgray(1);
				out.rectfill(xx[0]-6, yy[0], 12, 30);
				out.setgray(0);
				out.setlinewidth(1);
				out.rectstroke(xx[0]-6, yy[0], 12, 30);
				out.annot(xx[0], yy[0]+15, 1.0, 90.0, psstream::ANNOT_CENTER, true, name, bbox);
				
				/*
				// trapezoidal shapes with inscribed names;
				out.annot(xx[0], yy[0], 1.0, 90.0, psstream::ANNOT_RIGHT, false, name, bbox);
				out.moveto(bbox[0]+3.0, bbox[1]-1);
				out.lineto(bbox[2]+1.5, bbox[3]+1);
				out.lineto(bbox[4]-1.5, bbox[5]+1);
				out.lineto(bbox[6]-3.0, bbox[7]-1);
				out.lineto(bbox[0]+3.0, bbox[1]-1);
				out.setgray(0);
				out.fill();
				out.setgray(1);
				out.annot(xx[0], yy[0], 1.0, 90.0, psstream::ANNOT_RIGHT, true, name, bbox);
				*/

				/*
				out << "0 setgray\n" << xx[0] << " (" << name \
					<< ") stringwidth pop 2 div sub " << yy[0] + pSec->font_size \
					<< " moveto (" << name << ") outlineshow\n";
				{
					double x = xx[0], y = yy[0];
					double hh = pSec->font_size * 0.9, dd = 0.5 * hh / GOLDEN_RATIO;
					out << "newpath " << x-dd << ' ' << y << " moveto " << x+dd << ' ' << y << " lineto "
						<< x << ' ' << y+hh << " lineto " << x-dd << ' ' << y << " lineto closepath fill\n";
				}
				*/
			}
		}

    // P E R F O R A T I O N
    int n_per = pDoc->Perf(val, 0, 0);
    if (n_per > 0)
    {
      std::valarray<CActField::PerfItem*> per(n_per);
      pDoc->Perf(val, n_per, &per[0]);
      for (i = 0; i < n_per; ++i)
      {
        double *dd = &md[0];
        double d1 = per[i]->perf_upper;
        double d2 = per[i]->perf_lower;

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
				out.newpath();
				out.moveto(x1, y1);

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
					out.lineto(xx[j], yy[j]);
        }
				out.lineto(x2, y2);
				out.setgray(0);
				out.setlinewidth(2);
				out.stroke();

				/*
        // F L O W  I N F O R M A T I O N
        CCumulativeS* pSat = GetCumulativeS();
        // if (strcmp(pDoc->field->name, "scom") != 0 && !pSat->IsWindowVisible())
        if (!pSat->IsWindowVisible())
        {
          int n_prod = pDoc->Prod(val, 0, 0);
          if (n_prod > 0)
          {
            std::valarray<CActField::ProdItem*> prod(n_prod);
            pDoc->Prod(val, n_prod, &prod[0]);
            for (int j = 0; j < n_prod; ++j)
            {
              if (prod[j]->bed == per[i]->perf_form)
              {
                out << "0 setgray\n";

                out << x1+10 << ' ' << y1-10 << " moveto (oil = " \
                  << prod[j]->oil / prod[j]->days << ") show\n";

                out << x1+10 << ' ' << y1-20 << " moveto (wat = " \
                  << prod[j]->wat / prod[j]->days << ") show\n";

                out << x1+10 << ' ' << y1-30 << " moveto (gas = " \
                  << prod[j]->gas / prod[j]->days << ") show\n";
              }
            }
          }

          int n_pump = pDoc->Pump(val, 0, 0);
          if (n_pump > 0)
          {
            std::valarray<CActField::PumpItem*> pump(n_pump);
            pDoc->Pump(val, n_pump, &pump[0]);
            for (int j = 0; j < n_pump; ++j)
            {
              if (pump[j]->bed == per[i]->perf_form)
              {
                out << "0 setgray\n";

                out << x1+10 << ' ' << y1-10 << " moveto (fluid = " \
                  << pump[j]->fluid / pump[j]->days << ") show\n";
              }
            }
          }
        }
				*/
      }
    }
    
		bool show_logtrack = false;
		int n_inf = pDoc->Info(val, 0, 0);
		if (show_logtrack && n_inf > 0)
		{
			double dy = 30.0;
			double dx = dy * GOLDEN_RATIO;
			double x0 = xx[0]-6-30*GOLDEN_RATIO;
			double y0 = yy[0];
			out.setgray(1);
			out.rectfill(x0, y0, dx, dy);
			out.setgray(0);
			out.setlinewidth(1);
			out.rectstroke(x0, y0, dx, dy);

			std::valarray<CFieldDoc::InfoItem*> inf(n_inf);
			pDoc->Info(val, n_inf, &inf[0]);
			bool has_currentpoint = false;
			out.setlinewidth(0.5);
			out.setgray(0);
			int n_dist = md.size();
			double *dist = &md[0];
			double *offs = &yy[0];
			double x, y;
			for (int i = 0; i < n_inf; ++i)
			{
				CFieldDoc::InfoItem* item = inf[i];
				if (item->a_sp == -9999.0f)
				{
					if (has_currentpoint)
					{
						out.stroke();
						has_currentpoint = false;
					}
					continue;
				}
				x = x0 + item->a_sp * dx;
				int s = std::lower_bound(dist, dist+n_dist, (double) item->top) - dist;
				if (s != n_dist)
				{
					y = yy[s];
					if (inf[i]->top < md[s])
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
					if (inf[i]->bot < md[s])
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

void CCrossSection::Generate(const char* name)
{
  CActField* pDoc = GetFieldView()->GetDocument();
  ASSERT_VALID(pDoc);

  // initialize list of control points for cross-section
  // take into account horizontal displacement of well bores
  vertex_type ver;
  std::map<int, long> b_node;
  CListCtrl* pp = &m_ctrlPoints;
  int n = pp->GetItemCount();
  for (int i = 0; i < n; ++i)
  {
    coord_type p = *((coord_type*) pp->GetItemData(i));

    long bb;
    CActField::Point pt = { p.xcoord, p.ycoord };
    int nn = pDoc->RangeQuery(pt, 0.1, 1, &bb);
    ASSERT(nn <= 1);
    if (!nn)
    {
      if (ver.empty())
      {
        ver.push_back(p);
      }
      else
      {
        coord_type p0 = ver.back();
        ver.pop_back();
        blin(p0, p, ver);
      }
    }
    else
    {
      nn = pDoc->Traj(bb, 0, 0);
      std::valarray<double> pp(3*nn);
      double *q = &pp[0];
      nn = pDoc->Traj(bb, nn, q);

      vertex_type acc;
      double prev[3];
      VERIFY(INSIDE_DOM(pDoc->Cell(q, prev)));
      coord_type ctrl;
      ctrl.xcoord = prev[0];
      ctrl.ycoord = prev[1];
      acc.push_back(ctrl);
      for (int j = 1; j < nn; ++j)
      {
        q += 3;
        double curr[3];
        VERIFY(INSIDE_DOM(pDoc->Cell(q, curr)));
        double dx = curr[0] - prev[0];
        double dy = curr[1] - prev[1];
        if (dx*dx + dy*dy > 0.0)
        {
          ctrl.xcoord = curr[0];
          ctrl.ycoord = curr[1];
          acc.push_back(ctrl);
          prev[0] = curr[0];
          prev[1] = curr[1];
        }
      }

      vertex_type::iterator it = acc.begin();
      if (ver.empty())
      {
        ver.push_back(*it);
      }
      else
      {
        p = ver.back();
        ver.pop_back();
        blin(p, *it, ver);
      }

      b_node.insert(std::make_pair((int) ver.size()-1, bb));
      while (++it != acc.end())
      {
        p = ver.back();
        ver.pop_back();
        blin(p, *it, ver);
      }
    }
  }

  fltarray dist, elev, func, soil, sgas, coef;
  CCumulativeS* pSat = GetCumulativeS();
  if (!pSat->IsWindowVisible())
  {
    make_cross_grid(pDoc, ver, dist, elev, func);
  }
  else
  {
    make_combined_cross_grid(pDoc, ver, dist, elev, sgas, soil, coef);
  }

	std::list<char*> fmn_name;
	std::list<float*> fmn_data;
	bool formation_ok = show_formation;
	if (formation_ok)
	{
		H5E_auto_t e_func;
		void* e_data;
		herr_t err = H5Eget_auto(&e_func, &e_data);
		hid_t form = H5Gopen(pDoc->hdf5_model, "formation");
		err = H5Eset_auto(e_func, &e_data);
		if (0 > form)
		{
			formation_ok = false;
		}
		else
		{
			hsize_t num_objs = 0;
			err = H5Gget_num_objs(form, &num_objs);
			if (0 <= err)
			{
				for (hsize_t i = 0; i < num_objs; ++i)
				{
					char name[0x100];
					ssize_t o_buff = H5Gget_objname_by_idx(form, i, name, sizeof(name));
					if (0 < o_buff)
					{
						char* cc = new char[lstrlen(name)+1];
						lstrcpy(cc, name);
						fmn_name.push_back(cc);
						hid_t data = H5Dopen(form, name);
						if (0 <= data)
						{
							int n_surf = 2 * pDoc->cx * pDoc->cy;
							float *surf = new float[n_surf];
							err = H5Dread(data, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, surf);
							if (0 <= err)
							{
								int n_data = ver.size();
								float* f_data = new float[3 * n_data];
								make_cross_formation(pDoc, ver, surf, f_data);
								fmn_data.push_back(f_data);
							}
							delete[] surf;
							err = H5Dclose(data);
						}
					}
				}
			}
			err = H5Gclose(form);
			formation_ok = (fmn_name.size() == int(num_objs) && fmn_data.size() == int(num_objs));
		}
	}

  // generate graphics
	psstream out(name);
  print_prolog(out, this);

  int final, start = 0;
  float max_dist = dist[dist.size()-1];
  double y_range[2];
  y_range[0] = elev.min();
  y_range[1] = elev.max();

  int s_iter = 0;
	WellMatch match;
  for (i = 0; i < num_pages; ++i)
	{
    final = std::upper_bound(&dist[0], &dist[0] + dist.size(), \
      (i+1) * max_dist / num_pages) - &dist[0] - 1;
    double x_range[2];
    x_range[0] = floor(dist[start]);
    x_range[1] = ceil(dist[final]);
		match.new_page(x_range, y_range, start, final, &dist[0], b_node, pDoc, this);
    s_iter += final - start;
		start = final;
	}

	s_iter = start = 0;
	match.first_page();
  for (i = 0; i < num_pages; ++i)
  {
    final = std::upper_bound(&dist[0], &dist[0] + dist.size(), \
      (i+1) * max_dist / num_pages) - &dist[0] - 1;

    double x_range[2];
    x_range[0] = floor(dist[start]);
    x_range[1] = ceil(dist[final]);
		out.newpage();
    print_frame_grid(out, x_range, y_range, pDoc, this);

    if (pSat->IsWindowVisible())
    {
      int i, j, n, cx = dist.size(), cy = soil.size() / cx, nx = final - start + 1, nn = nx * cy;

			fltarray xx(nn);
      for (j = n = 0; j < cy; ++j)
      {
        for (i = start; i <= final; ++i)
        {
					xx[n++] = float(left_margin + (page_width - left_margin - right_margin) 
            * (dist[i] - x_range[0]) / (x_range[1] - x_range[0]));
        }
      }

			fltarray yy(nn);
      for (j = n = 0; j < cy; ++j)
      {
        for (i = start; i <= final; ++i)
        {
          yy[n++] = float(bottom_margin + (page_height - top_margin - bottom_margin) \
            * (y_range[1] - elev[j*cx+i]) / (y_range[1] - y_range[0]));
        }
      }

			COLORREF cr = pSat->m_minColor.GetColor();
			out.min_collector(pSat->m_minCollector, GetRValue(cr) / 255.0, GetGValue(cr) / 255.0, GetBValue(cr) / 255.0);
			cr = pSat->m_maxColor.GetColor();
			out.max_collector(pSat->m_maxCollector, GetRValue(cr) / 255.0, GetGValue(cr) / 255.0, GetBValue(cr) / 255.0);
			out.tricolor(pSat->m_nSize, pSat->m_pColorData);
			out.pixelplot(nx, cy, &xx[0], &yy[0], &sgas[0], &soil[0], &coef[0]);
    }
    else
    {
      vertex_type ver;
      std::valarray<edge_type> edg(pDoc->clut.nz), pgn(pDoc->clut.nz+2);
			int cx = dist.size(), cy = func.size() / cx;
			marching_squares(cx, cy, start, final, 0, cy-1, &func[0], pDoc->clut.nz, \
				pDoc->clut.zz, ver, &edg[0], &pgn[0]);
			update_profile_border(pgn.size(), ver, &pgn[0], pDoc->cz-1);
      std::transform(ver.begin(), ver.end(), ver.begin(), profile_cell_to_point(dist, elev));

			double sx = (page_width - left_margin - right_margin) / (x_range[1] - x_range[0]);
			double x0 = left_margin - sx * x_range[0];
			double sy = (page_height - top_margin - bottom_margin) / (y_range[0] - y_range[1]);
			double y0 = bottom_margin - sy * y_range[1];
			convert(x0, y0, sx, sy, ver);

			CActField::Palette* pal = &pDoc->clut;
			for (int i = 0; i <= pal->nz; ++i)
			{
				if (!pgn[i].empty())
				{
					plotpath(out, ver, pgn[i]);
					out.setrgbcolor(pal->sh[i].r, pal->sh[i].g, pal->sh[i].b);
					out.fill();
				}
			}
    }

		if (formation_ok)
		{
			double sx = (page_width - left_margin - right_margin) / (x_range[1] - x_range[0]);
			double x0 = left_margin - sx * x_range[0];
			double sy = (page_height - top_margin - bottom_margin) / (y_range[0] - y_range[1]);
			double y0 = bottom_margin - sy * y_range[1];

			int n_ver = ver.size();
			std::list<char*>::iterator c_iter = fmn_name.begin();
			std::list<float*>::iterator f_iter = fmn_data.begin();
			vertex_type g_ver;
			std::list<edge_type> gg;
			out.setfont(font_family, font_size);
			int n_fmn = 0;
			for (int i = 0, n = fmn_name.size(); i < n; ++i)
			{
				int n_data = ver.size();
				float* z1 = *f_iter;
				float* z2 = z1 + n_data;
				float* xx = z2 + n_data;
				int n_ver = g_ver.size();
				gg.push_back(edge_type());
				edge_type* g_pgn = &gg.back();
				curve_type* g_cur = 0;
				coord_type p;
				for (int j = start; j <= final; ++j)
				{
					if (z1[j] != z2[j])
					{
						if (!g_cur)
						{
							g_pgn->push_back(curve_type());
							g_cur = &g_pgn->back();
						}
						double x = x0 + sx * xx[j];
						p.xcoord = x;
						p.ycoord = y0 + sy * z1[j];
						g_ver.push_back(p);
						g_cur->push_back(n_ver++);
						p.ycoord = y0 + sy * z2[j];
						g_ver.push_back(p);
						g_cur->push_front(n_ver++); 
					}
					else if (g_cur)
					{
						g_cur->push_front(g_cur->back());
						g_cur = 0;
					}
				}
				if (g_cur)
				{
					g_cur->push_front(g_cur->back());
					g_cur = 0;
				}
				if (!g_pgn->empty())
				{
					enum psstream::Pattern hatch[] =
					{
						psstream::BDIAGONAL,
						psstream::CROSSHATCH,
						psstream::DIAGHATCH,
						psstream::HORIZONTAL,
						psstream::FDIAGONAL,
						psstream::VERTICAL
					};
					plotpath(out, g_ver, *g_pgn);
					out.gsave();
					psstream::Pattern h = hatch[i % (sizeof(hatch) / sizeof(*hatch))];
					out.setcolorspace(0.1, 2.0);
					out.setcolor(0.5, 0.5, 0.5, h);
					out.fill();
					out.grestore();
					out.setlinewidth(1);
					out.setgray(0.5);
					out.stroke();

					double bbox[8];
					double x0 = page_width - right_margin + 2;
					double y0 = page_height - top_margin - ++n_fmn * font_size - 2;
					double dy = font_size;
					double dx = dy * GOLDEN_RATIO;
					out.setgray(1);
					out.rectfill(x0, y0, dx, dy);
					out.setcolorspace(0.1, 2.0);
					out.setcolor(0.5, 0.5, 0.5, h);
					out.rectfill(x0, y0, dx, dy);
					out.setgray(0);
					out.rectstroke(x0, y0, dx, dy);
					out.annot(x0+dx, y0+dy-font_size*0.5, font_size*0.5, 0.0, \
						psstream::ANNOT_RIGHT, true, *c_iter, bbox);
				}
				++c_iter;
				++f_iter;
			}

			/*
			std::list<edge_type>::iterator g_iter = gg.begin();
			c_iter = fmn_name.begin();
			f_iter = fmn_data.begin();
			out.setfont("Helvetica", 4.0);
			back_polygon bgr(out);
			for (int i = 0, n = fmn_name.size(); i < n; ++i)
			{
				annotpath(out, g_ver, *g_iter, *c_iter, 200, 0.5, &bgr);
				++c_iter;
				++f_iter;
				++g_iter;
			}
			*/
		}

    // print_well_bores(out, x_range, y_range, start, final, &dist[0], b_node, pDoc, this);
		match.plot_page(out);
		match.next_page();
    s_iter += final - start;

		if (!description.IsEmpty())
		{
			out.setgray(0);
			out.setfont(font_family, font_size);
			CString str;
			double box[8];
			out.annot(page_width * 0.5, page_height - top_margin * 0.5, 1.0, 0.0, \
				psstream::ANNOT_CENTER, true, description, box);
		}

		if (page_numbering)
		{
			out.setgray(0);
			out.setfont(font_family, font_size);
			CString str;
			double box[8];
			str.Format("- %d/%d -", i+1, num_pages);
			out.annot(page_width * 0.5, bottom_margin * 0.5, 1.0, 0.0, \
				psstream::ANNOT_CENTER, true, str, box);
		}
		if (show_legend)
		{
			print_legend(out, pDoc, this);
		}

		CFieldApp* pApp = STATIC_DOWNCAST(CFieldApp, AfxGetApp());
		if (pApp->m_bDemoMode)
		{
			out.setcolorspace(0.1, 12.0);
			out.setcolor(0.8, 0.8, 0.8, psstream::EVALUATION);
			out.rectfill(0.0, 0.0, out.pagewidth(), out.pageheight());
		}

    start = final;
  }

	for (std::list<char*>::iterator c_iter = fmn_name.begin(); c_iter != fmn_name.end(); ++c_iter)
	{
		delete[] *c_iter;
	}
	for (std::list<float*>::iterator f_iter = fmn_data.begin(); f_iter != fmn_data.end(); ++f_iter)
	{
		delete[] *f_iter;
	}
}

void CCrossSection::OnPlotCrossSection() 
{
  CString str;
	m_ctrlPlot.GetWindowText(str);
  if (str.IsEmpty())
  {
    MessageBox("Please define filename for graphics output\n");
    return;
  }
  if (m_ctrlPoints.GetItemCount() < 2)
  {
    MessageBox("Please define at least two contol points for the cross-section\n");
    return;
  }
  CWaitCursor wait;
  int n = str.ReverseFind('.');
  if (n == -1 || str.Right(str.GetLength()-n).CompareNoCase(".ps") != 0)
  {
    str += ".ps";
  }

  Generate(str);

  if (m_bRun)
  {
		CString app;
		m_ctrlApp.GetWindowText(app);
    if (app.IsEmpty())
    {
			m_ctrlApp.OnBrowse();
			m_ctrlApp.GetWindowText(app);
			if (app.IsEmpty())
        return;
    }

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    char cmd[512];
		std::ostrstream ss(cmd, countof(cmd));
		ss << '"' << app << "\" \"" << str << '"' << std::ends;
    // sprintf(cmd, "%s %s", (const char*) app, (const char*) str);
    if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
      AfxGetApp()->WriteProfileString("Run", "App", app);
    }
  }
}

void CCrossSection::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
  m_bVisible = bShow;
  GetFieldView()->GetDocument()->UpdateAllViews(0);	
}

void CCrossSection::OnCancel() 
{
  m_bVisible = FALSE;
  GetFieldView()->GetDocument()->UpdateAllViews(0);	
	CDialog::OnCancel();
}

void CCrossSection::OnClose() 
{
  m_bVisible = FALSE;
  GetFieldView()->GetDocument()->UpdateAllViews(0);	
	CDialog::OnClose();
}

void CCrossSection::OnPlotProperties() 
{
  CPlotDialog dlg(this);
  dlg.x_major_tick = x_major_tick;
  dlg.x_minor_tick = x_minor_tick;
  dlg.y_major_tick = y_major_tick;
  dlg.y_minor_tick = y_minor_tick;
  dlg.show_grid = show_grid;
  dlg.show_legend = show_legend;
  dlg.left_margin = left_margin;
  dlg.right_margin = right_margin;
  dlg.top_margin = top_margin;
  dlg.bottom_margin = bottom_margin;
  dlg.num_pages = num_pages;
  dlg.font_family = font_family;
  dlg.orientation = orientation;
  dlg.font_size = font_size;
  dlg.format = format;
	dlg.page_numbering = page_numbering;
	dlg.description = description;
	dlg.show_formation = show_formation;
  if (dlg.DoModal() == IDOK)
  {
    x_major_tick = dlg.x_major_tick;
    x_minor_tick = dlg.x_minor_tick;
    y_major_tick = dlg.y_major_tick;
    y_minor_tick = dlg.y_minor_tick;
    show_grid = (dlg.show_grid != 0);
    show_legend = (dlg.show_legend != 0);
    left_margin = dlg.left_margin;
    right_margin = dlg.right_margin;
    top_margin = dlg.top_margin;
    bottom_margin = dlg.bottom_margin;
    num_pages = dlg.num_pages;
    lstrcpy(font_family, dlg.font_family);
    orientation = (Orientation) dlg.orientation;
    font_size = dlg.font_size;
    format = (Format) dlg.format;
		page_numbering = (dlg.page_numbering == TRUE);
		show_formation = (dlg.show_formation == TRUE);
		description = dlg.description;
    InitPageSizes();
  }
}
