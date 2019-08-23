// SVG.h: interface for the CSVG class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SVG_H_
#define _SVG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ICH\ImprovedCHWithEdgeValve.h"


class CSVG : public CImprovedCHWithEdgeValve
{
	std::set<int> destinations;
	int numDirectWindow;
protected:
	std::priority_queue<QuoteWindow> m_QueueForWindows;
	std::priority_queue<QuoteInfoAtVertex> m_QueueForPseudoSources;
protected:
	inline void AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource);
	inline void AddIntoQueueOfWindows(QuoteWindow& quoteW);
	inline bool UpdateTreeDepthBackWithChoice();
	inline double GetMinDisOfWindow(const Window& w) const;
		
	virtual void InitContainers();
	virtual void ClearContainers();

	void BuildSequenceTree_SVG(double distThreshold, std::set<int> &fixedDests, int max_covered_points);
public:
	CSVG(const CRichModel& inputModel, const std::vector<int> &indexOfSourceVerts);
	CSVG(const CRichModel& inputModel, const std::vector<int> &indexOfSourceVerts,
		const std::set<int>& destinations);
	virtual ~CSVG();

	void ExecuteLocally_SVG(double distThreshold, std::set<int> &fixedDests, int max_covered_points);
};

double CSVG::GetMinDisOfWindow(const Window& w) const
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

void CSVG::AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource)
{
	m_QueueForPseudoSources.push(quoteOfPseudoSource);
}

void CSVG::AddIntoQueueOfWindows(QuoteWindow& quoteW)
{
	quoteW.disUptodate = GetMinDisOfWindow(*quoteW.pWindow);
	if (quoteW.pWindow->indexOfRoot == indexOfSourceVerts[0]) {
		numDirectWindow++;
	}
	m_QueueForWindows.push(quoteW);
	++nCountOfWindows;
}

bool CSVG::UpdateTreeDepthBackWithChoice()
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
			depthOfResultingTree = std::max(depthOfResultingTree,
				infoOfHeadElemOfPseudoSources.level);
			fFromQueueOfPseudoSources = true;
		}
	}
	else
	{
		if (m_QueueForPseudoSources.empty())
		{
			const Window& infoOfHeadElemOfWindows = *m_QueueForWindows.top().pWindow;
			depthOfResultingTree = std::max(depthOfResultingTree,
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
				depthOfResultingTree = std::max(depthOfResultingTree,
					m_InfoAtVertices[headElemOfPseudoSources.indexOfVert].level);
				fFromQueueOfPseudoSources = true;
			}
			else
			{
				depthOfResultingTree = std::max(depthOfResultingTree,
					headElemOfWindows.pWindow->level);
				fFromQueueOfPseudoSources = false;
			}
		}
	}
	return fFromQueueOfPseudoSources;
}

#endif