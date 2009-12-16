// GlobulesDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "GlobulesSystem.h"
#include "WindCtrl.h"

const UINT MAX_GLOBULES_COUNT = 30;
const unsigned TIMER_PERIOD = 10;
const double MAX_WIND_POWER = 1;

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
    WindCtrl wind_chooser;
    afx_msg void OnStnClickedWind();
};
