// ImprovedCHWithEdgeValve.h: interface for the CImprovedCHWithEdgeValve class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _IMPROVEDCHWITHEDGEVALVE_H_
#define _IMPROVEDCHWITHEDGEVALVE_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "PreviousCH.h"
class CImprovedCHWithEdgeValve : public  CPreviousCH
{
protected:
	virtual bool CheckValidityOfWindow(Window& w);
public:
	CImprovedCHWithEdgeValve(const CRichModel& inputModel, const std::vector<int> &indexOfSourceVerts);
	virtual ~CImprovedCHWithEdgeValve();
};

#endif
