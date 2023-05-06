// ICHWithFurtherPriorityQueue.h: interface for the CICHWithFurtherPriorityQueue class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ICHWITHFURTHERPRIORITYQUEUE_H__0124D8F6_DE32_4BE0_9BC7_C0999A5299A5__INCLUDED_)
#define AFX_ICHWITHFURTHERPRIORITYQUEUE_H__0124D8F6_DE32_4BE0_9BC7_C0999A5299A5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ImprovedCHWithEdgeValve.h"
class CICHWithFurtherPriorityQueue : public CImprovedCHWithEdgeValve  
{
	set<int> destinations;
	int numDirectWindow;
protected:
	priority_queue<QuoteWindow> m_QueueForWindows;
	priority_queue<QuoteInfoAtVertex> m_QueueForPseudoSources;
protected:
	inline void AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource);
	inline void AddIntoQueueOfWindows(QuoteWindow& quoteW);
	inline bool UpdateTreeDepthBackWithChoice();
	inline double GetMinDisOfWindow(const Window& w) const;
	virtual void BuildSequenceTree();
	double BuildSequenceTree_vertNum(int levelNum, set<int> &fixedDests);
	void BuildSequenceTree_Dis(double distThreshold, set<int> &fixedDests);
	void BuildSequenceTree_DGG(double eps_vg, set<int> &fixedDests);
	void BuildSequenceTree_SVG(double distThreshold, set<int> &fixedDests, int max_covered_points);
	virtual void InitContainers();
	virtual void ClearContainers();
public:
	CICHWithFurtherPriorityQueue(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts);
	CICHWithFurtherPriorityQueue(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts,
		const set<int>& destinations);
	virtual ~CICHWithFurtherPriorityQueue();	
	double ExecuteLocally_vertNum(int levelNum, set<int> &fixedDests);
	void ExecuteLocally_Dis(double distThreshold, set<int> &fixedDests);
	void ExecuteLocally_DGG(double eps_vg, set<int> &fixedDests);
    void ExecuteLocally_SVG(double distThreshold, set<int> &fixedDests, int max_covered_points);
    static double GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest);
	static double GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest, vector<CPoint3D> &path);
};

#ifndef SECOND_OPTIMIZE
double CICHWithFurtherPriorityQueue::GetMinDisOfWindow(const Window& w) const
{
	double projProp = w.xUponUnfolding / model.Edge(w.indexOfCurEdge).length;
	if (projProp <= w.proportions[0])
	{
		double detaX = w.xUponUnfolding - w.proportions[0] * model.Edge(w.indexOfCurEdge).length;
		return w.disToRoot + sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
	}
	if (projProp >= w.proportions[1])
	{
		double detaX = w.xUponUnfolding - w.proportions[1] * model.Edge(w.indexOfCurEdge).length;
		return w.disToRoot + sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
	}
	return w.disToRoot - w.yUponUnfolding;
}
#else
double CICHWithFurtherPriorityQueue::GetMinDisOfWindow(const Window& w) const
{
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
	{
		return DBL_MAX;
	}
	else
	{
		double projProp = w.xUponUnfolding / model.Edge(w.indexOfCurEdge).length;
		if (projProp <= leftProp)
		{
			double detaX = w.xUponUnfolding - leftProp * model.Edge(w.indexOfCurEdge).length;
			return w.disToRoot + sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
		}
		if (projProp >= rightProp)
		{
			double detaX = w.xUponUnfolding - rightProp * model.Edge(w.indexOfCurEdge).length;
			return w.disToRoot + sqrt(detaX * detaX + w.yUponUnfolding * w.yUponUnfolding);
		}
		return w.disToRoot - w.yUponUnfolding;
	}
}
#endif

void CICHWithFurtherPriorityQueue::AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource)
{
	m_QueueForPseudoSources.push(quoteOfPseudoSource);
}

void CICHWithFurtherPriorityQueue::AddIntoQueueOfWindows(QuoteWindow& quoteW)
{
	quoteW.disUptodate = GetMinDisOfWindow(*quoteW.pWindow);
	if(quoteW.pWindow->indexOfRoot == indexOfSourceVerts[0]){
		numDirectWindow ++;
	}
	m_QueueForWindows.push(quoteW);
	++nCountOfWindows;
}

bool CICHWithFurtherPriorityQueue::UpdateTreeDepthBackWithChoice()
{
	while (!m_QueueForPseudoSources.empty()
		&& m_QueueForPseudoSources.top().birthTime 
		!= m_InfoAtVertices[m_QueueForPseudoSources.top().indexOfVert].birthTime)
		m_QueueForPseudoSources.pop();

	while (!m_QueueForWindows.empty())
	{
		const QuoteWindow& quoteW = m_QueueForWindows.top();
		if (quoteW.pWindow->fParentIsPseudoSource)
		{
			if (quoteW.pWindow->birthTimeOfParent != 
				m_InfoAtVertices[quoteW.pWindow->indexOfParent].birthTime)
			{
				delete quoteW.pWindow;
				m_QueueForWindows.pop();
			}
			else
				break;
		}
		else
		{
			if (quoteW.pWindow->birthTimeOfParent ==
				m_InfoAtAngles[quoteW.pWindow->indexOfParent].birthTime)
				break;
			else if (quoteW.pWindow->fIsOnLeftSubtree ==
				(quoteW.pWindow->entryPropOfParent < m_InfoAtAngles[quoteW.pWindow->indexOfParent].entryProp))
				break;
			else
			{
				delete quoteW.pWindow;
				m_QueueForWindows.pop();				
			}
		}
	}

	bool fFromQueueOfPseudoSources(false);		
	if (m_QueueForWindows.empty())
	{	
		if (!m_QueueForPseudoSources.empty())
		{
			const InfoAtVertex& infoOfHeadElemOfPseudoSources = m_InfoAtVertices[m_QueueForPseudoSources.top().indexOfVert];
			depthOfResultingTree = max(depthOfResultingTree, 
				infoOfHeadElemOfPseudoSources.level);
			fFromQueueOfPseudoSources = true;
		}
	}
	else 
	{
		if (m_QueueForPseudoSources.empty())
		{
			const Window& infoOfHeadElemOfWindows = *m_QueueForWindows.top().pWindow;
			depthOfResultingTree = max(depthOfResultingTree,
				infoOfHeadElemOfWindows.level);
			fFromQueueOfPseudoSources = false;
		}
		else
		{
			const QuoteInfoAtVertex& headElemOfPseudoSources = m_QueueForPseudoSources.top();
			const QuoteWindow& headElemOfWindows = m_QueueForWindows.top();
			if (headElemOfPseudoSources.disUptodate <= 
				headElemOfWindows.disUptodate)
			{
				depthOfResultingTree = max(depthOfResultingTree,
					m_InfoAtVertices[headElemOfPseudoSources.indexOfVert].level);
				fFromQueueOfPseudoSources = true;
			}
			else
			{
				depthOfResultingTree = max(depthOfResultingTree,
					headElemOfWindows.pWindow->level);
				fFromQueueOfPseudoSources = false;
			}
		}
	}	
	return fFromQueueOfPseudoSources;
}

#endif // !defined(AFX_ICHWITHFURTHERPRIORITYQUEUE_H__0124D8F6_DE32_4BE0_9BC7_C0999A5299A5__INCLUDED_)
