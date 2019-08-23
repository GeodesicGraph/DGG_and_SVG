// FastDGG.h: interface for the CFastDGG class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _FASTDGG_H_
#define _FASTDGG_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ICH\PreviousCH.h"
#include <queue>
#include <set>

class CFastDGG : public CPreviousCH
{
	std::set<int> destinations;
	int numDirectWindow;
	double epsilon; // epsilon value for DGG
protected:
	std::priority_queue<QuoteWindow> m_QueueForWindows;
	std::priority_queue<QuoteInfoAtVertex> m_QueueForPseudoSources;
protected:
	inline double GetMinDisOfWindow(const Window& w) const;
	inline void AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource);
	inline void AddIntoQueueOfWindows(QuoteWindow& quoteW);
	inline bool UpdateTreeDepthBackWithChoice();

	inline void BuildSequenceTree(double eps_vg, std::set<int> &fixedDests);
	
	inline void InitContainers();
	inline void ClearContainers();
	
	inline void ComputeAndStoreFirstMaxDistanceBound(Window& w, const Window& prevW);
	inline void ComputeAndStoreSecondMaxDistanceBound(Window& w, const Window& prevW);
	
	inline void ComputeChildrenOfPseudoSource(int indexOfParentVertex);
	inline void FillVertChildOfPseudoSource(int source, int subIndexOfVert);
	inline void CreateIntervalChildOfPseudoSource(int source, int subIndexOfIncidentEdge);
	
	inline void ComputeTheOnlyLeftChild(const Window& w, double disToAngle);
	inline void ComputeTheOnlyLeftTrimmedChild(const Window& w, double disToAngle);
	inline void ComputeLeftTrimmedChildWithParent(const Window& w, double disToAngle);
	inline void ComputeTheOnlyRightChild(const Window& w, double disToAngle);
	inline void ComputeTheOnlyRightTrimmedChild(const Window& w, double disToAngle);
	inline void ComputeRightTrimmedChildWithParent(const Window& w, double disToAngle);
	inline void ComputeChildrenOfWindow(QuoteWindow& quoteParentWindow);
public:
	CFastDGG(const CRichModel& inputModel, const std::vector<int> &indexOfSourceVerts);
	CFastDGG(const CRichModel& inputModel, const std::vector<int> &indexOfSourceVerts,
		const std::set<int>& destinations);
	virtual ~CFastDGG();

	void ExecuteLocally_FastDGG(double eps_vg, std::set<int> &fixedDests);
};

double CFastDGG::GetMinDisOfWindow(const Window& w) const
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

void CFastDGG::AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource)
{
	m_QueueForPseudoSources.push(quoteOfPseudoSource);
}

void CFastDGG::AddIntoQueueOfWindows(QuoteWindow& quoteW)
{
	quoteW.disUptodate = GetMinDisOfWindow(*quoteW.pWindow);
	if (quoteW.pWindow->indexOfRoot == indexOfSourceVerts[0]) {
		numDirectWindow++;
	}
	m_QueueForWindows.push(quoteW);
	++nCountOfWindows;
}

bool CFastDGG::UpdateTreeDepthBackWithChoice()
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