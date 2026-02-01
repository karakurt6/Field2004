#pragma once
// CLogtrackType dialog

class CLogtrackType : public CDialog
{
	DECLARE_DYNAMIC(CLogtrackType)

public:
	struct WellRecord
	{
		long bore;
		unsigned char run;
		unsigned char pass;
		unsigned char version;
		char type[16];
		char unit[64];
		float null;
		float min_value;
		float max_value;
		long num;
		char depth_type[2];
		char depth_unit[64];
		float depth_step;
		float min_depth;
		float max_depth;

		BEGIN_COLUMN_MAP(WellRecord)
			COLUMN_ENTRY(1, bore)
			COLUMN_ENTRY(2, run)
			COLUMN_ENTRY(3, pass)
			COLUMN_ENTRY(4, version)
			COLUMN_ENTRY(5, type)
			COLUMN_ENTRY(6, unit)
			COLUMN_ENTRY(7, null)
			COLUMN_ENTRY(8, min_value)
			COLUMN_ENTRY(9, max_value)
			COLUMN_ENTRY(10, num)
			COLUMN_ENTRY(11, depth_type)
			COLUMN_ENTRY(12, depth_unit)
			COLUMN_ENTRY(13, depth_step)
			COLUMN_ENTRY(14, min_depth)
			COLUMN_ENTRY(15, max_depth)
		END_COLUMN_MAP()
	};

	struct DictRecord
	{
		long item;
		long group;
		char m_rus[16];
		char m_eng[16];
		char t_rus[16];
		char t_eng[16];
		char d_rus[64];
		char d_eng[64];
		bool dirty;

		struct Status
		{
			DBSTATUS m_rus;
			DBSTATUS m_eng;
			DBSTATUS t_rus;
			DBSTATUS t_eng;
			DBSTATUS d_rus;
			DBSTATUS d_eng;
		};

		Status status;

		BEGIN_COLUMN_MAP(DictRecord)
			COLUMN_ENTRY(1, item)
			COLUMN_ENTRY(2, group)
			COLUMN_ENTRY_STATUS(3, m_rus, status.m_rus)
			COLUMN_ENTRY_STATUS(4, m_eng, status.m_eng)
			COLUMN_ENTRY_STATUS(5, t_rus, status.t_rus)
			COLUMN_ENTRY_STATUS(6, t_eng, status.t_eng)
			COLUMN_ENTRY_STATUS(7, d_rus, status.d_rus)
			COLUMN_ENTRY_STATUS(8, d_eng, status.d_eng)
		END_COLUMN_MAP()
	};

	CLogtrackType(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogtrackType();

// Dialog Data
	enum { IDD = IDD_LOGTRACK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CCheckListBox m_wndLogtrack;
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
