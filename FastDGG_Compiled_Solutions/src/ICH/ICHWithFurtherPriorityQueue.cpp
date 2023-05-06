// ICHWithFurtherPriorityQueue.cpp: implementation of the CICHWithFurtherPriorityQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ICHWithFurtherPriorityQueue.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CICHWithFurtherPriorityQueue::CICHWithFurtherPriorityQueue(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) : CImprovedCHWithEdgeValve(inputModel, indexOfSourceVerts)
{
	nameOfAlgorithm = "ICH2";
}

CICHWithFurtherPriorityQueue::CICHWithFurtherPriorityQueue(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts, const set<int>& destinations) : CImprovedCHWithEdgeValve(inputModel, indexOfSourceVerts), destinations(destinations)
{
	nameOfAlgorithm = "ICH2";
}

CICHWithFurtherPriorityQueue::~CICHWithFurtherPriorityQueue()
{
}

void CICHWithFurtherPriorityQueue::ExecuteLocally_DGG(double eps_vg, set<int> &fixedDests)
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
		BuildSequenceTree_DGG(eps_vg, fixedDests);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}

void CICHWithFurtherPriorityQueue::ExecuteLocally_Dis(double distThreshold, set<int> &fixedDests)
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
		BuildSequenceTree_Dis(distThreshold, fixedDests);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}

void CICHWithFurtherPriorityQueue::ExecuteLocally_SVG(double distThreshold, set<int> &fixedDests, int max_covered_points)
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
		BuildSequenceTree_SVG(distThreshold, fixedDests,max_covered_points);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
}




double CICHWithFurtherPriorityQueue::ExecuteLocally_vertNum(int levelNum, set<int>& fixedDests)
{
	if (fComputationCompleted)
		return DBL_MAX;
	double dis = DBL_MAX;
	if (!fLocked)
	{
		fLocked = true;
		nCountOfWindows = 0;	
		nMaxLenOfWindowQueue = 0;
		depthOfResultingTree = 0;
		InitContainers();
		nTotalMilliSeconds = GetTickCount();	
		dis = BuildSequenceTree_vertNum(levelNum, fixedDests);
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();

		fComputationCompleted = true;
		fLocked = false;
	}
	return dis;
}


void CICHWithFurtherPriorityQueue::InitContainers()
{
	//m_InfoAtAngles.resize(model.GetNumOfEdges());
}

void CICHWithFurtherPriorityQueue::ClearContainers()
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

double CICHWithFurtherPriorityQueue::BuildSequenceTree_vertNum(int levelNum, set<int> &fixedDests)
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
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

		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();			
			fixedDests.insert(indexOfVert);
			if (fixedDests.size() > levelNum)
			{
				return minCurrentDis;
			}			
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);				
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();		
	}
	return FLT_MAX;
}

void CICHWithFurtherPriorityQueue::BuildSequenceTree_DGG(double eps_vg, set<int> &fixedDests)
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	numDirectWindow = 0;
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	double d_max_current = DBL_MAX;
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{

		if (numDirectWindow <= 0)break; // for SVG
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
			double interval_dis = GetMinDisOfWindow(*(quoteW.pWindow));
			double dis = quoteW.pWindow->disToRoot;
			//printf("interval_dis - dis %.10lf\n", (interval_dis - dis));
			auto& e = model.Edge(quoteW.pWindow->indexOfCurEdge);
			double tmp_e = 0.5 * e.length;
			double tmp_d_max = 1e10;
			if (interval_dis - tmp_e > 0) {
				tmp_d_max = tmp_e * tmp_e / (2.0 * eps_vg * (interval_dis - tmp_e)) + tmp_e;
			}
			//printf("interval_dis %lf temp_d_max %lf \n", interval_dis, tmp_d_max);
			d_max_current = min(d_max_current, tmp_d_max);
			if (interval_dis > d_max_current - 1e-6) break;
			//if ()
		}

		if (fFromQueueOfPseudoSources) //pseudosource
		{
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();
			//if (m_QueueForPseudoSources.top().disUptodate <= minCurrentDis) {
			fixedDests.insert(indexOfVert);
			//}
			//if (indexOfVert == indexOfSourceVerts[0]){
			//if (!model.IsConvexVert(indexOfVert))
			//	ComputeChildrenOfPseudoSource(indexOfVert);
			//}
		}
		else
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			if (false) {
				int v0 = model.Edge(quoteW.pWindow->indexOfCurEdge).indexOfLeftVert;
				int v1 = model.Edge(quoteW.pWindow->indexOfCurEdge).indexOfRightVert;
				double distance_v0;
				if (m_InfoAtVertices.find(v0) != m_InfoAtVertices.end()) {
					distance_v0 = m_InfoAtVertices[v0].disUptodate;
				}
				double distance_v1;
				if (m_InfoAtVertices.find(v1) != m_InfoAtVertices.end()) {
					distance_v1 = m_InfoAtVertices[v1].disUptodate;
				}
				if (distance_v0 <= minCurrentDis) {
					//printf("v0 %d\n", v0);
					fixedDests.insert(v0);
				}
				if (distance_v1 <= minCurrentDis) {
					//printf("v1 %d\n", v1);
					fixedDests.insert(v1);
				}
			}
			ComputeChildrenOfWindow(quoteW);
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}


void CICHWithFurtherPriorityQueue::BuildSequenceTree_Dis(double distThrehold, set<int> &fixedDests)
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
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
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

//------------By YingXiang---------------
void CICHWithFurtherPriorityQueue::BuildSequenceTree_SVG(double distThrehold, set<int> &fixedDests, int max_covered_points )
{
	fixedDests.clear();
	fixedDests.insert(indexOfSourceVerts.begin(), indexOfSourceVerts.end());
	numDirectWindow = 0;
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
		if(numDirectWindow <= 0)break; // for SVG
        if(fixedDests.size() >= max_covered_points) break;
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
			if(quoteW.pWindow->indexOfRoot == indexOfSourceVerts[0]){
				numDirectWindow --;
			}
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}


void CICHWithFurtherPriorityQueue::BuildSequenceTree()
{
	set<int> tmpDests(destinations);
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (!m_QueueForPseudoSources.empty() || !m_QueueForWindows.empty())
	{	
		if (!destinations.empty() && tmpDests.empty())
			break;
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
		vector<int> tobedeleted;
		for (set<int>::iterator it = tmpDests.begin(); it != tmpDests.end(); ++it)
		{
			if (m_InfoAtVertices[*it].disUptodate <= minCurrentDis)
				tobedeleted.push_back(*it);
		}
		for (int i = 0; i < (int)tobedeleted.size(); ++i)
		{
			tmpDests.erase(tobedeleted[i]);
		}
		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			int indexOfVert = m_QueueForPseudoSources.top().indexOfVert;
			m_QueueForPseudoSources.pop();	
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);	
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.top();
			m_QueueForWindows.pop();
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}

		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

double CICHWithFurtherPriorityQueue::GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest, vector<CPoint3D>& path)
{
	vector<int> sources;
	sources.push_back(indexOfSource);
	set<int> dests;
	dests.insert(indexOfDest);
	CICHWithFurtherPriorityQueue alg(inputModel, sources, dests);
	alg.Execute();
	path.clear();
	alg.FindSourceVertex(indexOfDest, path);
	return alg.m_InfoAtVertices[indexOfDest].disUptodate;
}

double CICHWithFurtherPriorityQueue::GetExactGeoDisBetween(const CRichModel& inputModel, int indexOfSource, int indexOfDest)
{
	vector<int> sources;
	sources.push_back(indexOfSource);
	set<int> dests;
	dests.insert(indexOfDest);
	CICHWithFurtherPriorityQueue alg(inputModel, sources, dests);
	alg.Execute();
	return alg.m_InfoAtVertices[indexOfDest].disUptodate;
}