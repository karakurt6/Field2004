// DataMapping.cpp : implementation file
//

#include "stdafx.h"
#include "field.h"
#include "DataMapping.h"
#include "FieldView.h"
#include "FieldDoc.h"

#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorItem dialog

class CColorItem : public CColorDialog
{
// Construction
public:
	CColorItem(COLORREF cr, float value, DWORD flags, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CColorItem)
	enum { IDD = IDD_COLOR_ITEM };
	float	m_sValue;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorItem)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CColorItem)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CColorItem dialog


CColorItem::CColorItem(COLORREF cr, float value, DWORD flags, CWnd* pParent)
	: CColorDialog(cr, flags, pParent)
{
	//{{AFX_DATA_INIT(CColorItem)
	m_sValue = value;
	//}}AFX_DATA_INIT

	m_cc.lpTemplateName = MAKEINTRESOURCE(IDD);
	m_cc.Flags |= CC_ENABLETEMPLATE | CC_ENABLEHOOK | CC_FULLOPEN;
}


void CColorItem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorItem)
	DDX_Text(pDX, IDC_EDIT1, m_sValue);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorItem, CDialog)
	//{{AFX_MSG_MAP(CColorItem)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorItem message handlers


// offsets for first and other columns
#define OFFSET_FIRST    2
#define OFFSET_OTHER    6

static LPCTSTR MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset)
{
  static const _TCHAR szThreeDots[] = _T("...");

  int nStringLen = lstrlen(lpszLong);

  if(nStringLen == 0 || (pDC->GetTextExtent(lpszLong, nStringLen).cx + nOffset) <= nColumnLen)
  {
    return(lpszLong);
  }

  static _TCHAR szShort[MAX_PATH];

  lstrcpy(szShort,lpszLong);
  int nAddLen = pDC->GetTextExtent(szThreeDots,sizeof(szThreeDots)).cx;

  for(int i = nStringLen-1; i > 0; i--)
  {
    szShort[i] = 0;
    if((pDC->GetTextExtent(szShort, i).cx + nOffset + nAddLen) <= nColumnLen)
    {
      break;
    }
  }

  lstrcat(szShort, szThreeDots);
  return(szShort);
}

void CDataMapping::Color_ListCtrl::InsertText(int i)
{
	CString str;

  float z = m_pTable->data[i].value;
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
	InsertItem(i, str);

	str.Format("0x%08X", m_pTable->data[i].color);
	SetItemText(i, 1, str);
}

void CDataMapping::Color_ListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
  COLORREF m_clrText = ::GetSysColor(COLOR_WINDOWTEXT);
  COLORREF m_clrTextBk = ::GetSysColor(COLOR_WINDOW);
  COLORREF m_clrBkgnd = ::GetSysColor(COLOR_WINDOW);

  CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
  CRect rcItem(lpDrawItemStruct->rcItem);
  UINT uiFlags = ILD_TRANSPARENT;
  CImageList* pImageList;
  int nItem = lpDrawItemStruct->itemID;
  BOOL bFocus = (GetFocus() == this);
  COLORREF clrTextSave, clrBkSave;
  COLORREF clrImage = m_clrBkgnd;
  static _TCHAR szBuff[MAX_PATH];
  LPCTSTR pszText;

// get item data

  LV_ITEM lvi;
  lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
  lvi.iItem = nItem;
  lvi.iSubItem = 0;
  lvi.pszText = szBuff;
  lvi.cchTextMax = sizeof(szBuff);
  lvi.stateMask = 0xFFFF;     // get all state flags
  GetItem(&lvi);

  BOOL bSelected = (bFocus || (GetStyle() & LVS_SHOWSELALWAYS)) && lvi.state & LVIS_SELECTED;
  bSelected = bSelected || (lvi.state & LVIS_DROPHILITED);

// set colors if item is selected

  CRect rcAllLabels;
  GetItemRect(nItem, rcAllLabels, LVIR_BOUNDS);

  CRect rcLabel;
  GetItemRect(nItem, rcLabel, LVIR_LABEL);

	CRect rectClient;
	GetClientRect(&rectClient);
	int m_cxClient = rectClient.Width();
	BOOL m_bClientWidthSel = TRUE;

  rcAllLabels.left = rcLabel.left;
  if (m_bClientWidthSel && rcAllLabels.right < m_cxClient)
      rcAllLabels.right = m_cxClient;

  if (bSelected)
  {
    clrTextSave = pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
    clrBkSave = pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));

    pDC->FillRect(rcAllLabels, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));
  }
  else
  {
    pDC->FillRect(rcAllLabels, &CBrush(m_clrTextBk));
  }

// set color and mask for the icon

  if (lvi.state & LVIS_CUT)
  {
    clrImage = m_clrBkgnd;
    uiFlags |= ILD_BLEND50;
  }
  else if (bSelected)
  {
    clrImage = ::GetSysColor(COLOR_HIGHLIGHT);
    uiFlags |= ILD_BLEND50;
  }

// draw state icon

  UINT nStateImageMask = lvi.state & LVIS_STATEIMAGEMASK;
  if (nStateImageMask)
  {
    int nImage = (nStateImageMask>>12) - 1;
    pImageList = GetImageList(LVSIL_STATE);
    if (pImageList)
    {
      pImageList->Draw(pDC, nImage, CPoint(rcItem.left, rcItem.top), ILD_TRANSPARENT);
    }
  }

// draw normal and overlay icon

  CRect rcIcon;
  GetItemRect(nItem, rcIcon, LVIR_ICON);

  pImageList = GetImageList(LVSIL_SMALL);
  if (pImageList)
  {
    UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
    if (rcItem.left<rcItem.right-1)
    {
      ImageList_DrawEx(pImageList->m_hImageList, lvi.iImage,
        pDC->m_hDC,rcIcon.left,rcIcon.top, 16, 16, 
        m_clrBkgnd, clrImage, uiFlags | nOvlImageMask);
    }
  }

// draw item label

	int m_cxStateImageOffset = 0;
  GetItemRect(nItem, rcItem, LVIR_LABEL);
  rcItem.right -= m_cxStateImageOffset;

  pszText = MakeShortString(pDC, szBuff, rcItem.right-rcItem.left, 2*OFFSET_FIRST);

  rcLabel = rcItem;
  rcLabel.left += OFFSET_FIRST;
  rcLabel.right -= OFFSET_FIRST;

  pDC->DrawText(pszText,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);

// draw labels for extra columns

  LV_COLUMN lvc;
  lvc.mask = LVCF_FMT | LVCF_WIDTH;

  for(int nColumn = 1; GetColumn(nColumn, &lvc); nColumn++)
  {
    rcItem.left = rcItem.right;
    rcItem.right += lvc.cx;

    int nRetLen = GetItemText(nItem, nColumn,
                    szBuff, sizeof(szBuff));
    if (nRetLen == 0)
        continue;

    pszText = MakeShortString(pDC, szBuff,
        rcItem.right - rcItem.left, 2*OFFSET_OTHER);

    UINT nJustify = DT_LEFT;

    if(pszText == szBuff)
    {
      switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
      {
      case LVCFMT_RIGHT:
        nJustify = DT_RIGHT;
        break;
      case LVCFMT_CENTER:
        nJustify = DT_CENTER;
        break;
      default:
        break;
      }
    }

    rcLabel = rcItem;
    // rcLabel.left += OFFSET_OTHER;
    // rcLabel.right -= OFFSET_OTHER;

		COLORREF cr;
		if (1 == _stscanf(pszText, _T("0x%08X"), &cr))
		{
			// rcLabel.DeflateRect(CSize(2, 2));
			pDC->FillSolidRect(rcLabel, cr);
		}

		/*
        pDC->DrawText(pszText, -1, rcLabel,
            nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER);
		*/
  }

// draw focus rectangle if item has focus

  if (lvi.state & LVIS_FOCUSED && bFocus)
      pDC->DrawFocusRect(rcAllLabels);

// set original colors if item was selected

  if (bSelected)
  {
    pDC->SetTextColor(clrTextSave);
    pDC->SetBkColor(clrBkSave);
  }
}

void CDataMapping::Color_ListCtrl::Initialize(DATA_TABLE* table, int nSelect)
{
	TCHAR buff[10];
	LVCOLUMN col;
	col.mask = LVCF_TEXT;
	col.pszText = buff;
	col.cchTextMax = 10;
	if (!GetColumn(0, &col) || _tcsicmp(_T("value"), buff) != 0)
	{
		InsertColumn(0, _T("value"));
		InsertColumn(1, _T("color"));
	}

	SetRedraw(FALSE);
	m_pTable = table;
	DeleteAllItems();
	for (int i = 0, n = table->count; i < n; i++)
	{
    InsertText(i);
	}
	SetColumnWidth(0, LVSCW_AUTOSIZE);
	SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
	SetFocus();
	if (n > 0)
	{
		if (nSelect < 0 || nSelect >= n)
		{
			nSelect = 0;
		}
		EnsureVisible(nSelect, FALSE);
		SetItemState(nSelect, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	SetRedraw(TRUE);
}

BOOL CDataMapping::Color_ListCtrl::UpdateItem(int n1, float d)
{
	int n2;

	if (n1 < 0 || n1 >= m_pTable->size)
	{
		AfxMessageBox(_T("data item index is out of range"));
		return FALSE;
	}

	if (n1+1 < m_pTable->count && m_pTable->data[n1+1].value <= d)
	{
		COLORREF cr = m_pTable->data[n1].color;
		for (n2 = n1+1; n2 < m_pTable->count; n2++)
		{
			float value = m_pTable->data[n2].value;
			if (value > d)
			{
				break;
			}
			if (value < d)
			{
				continue;
			}
			CString str;
			str.Format(_T("entry for the value of '%f' is already defined in the table"), d);
			AfxMessageBox(str);
			return FALSE;
		}

		for (int i = n1+1; i < n2; i++)
		{
			m_pTable->data[i-1] = m_pTable->data[i];
		}
		m_pTable->data[--n2].color = cr;
	}
	else if (n1-1 > -1 && m_pTable->data[n1-1].value >= d)
	{
		COLORREF cr = m_pTable->data[n1].color;
		for (n2 = n1-1; n2 > -1; n2--)
		{
			float value = m_pTable->data[n2].value;
			if (value < d)
			{
				break;
			}
			if (value > d)
			{
				continue;
			}

			CString str;
			str.Format(_T("entry for the value of '%f' is already defined in the table"), d);
			AfxMessageBox(str);
			return FALSE;
		}

		for (int i = n1-1; i > n2; i--)
		{
			m_pTable->data[i+1] = m_pTable->data[i];
		}
		m_pTable->data[++n2].color = cr;
	}
	else
	{
		n2 = n1;
	}

	m_pTable->data[n2].value = d;
	Initialize(m_pTable, n2);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDataMapping dialog


CDataMapping::CDataMapping(CWnd* pParent /*=NULL*/)
	: CDialog(CDataMapping::IDD, pParent)
{
  table.size = COLOR_MAXDATA;
	//{{AFX_DATA_INIT(CDataMapping)
	//}}AFX_DATA_INIT
}


void CDataMapping::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDataMapping)
	DDX_Control(pDX, IDC_LIST1, m_ctrlList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDataMapping, CDialog)
	//{{AFX_MSG_MAP(CDataMapping)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LIST1, OnEndlabeleditList1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnDblclkList1)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDataMapping message handlers

BOOL CDataMapping::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
  m_ctrlList.Initialize(&table, 0);

  HINSTANCE hh = GetModuleHandle(NULL);

  CButton* pWnd = (CButton*) GetDlgItem(IDC_BUTTON3);
  pWnd->SetBitmap(::LoadBitmap(hh, MAKEINTRESOURCE(IDB_EDIT_ADD)));

  pWnd = (CButton*) GetDlgItem(IDC_BUTTON4);
  pWnd->SetBitmap(::LoadBitmap(hh, MAKEINTRESOURCE(IDB_EDIT_DEL)));

  pWnd = (CButton*) GetDlgItem(IDC_BUTTON1);
  pWnd->SetIcon((HICON) ::LoadImage(hh, MAKEINTRESOURCE(IDI_OPEN), IMAGE_ICON, 16, 16, 0));

  pWnd = (CButton*) GetDlgItem(IDC_BUTTON2);
  pWnd->SetIcon((HICON) ::LoadImage(hh, MAKEINTRESOURCE(IDI_SAVE), IMAGE_ICON, 16, 16, 0));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDataMapping::OnEndlabeleditList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDI = (LV_DISPINFO*)pNMHDR;
  LPCTSTR ch = pDI->item.pszText;
  *pResult = 0;
  if (!ch)
  {
    return;
  }

  float d = (float) _tcstod(ch, (LPTSTR*) &ch);
  if (d == 0.0 && pDI->item.pszText[0] != _T('0'))
  {
    CString str;
    str.Format(_T("could not convert string '%s' to floating-point number"), pDI->item.pszText);
    AfxMessageBox(str);
    return;
  }

  if (ch[0] != _T('\0'))
  {
    CString str;
    str.Format(_T("string '%s' is not properly terminated"), pDI->item.pszText);
    AfxMessageBox(str);
    return;
  }
  
  m_ctrlList.UpdateItem(pDI->item.iItem, d);	
	*pResult = 0;
}

void CDataMapping::OnDblclkList1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int nItem = m_ctrlList.GetNextItem(-1, LVNI_SELECTED);
	if (nItem != -1)
	{
    Color_ListCtrl::DATA_COLOR* data = &table.data[nItem];
		CColorItem dlg(data->color, data->value, 0, this);
		if (dlg.DoModal() == IDOK)
		{
      table.data[nItem].color = dlg.GetColor();
			if (data->value != dlg.m_sValue)
			{
        m_ctrlList.UpdateItem(nItem, dlg.m_sValue);
			}
      else
      {
				CString str;
				str.Format(_T("0x%08X"), data->color);
        m_ctrlList.SetItemText(nItem, 1, str);
      }
		}
	}
	*pResult = 0;
}


void CDataMapping::OnButton3() 	// add
{
  CColorItem dlg(RGB(128, 128, 128), 0.0f, 0, this);
  if (dlg.DoModal() == IDOK)
  {
    if (table.count >= table.size)
		{
			AfxMessageBox(_T("color data buffer is full"));
			return;
		}

		CString str;
		for (int i = 0; i < table.count; i++)
		{
			if (dlg.m_sValue < table.data[i].value)
			{
				break;
			}
			if (dlg.m_sValue > table.data[i].value)
			{
				continue;
			}
			str.Format(_T("entry for the value of '%f' is already defined in the table"), dlg.m_sValue);
			AfxMessageBox(str);
			return;
		}

		for (int j = table.count++; j > i; j--)
		{
			table.data[j] = table.data[j-1];
		}
		table.data[j].color = dlg.GetColor();
		table.data[j].value = dlg.m_sValue;

    m_ctrlList.InsertText(j);
  }
}

void CDataMapping::OnButton4() 	// del
{
  if (table.count <= 2)
  {
    AfxMessageBox("You should have at least two items in color mapping");
    return;
  }

  int n = m_ctrlList.GetNextItem(-1, LVNI_SELECTED);
  if (n != -1 && n >= 0)
  {
    for (int i = n+1; i < table.count; ++i)
    {
      table.data[i-1] = table.data[i];
    }
    --table.count;
    m_ctrlList.DeleteItem(n);
  }
}

bool get_next_record(std::istream& in, std::string& rec)
{
  while (std::getline(in, rec))
  {
    if (rec.empty())
      continue;

    std::string::size_type pos = rec.find_first_not_of(" \n\r\t");
    if (pos == std::string::npos)
      continue;

    if (rec[pos] == '#')
      continue;

    return true;
  }
  return false;
}

void CDataMapping::OnButton1() // load
{
  CFileDialog dlg(TRUE);
  if (dlg.DoModal() == IDOK)
  {
    std::ifstream in(dlg.GetPathName());

    std::string rec;
    if (!get_next_record(in, rec))
    {
      AfxMessageBox("Input file is empty");
      return;
    }

    float z1, z2;
    int r1, g1, b1, r2, g2, b2;
    {
      std::istringstream ss(rec);
      if (ss >> z1 >> r1 >> g1 >> b1 >> z2 >> r2 >> g2 >> b2)
      {
        if (z1 < 2)
        {
          table.data[0].value = z1;
          table.data[0].color = RGB(r1, g1, b1);
          table.data[1].value = z2;
          table.data[1].color = RGB(r2, g2, b2);
        }
        else
        {
          AfxMessageBox("Strictly increasing order required");
          return;
        }
      }
      else
      {
        AfxMessageBox("File format is invalid");
        return;
      }
    }

    for (int i = 2; get_next_record(in, rec); ++i)
    {
      std::istringstream ss(rec);
      if (ss >> z1 >> r1 >> g1 >> b1 && z1 == z2 && ss >> z2 >> r2 >> g2 >> b2)
      {
        table.data[i].value = z2;
        table.data[i].color = RGB(r2, g2, b2);
      }
      else
      {
        break;
      }
    }
    table.count = i;
    m_ctrlList.Initialize(&table, 0);
  }
}

void CDataMapping::OnButton2() // store
{
  CFileDialog dlg(FALSE);
  if (dlg.DoModal() == IDOK)
  {
    std::ofstream out(dlg.GetPathName());
    for (int i = 1; i < table.count; ++i)
    {
      out << table.data[i-1].value << ' ' 
        << (int) GetRValue(table.data[i-1].color) << ' '
        << (int) GetGValue(table.data[i-1].color) << ' ' 
        << (int) GetBValue(table.data[i-1].color) << ' '

        << table.data[i].value << ' ' 
        << (int) GetRValue(table.data[i].color) << ' '
        << (int) GetGValue(table.data[i].color) << ' ' 
        << (int) GetBValue(table.data[i].color) << '\n';
    }
  }
}
