// GlobulesDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

const UINT MAX_GLOBULES_COUNT = 10;

class GlobulesSystem
{
protected:
    RGBQUAD * bits_buffers[1];
    SIZE size;
public:
    GlobulesSystem(LONG buffer_width, LONG buffer_height);
    ~GlobulesSystem();

    static DWORD WINAPI CalcAndRender(LPVOID param);
    RGBQUAD * GetBufferForRead();
    RGBQUAD * GetBufferForWrite();
};


// CGlobulesDlg dialog
class CGlobulesDlg : public CDialog
{
// Construction
public:
	CGlobulesDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GLOBULES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
    GlobulesSystem *gs;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

    virtual void PostNcDestroy();
public:
    CString template_name;
    afx_msg void OnBnClickedButton1();
    CSliderCtrl gravity_slider;
    UINT globules_count;
    CSpinButtonCtrl globules_count_spiner;
    afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
    CStatic canvas;
    void Redraw();
};
