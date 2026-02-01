// CFragmentViewer

class SoWinExaminerViewer;
class SoSeparator;

class CFragmentViewer : public CWnd
{
	DECLARE_DYNAMIC(CFragmentViewer)

public:
	CFragmentViewer();
	virtual ~CFragmentViewer();

protected:
	DECLARE_MESSAGE_MAP()

private:
	SoWinExaminerViewer* view;
  friend void CFieldView::OnFieldStart3dviewer();
public:
	afx_msg void OnFileSave();
	afx_msg void OnFileClose();
};


