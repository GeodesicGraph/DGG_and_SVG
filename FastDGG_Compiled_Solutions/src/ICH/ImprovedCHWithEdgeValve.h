// ImprovedCHWithEdgeValve.h: interface for the CImprovedCHWithEdgeValve class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMPROVEDCHWITHEDGEVALVE_H__9012603F_B3CB_47E3_9F8A_5B746411D0A3__INCLUDED_)
#define AFX_IMPROVEDCHWITHEDGEVALVE_H__9012603F_B3CB_47E3_9F8A_5B746411D0A3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "PreviousCH.h"
class CImprovedCHWithEdgeValve : public  CPreviousCH
{
protected:
	virtual bool CheckValidityOfWindow(Window& w);
public:
	CImprovedCHWithEdgeValve(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts);
	virtual ~CImprovedCHWithEdgeValve();
};

#endif // !defined(AFX_IMPROVEDCHWITHEDGEVALVE_H__9012603F_B3CB_47E3_9F8A_5B746411D0A3__INCLUDED_)
