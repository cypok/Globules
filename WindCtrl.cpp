// WindCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Globules.h"
#include "WindCtrl.h"

// WindCtrl

const double ZERO_ZONE = 0.1;

IMPLEMENT_DYNAMIC(WindCtrl, CStatic)

WindCtrl::WindCtrl()
{
    power = 1;
    angle = 0;
}

WindCtrl::~WindCtrl()
{
}


BEGIN_MESSAGE_MAP(WindCtrl, CStatic)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()



// WindCtrl message handlers

void WindCtrl::OnPaint()
{
    CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(&rect);
    unsigned width = rect.right;
    unsigned height = rect.bottom;
    
    // frame
    dc.Ellipse(0, 0, width, height);

    // current vector
    CPen red_pen(PS_SOLID, 1, RGB(255, 0, 0));
    CPen *old = dc.SelectObject(&red_pen);
    dc.MoveTo(width/2, height/2);
    dc.LineTo(int(width/2*(1+power*cos(angle))), int(height/2*(1-power*sin(angle))));
    dc.SelectObject(old);

    // zero zone
    dc.Ellipse( width*(0.5-ZERO_ZONE/2), height*(0.5-ZERO_ZONE/2),
                width*(0.5+ZERO_ZONE/2), height*(0.5+ZERO_ZONE/2) );
}

void WindCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);
    unsigned width = rect.right;
    unsigned height = rect.bottom;

    double dx =  double(point.x - double(width)/2 )/(width/2);
    double dy = -double(point.y - double(height)/2)/(height/2);

    if (dx*dx + dy*dy <= 1.0)
    {
        angle = atan2(dy, dx);
        power = sqrt(dx*dx + dy*dy);
        if (power < ZERO_ZONE)
            power = 0;

        InvalidateRect(&rect, 0);
        UpdateWindow();
    }

    CStatic::OnLButtonDown(nFlags, point);
}

double WindCtrl::GetAngle()
{
    return angle;
}

double WindCtrl::GetPower()
{
    return power;
}