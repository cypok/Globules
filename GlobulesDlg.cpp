// GlobulesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Globules.h"
#include "GlobulesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct Template
{
    TCHAR name[20];
    double gravity;
    double elasticity;
    double viscosity;
    double wind_power;
    double wind_angle;
};

const static Template TEMPLATES[] = {
    {_T("Earth"),   0.98,   1,      0.1,    0, 0},
    {_T("Moon"),    0.3,    1,      0.01,   0.2, 0},
    {_T("Oil"),     0.0,    0.5,    0.05,   0.25, 3.14/2},
    {_T("Space"),   0.0,    1,      0.00,   0, 0},
};
const unsigned TEMPLATES_COUNT = sizeof(TEMPLATES)/sizeof(TEMPLATES[0]);

CGlobulesDlg::CGlobulesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobulesDlg::IDD, pParent)
    , template_name(_T(""))
    , globules_count(1)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    srand( (unsigned)time( NULL ) );
}

void CGlobulesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_CBString(pDX, IDC_COMBO1, template_name);
    DDX_Control(pDX, IDC_SLIDER1, gravity_slider);
    DDX_Text(pDX, IDC_EDIT1, globules_count);
    DDV_MinMaxUInt(pDX, globules_count, 0, MAX_GLOBULES_COUNT);
    DDX_Control(pDX, IDC_SPIN1, globules_count_spiner);
    DDX_Control(pDX, IDC_STATIC2, canvas);
    DDX_Control(pDX, IDC_SLIDER2, elasticity_slider);
    DDX_Control(pDX, IDC_SLIDER3, viscosity_slider);
    DDX_Control(pDX, IDC_WIND, wind_chooser);
}

BEGIN_MESSAGE_MAP(CGlobulesDlg, CDialog)
	ON_WM_PAINT()
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CGlobulesDlg::OnDeltaposSpin1)
    ON_WM_TIMER()
    ON_WM_HSCROLL()
    ON_STN_CLICKED(IDC_WIND, &CGlobulesDlg::OnStnClickedWind)
    ON_CBN_SELCHANGE(IDC_COMBO1, &CGlobulesDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


BOOL CGlobulesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


    template_name = _T("Space");
    globules_count = 5;

    gravity_slider.SetRange(0, 50, TRUE);

    elasticity_slider.SetRange(0, 50, TRUE);

    viscosity_slider.SetRange(0, 50, TRUE);

    UpdateData(FALSE);


    SetTimer(0, TIMER_PERIOD, NULL);

    CRect size;
    canvas.GetWindowRect(&size);
    ScreenToClient(&size);

    gs = new GlobulesSystem(size.Width(), size.Height(), globules_count);

    SelectTemplate();
    
    gs->CreateThread();

	return TRUE;
}

void CGlobulesDlg::OnPaint()
{
	CDialog::OnPaint();

    const RGBQUAD *buf = gs->GetBufferForRead();

	CPaintDC dc(&canvas); // device context for painting

    CRect size;
    canvas.GetClientRect(&size);

    static CBitmap bmp;
    static bool initialized = false;
    if (!initialized)
    {
        bmp.CreateCompatibleBitmap(&dc, size.Width(), size.Height());
        initialized = true;
    }

    CDC mem;
    mem.CreateCompatibleDC(&dc);
    CBitmap *old = mem.SelectObject(&bmp);

    // copy from buffer if it is given
    if (buf)
    {
        bmp.SetBitmapBits(sizeof(RGBQUAD)*size.Width()*size.Height(), buf);
        gs->ChangeBufferForRead();
    }

    dc.BitBlt(size.left, size.top, size.Width(), size.Height(), &mem, 0, 0, SRCCOPY);
    mem.SelectObject(old);
}

void CGlobulesDlg::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

    int count = globules_count - pNMUpDown->iDelta;
    globules_count = min(max(0,count), MAX_GLOBULES_COUNT);

    UpdateData(FALSE);

    LoadDataToGS();

    *pResult = 0;
}

void CGlobulesDlg::Redraw()
{
    static unsigned counter = 0;
    CRect size;
    canvas.GetWindowRect(&size);
    ScreenToClient(&size);

    InvalidateRect(size, 0);
    UpdateWindow();
}

void CGlobulesDlg::OnTimer(UINT_PTR nIDEvent)
{
    Redraw();
    CDialog::OnTimer(nIDEvent);
}


void CGlobulesDlg::PostNcDestroy()
{
    gs->Stop();
    delete gs;

    CDialog::PostNcDestroy();
}

void CGlobulesDlg::LoadDataToGS()
{
    UpdateData();
    gs->SetGlobulesCount(globules_count);
    gs->SetGravity(static_cast<double>(gravity_slider.GetPos())/25);
    gs->SetElasticity(static_cast<double>(elasticity_slider.GetPos())/50);
    gs->SetViscosity(static_cast<double>(viscosity_slider.GetPos())/50000);
    gs->SetWind(MAX_WIND_POWER*wind_chooser.GetPower(), wind_chooser.GetAngle());
}

void CGlobulesDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default
    LoadDataToGS();

    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGlobulesDlg::OnOK()
{
    LoadDataToGS();
}

void CGlobulesDlg::OnStnClickedWind()
{
    LoadDataToGS();
}

void CGlobulesDlg::SelectTemplate()
{
    if (template_name == _T("[none]"))
        return;
    
    for(unsigned i = 0; i < TEMPLATES_COUNT; ++i)
        if (template_name == TEMPLATES[i].name)
        {
            Template t = TEMPLATES[i];
            gravity_slider.SetPos(int(t.gravity*25));
            elasticity_slider.SetPos(int(t.elasticity*50));
            viscosity_slider.SetPos(int(t.viscosity*50000));
            wind_chooser.SetVars(t.wind_power, t.wind_angle);
            LoadDataToGS();
            UpdateData(FALSE);
        }
}

void CGlobulesDlg::OnCbnSelchangeCombo1()
{
    UpdateData();
    SelectTemplate();
}
