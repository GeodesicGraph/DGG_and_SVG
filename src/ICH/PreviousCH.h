// PreviousCH.h: interface for the CPreviousCH class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _PREVIOUSCH_H
#define _PREVIOUSCH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ExactMethodForDGP.h"


class CPreviousCH : public CExactMethodForDGP
{
protected:
	struct InfoAtAngle
	{
		char birthTime;
		double disUptodate;
		double entryProp;
		InfoAtAngle()
		{
			birthTime = -1;
			disUptodate = DBL_MAX;
		}
	};
	struct Window
	{
		bool fIsOnLeftSubtree;
		bool fParentIsPseudoSource;
		bool fDirectParentEdgeOnLeft; //may removed
		bool fDirectParenIsPseudoSource; //may removed
		bool usefulvertex;
		char birthTimeOfParent;
		int indexOfParent;
		int indexOfRoot;
		int indexOfCurEdge;
		int level;//may removed
		double disToRoot;
		double proportions[2];
		double entryPropOfParent;
		double leftLen;
		double rightLen;
		double maxDistance = 1e10; // max distance bound variable of the window
		double distanceLeftCHVertex;
		double distanceRightCHVertex;
		double xUponUnfolding;
		double yUponUnfolding;
	};//at least 64 bytes.
	struct QuoteWindow
	{
		Window* pWindow;
		double disUptodate;
		bool operator<(const QuoteWindow& another) const
		{
			return disUptodate > another.disUptodate;
		}
	};
protected:
	std::queue<QuoteWindow> m_QueueForWindows;
	std::queue<QuoteInfoAtVertex> m_QueueForPseudoSources;
	std::map<int, InfoAtAngle> m_InfoAtAngles;
	//vector<InfoAtAngle> m_InfoAtAngles;
protected:
	inline bool IsTooNarrowWindow(const Window& w) const;		

	virtual void InitContainers();
	virtual void ClearContainers();	
	virtual void BuildSequenceTree();
	virtual void AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource);
	virtual void AddIntoQueueOfWindows(QuoteWindow& quoteW);	
	virtual bool UpdateTreeDepthBackWithChoice();
	virtual bool CheckValidityOfWindow(Window& w);
	virtual void FillExperimentalResults();

	
	inline void ComputeChildrenOfPseudoSourceFromPseudoSource(int indexOfParentVertex);
	inline void ComputeChildrenOfPseudoSourceFromWindow(int indexOfParentVertex);
	inline void ComputeChildrenOfSource();
	inline void ComputeChildrenOfSource(int indexOfSourceVert);

	virtual void CreateIntervalChildOfPseudoSource(int source, int subIndexOfIncidentEdge);
	virtual void FillVertChildOfPseudoSource(int source, int subIndexOfVert);
	virtual void ComputeChildrenOfPseudoSource(int indexOfParentVertex);

	virtual void ComputeTheOnlyLeftChild(const Window& w, double disToAngle);
	virtual void ComputeTheOnlyLeftTrimmedChild(const Window& w, double disToAngle);
	virtual void ComputeLeftTrimmedChildWithParent(const Window& w, double disToAngle);
	virtual void ComputeTheOnlyRightChild(const Window& w, double disToAngle);
	virtual void ComputeTheOnlyRightTrimmedChild(const Window& w, double disToAngle);
	virtual void ComputeRightTrimmedChildWithParent(const Window& w, double disToAngle);
	virtual void ComputeChildrenOfWindow(QuoteWindow& quoteParentWindow);

public:
	CPreviousCH(const CRichModel& inputModel, const std::vector<int> &indexOfSourceVerts);
	virtual ~CPreviousCH();	
};

bool CPreviousCH::IsTooNarrowWindow(const Window& w) const
{
	return w.proportions[1] - w.proportions[0] < LENGTH_EPSILON_CONTROL;
}

void CPreviousCH::ComputeChildrenOfSource(int indexOfSourceVert)
{
	//m_InfoAtVertices[indexOfSourceVert].fParentIsPseudoSource;
	++m_InfoAtVertices[indexOfSourceVert].birthTime;
	//m_InfoAtVertices[indexOfSourceVert].indexOfParent;
	//m_InfoAtVertices[indexOfSourceVert].indexOfRootVertOfParent;
	m_InfoAtVertices[indexOfSourceVert].level = 0;
	m_InfoAtVertices[indexOfSourceVert].disUptodate = 0;
	//m_InfoAtVertices[indexOfSourceVert].entryProp;	

	int degree = (int)model.Neigh(indexOfSourceVert).size();
	for (int i = 0; i < degree; ++i) // vertex-nodes
	{
		FillVertChildOfPseudoSource(indexOfSourceVert, i);
	}
	
	for (int i = 0; i < degree; ++i)
	{
		CreateIntervalChildOfPseudoSource(indexOfSourceVert, i);	
	}
}

void CPreviousCH::ComputeChildrenOfSource()
{
	for (int i = 0; i < (int)indexOfSourceVerts.size(); ++i)
	{
		if (indexOfSourceVerts[i] >= model.GetNumOfVerts())
			continue;
		ComputeChildrenOfSource(indexOfSourceVerts[i]);
	}
}

void CPreviousCH::ComputeChildrenOfPseudoSourceFromPseudoSource(int indexOfParentVertex)
{
	int degree = (int)model.Neigh(indexOfParentVertex).size();
	const std::vector<std::pair<int, double> >& neighs = model.Neigh(indexOfParentVertex);
	int indexOfParentOfParent = m_InfoAtVertices[indexOfParentVertex].indexOfParent;
	int subIndex = model.GetSubindexToVert(indexOfParentVertex, indexOfParentOfParent);
	double angleSum(0);
	int indexPlus;
	for (indexPlus = subIndex; indexPlus != (subIndex - 1 + degree) % degree; indexPlus = (indexPlus + 1) % degree)
	{
		angleSum += neighs[indexPlus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	angleSum = 0;
	int indexMinus;
	for (indexMinus = (subIndex - 1 + degree) % degree; 
	indexMinus == (subIndex - 1 + degree) % degree || indexMinus != (indexPlus - 1 + degree) % degree; 
	indexMinus = (indexMinus - 1 + degree) % degree)
	{
		angleSum += neighs[indexMinus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	if (indexMinus == (indexPlus - 1 + degree) % degree)
		return;
	//vertices;
	for (int i = (indexPlus + 1) % degree; i != (indexMinus + 1) % degree; i = (i + 1) % degree)
	{
		FillVertChildOfPseudoSource(indexOfParentVertex, i);
	}
	
	//windows
	for (int i = indexPlus; i != (indexMinus + 1) % degree; i = (i + 1) % degree)
	{
		CreateIntervalChildOfPseudoSource(indexOfParentVertex, i);
	}	
}

void CPreviousCH::ComputeChildrenOfPseudoSourceFromWindow(int indexOfParentVertex)
{
	int degree = (int)model.Neigh(indexOfParentVertex).size();
	const std::vector<std::pair<int, double> >& neighs = model.Neigh(indexOfParentVertex);	
	int indexOfParentOfParent = m_InfoAtVertices[indexOfParentVertex].indexOfParent;
	int leftVert = model.Edge(indexOfParentOfParent).indexOfLeftVert;
	int rightVert = model.Edge(indexOfParentOfParent).indexOfRightVert;
	int subIndexLeft = model.GetSubindexToVert(indexOfParentVertex, leftVert);
	int subIndexRight = (subIndexLeft + 1) % degree;
	double x1 = m_InfoAtVertices[indexOfParentVertex].entryProp * model.Edge(indexOfParentOfParent).length;
	double y1 = 0;
	double x2 = model.Edge(indexOfParentOfParent).length;
	double y2 = 0;
	x1 -= model.Edge(indexOfParentOfParent).xOfPlanarCoordOfOppositeVert;
	y1 -= model.Edge(indexOfParentOfParent).yOfPlanarCoordOfOppositeVert;
	x2 -= model.Edge(indexOfParentOfParent).xOfPlanarCoordOfOppositeVert;
	y2 -= model.Edge(indexOfParentOfParent).yOfPlanarCoordOfOppositeVert;

	double anglePlus = acos((x1 * x2 + y1 * y2) / sqrt((x1 * x1 + y1 * y1) * (x2 * x2 + y2 * y2)));
	double angleSum(anglePlus);
	int indexPlus;
	for (indexPlus = subIndexRight; indexPlus != subIndexLeft; indexPlus = (indexPlus + 1) % degree)
	{
		angleSum += neighs[indexPlus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	angleSum = neighs[subIndexLeft].second - anglePlus;
	int indexMinus;
	for (indexMinus = (subIndexLeft - 1 + degree) % degree; indexMinus != (indexPlus - 1 + degree) % degree; indexMinus = (indexMinus - 1 + degree) % degree)
	{
		angleSum += neighs[indexMinus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	if (indexMinus == (indexPlus - 1 + degree) % degree)
		return;
	for (int i = 0; i < degree; ++i)
	{
		FillVertChildOfPseudoSource(indexOfParentVertex, i);
	}	
	//windows
	for (int i = indexPlus; i != (indexMinus + 1) % degree; i = (i + 1) % degree)
	{
		CreateIntervalChildOfPseudoSource(indexOfParentVertex, i);
	}
}

#endif
