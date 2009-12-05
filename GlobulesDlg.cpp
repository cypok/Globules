// GlobulesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Globules.h"
#include "GlobulesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static float randf()
{
    return static_cast<float>(rand()) / RAND_MAX;
}

static RGBQUAD RandColor()
{
    RGBQUAD c = {rand() % 200, rand() % 200, rand() % 200, 0};
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
    srand( (unsigned)time( NULL ) );
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
    globules_count = 5;

    gravity_slider.SetRange(0, 50, TRUE);
    gravity_slider.SetPos(10);

    UpdateData(FALSE);

    SetTimer(0, TIMER_PERIOD, NULL);

    CRect size;
    canvas.GetWindowRect(&size);
    ScreenToClient(&size);

    gs = new GlobulesSystem(size.Width(), size.Height(), globules_count);
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

void CGlobulesDlg::OnBnClickedButton1()
{
    UpdateData();
    LoadDataToGS();
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

void GlobulesSystem::CreateThread()
{
    thread = ::CreateThread(NULL, 0, &GlobulesSystem::CalcAndRender, this, 0, NULL);
}

void GlobulesSystem::Stop()
{
    working = false;
    WaitForSingleObject(thread, INFINITE);
}

static ULONGLONG GetTimeInMS()
{
    SYSTEMTIME system_time;
    GetSystemTime( &system_time );

    FILETIME file_time;
    SystemTimeToFileTime( &system_time, &file_time );

    ULARGE_INTEGER uli;
    uli.LowPart = file_time.dwLowDateTime;
    uli.HighPart = file_time.dwHighDateTime;

    return uli.QuadPart/10000;
}

void GlobulesSystem::CollideWithWalls(unsigned i)
{
    Globule &g = globules[i];

    if((g.r.x < 0.0f + g.radius && g.v.x < 0) || (g.r.x > 1.0f - g.radius && g.v.x > 0))
        g.v.x *= -1;

    if((g.r.y < 0.0f + g.radius && g.v.y < 0) || (g.r.y > 1.0f - g.radius && g.v.y > 0))
        g.v.y *= -1;
}

void GlobulesSystem::CollideThem(unsigned i, unsigned j)
{
    Globule &g1 = globules[i], &g2 = globules[j];
    if( (g1.r - g2.r).abs() < g1.radius + g2.radius )
    {
        Vector n = (g2.r - g1.r)/(g2.r - g1.r).abs();
        float a1 = g1.v * n;
        float a2 = g2.v * n;

        if( (a1 < 0 && a1 < a2) ||
            (a2 > 0 && a2 > a1))
            return;

        Vector v1 = n * a1;
        Vector v2 = n * a2;

        g1.v += v2 - v1;
        g2.v += v1 - v2;
    }
}

void GlobulesSystem::MoveOne(unsigned i, float delta)
{
    globules[i].r += globules[i].v * delta;
}

void GlobulesSystem::ProcessPhysics()
{
    static ULONGLONG before_now = GetTimeInMS();
    ULONGLONG now = GetTimeInMS();
    float delta = static_cast<float>(now - before_now) / 1000.0f;
    before_now = now;

    for(unsigned i = 0; i < globules.size(); ++i)
        CollideWithWalls(i);
    
    for(unsigned i = 0; i < globules.size(); ++i)
        for(unsigned j = 0; j < i; ++j)
            CollideThem(i, j);

    for(unsigned i = 0; i < globules.size(); ++i)
        MoveOne(i, delta);
}

DWORD WINAPI GlobulesSystem::CalcAndRender(LPVOID param)
{
    GlobulesSystem *gs = reinterpret_cast<GlobulesSystem *>(param);
    gs->working = true;
    
    while (gs->working)
    {
        gs->ProcessPhysics();

        RGBQUAD *buf = gs->GetBufferForWrite();

        if (buf == NULL) // it's nothing to draw
            continue; 

        // draw background
        for(int i = 0; i < gs->size.cy; ++i)
            for(int j = 0; j < gs->size.cx; ++j)
            {
                bool wall = (i == 0 || i+1 == gs->size.cy ||
                             j == 0 || j+1 == gs->size.cx);
                if(!wall)
                    buf[gs->size.cx*i + j] = Color(255, 255, 255);
            }

        gs->DrawGlobules(buf);

        // draw walls
        for(int i = 0; i < gs->size.cy; ++i)
            for(int j = 0; j < gs->size.cx; ++j)
            {
                bool wall = (i == 0 || i+1 == gs->size.cy ||
                             j == 0 || j+1 == gs->size.cx);
                if(wall)
                    buf[gs->size.cx*i + j] = Color(0, 0, 0);
            }

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

const RGBQUAD * GlobulesSystem::GetBufferForRead()
{
    bufCS.Lock();
    RGBQUAD * res = empty ? NULL : bits_buffers[reader];
    bufCS.Unlock();
    return res;
}

RGBQUAD * GlobulesSystem::GetBufferForWrite()
{
    bufCS.Lock();
    RGBQUAD * res = (!empty && reader == writer) ? NULL : bits_buffers[writer];
    bufCS.Unlock();
    return res;
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

void GlobulesSystem::AddRandomGlobule()
{
    Globule g;
    g.color = RandColor();
    g.radius = 0.01f + randf() * 0.1f; // 10..50
    g.r.x = g.radius + (1.0f - 2*g.radius) * randf();
    g.r.y = g.radius + (1.0f - 2*g.radius) * randf();
    g.v.x = -0.2f + 0.4f * randf();
    g.v.y = -0.2f + 0.4f * randf();

    globules.push_back(g);
}

void GlobulesSystem::RemoveGlobule()
{
    if(!globules.empty())
        globules.pop_back();
}

GlobulesSystem::GlobulesSystem(LONG buffer_width, LONG buffer_height, unsigned g_count)
{
    ASSERT(buffer_width == buffer_height);

    size.cx = buffer_width;
    size.cy = buffer_height;


    for(unsigned i = 0; i < BUFFERS_COUNT; ++i)
        bits_buffers[i] = new RGBQUAD[size.cx * size.cy];

    empty = true;
    reader = 0;
    writer = 0;

    for(unsigned i = 0; i < g_count; ++i)
        AddRandomGlobule();

    thread = NULL;
}

GlobulesSystem::~GlobulesSystem()
{
    delete[] bits_buffers[0];
}

void GlobulesSystem::SetGlobulesCount(unsigned count)
{
    while( count > globules.size() )
        AddRandomGlobule();
    while( count < globules.size() )
        RemoveGlobule();
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
    for(unsigned i = 0; i < globules.size(); ++i)
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

void CGlobulesDlg::LoadDataToGS()
{
    gs->SetGlobulesCount(globules_count);
}
