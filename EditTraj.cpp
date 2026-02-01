// EditTraj.cpp : implementation file
//

#include "stdafx.h"
#include "Field.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "EditTraj.h"
#include "libh5/hdf5.h"
#include "valarray"
#include "oledberr.h"
#include ".\edittraj.h"

#ifndef countof
#define countof(arr) (sizeof(arr) / sizeof(*arr))
#endif

#ifndef offsetof
#define offsetof(type, field) ((size_t)(&((type*)0)->field))
#endif

#ifndef filedsize
#define fieldsize(type, field) (sizeof(((type*)0)->field))
#endif

#define STRINGIZE(lex) #lex
#define STRINGIZE2(lex) STRINGIZE(lex)
#define FILEPOS TEXT(__FILE__) TEXT("(") TEXT(STRINGIZE2(__LINE__)) TEXT("): ")

// CEditTraj dialog

IMPLEMENT_DYNAMIC(CEditTraj, CDialog)
CEditTraj::CEditTraj(CActField* doc, long bore, CWnd* pParent /*=NULL*/)
	: CDialog(CEditTraj::IDD, pParent)
	, m_pDoc(doc)
	, m_nBore(bore)
{
}

CEditTraj::~CEditTraj()
{
}

void CEditTraj::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_wndList);
	DDX_Control(pDX, IDC_EDIT2, m_wndDX);
	DDX_Control(pDX, IDC_EDIT6, m_wndDY);
	DDX_Control(pDX, IDC_EDIT7, m_wndDZ);
	DDX_Control(pDX, IDC_EDIT8, m_wndMD);
	DDX_Control(pDX, IDC_EDIT1, m_wndWellbore);
	DDX_Control(pDX, IDC_EDIT3, m_wndParentbore);
}


BEGIN_MESSAGE_MAP(CEditTraj, CDialog)
//	ON_WM_CHAR()
//ON_EN_CHANGE(IDC_EDIT1, OnEnChangeEdit1)
ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LIST1, OnLvnItemActivateList1)
ON_BN_CLICKED(IDC_BUTTON5, OnBnClickedButton5)
ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
END_MESSAGE_MAP()


// CEditTraj message handlers

void CFieldView::OnInclinometry()
{
	CEditTraj dlg(GetDocument(), m_nBoreSelected, this);
	dlg.DoModal();
}

BOOL CEditTraj::OnInitDialog()
{
	CDialog::OnInitDialog();

  m_wndList.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES);
  m_wndList.InsertColumn(0, "dx");
  m_wndList.InsertColumn(1, "dy");
  m_wndList.InsertColumn(2, "dz");
  m_wndList.InsertColumn(3, "md");

	CRect rc;
	m_wndList.GetClientRect(rc);
	m_wndList.SetColumnWidth(0, rc.Width() / 4);
	m_wndList.SetColumnWidth(1, rc.Width() / 4);
	m_wndList.SetColumnWidth(2, rc.Width() / 4);
	m_wndList.SetColumnWidth(3, (rc.Width()+3) / 4);

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

  CComQIPtr < IRowsetIndex > well_index;
  CTable < CAccessor < WellRecord > > well;
  HRESULT hr = well.Open(m_pDoc->session, _T("well"), &dbPropSet);
  if (SUCCEEDED(hr))
  {
    CComQIPtr < IRowsetCurrentIndex > spRowsetCurrentIndex = well.m_spRowset;
    if (spRowsetCurrentIndex != NULL)
    {
      DBID IndexID;
      IndexID.eKind = DBKIND_NAME;
      IndexID.uName.pwszName = L"well_bore";
      hr = spRowsetCurrentIndex->SetIndex(&IndexID);
      if (SUCCEEDED(hr))
      {
        well_index = well.m_spRowset;
      }
    }
  }

	well.well_bore = m_nBore;
	if (!well_index || S_OK != well_index->Seek(well.GetHAccessor(0), \
		1, (WellRecord*) &well, DBSEEK_FIRSTEQ) || S_OK != well.MoveNext())
	{
		return FALSE;
	}

	CString str;
	str.Format(_T("%d"), (long) well.well_bore);
	m_wndWellbore.SetWindowText(str);
	if (well.status.parent_bore == DBSTATUS_S_OK)
	{
		str.Format(_T("%d"), (long) well.parent_bore);
		m_wndParentbore.SetWindowText(str);
	}

  CComQIPtr < IRowsetIndex > traj_index;
  CTable < CAccessor < TrajRecord > > traj;
  hr = traj.Open(m_pDoc->session, _T("traj"), &dbPropSet);
  if (SUCCEEDED(hr))
  {
    CComQIPtr < IRowsetCurrentIndex > spRowsetCurrentIndex = traj.m_spRowset;
    if (spRowsetCurrentIndex != NULL)
    {
      DBID IndexID;
      IndexID.eKind = DBKIND_NAME;
      IndexID.uName.pwszName = L"bore_id";
      hr = spRowsetCurrentIndex->SetIndex(&IndexID);
      if (SUCCEEDED(hr))
      {
        traj_index = traj.m_spRowset;
      }
    }
  }

	traj.traj_wb = m_nBore;
	traj.traj_id = 1;
	if (!traj_index || S_OK != traj_index->Seek(traj.GetHAccessor(0), \
		2, (TrajRecord*) &traj, DBSEEK_FIRSTEQ) || S_OK != traj.MoveNext())
	{
		return FALSE;
	}

	if (traj.traj_ns > 0)
	{
		H5E_auto_t e_func;
		void* e_data;
		herr_t err = H5Eget_auto(&e_func, &e_data);
		err = H5Eset_auto(0, 0);
		str.Format(_T("/traj/version_%d/%d"), m_nBore);
		hid_t dataset = H5Dopen(m_pDoc->hdf5_file, str);
		err = H5Eset_auto(e_func, e_data);
		if (0 > dataset)
		{
			return FALSE;
		}

		TrajSurvey* ss = new TrajSurvey[traj.traj_ns];
		err = H5Dread(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ss);
		if (0 > err)
		{
			H5Dclose(dataset);
			delete ss;
			return FALSE;
		}
		for (int i = 0; i < traj.traj_ns; ++i)
		{
			str.Format(_T("%g"), ss[i].dx);
			m_wndList.InsertItem(i, str);
			str.Format(_T("%g"), ss[i].dy);
			m_wndList.SetItemText(i, 1, str);
			str.Format(_T("%g"), ss[i].dz-well.well_elev);
			m_wndList.SetItemText(i, 2, str);
			str.Format(_T("%g"), ss[i].md);
			m_wndList.SetItemText(i, 3, str);
		}
		delete[] ss;

		str.Format(_T("%g"), traj.tail.dx);
		m_wndList.InsertItem(i, str);
		str.Format(_T("%g"), traj.tail.dy);
		m_wndList.SetItemText(i, 1, str);
		str.Format(_T("%g"), traj.tail.dz-well.well_elev);
		m_wndList.SetItemText(i, 2, str);
		str.Format(_T("%g"), traj.tail.md);
		m_wndList.SetItemText(i, 3, str);
	}
	else
	{
		str.Format(_T("%g"), traj.tail.dx);
		m_wndList.InsertItem(0, str);
		str.Format(_T("%g"), traj.tail.dy);
		m_wndList.SetItemText(0, 1, str);
		str.Format(_T("%g"), traj.tail.dz-well.well_elev);
		m_wndList.SetItemText(0, 2, str);
		str.Format(_T("%g"), traj.tail.md);
		m_wndList.SetItemText(0, 3, str);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CEditTraj::OnLvnItemActivateList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	m_wndDX.SetWindowText(m_wndList.GetItemText(pNMIA->iItem, 0));
	m_wndDY.SetWindowText(m_wndList.GetItemText(pNMIA->iItem, 1));
	m_wndDZ.SetWindowText(m_wndList.GetItemText(pNMIA->iItem, 2));
	m_wndMD.SetWindowText(m_wndList.GetItemText(pNMIA->iItem, 3));
	*pResult = 0;
}

void CEditTraj::OnBnClickedButton5()
{
	int n = m_wndList.GetNextItem(-1, LVNI_SELECTED);
	m_wndList.SetRedraw(FALSE);
	while (n != -1)
	{
		m_wndList.DeleteItem(n);
		n = m_wndList.GetNextItem(-1, LVNI_SELECTED);
	}
	m_wndList.SetRedraw(TRUE);
}


void CEditTraj::OnBnClickedButton4()
{
	CString str;

	float dx;
	m_wndDX.GetWindowText(str);
	if (1 != sscanf(str, "%g", &dx))
	{
		MessageBox("Please define dx with floating point number");
		m_wndDX.SetFocus();
		return;
	}

	float dy;
	m_wndDY.GetWindowText(str);
	if (1 != sscanf(str, "%g", &dy))
	{
		MessageBox("Please define dy with floating point number");
		m_wndDY.SetFocus();
		return;
	}

	float dz;
	m_wndDZ.GetWindowText(str);
	if (1 != sscanf(str, "%g", &dz))
	{
		MessageBox("Please define dz with floating point number");
		m_wndDZ.SetFocus();
		return;
	}

	float md;
	m_wndMD.GetWindowText(str);
	if (1 != sscanf(str, "%g", &md))
	{
		MessageBox("Please define md with floating point number");
		m_wndMD.SetFocus();
		return;
	}

	CWaitCursor wait;
	int n = m_wndList.GetItemCount();
	while (n--)
	{
		float md_org;
		str = m_wndList.GetItemText(n, 3);
		VERIFY(1 == sscanf(str, "%g", &md_org));

		if (fabs(md_org - md) < 0.4)
		{
			str.Format(_T("%g"), dx);
			m_wndList.SetItemText(n, 0, str);
			str.Format(_T("%g"), dy);
			m_wndList.SetItemText(n, 1, str);
			str.Format(_T("%g"), dz);
			m_wndList.SetItemText(n, 2, str);
			str.Format(_T("%g"), md);
			m_wndList.SetItemText(n, 3, str);
			m_wndList.EnsureVisible(n, FALSE);
			return;
		}
		else if (md > md_org)
		{
			break;
		}
	}
	++n;
	str.Format(_T("%g"), dx);
	m_wndList.InsertItem(n, str);
	str.Format(_T("%g"), dy);
	m_wndList.SetItemText(n, 1, str);
	str.Format(_T("%g"), dz);
	m_wndList.SetItemText(n, 2, str);
	str.Format(_T("%g"), md);
	m_wndList.SetItemText(n, 3, str);
	m_wndList.EnsureVisible(n, FALSE);
}

HRESULT CEditTraj::BindWell(IAccessor* pAccessor, HACCESSOR* pResult)
{
	const cColumns = 24;
	DBBINDING dbBinding[cColumns];
	DBBINDSTATUS dbBindStatus[cColumns];

  dbBinding[0].iOrdinal =    2;
  dbBinding[0].obValue =     offsetof(WellRecord, well_bore);
  dbBinding[0].obLength =    0; 
  dbBinding[0].obStatus =    0;
  dbBinding[0].pTypeInfo =   NULL;
  dbBinding[0].pObject =     NULL;
  dbBinding[0].pBindExt =    NULL;
  dbBinding[0].dwPart =      DBPART_VALUE;
  dbBinding[0].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[0].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[0].cbMaxLen =    fieldsize(WellRecord, well_bore);
  dbBinding[0].dwFlags =     0;
  dbBinding[0].wType =       DBTYPE_I4;
  dbBinding[0].bPrecision =  0;
  dbBinding[0].bScale =      0;

  dbBinding[1].iOrdinal =    1;
  dbBinding[1].obValue =     offsetof(WellRecord, well_name);
  dbBinding[1].obLength =    0; 
  dbBinding[1].obStatus =    0;
  dbBinding[1].pTypeInfo =   NULL;
  dbBinding[1].pObject =     NULL;
  dbBinding[1].pBindExt =    NULL;
  dbBinding[1].dwPart =      DBPART_VALUE;
  dbBinding[1].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[1].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[1].cbMaxLen =    fieldsize(WellRecord, well_name);
  dbBinding[1].dwFlags =     0;
  dbBinding[1].wType =       DBTYPE_STR;
  dbBinding[1].bPrecision =  0;
  dbBinding[1].bScale =      0;

  dbBinding[2].iOrdinal =    3;
  dbBinding[2].obValue =     offsetof(WellRecord, well_dx);
  dbBinding[2].obLength =    0; 
  dbBinding[2].obStatus =    offsetof(WellRecord, status.well_dx);
  dbBinding[2].pTypeInfo =   NULL;
  dbBinding[2].pObject =     NULL;
  dbBinding[2].pBindExt =    NULL;
  dbBinding[2].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[2].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[2].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[2].cbMaxLen =    fieldsize(WellRecord, well_dx);
  dbBinding[2].dwFlags =     0;
  dbBinding[2].wType =       DBTYPE_R4;
  dbBinding[2].bPrecision =  0;
  dbBinding[2].bScale =      0;

  dbBinding[3].iOrdinal =    4;
  dbBinding[3].obValue =     offsetof(WellRecord, well_dy);
  dbBinding[3].obLength =    0; 
  dbBinding[3].obStatus =    offsetof(WellRecord, status.well_dy);
  dbBinding[3].pTypeInfo =   NULL;
  dbBinding[3].pObject =     NULL;
  dbBinding[3].pBindExt =    NULL;
  dbBinding[3].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[3].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[3].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[3].cbMaxLen =    fieldsize(WellRecord, well_dx);
  dbBinding[3].dwFlags =     0;
  dbBinding[3].wType =       DBTYPE_R4;
  dbBinding[3].bPrecision =  0;
  dbBinding[3].bScale =      0;

  dbBinding[4].iOrdinal =    5;
  dbBinding[4].obValue =     offsetof(WellRecord, well_dz);
  dbBinding[4].obLength =    0; 
  dbBinding[4].obStatus =    offsetof(WellRecord, status.well_dz);
  dbBinding[4].pTypeInfo =   NULL;
  dbBinding[4].pObject =     NULL;
  dbBinding[4].pBindExt =    NULL;
  dbBinding[4].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[4].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[4].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[4].cbMaxLen =    fieldsize(WellRecord, well_dz);
  dbBinding[4].dwFlags =     0;
  dbBinding[4].wType =       DBTYPE_R4;
  dbBinding[4].bPrecision =  0;
  dbBinding[4].bScale =      0;

  dbBinding[5].iOrdinal =    6;
  dbBinding[5].obValue =     offsetof(WellRecord, well_md);
  dbBinding[5].obLength =    0; 
  dbBinding[5].obStatus =    offsetof(WellRecord, status.well_md);
  dbBinding[5].pTypeInfo =   NULL;
  dbBinding[5].pObject =     NULL;
  dbBinding[5].pBindExt =    NULL;
  dbBinding[5].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[5].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[5].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[5].cbMaxLen =    fieldsize(WellRecord, well_md);
  dbBinding[5].dwFlags =     0;
  dbBinding[5].wType =       DBTYPE_R4;
  dbBinding[5].bPrecision =  0;
  dbBinding[5].bScale =      0;

  dbBinding[6].iOrdinal =    7;
  dbBinding[6].obValue =     offsetof(WellRecord, well_hx);
  dbBinding[6].obLength =    0; 
  dbBinding[6].obStatus =    0;
  dbBinding[6].pTypeInfo =   NULL;
  dbBinding[6].pObject =     NULL;
  dbBinding[6].pBindExt =    NULL;
  dbBinding[6].dwPart =      DBPART_VALUE;
  dbBinding[6].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[6].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[6].cbMaxLen =    fieldsize(WellRecord, well_hx);
  dbBinding[6].dwFlags =     0;
  dbBinding[6].wType =       DBTYPE_R4;
  dbBinding[6].bPrecision =  0;
  dbBinding[6].bScale =      0;

  dbBinding[7].iOrdinal =    8;
  dbBinding[7].obValue =     offsetof(WellRecord, well_hy);
  dbBinding[7].obLength =    0; 
  dbBinding[7].obStatus =    0;
  dbBinding[7].pTypeInfo =   NULL;
  dbBinding[7].pObject =     NULL;
  dbBinding[7].pBindExt =    NULL;
  dbBinding[7].dwPart =      DBPART_VALUE;
  dbBinding[7].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[7].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[7].cbMaxLen =    fieldsize(WellRecord, well_hy);
  dbBinding[7].dwFlags =     0;
  dbBinding[7].wType =       DBTYPE_R4;
  dbBinding[7].bPrecision =  0;
  dbBinding[7].bScale =      0;

  dbBinding[8].iOrdinal =    9;
  dbBinding[8].obValue =     offsetof(WellRecord, well_type);
  dbBinding[8].obLength =    0; 
  dbBinding[8].obStatus =    0;
  dbBinding[8].pTypeInfo =   NULL;
  dbBinding[8].pObject =     NULL;
  dbBinding[8].pBindExt =    NULL;
  dbBinding[8].dwPart =      DBPART_VALUE;
  dbBinding[8].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[8].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[8].cbMaxLen =    fieldsize(WellRecord, well_type);
  dbBinding[8].dwFlags =     0;
  dbBinding[8].wType =       DBTYPE_I4;
  dbBinding[8].bPrecision =  0;
  dbBinding[8].bScale =      0;

  dbBinding[9].iOrdinal =    10;
  dbBinding[9].obValue =     offsetof(WellRecord, undertaker);
  dbBinding[9].obLength =    0; 
  dbBinding[9].obStatus =    0;
  dbBinding[9].pTypeInfo =   NULL;
  dbBinding[9].pObject =     NULL;
  dbBinding[9].pBindExt =    NULL;
  dbBinding[9].dwPart =      DBPART_VALUE;
  dbBinding[9].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[9].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[9].cbMaxLen =    fieldsize(WellRecord, undertaker);
  dbBinding[9].dwFlags =     0;
  dbBinding[9].wType =       DBTYPE_I4;
  dbBinding[9].bPrecision =  0;
  dbBinding[9].bScale =      0;

  dbBinding[10].iOrdinal =    11;
  dbBinding[10].obValue =     offsetof(WellRecord, well_elev);
  dbBinding[10].obLength =    0; 
  dbBinding[10].obStatus =    offsetof(WellRecord, status.well_elev);
  dbBinding[10].pTypeInfo =   NULL;
  dbBinding[10].pObject =     NULL;
  dbBinding[10].pBindExt =    NULL;
  dbBinding[10].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[10].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[10].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[10].cbMaxLen =    fieldsize(WellRecord, well_elev);
  dbBinding[10].dwFlags =     0;
  dbBinding[10].wType =       DBTYPE_R4;
  dbBinding[10].bPrecision =  0;
  dbBinding[10].bScale =      0;

  dbBinding[11].iOrdinal =    12;
  dbBinding[11].obValue =     offsetof(WellRecord, well_gnd);
  dbBinding[11].obLength =    0; 
  dbBinding[11].obStatus =    offsetof(WellRecord, status.well_gnd);
  dbBinding[11].pTypeInfo =   NULL;
  dbBinding[11].pObject =     NULL;
  dbBinding[11].pBindExt =    NULL;
  dbBinding[11].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[11].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[11].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[11].cbMaxLen =    fieldsize(WellRecord, well_gnd);
  dbBinding[11].dwFlags =     0;
  dbBinding[11].wType =       DBTYPE_R4;
  dbBinding[11].bPrecision =  0;
  dbBinding[11].bScale =      0;

  dbBinding[12].iOrdinal =    13;
  dbBinding[12].obValue =     offsetof(WellRecord, condition);
  dbBinding[12].obLength =    0; 
  dbBinding[12].obStatus =    0;
  dbBinding[12].pTypeInfo =   NULL;
  dbBinding[12].pObject =     NULL;
  dbBinding[12].pBindExt =    NULL;
  dbBinding[12].dwPart =      DBPART_VALUE;
  dbBinding[12].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[12].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[12].cbMaxLen =    fieldsize(WellRecord, condition);
  dbBinding[12].dwFlags =     0;
  dbBinding[12].wType =       DBTYPE_I4;
  dbBinding[12].bPrecision =  0;
  dbBinding[12].bScale =      0;

  dbBinding[13].iOrdinal =    14;
  dbBinding[13].obValue =     offsetof(WellRecord, spud_date);
  dbBinding[13].obLength =    0; 
  dbBinding[13].obStatus =    offsetof(WellRecord, status.spud_date);
  dbBinding[13].pTypeInfo =   NULL;
  dbBinding[13].pObject =     NULL;
  dbBinding[13].pBindExt =    NULL;
  dbBinding[13].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[13].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[13].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[13].cbMaxLen =    fieldsize(WellRecord, spud_date);
  dbBinding[13].dwFlags =     0;
  dbBinding[13].wType =       DBTYPE_DATE;
  dbBinding[13].bPrecision =  0;
  dbBinding[13].bScale =      0;

  dbBinding[14].iOrdinal =    15;
  dbBinding[14].obValue =     offsetof(WellRecord, comp_date);
  dbBinding[14].obLength =    0; 
  dbBinding[14].obStatus =    offsetof(WellRecord, status.comp_date);
  dbBinding[14].pTypeInfo =   NULL;
  dbBinding[14].pObject =     NULL;
  dbBinding[14].pBindExt =    NULL;
  dbBinding[14].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[14].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[14].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[14].cbMaxLen =    fieldsize(WellRecord, comp_date);
  dbBinding[14].dwFlags =     0;
  dbBinding[14].wType =       DBTYPE_DATE;
  dbBinding[14].bPrecision =  0;
  dbBinding[14].bScale =      0;

  dbBinding[15].iOrdinal =    16;
  dbBinding[15].obValue =     offsetof(WellRecord, prod_date);
  dbBinding[15].obLength =    0; 
  dbBinding[15].obStatus =    offsetof(WellRecord, status.prod_date);
  dbBinding[15].pTypeInfo =   NULL;
  dbBinding[15].pObject =     NULL;
  dbBinding[15].pBindExt =    NULL;
  dbBinding[15].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[15].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[15].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[15].cbMaxLen =    fieldsize(WellRecord, prod_date);
  dbBinding[15].dwFlags =     0;
  dbBinding[15].wType =       DBTYPE_DATE;
  dbBinding[15].bPrecision =  0;
  dbBinding[15].bScale =      0;

  dbBinding[16].iOrdinal =    17;
  dbBinding[16].obValue =     offsetof(WellRecord, drill_date);
  dbBinding[16].obLength =    0; 
  dbBinding[16].obStatus =    offsetof(WellRecord, status.drill_date);
  dbBinding[16].pTypeInfo =   NULL;
  dbBinding[16].pObject =     NULL;
  dbBinding[16].pBindExt =    NULL;
  dbBinding[16].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[16].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[16].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[16].cbMaxLen =    fieldsize(WellRecord, drill_date);
  dbBinding[16].dwFlags =     0;
  dbBinding[16].wType =       DBTYPE_DATE;
  dbBinding[16].bPrecision =  0;
  dbBinding[16].bScale =      0;

  dbBinding[17].iOrdinal =    18;
  dbBinding[17].obValue =     offsetof(WellRecord, traj_id);
  dbBinding[17].obLength =    0; 
  dbBinding[17].obStatus =    offsetof(WellRecord, status.traj_md);
  dbBinding[17].pTypeInfo =   NULL;
  dbBinding[17].pObject =     NULL;
  dbBinding[17].pBindExt =    NULL;
  dbBinding[17].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[17].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[17].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[17].cbMaxLen =    fieldsize(WellRecord, traj_id);
  dbBinding[17].dwFlags =     0;
  dbBinding[17].wType =       DBTYPE_UI1;
  dbBinding[17].bPrecision =  0;
  dbBinding[17].bScale =      0;

  dbBinding[18].iOrdinal =    19;
  dbBinding[18].obValue =     offsetof(WellRecord, plugback);
  dbBinding[18].obLength =    0; 
  dbBinding[18].obStatus =    offsetof(WellRecord, status.plugback);
  dbBinding[18].pTypeInfo =   NULL;
  dbBinding[18].pObject =     NULL;
  dbBinding[18].pBindExt =    NULL;
  dbBinding[18].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[18].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[18].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[18].cbMaxLen =    fieldsize(WellRecord, plugback);
  dbBinding[18].dwFlags =     0;
  dbBinding[18].wType =       DBTYPE_R4;
  dbBinding[18].bPrecision =  0;
  dbBinding[18].bScale =      0;

  dbBinding[19].iOrdinal =    20;
  dbBinding[19].obValue =     offsetof(WellRecord, explore_area);
  dbBinding[19].obLength =    0; 
  dbBinding[19].obStatus =    0;
  dbBinding[19].pTypeInfo =   NULL;
  dbBinding[19].pObject =     NULL;
  dbBinding[19].pBindExt =    NULL;
  dbBinding[19].dwPart =      DBPART_VALUE;
  dbBinding[19].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[19].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[19].cbMaxLen =    fieldsize(WellRecord, explore_area);
  dbBinding[19].dwFlags =     0;
  dbBinding[19].wType =       DBTYPE_I4;
  dbBinding[19].bPrecision =  0;
  dbBinding[19].bScale =      0;

  dbBinding[20].iOrdinal =    21;
  dbBinding[20].obValue =     offsetof(WellRecord, department);
  dbBinding[20].obLength =    0; 
  dbBinding[20].obStatus =    0;
  dbBinding[20].pTypeInfo =   NULL;
  dbBinding[20].pObject =     NULL;
  dbBinding[20].pBindExt =    NULL;
  dbBinding[20].dwPart =      DBPART_VALUE;
  dbBinding[20].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[20].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[20].cbMaxLen =    fieldsize(WellRecord, department);
  dbBinding[20].dwFlags =     0;
  dbBinding[20].wType =       DBTYPE_I4;
  dbBinding[20].bPrecision =  0;
  dbBinding[20].bScale =      0;

  dbBinding[21].iOrdinal =    22;
  dbBinding[21].obValue =     offsetof(WellRecord, lease_area);
  dbBinding[21].obLength =    0; 
  dbBinding[21].obStatus =    offsetof(WellRecord, status.lease_area);
  dbBinding[21].pTypeInfo =   NULL;
  dbBinding[21].pObject =     NULL;
  dbBinding[21].pBindExt =    NULL;
  dbBinding[21].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[21].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[21].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[21].cbMaxLen =    fieldsize(WellRecord, lease_area);
  dbBinding[21].dwFlags =     0;
  dbBinding[21].wType =       DBTYPE_STR;
  dbBinding[21].bPrecision =  0;
  dbBinding[21].bScale =      0;

  dbBinding[22].iOrdinal =    23;
  dbBinding[22].obValue =     offsetof(WellRecord, cluster);
  dbBinding[22].obLength =    0; 
  dbBinding[22].obStatus =    offsetof(WellRecord, status.cluster);
  dbBinding[22].pTypeInfo =   NULL;
  dbBinding[22].pObject =     NULL;
  dbBinding[22].pBindExt =    NULL;
  dbBinding[22].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[22].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[22].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[22].cbMaxLen =    fieldsize(WellRecord, cluster);
  dbBinding[22].dwFlags =     0;
  dbBinding[22].wType =       DBTYPE_STR;
  dbBinding[22].bPrecision =  0;
  dbBinding[22].bScale =      0;

  dbBinding[23].iOrdinal =    24;
  dbBinding[23].obValue =     offsetof(WellRecord, parent_bore);
  dbBinding[23].obLength =    0; 
  dbBinding[23].obStatus =    offsetof(WellRecord, status.parent_bore);
  dbBinding[23].pTypeInfo =   NULL;
  dbBinding[23].pObject =     NULL;
  dbBinding[23].pBindExt =    NULL;
  dbBinding[23].dwPart =      DBPART_VALUE | DBPART_STATUS;
  dbBinding[23].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[23].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[23].cbMaxLen =    fieldsize(WellRecord, parent_bore);
  dbBinding[23].dwFlags =     0;
  dbBinding[23].wType =       DBTYPE_I4;
  dbBinding[23].bPrecision =  0;
  dbBinding[23].bScale =      0;

	return pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cColumns, dbBinding, 0, pResult, dbBindStatus);
}

HRESULT CEditTraj::UpdateWell(long bore, long parent, \
	int ns, TrajSurvey* ss, ITransactionLocal** pResult)
{
	ITransactionLocal* pTransaction = 0;
	*pResult = 0;
	HRESULT hr = m_pDoc->session.m_spOpenRowset->QueryInterface(IID_ITransactionLocal, (void**) &pTransaction);
	if (SUCCEEDED(hr))
	{
		ULONG ulTransactionLevel = 0;
		hr = pTransaction->StartTransaction(ISOLATIONLEVEL_READCOMMITTED, 0, NULL, &ulTransactionLevel);
		if (SUCCEEDED(hr))
		{
			DBPROP dbProp[3];
			DBPROPSET dbPropSet;
			DBID dbidTable, dbidIndex;

			dbProp[0].dwPropertyID = DBPROP_UPDATABILITY;
			dbProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
			dbProp[0].dwStatus = DBPROPSTATUS_OK;
			dbProp[0].colid = DB_NULLID;
			dbProp[0].vValue.vt = VT_I4;
			dbProp[0].vValue.lVal = DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE;

			dbProp[1].dwPropertyID = DBPROP_IRowsetIndex;
			dbProp[1].dwOptions = DBPROPOPTIONS_REQUIRED;
			dbProp[1].dwStatus = DBPROPSTATUS_OK;
			dbProp[1].colid = DB_NULLID;
			dbProp[1].vValue.vt = VT_BOOL;
			dbProp[1].vValue.boolVal = VARIANT_TRUE;

			dbProp[2].dwPropertyID = DBPROP_IRowsetChange;
			dbProp[2].dwOptions = DBPROPOPTIONS_REQUIRED;
			dbProp[2].dwStatus = DBPROPSTATUS_OK;
			dbProp[2].colid = DB_NULLID;
			dbProp[2].vValue.vt = VT_BOOL;
			dbProp[2].vValue.boolVal = VARIANT_TRUE;

			dbPropSet.guidPropertySet = DBPROPSET_ROWSET;
			dbPropSet.cProperties = 3;
			dbPropSet.rgProperties = dbProp;

			dbidTable.eKind = dbidIndex.eKind = DBKIND_NAME;
			dbidTable.uName.pwszName = L"well";
			dbidIndex.uName.pwszName = L"well_bore";

			IRowset* pRowset = 0;
			HRESULT hr = m_pDoc->session.m_spOpenRowset->OpenRowset(NULL, &dbidTable, &dbidIndex, \
				IID_IRowset, 1, &dbPropSet, (IUnknown**) &pRowset);

			if (SUCCEEDED(hr))
			{
				IRowsetIndex* pRowsetIndex = 0;
				hr = pRowset->QueryInterface(IID_IRowsetIndex, (void**) &pRowsetIndex);
				if (SUCCEEDED(hr))
				{
					IRowsetChange* pRowsetChange = 0;
					hr = pRowset->QueryInterface(IID_IRowsetChange, (void**) &pRowsetChange);
					if (SUCCEEDED(hr))
					{
						IAccessor* pAccessor = 0;
						hr = pRowset->QueryInterface(IID_IAccessor, (void**) &pAccessor);
						if (SUCCEEDED(hr))
						{
							HACCESSOR hAccessor = DB_NULL_HACCESSOR;
							hr = BindWell(pAccessor, &hAccessor);
							if (SUCCEEDED(hr))
							{
								WellRecord record;
								record.well_bore = bore;
								hr = pRowsetIndex->Seek(hAccessor, 1, &record, DBSEEK_FIRSTEQ);
								if (hr == DB_E_NOTFOUND)
								{
									if (ns > 0 && parent != 0)
									{
										record.well_bore = parent;
										hr = pRowsetIndex->Seek(hAccessor, 1, &record, DBSEEK_FIRSTEQ);
										if (hr == S_OK)
										{
											HROW hRow;
											ULONG cRowsObtained;
											HROW* pRows = &hRow;
											hr = pRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &pRows);
											if (SUCCEEDED(hr))
											{
												hr = pRowset->GetData(hRow, hAccessor, &record);
												if (SUCCEEDED(hr))
												{
													/*
													// insert record
													for (int i = 0; i < ns; ++i)
													{
														ss[i].dz += record.well_elev;
													}
													record.status.well_dx = DBSTATUS_S_ISNULL;
													record.status.well_dy = DBSTATUS_S_ISNULL;
													record.status.well_dz = DBSTATUS_S_ISNULL;
													record.status.well_md = DBSTATUS_S_ISNULL;
													record.status.spud_date = DBSTATUS_S_ISNULL;
													record.status.comp_date = DBSTATUS_S_ISNULL;
													record.status.prod_date = DBSTATUS_S_ISNULL;
													record.status.drill_date = DBSTATUS_S_ISNULL;
													record.status.plugback = DBSTATUS_S_ISNULL;
													record.status.traj_id = DBSTATUS_S_OK;
													record.status.parent_bore = DBSTATUS_S_OK;
													record.well_bore = bore;
													record.traj_id = ss[ns-1].id;
													record.parent_bore = parent;
													hr = pRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, &record, NULL);
													if (SUCCEEDED(hr))
													{
														*pResult = pTransaction;
														pTransaction->AddRef();
													}
													*/
												}
												pRowset->ReleaseRows(cRowsObtained, pRows, NULL, NULL, NULL);
											}
										}
										else
										{
											TRACE(FILEPOS _T("error, parent bore does not exist or trajectory size is empty for newly created well bore\n"));
											hr = E_FAIL;
										}
									}
									else
									{
										TRACE(FILEPOS _T("error, parent bore does not defined for newly created well bore\n"));
										hr = E_FAIL;
									}
								}
								else if (hr == S_OK)
								{
									HROW hRow;
									ULONG cRowsObtained;
									HROW* pRows = &hRow;
									hr = pRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &pRows);
									if (SUCCEEDED(hr))
									{
										if (ns > 0)
										{
											// update record
											hr = pRowset->GetData(hRow, hAccessor, &record);
											if (SUCCEEDED(hr))
											{
												for (int i = 0; i < ns; ++i)
												{
													ss[i].dz += record.well_elev;
												}
												record.status.traj_md = DBSTATUS_S_OK;
												record.traj_md = ss[ns-1].md;
												hr = pRowsetChange->SetData(hRow, hAccessor, &record);
												if (SUCCEEDED(hr))
												{
													*pResult = pTransaction;
													pTransaction->AddRef();
												}
											}
										}
										else
										{
											// drop record
											CString str;
											str.Format(_T("Are you sure to delete well bore number %d?"), bore);
											if (IDOK == AfxMessageBox(str, MB_OKCANCEL))
											{
												hr = pRowsetChange->DeleteRows(DB_NULL_HCHAPTER, cRowsObtained, pRows, NULL);
												if (SUCCEEDED(hr))
												{
													*pResult = pTransaction;
													pTransaction->AddRef();
												}
											}
											else
											{
												TRACE(FILEPOS _T("error, user break the operation\n"));
												hr = E_FAIL;
											}
										}
										pRowset->ReleaseRows(cRowsObtained, pRows, NULL, NULL, NULL);
									}
								}
								else
								{
									TRACE(FILEPOS _T("error, bad status code returned from Seek() method\n"));
								}
								pAccessor->ReleaseAccessor(hAccessor, NULL);
							}
							pAccessor->Release();
						}
						pRowsetChange->Release();
					}
					pRowsetIndex->Release();
				}
				pRowset->Release();
			}
		}
		pTransaction->Release();
	}
	return hr;
}

HRESULT CEditTraj::BindTraj(IAccessor* pAccessor, HACCESSOR* pResult)
{
	const cColumns = 7;
	DBBINDING dbBinding[cColumns];
	DBBINDSTATUS dbBindStatus[cColumns];

  dbBinding[0].iOrdinal =    1;
  dbBinding[0].obValue =     offsetof(TrajRecord, traj_wb);
  dbBinding[0].obLength =    0; 
  dbBinding[0].obStatus =    0;
  dbBinding[0].pTypeInfo =   NULL;
  dbBinding[0].pObject =     NULL;
  dbBinding[0].pBindExt =    NULL;
  dbBinding[0].dwPart =      DBPART_VALUE;
  dbBinding[0].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[0].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[0].cbMaxLen =    fieldsize(TrajRecord, traj_wb);
  dbBinding[0].dwFlags =     0;
  dbBinding[0].wType =       DBTYPE_I4;
  dbBinding[0].bPrecision =  0;
  dbBinding[0].bScale =      0;

  dbBinding[1].iOrdinal =    2;
  dbBinding[1].obValue =     offsetof(TrajRecord, traj_id);
  dbBinding[1].obLength =    0; 
  dbBinding[1].obStatus =    0;
  dbBinding[1].pTypeInfo =   NULL;
  dbBinding[1].pObject =     NULL;
  dbBinding[1].pBindExt =    NULL;
  dbBinding[1].dwPart =      DBPART_VALUE;
  dbBinding[1].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[1].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[1].cbMaxLen =    fieldsize(TrajRecord, traj_id);
  dbBinding[1].dwFlags =     0;
  dbBinding[1].wType =       DBTYPE_UI1;
  dbBinding[1].bPrecision =  0;
  dbBinding[1].bScale =      0;

  dbBinding[2].iOrdinal =    3;
  dbBinding[2].obValue =     offsetof(TrajRecord, tail.dx);
  dbBinding[2].obLength =    0; 
  dbBinding[2].obStatus =    0;
  dbBinding[2].pTypeInfo =   NULL;
  dbBinding[2].pObject =     NULL;
  dbBinding[2].pBindExt =    NULL;
  dbBinding[2].dwPart =      DBPART_VALUE;
  dbBinding[2].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[2].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[2].cbMaxLen =    fieldsize(TrajRecord, tail.dx);
  dbBinding[2].dwFlags =     0;
  dbBinding[2].wType =       DBTYPE_R4;
  dbBinding[2].bPrecision =  0;
  dbBinding[2].bScale =      0;

  dbBinding[3].iOrdinal =    4;
  dbBinding[3].obValue =     offsetof(TrajRecord, tail.dy);
  dbBinding[3].obLength =    0; 
  dbBinding[3].obStatus =    0;
  dbBinding[3].pTypeInfo =   NULL;
  dbBinding[3].pObject =     NULL;
  dbBinding[3].pBindExt =    NULL;
  dbBinding[3].dwPart =      DBPART_VALUE;
  dbBinding[3].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[3].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[3].cbMaxLen =    fieldsize(TrajRecord, tail.dy);
  dbBinding[3].dwFlags =     0;
  dbBinding[3].wType =       DBTYPE_R4;
  dbBinding[3].bPrecision =  0;
  dbBinding[3].bScale =      0;

  dbBinding[4].iOrdinal =    5;
  dbBinding[4].obValue =     offsetof(TrajRecord, tail.dz);
  dbBinding[4].obLength =    0; 
  dbBinding[4].obStatus =    0;
  dbBinding[4].pTypeInfo =   NULL;
  dbBinding[4].pObject =     NULL;
  dbBinding[4].pBindExt =    NULL;
  dbBinding[4].dwPart =      DBPART_VALUE;
  dbBinding[4].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[4].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[4].cbMaxLen =    fieldsize(TrajRecord, tail.dz);
  dbBinding[4].dwFlags =     0;
  dbBinding[4].wType =       DBTYPE_R4;
  dbBinding[4].bPrecision =  0;
  dbBinding[4].bScale =      0;

  dbBinding[5].iOrdinal =    6;
  dbBinding[5].obValue =     offsetof(TrajRecord, tail.md);
  dbBinding[5].obLength =    0; 
  dbBinding[5].obStatus =    0;
  dbBinding[5].pTypeInfo =   NULL;
  dbBinding[5].pObject =     NULL;
  dbBinding[5].pBindExt =    NULL;
  dbBinding[5].dwPart =      DBPART_VALUE;
  dbBinding[5].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[5].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[5].cbMaxLen =    fieldsize(TrajRecord, tail.md);
  dbBinding[5].dwFlags =     0;
  dbBinding[5].wType =       DBTYPE_R4;
  dbBinding[5].bPrecision =  0;
  dbBinding[5].bScale =      0;

  dbBinding[6].iOrdinal =    7;
  dbBinding[6].obValue =     offsetof(TrajRecord, traj_ns);
  dbBinding[6].obLength =    0; 
  dbBinding[6].obStatus =    0;
  dbBinding[6].pTypeInfo =   NULL;
  dbBinding[6].pObject =     NULL;
  dbBinding[6].pBindExt =    NULL;
  dbBinding[6].dwPart =      DBPART_VALUE;
  dbBinding[6].dwMemOwner =  DBMEMOWNER_CLIENTOWNED;
  dbBinding[6].eParamIO =    DBPARAMIO_NOTPARAM;
  dbBinding[6].cbMaxLen =    fieldsize(TrajRecord, traj_ns);
  dbBinding[6].dwFlags =     0;
  dbBinding[6].wType =       DBTYPE_I2;
  dbBinding[6].bPrecision =  0;
  dbBinding[6].bScale =      0;

	return pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cColumns, dbBinding, 0, pResult, dbBindStatus);
}

HRESULT CEditTraj::UpdateTraj(long bore, int ns, \
	TrajSurvey* ss, ITransactionLocal** pResult)
{
	ITransactionLocal* pTransaction = 0;
	*pResult = 0;
	HRESULT hr = m_pDoc->session.m_spOpenRowset->QueryInterface(IID_ITransactionLocal, (void**) &pTransaction);
	if (SUCCEEDED(hr))
	{
		ULONG ulTransactionLevel = 0;
		hr = pTransaction->StartTransaction(ISOLATIONLEVEL_READCOMMITTED, 0, NULL, &ulTransactionLevel);
		if (SUCCEEDED(hr))
		{
			DBPROP dbProp[3];
			DBPROPSET dbPropSet;
			DBID dbidTable, dbidIndex;

			dbProp[0].dwPropertyID = DBPROP_UPDATABILITY;
			dbProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
			dbProp[0].dwStatus = DBPROPSTATUS_OK;
			dbProp[0].colid = DB_NULLID;
			dbProp[0].vValue.vt = VT_I4;
			dbProp[0].vValue.lVal = DBPROPVAL_UP_INSERT | DBPROPVAL_UP_CHANGE | DBPROPVAL_UP_DELETE;

			dbProp[1].dwPropertyID = DBPROP_IRowsetIndex;
			dbProp[1].dwOptions = DBPROPOPTIONS_REQUIRED;
			dbProp[1].dwStatus = DBPROPSTATUS_OK;
			dbProp[1].colid = DB_NULLID;
			dbProp[1].vValue.vt = VT_BOOL;
			dbProp[1].vValue.boolVal = VARIANT_TRUE;

			dbProp[2].dwPropertyID = DBPROP_IRowsetChange;
			dbProp[2].dwOptions = DBPROPOPTIONS_REQUIRED;
			dbProp[2].dwStatus = DBPROPSTATUS_OK;
			dbProp[2].colid = DB_NULLID;
			dbProp[2].vValue.vt = VT_BOOL;
			dbProp[2].vValue.boolVal = VARIANT_TRUE;

			dbPropSet.guidPropertySet = DBPROPSET_ROWSET;
			dbPropSet.cProperties = 3;
			dbPropSet.rgProperties = dbProp;

			dbidTable.eKind = dbidIndex.eKind = DBKIND_NAME;
			dbidTable.uName.pwszName = L"traj";
			dbidIndex.uName.pwszName = L"bore_id";

			IRowset* pRowset = 0;
			HRESULT hr = m_pDoc->session.m_spOpenRowset->OpenRowset(NULL, &dbidTable, &dbidIndex, \
				IID_IRowset, 1, &dbPropSet, (IUnknown**) &pRowset);
			if (SUCCEEDED(hr))
			{
				IRowsetIndex* pRowsetIndex = 0;
				hr = pRowset->QueryInterface(IID_IRowsetIndex, (void**) &pRowsetIndex);
				if (SUCCEEDED(hr))
				{
					IRowsetChange* pRowsetChange = 0;
					hr = pRowset->QueryInterface(IID_IRowsetChange, (void**) &pRowsetChange);
					if (SUCCEEDED(hr))
					{
						IAccessor* pAccessor = 0;
						hr = pRowset->QueryInterface(IID_IAccessor, (void**) &pAccessor);
						if (SUCCEEDED(hr))
						{
							HACCESSOR hAccessor = DB_NULL_HACCESSOR;
							hr = BindTraj(pAccessor, &hAccessor);
							if (SUCCEEDED(hr))
							{
								TrajRecord record;
								record.traj_wb = bore;
								record.traj_id = 1;
								hr = pRowsetIndex->Seek(hAccessor, 2, &record, DBSEEK_FIRSTEQ);
								if (hr == DB_E_NOTFOUND)
								{
									if (ns > 0)
									{
										// insert record
										record.traj_wb = bore;
										record.traj_id = 1;
										record.tail.dx = ss[ns-1].dx;
										record.tail.dy = ss[ns-1].dy;
										record.tail.dz = ss[ns-1].dz;
										record.tail.md = ss[ns-1].md;
										record.traj_ns = ns-1;
										hr = pRowsetChange->InsertRow(DB_NULL_HCHAPTER, hAccessor, &record, NULL);
										if (SUCCEEDED(hr))
										{
											if (ns > 1)
											{
												char name[64];
												sprintf(name, "traj/version_1/%d", bore);

												hsize_t count[2];
												count[0] = ns-1;
												count[1] = 4;

												hid_t dataspace = H5Screate_simple(2, count, NULL);
												if (0 > dataspace)
												{
													TRACE(FILEPOS _T("error, failed to create HDF5 dataset\n"));
													hr = E_FAIL;
												}
												else
												{
													hid_t dataset = H5Dcreate(m_pDoc->hdf5_file, name, H5T_NATIVE_FLOAT, dataspace, H5P_DEFAULT);
													if (0 > dataset)
													{
														TRACE(FILEPOS _T("error, failed to create HDF5 dataset\n"));
														hr = E_FAIL;
													}
													else
													{
														herr_t err = H5Dwrite(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ss);
														if (0 > err)
														{
															TRACE(FILEPOS _T("error, failed to write HDF5 dataset\n"));
															H5Gunlink(m_pDoc->hdf5_file, name);
															hr = E_FAIL;
														}
														else
														{
															*pResult = pTransaction;
															pTransaction->AddRef();
														}
														H5Dclose(dataset);
													}
													H5Sclose(dataspace);
												}
											}
											else
											{
												*pResult = pTransaction;
												pTransaction->AddRef();
											}
										}
									}
									else
									{
										TRACE(_T("error, parent bore does not defined for newly created well bore\n"));
										hr = E_FAIL;
									}
								}
								else if (hr == S_OK)
								{
									HROW hRow;
									ULONG cRowsObtained;
									HROW* pRows = &hRow;
									hr = pRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &pRows);
									if (SUCCEEDED(hr))
									{
										if (ns > 0)
										{
											// update record
											hr = pRowset->GetData(hRow, hAccessor, &record);
											if (SUCCEEDED(hr))
											{
												bool have_dataset = (record.traj_ns > 0);
												record.traj_wb = bore;
												record.traj_id = 1;
												record.tail.dx = ss[ns-1].dx;
												record.tail.dy = ss[ns-1].dy;
												record.tail.dz = ss[ns-1].dz;
												record.tail.md = ss[ns-1].md;
												record.traj_ns = ns-1;
												hr = pRowsetChange->SetData(hRow, hAccessor, &record);
												if (SUCCEEDED(hr))
												{
													char name[64];
													sprintf(name, "traj/version_1/%d", bore);
													
													if (have_dataset)
													{
														H5Gunlink(m_pDoc->hdf5_file, name);
													}

													if (ns > 1)
													{
														hsize_t count[2];
														count[0] = ns-1;
														count[1] = 4;

														hid_t dataspace = H5Screate_simple(2, count, NULL);
														if (0 > dataspace)
														{
															TRACE(FILEPOS _T("error, failed to create HDF5 dataset\n"));
															hr = E_FAIL;
														}
														else
														{
															hid_t dataset = H5Dcreate(m_pDoc->hdf5_file, name, H5T_NATIVE_FLOAT, dataspace, H5P_DEFAULT);
															if (0 > dataset)
															{
																TRACE(FILEPOS _T("error, failed to create HDF5 dataset\n"));
																hr = E_FAIL;
															}
															else
															{
																herr_t err = H5Dwrite(dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ss);
																if (0 > err)
																{
																	TRACE(FILEPOS _T("error, failed to write HDF5 dataset\n"));
																	H5Gunlink(m_pDoc->hdf5_file, name);
																	hr = E_FAIL;
																}
																else
																{
																	*pResult = pTransaction;
																	pTransaction->AddRef();
																}
																H5Dclose(dataset);
															}
															H5Sclose(dataspace);
														}
													}
												}
											}
										}
										else
										{
											// drop record
											hr = pRowset->GetData(hRow, hAccessor, &record);
											if (SUCCEEDED(hr))
											{
												hr = pRowsetChange->DeleteRows(DB_NULL_HCHAPTER, cRowsObtained, pRows, NULL);
												if (SUCCEEDED(hr))
												{
													if (record.traj_ns > 0)
													{
														char name[64];
														sprintf(name, "traj/version_1/%d", bore);
														hid_t err = H5Gunlink(m_pDoc->hdf5_file, name);
														if (0 > err)
														{
															TRACE(FILEPOS _T("error, failed to remove HDF5 dataset from file\n"));
															hr = E_FAIL;
														}
														else
														{
															*pResult = pTransaction;
															pTransaction->AddRef();
														}
													}
													else
													{
														*pResult = pTransaction;
														pTransaction->AddRef();
													}
												}
											}
										}
										pRowset->ReleaseRows(cRowsObtained, pRows, NULL, NULL, NULL);
									}
								}
								else
								{
									TRACE(_T("error, bad status code returned from Seek() method\n"));
								}
								pAccessor->ReleaseAccessor(hAccessor, NULL);
							}
							pAccessor->Release();
						}
						pRowsetChange->Release();
					}
					pRowsetIndex->Release();
				}
				pRowset->Release();
			}
		}
		pTransaction->Release();
	}
	return hr;
}

void CEditTraj::OnOK()
{
  long bore;
  CString str;
  m_wndWellbore.GetWindowText(str);
  if (1 != sscanf(str, "%d", &bore))
  {
    MessageBox(_T("Underined bore number"));
    m_wndWellbore.SetFocus();
    return;
  }

  long parent = 0;
  bool has_parent = false;
  m_wndParentbore.GetWindowText(str);
  if (!str.IsEmpty())
  {
    has_parent = true;
    if (1 != sscanf(str, "%d", &parent))
    {
      MessageBox(_T("Parent bore should be an integer"));
      m_wndParentbore.SetFocus();
      return;
    }
  }

  int ns = m_wndList.GetItemCount();
  std::valarray<TrajSurvey> ss(ns);
  for (int i = 0; i < ns; ++i)
  {
    VERIFY(1 == sscanf(m_wndList.GetItemText(i, 0), "%g", &ss[i].dx));
    VERIFY(1 == sscanf(m_wndList.GetItemText(i, 1), "%g", &ss[i].dy));
    VERIFY(1 == sscanf(m_wndList.GetItemText(i, 2), "%g", &ss[i].dz));
    VERIFY(1 == sscanf(m_wndList.GetItemText(i, 3), "%g", &ss[i].md));
  }

	ITransactionLocal* pT1 = 0;
	HRESULT hr = UpdateWell(bore, parent, ns, (ns > 0? &ss[0]: 0), &pT1);
	if (SUCCEEDED(hr))
	{
		ITransactionLocal* pT2 = 0;
		HRESULT hr = UpdateTraj(bore, ns, (ns > 0? &ss[0]: 0), &pT2);
		if (SUCCEEDED(hr))
		{
			hr = pT1->Commit(FALSE, XACTTC_SYNC, 0);
			hr = pT2->Commit(FALSE, XACTTC_SYNC, 0);
			m_pDoc->UpdateWellStock();
			CDialog::OnOK();
			pT2->Release();
		}
		pT1->Release();
	}
}
