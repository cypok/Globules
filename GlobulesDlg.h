// GlobulesDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

const UINT MAX_GLOBULES_COUNT = 10;
const unsigned BUFFERS_COUNT = 5;

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
};

struct Globule
{
    Vector r, v;
    float radius;
    RGBQUAD color;
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
    Globule *globules;
    unsigned globules_count;
    volatile bool working;
    HANDLE thread;

public:

    GlobulesSystem(LONG buffer_width, LONG buffer_height, unsigned g_count);
    ~GlobulesSystem();

    static DWORD WINAPI CalcAndRender(LPVOID param);
    RGBQUAD * GetBufferForRead();
    RGBQUAD * GetBufferForWrite();
    void ChangeBufferForRead();
    void ChangeBufferForWrite();
    void DrawGlobules(RGBQUAD *buf);
    void SetGlobule(unsigned num, Globule g);
    void Stop();
    void CreateThread();
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
};
