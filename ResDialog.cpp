// ResDialog.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "ResDialog.h"
#include "FieldForm.h"
#include "FieldView.h"
#include "FieldDoc.h"
#include "libh5/hdf5.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <numeric>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BOOL CResGrid::SetCurrentCell(ROWCOL nRow, ROWCOL nCol, UINT flags)
{
  CResDialog* pRes = (CResDialog*) GetParent();
  pRes->SetCurrentCell(nRow, nCol);
  return CGXGridWnd::SetCurrentCell(nRow, nCol, flags);
}

/////////////////////////////////////////////////////////////////////////////
// CResConstr dialog

// #include <OxBrowseDirEdit.h>
#include <Toolkit\browedit.h>

class CResConstr : public CDialog
{
// Construction
public:
	CResConstr(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CResConstr)
	enum { IDD = IDD_RESCONSTR };
	SECBrowseFileEdit	m_ctrlBase;
	SECBrowseFileEdit	m_ctrlTop;
	CListBox	m_ctrlList;
	CString	m_strName;
	//}}AFX_DATA

  CResDialog::DataList m_listData;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResConstr)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResConstr)
	virtual BOOL OnInitDialog();
	afx_msg void OnUpdate();
	afx_msg void OnRemove();
	afx_msg void OnDestroy();
	afx_msg void OnSelchangeList1();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

static BOOL HasDataset(const char* name)
{
  CFieldForm* pFieldForm = GetFieldForm();
  CActField* pDoc = pFieldForm->GetDocument();

  H5E_auto_t e_func;
  void* e_data;
  herr_t err = H5Eget_auto(&e_func, &e_data);
  err = H5Eset_auto(0, 0);
  hid_t id = H5Dopen(pDoc->hdf5_model, name);
  err = H5Eset_auto(e_func, e_data);
  if (id >= 0) H5Dclose(id);

  return (id >= 0);
}

/////////////////////////////////////////////////////////////////////////////
// CResConstr dialog


CResConstr::CResConstr(CWnd* pParent /*=NULL*/)
	: CDialog(CResConstr::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResConstr)
	m_strName = _T("");
	//}}AFX_DATA_INIT
}


void CResConstr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResConstr)
	DDX_Control(pDX, IDC_EDIT6, m_ctrlBase);
	DDX_Control(pDX, IDC_EDIT2, m_ctrlTop);
	DDX_Control(pDX, IDC_LIST1, m_ctrlList);
	DDX_Text(pDX, IDC_EDIT1, m_strName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResConstr, CDialog)
	//{{AFX_MSG_MAP(CResConstr)
	ON_BN_CLICKED(ID_UPDATE, OnUpdate)
	ON_BN_CLICKED(ID_REMOVE, OnRemove)
	ON_WM_DESTROY()
	ON_LBN_SELCHANGE(IDC_LIST1, OnSelchangeList1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResConstr message handlers

BOOL CResConstr::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  for (CResDialog::DataList::iterator it = m_listData.begin(); it != m_listData.end(); ++it)
  {
    CResDialog::DataItem* data = *it;
    int n = m_ctrlList.AddString(data->name);
    m_ctrlList.SetItemDataPtr(n, data);
  }
  
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CResConstr::OnDestroy() 
{
  m_listData.clear();
  int n = m_ctrlList.GetCount();
  for (int i = 0; i < n; ++i)
  {
    CResDialog::DataItem* data = (CResDialog::DataItem*) m_ctrlList.GetItemDataPtr(i);
    m_listData.push_back(data);
  }
	CDialog::OnDestroy();
}

static bool read_surfer_grid(const CFieldDoc* pDoc, const char* name, float* ff)
{
  char ch[4];
  std::ifstream in(name, std::ios::binary);
  if (!in.read(ch, sizeof(ch)))
    return false;

  if (strncmp(ch, "DSBB", 4) == 0)
  {
    short cx, cy;
    double x1, x2, y1, y2, z1, z2;
    if (!in.read((char*) &cx, sizeof(cx)))
      return false;
    if (cx != pDoc->cx)
      return false;
    if (!in.read((char*) &cy, sizeof(cy)))
      return false;
    if (cy != pDoc->cy)
      return false;
    if (!in.read((char*) &x1, sizeof(x1)))
      return false;
    if (x1 != pDoc->x1)
      return false;
    if (!in.read((char*) &x2, sizeof(x2)))
      return false;
    if (x2 != pDoc->x2)
      return false;
    if (!in.read((char*) &y1, sizeof(y1)))
      return false;
    if (y1 != pDoc->y1)
      return false;
    if (!in.read((char*) &y2, sizeof(y2)))
      return false;
    if (y2 != pDoc->y2)
      return false;
    if (!in.read((char*) &z1, sizeof(z1)))
      return false;
    if (z1 != pDoc->z1)
      return false;
    if (!in.read((char*) &z2, sizeof(z2)))
      return false;
    if (z2 != pDoc->z2)
      return false;
    if (!in.read((char*) ff, cx * cy * sizeof(float)))
      return false;
  }
  else if (strncmp(ch, "DSAA", 4) == 0)
  {
    std::ifstream in(name);
    std::string ch;
    short cx, cy;
    double x1, x2, y1, y2, z1, z2;
    if (in >> ch >> cx >> cy >> x1 >> x2 >> y1 >> y2 >> z1 >> z2)
    {
      if (cx == pDoc->cx && cy == pDoc->cy && x1 == pDoc->x1 
        && x2 == pDoc->x2 && y1 == pDoc->y1 && y2 == pDoc->y2)
      {
        int n = std::copy(std::istream_iterator<float>(in), std::istream_iterator<float>(), ff) - ff;
        return (n == cx * cy);
      }
    }
    return false;
  }
  else
    return false;
  return true;
}

static float id(float x)
{
  return x;
}

static bool SaturatedH(const CActField* pDoc, const char* phase, float* data)
{
	return false;
#if 1
  void* e_data;
  H5E_auto_t e_func;
  herr_t err = H5Eget_auto(&e_func, &e_data);
  H5Eset_auto(0, 0);

  hid_t dset = H5Dopen(pDoc->hdf5_model, phase);
  H5Eset_auto(e_func, e_data);
  if (dset < 0)
    return false;

  for (int j = 0, n = 0; j < pDoc->cy; ++j)
  {
    for (int i = 0; i < pDoc->cx; ++i)
    {
      double xx[3], pp[3];
      xx[0] = pDoc->x1 + (pDoc->x2 - pDoc->x1) * i / (pDoc->cx - 1);
      xx[1] = pDoc->y1 + (pDoc->y2 - pDoc->y1) * j / (pDoc->cy - 1);
      xx[2] = data[n];
      int flags = pDoc->Cell(xx, pp);
      if (flags & CFieldDoc::TOP_SIDE)
      {
        pp[0] = i;
        pp[1] = j;
        pp[2] = pDoc->cz-1;
      }
      else if (flags & CFieldDoc::BOTTOM_SIDE)
      {
        pp[0] = i;
        pp[1] = j;
        pp[2] = 0;
      }
      data[n++] = pDoc->Value(pp, dset, id);
    }
  }
  err = H5Dclose(dset);
  return true;
#endif
}

void CResConstr::OnUpdate() 
{
  CWaitCursor wait;
  VERIFY(UpdateData(TRUE));

  if (m_strName.IsEmpty())
  {
    AfxMessageBox(_T("You should give a constraint name"));
    GetDlgItem(IDC_EDIT1)->SetFocus();
    return;
  }

  CString strTop;
	m_ctrlTop.GetWindowText(strTop);
  {
    CFile file;
    if (strTop.IsEmpty() || !file.Open(strTop, CFile::modeRead))
    {
      AfxMessageBox(_T("You should define a valid pathname for top surface"));
      m_ctrlTop.SetFocus();
      return;
    }
  }

  CString strBot;
	m_ctrlBase.GetWindowText(strBot);
  {
    CFile file;
    if (strBot.IsEmpty() || !file.Open(strBot, CFile::modeRead))
    {
      AfxMessageBox(_T("You should define a valid pathname for base surface"));
      m_ctrlBase.SetFocus();
      return;
    }
  }

  CActField* pDoc = GetFieldForm()->GetDocument();
  int nn = pDoc->cx * pDoc->cy;
  std::valarray<float> top(nn), bot(nn);
  if (!read_surfer_grid(pDoc, strTop, &top[0]))
  {
    AfxMessageBox(_T("Failed to load top surface as text or binary surfer grid"));
    m_ctrlTop.SetFocus();
    return;
  }
  if (!read_surfer_grid(pDoc, strBot, &bot[0]))
  {
    AfxMessageBox(_T("Failed to load base surface as text or binary surfer grid"));
    m_ctrlBase.SetFocus();
    return;
  }

  CResDialog::DataItem* data = 0;
  int n = m_ctrlList.FindStringExact(-1, m_strName);
  if (n != LB_ERR)
  {
    data = (CResDialog::DataItem*) m_ctrlList.GetItemDataPtr(n);
  }
  else
  {
    int nn = pDoc->cx * pDoc->cy;
    data = new CResDialog::DataItem;
    if (HasDataset("reserves/soil"))
    {
      data->oil = new float[2+nn];
      data->scale_o = 1.0f;
    }
    else
    {
      data->oil = 0;
    }
    if (HasDataset("reserves/sgas"))
    {
      data->gas = new float[2+nn];
      data->scale_g = 1.0;
    }
    else
    {
      data->gas = 0;
    }
    int n = m_ctrlList.AddString(m_strName);
    m_ctrlList.SetItemDataPtr(n, data);
  }
  strcpy(data->name, m_strName);
  strcpy(data->top, strTop);
  strcpy(data->base, strBot);

  // now we are ready to calculate...
  if (data->oil)
  {
    std::valarray<float> s1(&top[0], nn), s2(&bot[0], nn);
    float *p1 = &s1[0], *p2 = &s2[0];
    VERIFY(SaturatedH(pDoc, "reserves/soil", p1));
    VERIFY(SaturatedH(pDoc, "reserves/soil", p2));
    float* p = data->oil+2;
    for (int i = 0; i < nn; ++i)
    {
      float ds = *p2++ - *p1++;
      VERIFY(ds >= 0.0f);
      *p++ = ds;
    }
    data->oil[0] = *std::min_element(&s1[0]+2, &s1[0]+2+nn);
    data->oil[1] = *std::max_element(&s2[0]+2, &s2[0]+2+nn);
  }
  if (data->gas)
  {
    std::valarray<float> s1(&top[0], nn), s2(&bot[0], nn);
    float *p1 = &s1[0], *p2 = &s2[0];
    VERIFY(SaturatedH(pDoc, "reserves/sgas", p1));
    VERIFY(SaturatedH(pDoc, "reserves/sgas", p2));
    float* p = data->gas+2;
    for (int i = 0; i < nn; ++i)
    {
      float ds = *p2++ - *p1++;
      VERIFY(ds >= 0.0f);
      *p++ = ds;
    }
    data->gas[0] = *std::min_element(&s1[0]+2, &s1[0]+2+nn);
    data->gas[1] = *std::max_element(&s2[0]+2, &s2[0]+2+nn);
  }
}

void CResConstr::OnRemove() 
{
  int n = m_ctrlList.GetCurSel();
  if (n == LB_ERR)
    return;

  CResDialog::DataItem* data = (CResDialog::DataItem*) m_ctrlList.GetItemDataPtr(n);
  delete[] data->oil;
  delete[] data->gas;
  delete[] data;

  m_ctrlList.DeleteString(n);
}

void CResConstr::OnSelchangeList1() 
{
  int n = m_ctrlList.GetCurSel();
  if (n == LB_ERR)
    return;

  CResDialog::DataItem* data = (CResDialog::DataItem*) m_ctrlList.GetItemDataPtr(n);
  m_strName = data->name;
  m_ctrlTop.SetWindowText(data->top);
  m_ctrlBase.SetWindowText(data->base);
  UpdateData(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CResDialog dialog


CResDialog::CResDialog(): m_pOilTabl(0), m_pGasTabl(0)
{
  m_pDoc = 0;
  m_nOilCol = m_nGasCol = 0;

	//{{AFX_DATA_INIT(CResDialog)
	m_sOilDensity = 0.0f;
	m_sShrinkingFactor = 0.0f;
	m_sBaricCoef = 0.0f;
	m_sThermalFactor = 0.0f;
	//}}AFX_DATA_INIT
}

BOOL CResDialog::IsAvailable() const
{
  CFieldDoc* pDoc = GetFieldForm()->GetDocument();
  return (HasDataset("reserves/soil") || HasDataset("reserves/sgas"));
}

void CResDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResDialog)
	DDX_Text(pDX, IDC_EDIT1, m_sOilDensity);
	DDX_Text(pDX, IDC_EDIT3, m_sShrinkingFactor);
	DDX_Text(pDX, IDC_EDIT5, m_sBaricCoef);
	DDX_Text(pDX, IDC_EDIT4, m_sThermalFactor);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CResDialog, CDialog)
	//{{AFX_MSG_MAP(CResDialog)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(ID_EDIT, OnEdit)
	ON_WM_DESTROY()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResDialog message handlers

BOOL CResDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

  m_wndGrid.SubclassDlgItem(IDC_CUSTOM1, this);
  m_wndGrid.Initialize();
  m_wndGrid.GetParam()->EnableUndo();
  m_wndGrid.SetReadOnly();
  m_wndGrid.GetParam()->EnableUndo(TRUE);
  m_wndGrid.GetParam()->SetNewGridLineMode(TRUE);
  m_wndGrid.GetParam()->EnableSelection(GX_SELNONE);
  m_wndGrid.GetParam()->EnableThumbTrack(TRUE);

	CGXProperties* pProp = m_wndGrid.GetParam()->GetProperties();
	pProp->SetColor(GX_COLOR_BACKGROUND, GetSysColor(COLOR_BTNFACE));
  pProp->SetUserProperty(GX_IDS_OUTLINECURRENTCELL, \
    (CGXStyle) pProp->sInvertThick);

  m_nRowSel = m_nColSel = 1;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

static CActField::Palette* MakeTabl(float* ff)
{
  CActField::Palette* clut = new CActField::Palette;

  const int n = 5;
  clut->nz = n;
  clut->zz = new float[n+2];
  clut->sh = new CActField::Color[n+2];

  clut->sh[0].r = 0.0, clut->sh[0].g = 0.0, clut->sh[0].b = 0.5;
  clut->sh[1].r = 0.0, clut->sh[1].g = 0.0, clut->sh[1].b = 1.0;
  clut->sh[2].r = 0.0, clut->sh[2].g = 1.0, clut->sh[2].b = 1.0;
  clut->sh[3].r = 0.0, clut->sh[3].g = 1.0, clut->sh[3].b = 0.0;
  clut->sh[4].r = 1.0, clut->sh[4].g = 1.0, clut->sh[4].b = 0.0;
  clut->sh[5].r = 1.0, clut->sh[5].g = 0.0, clut->sh[5].b = 0.0;
  clut->sh[6].r = 1.0, clut->sh[6].g = 0.5, clut->sh[6].b = 0.5;

  clut->fwd = clut->bck = id;

  float f1 = clut->fwd(ff[0]);
  float f2 = clut->fwd(ff[1]);
  for (int i = 0; i <= (n+1); ++i) 
    clut->zz[i] = f1 + i * (f2 - f1) / (n+1);

  return clut;
}

void CResDialog::Initialize(CActField* pDoc)
{
#if 1
  int nCol = 0;
  if (!HasDataset("reserves/soil"))
  {
    GetDlgItem(IDC_EDIT4)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT5)->EnableWindow(FALSE);
  }
  else
  {
    m_nOilCol = ++nCol;
  }

  if (!HasDataset("reserves/sgas"))
  {
    GetDlgItem(IDC_EDIT1)->EnableWindow(FALSE);
    GetDlgItem(IDC_EDIT3)->EnableWindow(FALSE);
  }
  else
  {
    m_nGasCol = ++nCol;
  }
  ASSERT(nCol);
  m_pDoc = pDoc;

  DataItem* data = new DataItem;
  strcpy(data->name, "total");
  data->top[0] = data->base[0] = '\0';
  data->oil = data->gas = 0;
  
  m_wndGrid.GetParam()->SetLockReadOnly(FALSE);
  BOOL bOldLock = m_wndGrid.LockUpdate(TRUE);
  m_wndGrid.SetColCount(nCol);
  if (m_nGasCol > 0)
  {
    m_sThermalFactor = 1.0f;
    m_sBaricCoef = 1.0f;

    int nn = pDoc->cx * pDoc->cy;
    float* ff = new float[nn+2];
    hid_t dset = H5Dopen(pDoc->hdf5_model, "reserves/sgas");
    hid_t dspace = H5Dget_space(dset);
    hsize_t start[3] = { pDoc->cz-1, 0, 0 };
    hsize_t count[3] = { 1, pDoc->cy, pDoc->cx };
    herr_t err = H5Sselect_hyperslab(dspace, H5S_SELECT_SET, \
      start, NULL, count, NULL);
    count[0] = nn;
    hid_t mspace = H5Screate_simple(1, count, NULL);
    err = H5Dread(dset, H5T_NATIVE_FLOAT, mspace, dspace, \
      H5P_DEFAULT, ff+2);
    err = H5Sclose(mspace);
    err = H5Sclose(dspace);
    err = H5Dclose(dset);

    ff[0] = *std::min_element(ff+2, ff+2+nn);
    ff[1] = *std::max_element(ff+2, ff+2+nn);
    data->gas = ff;
    data->scale_g = 1.0f;

    m_wndGrid.SetStyleRange(CGXRange(0, m_nGasCol), CGXStyle().SetValue("gas"));
  }

  if (m_nOilCol > 0)
  {
    m_sOilDensity = 0.866f;
    m_sShrinkingFactor = 0.852f;

    int nn = pDoc->cx * pDoc->cy;
    float* ff = new float[nn+2];
    hid_t dset = H5Dopen(pDoc->hdf5_model, "reserves/soil");
    hid_t dspace = H5Dget_space(dset);
    hsize_t start[3] = { pDoc->cz-1, 0, 0 };
    hsize_t count[3] = { 1, pDoc->cy, pDoc->cx };
    herr_t err = H5Sselect_hyperslab(dspace, H5S_SELECT_SET, \
      start, NULL, count, NULL);
    count[0] = nn;
    hid_t mspace = H5Screate_simple(1, count, NULL);
    err = H5Dread(dset, H5T_NATIVE_FLOAT, mspace, dspace, \
      H5P_DEFAULT, ff+2);
    err = H5Sclose(mspace);
    err = H5Sclose(dspace);
    err = H5Dclose(dset);

    ff[0] = *std::min_element(ff+2, ff+2+nn);
    ff[1] = *std::max_element(ff+2, ff+2+nn);
    data->oil = ff;
    data->scale_o = 1.0f;

    m_wndGrid.SetStyleRange(CGXRange(0, m_nOilCol), CGXStyle().SetValue("oil, t."));
  }

  m_listData.push_back(data);
  m_wndGrid.LockUpdate(bOldLock);
  m_wndGrid.GetParam()->SetLockReadOnly(TRUE);
  RecalcData();
#endif
}

void CResDialog::RecalcData()
{
  VERIFY(UpdateData(FALSE));
  int n = m_listData.size();
  m_wndGrid.GetParam()->SetLockReadOnly(FALSE);
  BOOL bOldLock = m_wndGrid.LockUpdate(TRUE);
  m_wndGrid.SetRowCount(n);
  DataList::iterator it = m_listData.begin();
  int nn = m_pDoc->cx * m_pDoc->cy;
  for (int i = 1; i <= n; ++i)
  {
    DataItem* data = *it;
    if (data->gas)
    {
      float scale_g = m_sThermalFactor * m_sBaricCoef;

      std::transform(data->gas+2, data->gas+2+nn, data->gas+2, \
        std::bind2nd(std::multiplies<float>(), scale_g / data->scale_g));
      data->gas[0] = *std::min_element(data->gas+2, data->gas+2+nn);
      data->gas[1] = *std::max_element(data->gas+2, data->gas+2+nn);
      double total = std::accumulate(data->gas+2, data->gas+2+nn, 0.0f) \
        * (m_pDoc->x2 - m_pDoc->x1) / (m_pDoc->cx - 1) \
        * (m_pDoc->y2 - m_pDoc->y1) / (m_pDoc->cy - 1);
      data->scale_g = scale_g;

      if (m_pGasTabl)
      {
        delete[] m_pGasTabl->sh;
        delete[] m_pGasTabl->zz;
        delete m_pGasTabl;
      }
      m_pGasTabl = MakeTabl(data->gas);

      m_wndGrid.SetStyleRange(CGXRange(i, m_nGasCol), CGXStyle().SetValue(total));
    }
    if (data->oil)
    {
      // thousand tons per meter ** 2
      float scale_o = m_sOilDensity * m_sShrinkingFactor;

      std::transform(data->oil+2, data->oil+2+nn, data->oil+2, \
        std::bind2nd(std::multiplies<float>(), scale_o / data->scale_o));
      data->oil[0] = *std::min_element(data->oil+2, data->oil+2+nn);
      data->oil[1] = *std::max_element(data->oil+2, data->oil+2+nn);
      double total = std::accumulate(data->oil+2, data->oil+2+nn, 0.0f) \
        * (m_pDoc->x2 - m_pDoc->x1) / (m_pDoc->cx - 1) \
        * (m_pDoc->y2 - m_pDoc->y1) / (m_pDoc->cy - 1);
      data->scale_o = scale_o;

      if (m_pOilTabl)
      {
        delete[] m_pOilTabl->sh;
        delete[] m_pOilTabl->zz;
        delete m_pOilTabl;
      }
      m_pOilTabl = MakeTabl(data->oil);

      m_wndGrid.SetStyleRange(CGXRange(i, m_nOilCol), CGXStyle().SetValue(total));
    }
    m_wndGrid.SetStyleRange(CGXRange(i, 0), CGXStyle().SetValue(data->name));
    ++it;
  }

  m_wndGrid.LockUpdate(bOldLock);
  m_wndGrid.GetParam()->SetLockReadOnly(TRUE);
  SetCurrentCell(m_nRowSel, m_nColSel);
}

void CResDialog::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

  CFieldForm* pFieldForm = GetFieldForm();
  pFieldForm->GetDlgItem(IDC_SLIDER1)->EnableWindow(!bShow);
  pFieldForm->GetDlgItem(IDC_EDIT1)->EnableWindow(!bShow);
  pFieldForm->GetDlgItem(IDC_SPIN1)->EnableWindow(!bShow);
  pFieldForm->GetDlgItem(IDC_COMBO1)->EnableWindow(!bShow);

  if (bShow)
  {
    if (!m_pDoc)
      Initialize(pFieldForm->GetDocument());
  }

  // CFieldView* pView = GetFieldView();
  // pView->m_wndColorPalette.Invalidate();
  // pView->Invalidate();
  m_pDoc->UpdateAllViews(0, IDD);
}

void CResDialog::OnOK() 
{
  OnShowWindow(FALSE, 0);
	CDialog::OnOK();
}

void CResDialog::OnCancel() 
{
  OnShowWindow(FALSE, 0);
	CDialog::OnCancel();
}

void CResDialog::OnEdit() 
{
  CResConstr dlg(this);

  int nn = m_pDoc->cx * m_pDoc->cy;
  DataList::iterator it = m_listData.begin();
  while (++it != m_listData.end())
  {
    DataItem* orig = *it;
    DataItem* data = new DataItem;
    strcpy(data->name, orig->name);
    strcpy(data->top, orig->top);
    strcpy(data->base, orig->base);
    if (orig->oil)
    {
      data->oil = new float[nn]; std::copy(orig->oil, orig->oil+nn, data->oil);
    }
    else
    {
      data->oil = 0;
    }
    if (orig->gas)
    {
      data->gas = new float[nn]; std::copy(orig->gas, orig->gas+nn, data->gas);
    }
    else
    {
      data->gas = 0;
    }
    dlg.m_listData.push_back(data);
  }

  if (dlg.DoModal() == IDOK)
  {
    it = ++m_listData.begin();
    while (it != m_listData.end())
    {
      DataItem* data = *it;
      delete[] data->oil;
      delete[] data->gas;
      delete data;
      it = m_listData.erase(it);
    }

    for (it = dlg.m_listData.begin(); it != dlg.m_listData.end(); ++it)
    {
      DataItem* data = *it;
      m_listData.push_back(data);
    }
    RecalcData();
  }
  else
  {
    for (it = dlg.m_listData.begin(); it != dlg.m_listData.end(); ++it)
    {
      DataItem* data = *it;
      delete[] data->oil;
      delete[] data->gas;
      delete data;
    }
  }
}

void CResDialog::OnDestroy() 
{
	CDialog::OnDestroy();

  for(DataList::iterator it = m_listData.begin(); it != m_listData.end(); ++it)
  {
    DataItem *data = *it;
    delete[] data->oil;
    delete[] data->gas;
    delete data;
  }

  if (m_pOilTabl)
  {
    delete[] m_pOilTabl->sh;
    delete[] m_pOilTabl->zz;
    delete m_pOilTabl;
  }

  if (m_pGasTabl)
  {
    delete[] m_pGasTabl->sh;
    delete[] m_pGasTabl->zz;
    delete m_pGasTabl;
  }
}

void CResDialog::SetCurrentCell(int nRow, int nCol)
{
  if (nRow == 0 || nCol == 0)
    return;

  if ((ROWCOL) m_nRowSel > m_wndGrid.GetRowCount() \
    || (ROWCOL) m_nColSel > m_wndGrid.GetColCount())
  {
    m_nRowSel = m_nColSel = 1;
  }
  else
  {
    m_nRowSel = nRow;
    m_nColSel = nCol;
  }

  m_wndGrid.GetParam()->SetLockReadOnly(FALSE);
  BOOL bOldLock = m_wndGrid.LockUpdate(TRUE);

  m_wndGrid.SetStyleRange(CGXRange(1,1,m_wndGrid.GetRowCount(),m_wndGrid.GetColCount()), 
    CGXStyle()
    .SetControl(GX_IDS_CTRL_STATIC)
    .SetReadOnly(TRUE)
    .SetFont(CGXFont( ).SetBold(FALSE).SetSize(8))
    .SetInterior(RGB(192,192,192))   // light grey
    .SetHorizontalAlignment(DT_CENTER)
    .SetVerticalAlignment(DT_VCENTER)
  );

  m_wndGrid.SetStyleRange(CGXRange(m_nRowSel, m_nColSel), 
    CGXStyle().SetFont(CGXFont( ).SetBold(TRUE).SetSize(8))
  );

  m_wndGrid.LockUpdate(bOldLock);
  m_wndGrid.GetParam()->SetLockReadOnly(TRUE);
  m_wndGrid.Redraw();

  m_pDoc->UpdateAllViews(GetFieldForm(), IDD);
}

void CResDialog::OnFileOpen() 
{
  CFileDialog dlg(TRUE);
  if (dlg.DoModal() == IDOK)
  {
    CWaitCursor wait;

    CFile file;
    if (file.Open(dlg.GetPathName(), CFile::modeWrite|CFile::typeBinary))
    {
      int nn = (m_pDoc->cx * m_pDoc->cy + 2) * sizeof(float);
      int n = m_listData.size();
      file.Write(&n, sizeof(n));
      for (DataList::iterator it = m_listData.begin(); it != m_listData.end(); ++it)
      {
        DataItem* data = *it;
        file.Write(data->name, sizeof(data->name));
        file.Write(data->top, sizeof(data->top));
        file.Write(data->base, sizeof(data->base));
        file.Write(&data->scale_o, sizeof(data->scale_o));
        file.Write(&data->scale_g, sizeof(data->scale_g));
        if (data->oil)
        {
          file.Write(&nn, sizeof(nn));
          file.Write(data->oil, nn);
        }
        else
        {
          int nn = 0;
          file.Write(&nn, sizeof(nn));
        }
        if (data->gas)
        {
          file.Write(&nn, sizeof(nn));
          file.Write(data->gas, nn);
        }
        else
        {
          int nn = 0;
          file.Write(&nn, sizeof(nn));
        }
      }
    }
  }
}

void CResDialog::OnFileSave() 
{
  CWaitCursor wait;
  CFileDialog dlg(FALSE);
  if (dlg.DoModal() == IDOK)
  {
    CFile file;
    if (file.Open(dlg.GetPathName(), CFile::modeRead|CFile::typeBinary))
    {
      while (!m_listData.empty())
      {
        DataItem* data = m_listData.back();
        m_listData.pop_back();
        delete[] data->oil;
        delete[] data->gas;
        delete data;
      }

      int n;
      file.Read(&n, sizeof(n));
      for (int i = 0; i < n; ++i)
      {
        DataItem* data = new DataItem;
        file.Read(data->name, sizeof(data->name));
        file.Read(data->top, sizeof(data->top));
        file.Read(data->base, sizeof(data->base));
        file.Read(&data->scale_o, sizeof(data->scale_o));
        file.Read(&data->scale_g, sizeof(data->scale_g));

        int nn;
        file.Read(&nn, sizeof(nn));
        if (nn > 0)
        {
          data->oil = new float[nn / sizeof(float)];
          file.Read(data->oil, nn);
        }
        else
        {
          data->oil = 0;
        }

        file.Read(&nn, sizeof(nn));
        if (nn > 0)
        {
          data->gas = new float[nn / sizeof(float)];
          file.Read(data->gas, nn);
        }

        m_listData.push_back(data);
      }
      RecalcData();
    }
  }
}
