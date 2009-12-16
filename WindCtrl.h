#pragma once


// WindCtrl

class WindCtrl : public CStatic
{
	DECLARE_DYNAMIC(WindCtrl)

public:
	WindCtrl();
	virtual ~WindCtrl();

protected:
	DECLARE_MESSAGE_MAP()

    double power; // 0..1
    double angle; // 0..2*PI

public:
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    double GetAngle();
    double GetPower();
    void SetVars(double p, double a);
};


