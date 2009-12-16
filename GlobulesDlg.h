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

    CComboBox templates;
    CString template_name;
    CStatic canvas;
    CSpinButtonCtrl globules_count_spiner;
    UINT globules_count;
    CSliderCtrl gravity_slider;
    CSliderCtrl elasticity_slider;
    CSliderCtrl viscosity_slider;
    WindCtrl wind_chooser;

    void SelectTemplate();
    void LoadDataToGS();
    void Redraw();

	// Generated message map functions
	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void PostNcDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnStnClickedWind();
    afx_msg void OnCbnSelchangeCombo1();
};
