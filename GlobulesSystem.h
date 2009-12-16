#pragma once

const UINT MAX_GLOBULES_COUNT = 30;
const unsigned BUFFERS_COUNT = 10;
const unsigned TIMER_PERIOD = 10;

struct Vector
{
    double x, y;
    Vector(double x = 0.0f, double y = 0.0f) : x(x), y(y) {}
    Vector & operator+=(const Vector & other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }
    Vector operator+(const Vector & other) const
    {
        Vector result = *this;
        result += other;
        return result;
    }
    Vector & operator-=(const Vector & other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }
    Vector operator-(const Vector & other) const
    {
        Vector result = *this;
        result -= other;
        return result;
    }
    Vector operator-() const
    {
        return Vector(-this->x, -this->y);
    }
    Vector & operator*=(const double a)
    {
        x *= a;
        y *= a;
        return *this;
    }
    Vector operator*(const double a) const
    {
        Vector result = *this;
        result *= a;
        return result;
    }
    Vector & operator/=(const double a)
    {
        x /= a;
        y /= a;
        return *this;
    }
    Vector operator/(const double a) const
    {
        Vector result = *this;
        result /= a;
        return result;
    }
    double operator*(const Vector & other) const
    {
        return x*other.x + y*other.y;
    }
    double abs()
    {
        return sqrt(*this * *this);
    }
};

struct Globule
{
    Vector r, v;
    double radius;
    RGBQUAD color;
    inline double mass() { return radius * radius; }
};

class GlobulesSystem
{
protected:
    RGBQUAD * bits_buffers[BUFFERS_COUNT];
    volatile bool empty;
    volatile unsigned reader;
    volatile unsigned writer;
    CCriticalSection bufCS;

    SIZE size;
    volatile bool working;
    HANDLE thread;

    std::vector<Globule> globules;
    double gravity;
    double elasticity;
    double viscosity;
    Vector wind;

    void CollideWithWalls(Globule &g);
    void CollideThem(Globule &g1, Globule &g2);
    void AccelerateOne(Globule &g, double delta);
    void MoveOne(Globule &g, double delta);
    void ProcessPhysics();

    void AddRandomGlobule();
    void RemoveGlobule();

    void DrawGlobules(RGBQUAD *buf);

    RGBQUAD * GetBufferForWrite();
    void ChangeBufferForWrite();

public:
    GlobulesSystem(LONG buffer_width, LONG buffer_height, unsigned g_count);
    ~GlobulesSystem();

    void CreateThread();
    void Stop();

    static DWORD WINAPI CalcAndRender(LPVOID param);

    const RGBQUAD * GetBufferForRead();
    void ChangeBufferForRead();

    void SetGlobulesCount(unsigned count);
    void SetGravity(double g);
    void SetElasticity(double e);
    void SetViscosity(double v);
    void SetWind(double power, double angle); // 0..2*Pi
};
