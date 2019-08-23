// ImprovedCHWithEdgeValve.cpp: implementation of the CImprovedCHWithEdgeValve class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImprovedCHWithEdgeValve.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImprovedCHWithEdgeValve::CImprovedCHWithEdgeValve(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) :  CPreviousCH(inputModel, indexOfSourceVerts)
{
	nameOfAlgorithm = "ICH1";
}

CImprovedCHWithEdgeValve::~CImprovedCHWithEdgeValve()
{

}

#ifndef SECOND_OPTIMIZE
bool CImprovedCHWithEdgeValve::CheckValidityOfWindow(Window& w)
{
	if (w.fDirectParenIsPseudoSource)
		return true;
	const CRichModel::CEdge& edge = model.Edge(w.indexOfCurEdge);
	//TRACE
	//out << setw(10) << setiosflags(ios_base::fixed) << min(10000, m_InfoAtVertices[edge.indexOfLeftVert].disUptodate) << "\t";
	//out << setw(10) << setiosflags(ios_base::fixed) << min(10000, m_InfoAtVertices[edge.indexOfRightVert].disUptodate) << "\t";
	//out << setw(10) << setiosflags(ios_base::fixed) << min(10000, m_InfoAtVertices[model.Edge(edge.indexOfReverseEdge).indexOfOppositeVert].disUptodate) << "\n";
	//out.flush();
	int leftVert = edge.indexOfLeftVert;
	double detaX = w.xUponUnfolding - w.proportions[1] * edge.length;
	double rightLen = sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
	if (m_InfoAtVertices[leftVert].disUptodate < 10000  / model.m_scale && m_InfoAtVertices[leftVert].disUptodate + w.proportions[1] * edge.length
		< w.disToRoot + rightLen)
	{
		return false;
	}
	int rightVert = edge.indexOfRightVert;
	detaX = w.xUponUnfolding - w.proportions[0] * edge.length;
	double leftLen = sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
	if (m_InfoAtVertices[rightVert].disUptodate < 10000  / model.m_scale && m_InfoAtVertices[rightVert].disUptodate + (1 - w.proportions[0]) * edge.length
		< w.disToRoot + leftLen)
	{
		return false;
	}
	const CRichModel::CEdge& oppositeEdge = model.Edge(edge.indexOfReverseEdge);
	double xOfVert = edge.length - oppositeEdge.xOfPlanarCoordOfOppositeVert;
	double yOfVert = -oppositeEdge.yOfPlanarCoordOfOppositeVert;
	if (m_InfoAtVertices[oppositeEdge.indexOfOppositeVert].disUptodate < 10000  / model.m_scale)	
	{
		if (w.fDirectParentEdgeOnLeft)
		{
			double deta = w.disToRoot + leftLen - m_InfoAtVertices[oppositeEdge.indexOfOppositeVert].disUptodate;
			if (deta <= 0)
				return true;
			detaX = xOfVert - w.proportions[0] * edge.length;
			if (detaX * detaX + yOfVert * yOfVert < deta * deta)
				return false;
		}
		else
		{
			double deta = w.disToRoot + rightLen - m_InfoAtVertices[oppositeEdge.indexOfOppositeVert].disUptodate;
			if (deta <= 0)
				return true;
			detaX = xOfVert - w.proportions[1] * edge.length;
			if (detaX * detaX + yOfVert * yOfVert < deta * deta)
				return false;
		}	
	}
	return true;
}
#else
bool CImprovedCHWithEdgeValve::CheckValidityOfWindow(Window& w)
{
	if (w.fDirectParenIsPseudoSource)
		return true;
	const CRichModel::CEdge& edge = model.Edge(w.indexOfCurEdge);
	double squareSum = w.xUponUnfolding * w.xUponUnfolding + w.yUponUnfolding * w.yUponUnfolding;
	double leftProp = w.proportions[0];
	if (m_InfoAtVertices[edge.indexOfLeftVert].disUptodate < 10000)
	{
		double deta = m_InfoAtVertices[edge.indexOfLeftVert].disUptodate - w.disToRoot;
		if (fabs(deta) < LENGTH_EPSILON_CONTROL)
		{
			leftProp = squareSum / (2 * w.xUponUnfolding * edge.length);
		}
		else
		{
			leftProp = (squareSum / deta - deta) / (2 * edge.length * (1 + w.xUponUnfolding / deta));
		}
		if (leftProp < - LENGTH_EPSILON_CONTROL)
		{
			if (m_InfoAtVertices[edge.indexOfLeftVert].disUptodate < w.disToRoot + sqrt(squareSum))
			{
				leftProp = 1;
			}
			else
			{
				leftProp = w.proportions[0];
			}
		}
		else
		{
			leftProp = max(leftProp, w.proportions[0]);
		}
	}

	double rightProp = w.proportions[1];
	if (m_InfoAtVertices[edge.indexOfRightVert].disUptodate < 10000)
	{
		double deta = m_InfoAtVertices[edge.indexOfRightVert].disUptodate - w.disToRoot + edge.length;
		if (fabs(deta) < LENGTH_EPSILON_CONTROL)
		{
			rightProp = squareSum / (2 * w.xUponUnfolding * edge.length);
		}
		else
		{
			rightProp = (deta - squareSum / deta) / (2 * edge.length * (1 - w.xUponUnfolding / deta));
		}
		if (rightProp > 1 + LENGTH_EPSILON_CONTROL)
		{
			if (m_InfoAtVertices[edge.indexOfRightVert].disUptodate < w.disToRoot + sqrt((w.xUponUnfolding - edge.length) * (w.xUponUnfolding - edge.length) + w.yUponUnfolding * w.yUponUnfolding))	
			{
				rightProp = 0;
			}
			else
			{
				rightProp = 1;
			}
		}		
		else
		{
			rightProp = min(rightProp, w.proportions[1]);
		}
	}

	if (rightProp - leftProp < LENGTH_EPSILON_CONTROL)
		return false;
	const CRichModel::CEdge& oppositeEdge = model.Edge(edge.indexOfReverseEdge);
	if (m_InfoAtVertices[oppositeEdge.indexOfOppositeVert].disUptodate < 10000)
	{
		double xOfVert = edge.length - oppositeEdge.xOfPlanarCoordOfOppositeVert;
		double yOfVert = -oppositeEdge.yOfPlanarCoordOfOppositeVert;
		if (w.fDirectParentEdgeOnLeft)
		{
			double detaX = w.xUponUnfolding - leftProp * edge.length;
			double leftLen = sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
			detaX = xOfVert - leftProp * edge.length;
			if (m_InfoAtVertices[oppositeEdge.indexOfOppositeVert].disUptodate
				+ sqrt(detaX * detaX + yOfVert * yOfVert) < w.disToRoot + leftLen)
				return false;
		}
		else
		{ 
			double detaX = w.xUponUnfolding - rightProp * edge.length;
			double rightLen = sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
			detaX = xOfVert - rightProp * edge.length;
			if (m_InfoAtVertices[oppositeEdge.indexOfOppositeVert].disUptodate
				+ sqrt(detaX * detaX + yOfVert * yOfVert) < w.disToRoot + rightLen)
				return false;
		}	
	}
	return true;
}
#endif