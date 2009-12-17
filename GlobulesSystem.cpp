#include "stdafx.h"
#include "GlobulesSystem.h"

static double randf()
{
    return static_cast<double>(rand()) / RAND_MAX;
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

void GlobulesSystem::CollideWithWalls(Globule &g)
{
    double deepness;
    
    deepness = 0.0 + g.radius - g.r.x;
    if (deepness > 0)
    {
        if( g.v.x < 0)
            g.v.x *= -elasticity;
        g.r.x += deepness;
    }

    deepness = g.r.x - (1.0 - g.radius);
    if (deepness > 0)
    {
        if( g.v.x > 0 )
            g.v.x *= -elasticity;
        g.r.x -= deepness;
    }

    deepness = 0.0 + g.radius - g.r.y;
    if (deepness > 0)
    {
        if( g.v.y < 0)
            g.v.y *= -elasticity;
        g.r.y += deepness;
    }

    deepness = g.r.y - (1.0 - g.radius);
    if (deepness > 0)
    {
        if( g.v.y > 0 )
            g.v.y *= -elasticity;
        g.r.y -= deepness;
    }
}

static void CentralCollision(Vector n21, double elasticity, double m1, double m2, Vector v1, Vector v2, Vector *u1, Vector *u2)
{
    Vector V = (v1*m1 + v2*m2)/(m1 + m2);
    double energy = elasticity*elasticity*m1*((v1-V)*(v1-V)) + m2*((v2-V)*(v2-V));
    *u1 = V - n21 * sqrt(energy/m1/(1+m1/m2));
    *u2 = V + n21 * sqrt(energy/m2/(1+m2/m1));
}

void GlobulesSystem::CollideThem(Globule &g1, Globule &g2)
{
    double deepness = g1.radius + g2.radius - (g1.r - g2.r).abs();
    if( deepness > 0 )
    {
        Vector n = (g2.r - g1.r)/(g2.r - g1.r).abs();
        double a1 = g1.v * n;
        double a2 = g2.v * n;

        if( (a1 > 0 && a2 < 0)  || a1 > a2)
        {
            Vector v1 = n * a1;
            Vector v2 = n * a2;
            Vector u1, u2;

            CentralCollision(n, elasticity, g1.mass(), g2.mass(), v1, v2, &u1, &u2);

            g1.v += u1 - v1;
            g2.v += u2 - v2;
        }

        g1.r -= n * (deepness/2);
        g2.r += n * (deepness/2);
    }
}

void GlobulesSystem::AccelerateOne(Globule &g, double delta)
{
    // m*g
    Vector gravity_part = Vector(0, -1) * g.mass() * gravity;

    // - v * S * viscosity,  where S ~ r^2
    Vector viscosity_part = -g.v * g.radius * g.radius * viscosity;

    // wind
    Vector wind_part =  wind *                  // same direction and power as wind
                        max(0, 1 - g.v*wind) *  // project of v to wind vector
                        g.radius * g.radius;    // S ~ r^2

    g.v += ( gravity_part + viscosity_part + wind_part) / g.mass() * delta;
}

void GlobulesSystem::MoveOne(Globule &g, double delta)
{
    g.r += g.v * delta;
}

void GlobulesSystem::ProcessPhysics()
{
    static ULONGLONG before_now = GetTimeInMS();
    ULONGLONG now = GetTimeInMS();
    double delta = static_cast<double>(now - before_now) / 1000.0;
    before_now = now;

    for(unsigned i = 0; i < globules.size(); ++i)
        CollideWithWalls(globules[i]);
    
    for(unsigned i = 0; i < globules.size(); ++i)
        for(unsigned j = 0; j < i; ++j)
            CollideThem(globules[i], globules[j]);

    for(unsigned i = 0; i < globules.size(); ++i)
    {
        AccelerateOne(globules[i], delta);
        MoveOne(globules[i], delta);
    }
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
        for(int i = 1; i < gs->size.cy-1; ++i)
            for(int j = 1; j < gs->size.cx-1; ++j)
                buf[gs->size.cx*i + j] = Color(255, 255, 255);


        // draw walls
        for(int j = 0; j < gs->size.cx; ++j)
        {
            buf[j] = Color(0, 0, 0); // top
            buf[gs->size.cx*(gs->size.cy-1) + j] = Color(0, 0, 0); // bottom
        }
        for(int i = 1; i < gs->size.cy-1; ++i)
        {
            buf[gs->size.cx*i] = Color(0, 0, 0); // left
            buf[gs->size.cx*i + gs->size.cx-1] = Color(0, 0, 0); // right
        }
        gs->DrawGlobules(buf);

        gs->ChangeBufferForWrite();
    }
    return 0;
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
    g.radius = 0.01f + randf() * 0.1;
    g.r.x = g.radius + (1.0 - 2*g.radius) * randf();
    g.r.y = g.radius + (1.0 - 2*g.radius) * randf();
    g.v.x = -0.05 + 0.1 * randf();
    g.v.y = -0.05 + 0.1 * randf();

    globules.push_back(g);
}

void GlobulesSystem::RemoveGlobule()
{
    if(!globules.empty())
        globules.pop_back();
}

void GlobulesSystem::SetGravity(double g)
{
    gravity = g;
}

void GlobulesSystem::SetElasticity(double e)
{
    elasticity = e;
}

void GlobulesSystem::SetViscosity(double v)
{
    viscosity = v;
}

void GlobulesSystem::SetWind(double power, double angle)
{
    wind = Vector(cos(angle),sin(angle))*power;
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
    gravity = 0;
}

GlobulesSystem::~GlobulesSystem()
{
    for(unsigned i = 0; i < BUFFERS_COUNT; ++i)
        delete[] bits_buffers[i];
}

void GlobulesSystem::SetGlobulesCount(unsigned count)
{
    while( count > globules.size() )
        AddRandomGlobule();
    while( count < globules.size() )
        RemoveGlobule();
}

static void DrawCircle(RGBQUAD *buf, double x, double y, double radius, RGBQUAD color, int buf_width, int buf_height)
{
    for(int i = max(0, static_cast<int>(floor(y-radius))); i < min(buf_height-2, ceil(y+radius)); ++i)
        for(int j = max(0, static_cast<int>(floor(x-radius))); j < min(buf_width-2, ceil(x+radius)); ++j)
            if ((i-y)*(i-y) + (j-x)*(j-x) <= radius*radius)
                buf[(i+1)*buf_width + (j+1)] = color;
}

void GlobulesSystem::DrawGlobules(RGBQUAD *buf)
{
    for(unsigned i = 0; i < globules.size(); ++i)
    {
        Globule &g = globules[i];
        DrawCircle(buf,
            static_cast<double>(size.cx-2) * g.r.x,
            static_cast<double>(size.cy-2) * (1 - g.r.y),
            sqrt(static_cast<double>((size.cx-2) * (size.cy-2))) * g.radius,
            g.color,
            size.cx, size.cy);
    }
}