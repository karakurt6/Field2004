// FieldForm.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "FieldForm.h"
#include "FieldDoc.h"
#include "OverView.h"
#include "FieldView.h"
#include "CrossSection.h"

#include <limits>
#include <iostream>
#include <fstream>
#include <libh5/hdf5.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFieldForm

IMPLEMENT_DYNCREATE(CFieldForm, CFormView)

CFieldForm::CFieldForm()
	: CFormView(CFieldForm::IDD)
{
	//{{AFX_DATA_INIT(CFieldForm)
	m_nBed = 0;
	//}}AFX_DATA_INIT
}

CFieldForm::~CFieldForm()
{
}

void CFieldForm::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFieldForm)
	DDX_Control(pDX, IDC_COMBO1, m_ctrlProp);
	DDX_Control(pDX, IDC_SPIN1, m_ctrlSpin);
	DDX_Control(pDX, IDC_SLIDER1, m_ctrlBed);
	DDX_Control(pDX, IDC_INJLIST, m_ctrlExt);
	DDX_Control(pDX, IDC_EXTLIST, m_ctrlInj);
	DDX_Control(pDX, IDC_MONTHCALENDAR1, m_ctrlCal);
	DDX_Text(pDX, IDC_EDIT1, m_nBed);
	//}}AFX_DATA_MAP

  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);
  DDV_MinMaxInt(pDX, m_nBed, 1, pDoc->cz);
}


BEGIN_MESSAGE_MAP(CFieldForm, CFormView)
	//{{AFX_MSG_MAP(CFieldForm)
	ON_NOTIFY(MCN_SELCHANGE, IDC_MONTHCALENDAR1, OnSelchangeMonthcalendar1)
	ON_NOTIFY(MCN_SELECT, IDC_MONTHCALENDAR1, OnSelectMonthcalendar1)
	ON_NOTIFY(NM_CLICK, IDC_EXTLIST, OnClickExtlist)
	ON_NOTIFY(NM_CLICK, IDC_INJLIST, OnClickInjlist)
	ON_WM_PAINT()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, OnReleasedcaptureSlider1)
	ON_EN_KILLFOCUS(IDC_EDIT1, OnKillfocusEdit1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, OnDeltaposSpin1)
	ON_CBN_SELCHANGE(IDC_COMBO1, OnSelchangeCombo1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFieldForm diagnostics

#ifdef _DEBUG
void CFieldForm::AssertValid() const
{
	CFormView::AssertValid();
}

void CFieldForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CActField* CFieldForm::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CActField)));
	return (CActField*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFieldForm message handlers

void CFieldForm::Populate()
{
  CWaitCursor wait;
  DATE date = m_dateSel;

  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  short year = m_dateSel.GetYear();
  if (pDoc->year != year)
  {
    pDoc->UpdateYear(year);
  }

  m_listExt.clear();
  m_listInj.clear();

	char query[0x400];
	sprintf(query, "select hist.wellbore, well.well_name, d1.dict_mnemonics_ru as "
		"formation, d2.dict_description_ru as opcode, hist.startdate, hist.finaldate, "
		"hist.oil, hist.wat, hist.gas, hist.fluid, hist.puregas from hist, well, dict "
		"d1, dict d2 where %g between hist.startdate and hist.finaldate and d1.dict_item"
		"=hist.formation and d1.dict_group=1000101 and d2.dict_item=hist.opcode and "
		"d2.dict_group=1003001 and well.well_bore=hist.wellbore", (DATE) m_dateSel);

	CCommand< CAccessor<Well> > cmd;
	if (SUCCEEDED(cmd.Open(pDoc->session, query)))
	{
		while (cmd.MoveNext() == S_OK)
		{
			double h[3];
			if (pDoc->Traj(cmd.wellbore, 1, h))
			{
				if (cmd.status.oil == DBSTATUS_S_OK \
					&& cmd.status.water == DBSTATUS_S_OK && cmd.status.gas == DBSTATUS_S_OK)
				{
					ext_data_struct ext;
					ext.oil = cmd.oil;
					ext.gas = cmd.gas;
					ext.wat = cmd.water;
					strcpy(ext.layer, cmd.formation);
					strcpy(ext.well, cmd.wellname);
					ext.loc[0] = (float) h[0];
					ext.loc[1] = (float) h[1];
					ext.loc[2] = (float) h[2];
					m_listExt.push_back(ext);
				}
				else if (cmd.status.fluid == DBSTATUS_S_OK)
				{
					inj_data_struct inj;
					inj.bore = cmd.wellbore;
					inj.fluid = cmd.fluid;
					strcpy(inj.well, cmd.wellname);
					strcpy(inj.layer, cmd.formation);
					inj.loc[0] = (float) h[0];
					inj.loc[1] = (float) h[1];
					inj.loc[2] = (float) h[2];
					m_listInj.push_back(inj);
				}
			}
		}
	}

  GetOverView()->Invalidate();
  GetFieldView()->Invalidate();

  m_ctrlExt.SetRedraw(FALSE);
  m_ctrlExt.DeleteAllItems();
  m_ctrlExt.SetItemCount(m_listExt.size());
  for (ext_data_list::iterator e_iter = m_listExt.begin(); e_iter != m_listExt.end(); ++e_iter)
  {
    int n = m_ctrlExt.InsertItem(0, e_iter->well);
    if (n != -1)
    {
      CString str;
      m_ctrlExt.SetItemText(n, 1, e_iter->layer);
      str.Format("%lf", e_iter->oil);
      m_ctrlExt.SetItemText(n, 2, str);
      str.Format("%lf", e_iter->gas);
      m_ctrlExt.SetItemText(n, 3, str);
      str.Format("%lf", e_iter->wat);
      m_ctrlExt.SetItemText(n, 4, str);
    }
  }
  /*
  if (!m_listExt.empty())
  {
    m_ctrlExt.SetColumnWidth(0, LVSCW_AUTOSIZE);
    m_ctrlExt.SetColumnWidth(1, LVSCW_AUTOSIZE);
    m_ctrlExt.SetColumnWidth(2, LVSCW_AUTOSIZE);
    m_ctrlExt.SetColumnWidth(3, LVSCW_AUTOSIZE);
    m_ctrlExt.SetColumnWidth(4, LVSCW_AUTOSIZE);
  }
  else
  */
  {
    m_ctrlExt.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
    m_ctrlExt.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
    m_ctrlExt.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
    m_ctrlExt.SetColumnWidth(3, LVSCW_AUTOSIZE_USEHEADER);
    m_ctrlExt.SetColumnWidth(4, LVSCW_AUTOSIZE_USEHEADER);
  }
  m_ctrlExt.SetRedraw(TRUE);

  m_ctrlInj.SetRedraw(FALSE);
  m_ctrlInj.DeleteAllItems();
  m_ctrlInj.SetItemCount(m_listInj.size());
  for (inj_data_list::iterator i_iter = m_listInj.begin(); i_iter != m_listInj.end(); ++i_iter)
  {
    int n = m_ctrlInj.InsertItem(0, i_iter->well);
    if (n != -1)
    {
      CString str;
      m_ctrlInj.SetItemText(n, 1, i_iter->layer);
      str.Format("%lf", i_iter->fluid);
      m_ctrlInj.SetItemText(n, 2, str);
    }
  }
  /*
  if (!m_listInj.empty())
  {
    m_ctrlInj.SetColumnWidth(0, LVSCW_AUTOSIZE);
    m_ctrlInj.SetColumnWidth(1, LVSCW_AUTOSIZE);
    m_ctrlInj.SetColumnWidth(2, LVSCW_AUTOSIZE);
  }
  else
  */
  {
    m_ctrlInj.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
    m_ctrlInj.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
    m_ctrlInj.SetColumnWidth(2, LVSCW_AUTOSIZE_USEHEADER);
  }
  m_ctrlInj.SetRedraw(TRUE);

  GetFieldView()->UpdateCrossSection();
}

void CFieldForm::OnInitialUpdate() 
{
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

	CCommand< CAccessor<Date> > cmd;
	HRESULT hr = cmd.Open(pDoc->session, "select min(startdate), max(finaldate) from hist");
	VERIFY(SUCCEEDED(hr));
	VERIFY(SUCCEEDED(cmd.MoveNext()));
	m_dateStart = cmd.startdate;
	// VERIFY(SUCCEEDED(cmd.MoveLast()));
	m_dateFinal = cmd.finaldate;
  COleDateTime date(m_dateFinal);

  m_nBed = pDoc->level+1;
	CFormView::OnInitialUpdate();
	{
		hid_t id = H5Dopen(pDoc->hdf5_model, "soil");
		if (id >= 0)
		{
			hid_t fs = H5Dget_space(id);
			if (fs >= 0)
			{
				hsize_t count[4];
				int nd = H5Sget_simple_extent_dims(fs, count, NULL);
				if (nd == 4)
				{
					int nt = (int) count[0];
					hid_t t_attr = H5Aopen_name(id, "time");
					short* tt = new short[nt];
					herr_t err = H5Aread(t_attr, H5T_NATIVE_SHORT, tt);
					err = H5Aclose(t_attr);
					COleDateTime start(tt[0], 1, 1, 0, 0, 0);
					COleDateTime final(tt[nt-1], 1, 1, 0, 0, 0);
					if (m_dateFinal < final)
					{
						m_dateFinal = final;
					}
					delete[] tt;
				}
				H5Sclose(fs);
			}
			H5Dclose(id);
		}
	}
  m_ctrlCal.SetRange(&m_dateStart, &m_dateFinal);
	
  // m_ctrlCal.SetColor(MCSC_MONTHBK, GetSysColor(COLOR_BTNFACE));
  // m_ctrlCal.SetColor(MCSC_BACKGROUND, GetSysColor(COLOR_BTNFACE));
  m_dateSel = m_dateStart;
  m_ctrlCal.SetCurSel(m_dateSel);

	TCHAR buff[10];
	LVCOLUMN col;
	col.mask = LVCF_TEXT;
	col.pszText = buff;
	col.cchTextMax = 10;
	if (!m_ctrlInj.GetColumn(0, &col) || _tcsicmp(_T("name"), buff) != 0)
	{
    // m_ctrlInj.SetBkColor(GetSysColor(COLOR_BTNFACE));
    // m_ctrlInj.SetTextBkColor(GetSysColor(COLOR_BTNFACE));
    m_ctrlInj.SetExtendedStyle(LVS_EX_TRACKSELECT | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_ctrlInj.InsertColumn(0, "name");
    m_ctrlInj.InsertColumn(1, "layer");
    m_ctrlInj.InsertColumn(2, "liq/day");
	}
  else
  {
    m_ctrlInj.DeleteAllItems();
  }

	col.mask = LVCF_TEXT;
	col.pszText = buff;
	col.cchTextMax = 10;
	if (!m_ctrlExt.GetColumn(0, &col) || _tcsicmp(_T("name"), buff) != 0)
	{
    // m_ctrlExt.SetBkColor(GetSysColor(COLOR_BTNFACE));
    // m_ctrlExt.SetTextBkColor(GetSysColor(COLOR_BTNFACE));
    m_ctrlExt.SetExtendedStyle(LVS_EX_TRACKSELECT | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
    m_ctrlExt.InsertColumn(0, "name");
    m_ctrlExt.InsertColumn(1, "layer");
    m_ctrlExt.InsertColumn(2, "oil/day");
    m_ctrlExt.InsertColumn(3, "gas/day");
    m_ctrlExt.InsertColumn(4, "wat/day");
  }
  else
  {
    m_ctrlExt.DeleteAllItems();
  }

	CRect rc;
	m_ctrlBed.GetWindowRect(&rc);
  ScreenToClient(&rc);
	m_ctrlBed.DestroyWindow();
	m_ctrlBed.Create(WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_HORZ|TBS_TOOLTIPS, rc, this, IDC_SLIDER1);
  m_ctrlBed.SetRange(1, pDoc->cz);
  m_ctrlBed.SetTicFreq(pDoc->cz/10);
  m_ctrlBed.SetTipSide(TBTS_TOP);
  m_ctrlSpin.SetRange(1, pDoc->cz);

  Populate();
  InitPropList();
}

void CFieldForm::InitPropList()
{
  CActField* pDoc = GetDocument();

  const int nn = 20;
  const CActField::DataField* pp[nn];
  int n = pDoc->FieldEnum(nn, pp);
  ASSERT(n <= nn);
  for (int i = 0; i < n; ++i)
  {
    char str[0x100];
    sprintf(str, "%s, %s", pp[i]->info, pp[i]->units);
    m_ctrlProp.AddString(str);
  }
  m_ctrlProp.SetCurSel(0);
}

void CFieldForm::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
  /*
  if (lHint)
  {
    m_ctrlBed.SetPos(m_nBed);
  }
  */
}

void CFieldForm::OnSelchangeMonthcalendar1(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LPNMSELCHANGE lpNMSelChange = (LPNMSELCHANGE) pNMHDR;
  COleDateTime dateSel = lpNMSelChange->stSelStart;
  if (dateSel < m_dateStart)
    dateSel = m_dateStart;
  if (dateSel > m_dateFinal)
    dateSel = m_dateFinal;

  if (m_dateSel != dateSel)
  {
    m_ctrlCal.SetCurSel(dateSel);
    m_dateSel = dateSel;
    Populate();
  }

	*pResult = 0;
}

void CFieldForm::OnSelectMonthcalendar1(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LPNMSELCHANGE lpNMSelChange = (LPNMSELCHANGE) pNMHDR;
  COleDateTime dateSel = lpNMSelChange->stSelStart;
  if (dateSel < m_dateStart)
    dateSel = m_dateStart;
  if (dateSel > m_dateFinal)
    dateSel = m_dateFinal;

  if (m_dateSel != dateSel)
  {
    m_ctrlCal.SetCurSel(dateSel);
    m_dateSel = dateSel;
    Populate();
  }
	
	*pResult = 0;
}

void CFieldForm::OnClickExtlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  int n = m_ctrlInj.GetNextItem(-1, LVNI_SELECTED);
  if (n != -1)
  {
    char name[16];
    m_ctrlInj.GetItemText(n, 0, name, sizeof(name));

    for (inj_data_list::iterator i_iter = m_listInj.begin(); i_iter != m_listInj.end(); ++i_iter)
    {
      if (stricmp(name, i_iter->well) == 0)
      {
        CFieldView *pFieldView = GetFieldView();
        CSize sizeTotal = pFieldView->GetTotalSize();
        CActField* pDoc = GetDocument();
        int x = int(sizeTotal.cx * (i_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
        int y = int(sizeTotal.cy * (i_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);

        CRect rectClient;
        pFieldView->GetClientRect(&rectClient);
        CClientDC dc(pFieldView);
        pFieldView->OnPrepareDC(&dc);
        dc.DPtoLP(&rectClient);
        CPoint pointCenter = rectClient.CenterPoint();
        CSize sizeScroll = pointCenter - CPoint(x, -y);
        dc.LPtoDP(&sizeScroll);
        sizeScroll.cx = -sizeScroll.cx;
        pFieldView->OnScrollBy(sizeScroll, TRUE);
        break;
      }
    }
  }
		
	*pResult = 0;
}

void CFieldForm::OnClickInjlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
  int n = m_ctrlExt.GetNextItem(-1, LVNI_SELECTED);
  if (n != -1)
  {
    char name[16];
    m_ctrlExt.GetItemText(n, 0, name, sizeof(name));

    for (ext_data_list::iterator e_iter = m_listExt.begin(); e_iter != m_listExt.end(); ++e_iter)
    {
      if (stricmp(name, e_iter->well) == 0)
      {
        CFieldView *pFieldView = GetFieldView();
        CSize sizeTotal = pFieldView->GetTotalSize();
        CActField* pDoc = GetDocument();
        int x = int(sizeTotal.cx * (e_iter->loc[0] - pDoc->x1) / (pDoc->x2 - pDoc->x1) + 0.5);
        int y = int(sizeTotal.cy * (e_iter->loc[1] - pDoc->y2) / (pDoc->y1 - pDoc->y2) + 0.5);

        CRect rectClient;
        pFieldView->GetClientRect(&rectClient);
        CClientDC dc(pFieldView);
        pFieldView->OnPrepareDC(&dc);
        dc.DPtoLP(&rectClient);
        CPoint pointCenter = rectClient.CenterPoint();
        CSize sizeScroll = pointCenter - CPoint(x, -y);
        dc.LPtoDP(&sizeScroll);
        sizeScroll.cx = -sizeScroll.cx;
        pFieldView->OnScrollBy(sizeScroll, TRUE);
        break;
      }
    }
  }
		
	*pResult = 0;
}

void CFieldForm::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
  dc.SetTextAlign(TA_LEFT|TA_TOP);

  CWnd* pWnd = GetDlgItem(IDC_LAYER);
  if (IsWindow(*pWnd))
  {
    CActField* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    CRect rc;
    pWnd->GetWindowRect(&rc);
    ScreenToClient(&rc);

    CFont ft;
    VERIFY(ft.CreatePointFont(80, "Arial", &dc));
    CFont *pFt = dc.SelectObject(&ft);
    CSize sz = dc.GetTextExtent("XXX", 3);
    int n = rc.Width() / sz.cx;
    for (int i = 0; i <= n; ++i)
    {
      int layer = 1 + (pDoc->cz-1) * i / n;
      int x = rc.left + (rc.right - rc.left) * i / n;

      CString str;
      str.Format("%d", layer);
      sz = dc.GetTextExtent(str);
      dc.TextOut(x-sz.cx/2, rc.top+4, str);

      dc.MoveTo(x, rc.top);
      dc.LineTo(x, rc.top+2);
    }

    dc.MoveTo(rc.left, rc.top);
    dc.LineTo(rc.right, rc.top);

    if (pFt) dc.SelectObject(pFt);
  }
}

void CFieldForm::OnReleasedcaptureSlider1(NMHDR* pNMHDR, LRESULT* pResult) 
{
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  m_nBed = m_ctrlBed.GetPos();
  if (UpdateData(FALSE) && pDoc->level+1 != m_nBed)
  {
    CWaitCursor wait;
    pDoc->level = m_nBed-1;
    pDoc->UpdateSelection();
  }
	
	*pResult = 0;
}

void CFieldForm::OnKillfocusEdit1() 
{
  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  if (UpdateData(TRUE))
  {
    CWaitCursor wait;
    pDoc->level = m_nBed-1;
    m_ctrlBed.SetPos(m_nBed);
    pDoc->UpdateSelection();
  }
}

void CFieldForm::OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNM = (NM_UPDOWN*)pNMHDR;	
	*pResult = 0;

  CActField* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  int n = pNM->iPos + pNM->iDelta;
  if (1 <= n && n <= pDoc->cz)
  {
    CWaitCursor wait;
    pDoc->level = n-1;
    m_nBed = n;
    m_ctrlBed.SetPos(n);
    pDoc->UpdateSelection();
  }
}

void CFieldForm::OnSelchangeCombo1() 
{
  int n = m_ctrlProp.GetCurSel();
  GetDocument()->SelectProp(n);
}
