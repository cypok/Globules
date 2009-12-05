// GlobulesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Globules.h"
#include "GlobulesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static RGBQUAD RandColor()
{
    RGBQUAD c = {rand() % 256, rand() % 256, rand() % 256, 0};
    return c;
}

static RGBQUAD Color(BYTE r, BYTE g, BYTE b)
{
    RGBQUAD c = {b, g, r, 0};
    return c;
}

CGlobulesDlg::CGlobulesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGlobulesDlg::IDD, pParent)
    , template_name(_T(""))
    , globules_count(1)
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
    DDX_Control(pDX, IDC_STATIC2, canvas);
}

BEGIN_MESSAGE_MAP(CGlobulesDlg, CDialog)
	ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BUTTON1, &CGlobulesDlg::OnBnClickedButton1)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &CGlobulesDlg::OnDeltaposSpin1)
    ON_WM_TIMER()
END_MESSAGE_MAP()


BOOL CGlobulesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


    template_name = _T("[none]");
    globules_count = 2;

    gravity_slider.SetRange(0, 50, TRUE);
    gravity_slider.SetPos(10);

    UpdateData(FALSE);

    SetTimer(0, 100, NULL);

    CRect size;
    canvas.GetWindowRect(&size);
    ScreenToClient(&size);
    gs = new GlobulesSystem(size.Width(), size.Height(), globules_count);
    
    Globule g = {Vector(0.1f, 0.1f), Vector(1.0f, 2.0f), 0.05f, Color(255, 0, 0)};
    gs->SetGlobule(0, g);
    g.r = Vector(0.9f, 0.5f);
    g.color = Color(0, 0, 255);
    gs->SetGlobule(1, g);

    gs->CreateThread();

	return TRUE;
}

void CGlobulesDlg::OnPaint()
{
	CDialog::OnPaint();

    RGBQUAD *buf = gs->GetBufferForRead();

    if (buf == NULL) // it's nothing to draw
        return;

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

    bmp.SetBitmapBits(sizeof(RGBQUAD)*size.Width()*size.Height(), buf);

    dc.BitBlt(size.left, size.top, size.Width(), size.Height(), &mem, 0, 0, SRCCOPY);
    mem.SelectObject(old);

    gs->ChangeBufferForRead();
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

void GlobulesSystem::CreateThread()
{
    thread = ::CreateThread(NULL, 0, &GlobulesSystem::CalcAndRender, this, 0, NULL);
}

void GlobulesSystem::Stop()
{
    working = false;
    WaitForSingleObject(thread, INFINITE);
}

DWORD WINAPI GlobulesSystem::CalcAndRender(LPVOID param)
{
    GlobulesSystem *gs = reinterpret_cast<GlobulesSystem *>(param);
    gs->working = true;
    
    while (gs->working)
    {
        RGBQUAD *buf = gs->GetBufferForWrite();

        if (buf == NULL) // it's nothing to draw
        {
            Sleep(100);
            continue; 
        }

        gs->globules[0].r += Vector(0.01f, 0.005f);

        // draw walls
        for(int i = 0; i < gs->size.cy; ++i)
            for(int j = 0; j < gs->size.cx; ++j)
            {
                bool wall = (i == 0 || i+1 == gs->size.cy ||
                             j == 0 || j+1 == gs->size.cx);
                buf[gs->size.cx*i + j] = wall ? Color(0,0,0) : Color(255, 255, 255);
            }
        gs->DrawGlobules(buf);

        gs->ChangeBufferForWrite();
    }
    return 0;
}

void CGlobulesDlg::PostNcDestroy()
{
    gs->Stop();
    delete gs;

    CDialog::PostNcDestroy();
}

RGBQUAD * GlobulesSystem::GetBufferForRead()
{
    return empty ? NULL : bits_buffers[reader];
}

RGBQUAD * GlobulesSystem::GetBufferForWrite()
{
    return (!empty && reader == writer) ? NULL : bits_buffers[writer];
}

void GlobulesSystem::ChangeBufferForRead()
{
    bufCS.Lock();
    reader = (reader + 1) % BUFFERS_COUNT;
    if (reader == writer)
        empty = true;
    bufCS.Unlock();
}

void GlobulesSystem::ChangeBufferForWrite()
{
    bufCS.Lock();
    writer = (writer + 1) % BUFFERS_COUNT;
    if (empty)
        empty = false;
    bufCS.Unlock();
}

GlobulesSystem::GlobulesSystem(LONG buffer_width, LONG buffer_height, unsigned g_count) :
    globules_count(g_count)
{
    ASSERT(buffer_width == buffer_height);

    size.cx = buffer_width;
    size.cy = buffer_height;

    for(unsigned i = 0; i < BUFFERS_COUNT; ++i)
        bits_buffers[i] = new RGBQUAD[size.cx * size.cy];

    empty = true;
    reader = 0;
    writer = 0;

    globules = new Globule[globules_count];

    thread = NULL;
}

GlobulesSystem::~GlobulesSystem()
{
    delete[] bits_buffers[0];
    delete[] globules;
}

static void DrawCircle(RGBQUAD *buf, float x, float y, float radius, RGBQUAD color, int buf_width, int buf_height)
{
    for(int i = max(0, static_cast<int>(floor(y-radius))); i < min(buf_height-1, ceil(y+radius)); ++i)
        for(int j = max(0, static_cast<int>(floor(x-radius))); j < min(buf_width-1, ceil(x+radius)); ++j)
            if ((i-y)*(i-y) + (x-j)*(x-j) <= radius*radius)
                buf[i*buf_width + j] = color;
}

void GlobulesSystem::DrawGlobules(RGBQUAD *buf)
{
    for(unsigned i = 0; i < globules_count; ++i)
    {
        Globule &g = globules[i];
        DrawCircle(buf,
            static_cast<float>(size.cx) * g.r.x,
            static_cast<float>(size.cy) * (1 - g.r.y),
            sqrtf(static_cast<float>(size.cx * size.cy)) * g.radius,
            g.color,
            size.cx, size.cy);
    }
}

void GlobulesSystem::SetGlobule(unsigned num, Globule g)
{
    globules[num] = g;
}
