// GlobulesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Globules.h"
#include "GlobulesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CGlobulesDlg::CGlobulesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobulesDlg::IDD, pParent)
    , template_name(_T(""))
    , globules_count(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGlobulesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_CBString(pDX, IDC_COMBO1, template_name);
    DDX_Control(pDX, IDC_SLIDER1, gravity_slider);
    DDX_Text(pDX, IDC_EDIT1, globules_count);
    DDV_MinMaxUInt(pDX, globules_count, 0, 10);
    DDX_Control(pDX, IDC_SPIN1, globules_count_spiner);
}

BEGIN_MESSAGE_MAP(CGlobulesDlg, CDialog)
	ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BUTTON1, &CGlobulesDlg::OnBnClickedButton1)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CGlobulesDlg::OnDeltaposSpin1)
END_MESSAGE_MAP()


BOOL CGlobulesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


    template_name = _T("[none]");
    globules_count = 5;

    gravity_slider.SetRange(0, 50, TRUE);
    gravity_slider.SetPos(10);

    UpdateData(FALSE);

	return TRUE;
}

void CGlobulesDlg::OnPaint()
{
	CDialog::OnPaint();
}

void CGlobulesDlg::OnBnClickedButton1()
{
    UpdateData();
}

void CGlobulesDlg::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    // TODO: Add your control notification handler code here
    int count = globules_count - pNMUpDown->iDelta;
    globules_count = min(max(0,count), MAX_GLOBULES_COUNT);
    UpdateData(FALSE);
    // TODO: range!
    *pResult = 0;
}
