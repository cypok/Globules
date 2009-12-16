// GlobulesDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "GlobulesSystem.h"

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
    virtual void OnOK();

    virtual void PostNcDestroy();
public:
    CString template_name;
    CSliderCtrl gravity_slider;
    UINT globules_count;
    CSpinButtonCtrl globules_count_spiner;
    afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
    CStatic canvas;
    void Redraw();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    void LoadDataToGS();
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    CSliderCtrl elasticity_slider;
    CSliderCtrl viscosity_slider;
    CSliderCtrl wind_power_slider;
};
