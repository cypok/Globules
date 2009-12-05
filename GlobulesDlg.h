// GlobulesDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

const UINT MAX_GLOBULES_COUNT = 10;
const unsigned BUFFERS_COUNT = 2;
const unsigned TIMER_PERIOD = 10;

struct Vector
{
    float x, y;
    Vector(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
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
    Vector & operator*=(const float a)
    {
        x *= a;
        y *= a;
        return *this;
    }
    Vector operator*(const float a) const
    {
        Vector result = *this;
        result *= a;
        return result;
    }
    Vector & operator/=(const float a)
    {
        x /= a;
        y /= a;
        return *this;
    }
    Vector operator/(const float a) const
    {
        Vector result = *this;
        result /= a;
        return result;
    }
    float operator*(const Vector & other) const
    {
        return x*other.x + y*other.y;
    }
    float abs()
    {
        return sqrtf(*this * *this);
    }
};

struct Globule
{
    Vector r, v;
    float radius;
    RGBQUAD color;
    inline float mass() { return radius * radius * radius; }
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
    std::vector<Globule> globules;
    volatile bool working;
    HANDLE thread;

    void CollideWithWalls(unsigned i);
    void CollideThem(unsigned i, unsigned j);
    void MoveOne(unsigned i, float delta);
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
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    void LoadDataToGS();
};
