// LogtrackType.cpp : implementation file
//

#include "stdafx.h"
#include <atldbcli.h>
#include <string>
#include <map>
#include <iostream>
#include "Field.h"
#include "FieldDoc.h"
#include "FieldView.h"
#include "LogtrackType.h"

// CLogtrackType dialog

IMPLEMENT_DYNAMIC(CLogtrackType, CDialog)
CLogtrackType::CLogtrackType(CWnd* pParent /*=NULL*/)
	: CDialog(CLogtrackType::IDD, pParent)
{
}

CLogtrackType::~CLogtrackType()
{
}

void CLogtrackType::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_wndLogtrack);
}


BEGIN_MESSAGE_MAP(CLogtrackType, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

void CLogtrackType::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	if (bShow)
	{
		if (!m_wndLogtrack.GetCount())
		{
			CWaitCursor wait;
			CActField* pDoc = GetFieldView()->GetDocument();
			ASSERT_VALID(pDoc);

			CCommand < CAccessor < DictRecord > > dict;
			HRESULT hr = dict.Open(pDoc->session, "select * from dict where dict_group=1000701");
			if (SUCCEEDED(hr))
			{
				typedef std::map<std::string, DictRecord > text_table;
				text_table tt;
				while (dict.MoveNext() == S_OK)
				{
					if (dict.status.m_eng == S_OK)
					{
						std::string key = dict.m_eng;
						text_table::iterator it = tt.find(key);
						if (it == tt.end())
						{
							dict.dirty = false;
							tt.insert(std::make_pair(key, (DictRecord) dict));
						}
					}
				}

				CTable < CAccessor < WellRecord > > tabl;
				hr = tabl.Open(pDoc->session, "logtrack");
				if (SUCCEEDED(hr))
				{
					while (tabl.MoveNext() == S_OK)
					{
						char buff[0x100];
						std::string key = tabl.type;
						text_table::iterator it = tt.find(tabl.type);
						if (it != tt.end())
						{
							DictRecord* pRec = &it->second;
							const char* comment = 0;
							if (!pRec->dirty)
							{
								if (pRec->status.d_rus == S_OK)
								{
									comment = pRec->d_rus;
								}
								else if (pRec->status.d_eng == S_OK)
								{
									comment = pRec->d_eng;
								}
								else if (pRec->status.t_rus == S_OK)
								{
									comment = pRec->t_rus;
								}
								else if (pRec->status.t_eng == S_OK)
								{
									comment = pRec->t_eng;
								}
								else if (pRec->status.m_rus == S_OK)
								{
									comment = pRec->m_rus;
								}
								else if (pRec->status.m_eng == S_OK)
								{
									comment = pRec->m_eng;
								}
								else
								{
									fprintf(stderr, "%s\n", tabl.type);
								}
								pRec->dirty = true;
								sprintf(buff, " [%s] %s", tabl.type, comment? comment: "");
								m_wndLogtrack.AddString(buff);
							}
						}
						else
						{
							DictRecord rec;
							rec.status.d_eng = DBSTATUS_S_ISNULL;
							rec.status.d_rus = DBSTATUS_S_ISNULL;
							rec.status.t_eng = DBSTATUS_S_ISNULL;
							rec.status.t_rus = DBSTATUS_S_ISNULL;
							rec.status.m_eng = S_OK;
							rec.status.m_rus = DBSTATUS_S_ISNULL;
							strcpy(rec.m_eng, tabl.type);
							rec.dirty = true;
							tt.insert(std::make_pair(key, rec));
							sprintf(buff, " [%s]", tabl.type);
							m_wndLogtrack.AddString(buff);
						}
					}
				}
			}
		}

		for (int i = 0, n = m_wndLogtrack.GetCount(); i < n; ++i)
		{
			CString str;
			m_wndLogtrack.GetText(i, str);
			int n1 = str.Find('[');
			int n2 = str.Find(']', n1);
			str = str.Mid(n1, n2);
			if (str == "[SP]" || str == "[PZ]" || str == "[IL]")
			{
				m_wndLogtrack.SetCheck(i, 1);
			}
		}
	}
}
