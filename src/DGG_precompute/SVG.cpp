// SVG.cpp: implementation of the CSVG class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SVG.h"

using namespace std;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSVG::CSVG(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) : CImprovedCHWithEdgeValve(inputModel, indexOfSourceVerts)
{
	nameOfAlgorithm = "SVG";
}

CSVG::CSVG(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts, const set<int>& destinations) : CImprovedCHWithEdgeValve(inputModel, indexOfSourceVerts), destinations(destinations)
{
	nameOfAlgorithm = "SVG";
}

CSVG::~CSVG()
{
}

void CSVG::ExecuteLocally_SVG(double distThreshold, set<int> &fixedDests, int max_covered_points)
{
	if (fComputationCompleted)
		return;
	if (!fLocked)
	{
		fLocked = true;
		nCountOfWindows = 0;
		nMaxLenOfWindowQueue = 0;
		depthOfResultingTree = 0;
		InitContainers();
		nTotalMilliSeconds = GetTickCount();
		BuildSequenceTree_SVG(distThreshold, fixedDests, max_covered_points);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}

void CSVG::InitContainers()
{
	//m_InfoAtAngles.resize(model.GetNumOfEdges());
}

void CSVG::ClearContainers()
{
	while (!m_QueueForWindows.empty())
	{
		delete m_QueueForWindows.top().pWindow;
		m_QueueForWindows.pop();
	}

	while (!m_QueueForPseudoSources.empty())
	{
		m_QueueForPseudoSources.pop();
	}
}

//------------By YingXiang---------------
void CSVG::BuildSequenceTree_SVG(double distThrehold, set<int> &fixedDests, int max_covered_points)
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	numDirectWindow = 0;
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{
		if (numDirectWindow <= 0)break; // for SVG
		if (fixedDests.size() >= max_covered_points) break;
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = m_QueueForPseudoSources.size();
		double minCurrentDis = DBL_MAX;
		if (fFromQueueOfPseudoSources) //pseudosource
		{
			minCurrentDis = m_QueueForPseudoSources.top().disUptodate;
		}
		else
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			minCurrentDis = quoteW.disUptodate;
		}
		//fprintf(stderr, "numDirectWindow=%d, minCurrentDis=%g\n", numDirectWindow, minCurrentDis);

		if (fFromQueueOfPseudoSources) //pseudosource
		{
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();
			fixedDests.insert(indexOfVert);

			if (minCurrentDis > distThrehold)
			{
				return;
			}
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);
		}
		else
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			if (quoteW.pWindow->indexOfRoot == indexOfSourceVerts[0]) {
				numDirectWindow--;
			}
			ComputeChildrenOfWindow(quoteW);
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

