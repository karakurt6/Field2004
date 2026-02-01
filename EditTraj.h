#pragma once
// #include "d:\build\mfc-6.0\include\afxcmn.h"
// #include "d:\build\mfc-6.0\include\afx.h"
// #include "d:\build\mfc-6.0\include\afxwin.h"


// CEditTraj dialog

class CEditTraj : public CDialog
{
  DECLARE_DYNAMIC(CEditTraj)

public:
  CEditTraj(CActField* doc, long bore, CWnd* pParent = NULL);   // standard constructor
  virtual ~CEditTraj();

// Dialog Data
  enum { IDD = IDD_INCLINOMETRY };

protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

  DECLARE_MESSAGE_MAP()
public:
//  virtual BOOL OnInitDialog();
  CListCtrl m_wndList;
  CEdit m_wndDX;
  CEdit m_wndDY;
  CEdit m_wndDZ;
  CEdit m_wndMD;
  CEdit m_wndWellbore;
  CEdit m_wndParentbore;
  virtual BOOL OnInitDialog();

private:
  struct WellRecord
  {
    long well_bore;
    char well_name[20];
    float well_dx;
    float well_dy;
    float well_dz;
    float well_md;
    float well_hx;
    float well_hy;
    long well_type;
    long undertaker;
    float well_elev;
    float well_gnd;
    long condition;
    DATE spud_date;
    DATE comp_date;
    DATE prod_date;
    DATE drill_date;
    unsigned char traj_id;
    float plugback;
    long explore_area;
    long department;
    char lease_area[40];
    char cluster[6];
    long parent_bore;

    struct Status
    {
      DBSTATUS well_dx;
      DBSTATUS well_dy;
      DBSTATUS well_dz;
      DBSTATUS well_md;
      DBSTATUS well_elev;
      DBSTATUS well_gnd;
      DBSTATUS spud_date;
      DBSTATUS comp_date;
      DBSTATUS prod_date;
      DBSTATUS drill_date;
      DBSTATUS traj_id;
      DBSTATUS plugback;
      DBSTATUS lease_area;
      DBSTATUS cluster;
      DBSTATUS parent_bore;
    }
    status;

    BEGIN_COLUMN_MAP(WellRecord)
      COLUMN_ENTRY(2, well_bore)
      COLUMN_ENTRY(1, well_name)
      COLUMN_ENTRY_STATUS(3, well_dx, status.well_dx)
      COLUMN_ENTRY_STATUS(4, well_dy, status.well_dy)
      COLUMN_ENTRY_STATUS(5, well_dz, status.well_dz)
      COLUMN_ENTRY_STATUS(6, well_md, status.well_md)
      COLUMN_ENTRY(7, well_hx)
      COLUMN_ENTRY(8, well_hy)
      COLUMN_ENTRY(9, well_type)
      COLUMN_ENTRY(10, undertaker)
      COLUMN_ENTRY_STATUS(11, well_elev, status.well_elev)
      COLUMN_ENTRY_STATUS(12, well_gnd, status.well_gnd)
      COLUMN_ENTRY(13, condition)
      COLUMN_ENTRY_STATUS(14, spud_date, status.spud_date)
      COLUMN_ENTRY_STATUS(15, comp_date, status.comp_date)
      COLUMN_ENTRY_STATUS(16, prod_date, status.prod_date)
      COLUMN_ENTRY_STATUS(17, drill_date, status.drill_date)
      COLUMN_ENTRY_STATUS(18, traj_md, status.traj_md)
      COLUMN_ENTRY_STATUS(19, plugback, status.plugback)
      COLUMN_ENTRY(20, explore_area)
      COLUMN_ENTRY(21, department)
      COLUMN_ENTRY_STATUS(22, lease_area, status.lease_area)
      COLUMN_ENTRY_STATUS(23, cluster, status.cluster)
      COLUMN_ENTRY_STATUS(24, parent_bore, status.parent_bore)
    END_COLUMN_MAP()
  };

#pragma pack(push, 4)
  struct TrajSurvey
  {
    float dx;
    float dy;
    float dz;
    float md;
  };
#pragma pack(pop)

  struct TrajRecord
  {
    long traj_wb;
    unsigned char traj_id;
    TrajSurvey tail;
    short traj_ns;
    BEGIN_COLUMN_MAP(TrajRecord)
      COLUMN_ENTRY(1, traj_wb)
      COLUMN_ENTRY(2, traj_id)
      COLUMN_ENTRY(3, tail.dx)
      COLUMN_ENTRY(4, tail.dy)
      COLUMN_ENTRY(5, tail.dz)
      COLUMN_ENTRY(6, tail.md)
      COLUMN_ENTRY(7, traj_ns)
    END_COLUMN_MAP()
  };

  CActField* m_pDoc;
  long m_nBore;

	HRESULT CEditTraj::BindWell(IAccessor* pAccessor, HACCESSOR* pResult);
	HRESULT CEditTraj::UpdateWell(long bore, long parent, int ns, TrajSurvey* ss, ITransactionLocal** pResult);
	HRESULT CEditTraj::BindTraj(IAccessor* pAccessor, HACCESSOR* pResult);
	HRESULT CEditTraj::UpdateTraj(long bore, int ns, TrajSurvey* ss, ITransactionLocal** pResult);

public:
  afx_msg void OnLvnItemActivateList1(NMHDR *pNMHDR, LRESULT *pResult);
  afx_msg void OnBnClickedButton5();
  afx_msg void OnBnClickedButton4();
protected:
  virtual void OnOK();
};
