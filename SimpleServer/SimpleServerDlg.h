
// SimpleServerDlg.h : header file
//

#pragma once


// CSimpleServerDlg dialog
class CSimpleServerDlg : public CDialog
{
// Construction
public:
	CSimpleServerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SIMPLESERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
//	afx_msg void OnClose();
	afx_msg void OnDestroy();
};
