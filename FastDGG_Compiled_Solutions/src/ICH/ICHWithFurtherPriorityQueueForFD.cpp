// ICHWithFurtherPriorityQueue.cpp: implementation of the CICHWithFurtherPriorityQueueForFD class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ICHWithFurtherPriorityQueueForFD.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CICHWithFurtherPriorityQueueForFD::CICHWithFurtherPriorityQueueForFD(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) : CPreviousCHForFD(inputModel, indexOfSourceVerts)
{
	nameOfAlgorithm = "ICH2";
}

CICHWithFurtherPriorityQueueForFD::CICHWithFurtherPriorityQueueForFD(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts, const set<int>& destinations) : CPreviousCHForFD(inputModel, indexOfSourceVerts), destinations(destinations)
{
	nameOfAlgorithm = "ICH2";
}

CICHWithFurtherPriorityQueueForFD::~CICHWithFurtherPriorityQueueForFD()
{
}

void CICHWithFurtherPriorityQueueForFD::ExecuteLocally_FastDGG(double eps_vg, set<int> &fixedDests)
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
		BuildSequenceTree_FastDGG(eps_vg, fixedDests);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}

void CICHWithFurtherPriorityQueueForFD::InitContainers()
{
	//m_InfoAtAngles.resize(model.GetNumOfEdges());
}

void CICHWithFurtherPriorityQueueForFD::ClearContainers()
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

void CICHWithFurtherPriorityQueueForFD::BuildSequenceTree_FastDGG(double eps_vg, set<int> &fixedDests)
{
	fixedDests.clear();
	epsilon = eps_vg;
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	numDirectWindow = 0;
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	double d_max_current = DBL_MAX;
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{
		// DGG Construction Code
		if (numDirectWindow <= 0) break;
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = m_QueueForPseudoSources.size();
		double minCurrentDis = DBL_MAX;

		if (fFromQueueOfPseudoSources) //pseudosource
		{
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();
			fixedDests.insert(indexOfVert);
		}
		else
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			if (quoteW.pWindow->maxDistance > GetMinDisOfWindow(*(quoteW.pWindow)))
				ComputeChildrenOfWindow(quoteW);
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

double CICHWithFurtherPriorityQueueForFD::GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest, vector<CPoint3D>& path)
{
	vector<int> sources;
	sources.push_back(indexOfSource);
	set<int> dests;
	dests.insert(indexOfDest);
	CICHWithFurtherPriorityQueueForFD alg(inputModel, sources, dests);
	alg.Execute();
	path.clear();
	alg.FindSourceVertex(indexOfDest, path);
	return alg.m_InfoAtVertices[indexOfDest].disUptodate;
}

double CICHWithFurtherPriorityQueueForFD::GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest)
{
	vector<int> sources;
	sources.push_back(indexOfSource);
	set<int> dests;
	dests.insert(indexOfDest);
	CICHWithFurtherPriorityQueueForFD alg(inputModel, sources, dests);
	alg.Execute();
	return alg.m_InfoAtVertices[indexOfDest].disUptodate;
}