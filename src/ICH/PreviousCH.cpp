// PreviousCH.cpp: implementation of the CPreviousCH class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreviousCH.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPreviousCH::CPreviousCH(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) : CExactMethodForDGP(inputModel, indexOfSourceVerts)
{
	nameOfAlgorithm = "CH";
}

CPreviousCH::~CPreviousCH()
{
}

void CPreviousCH::InitContainers()
{
	//m_InfoAtAngles.resize(model.GetNumOfEdges());
	memory += double(model.GetNumOfEdges()) * sizeof(InfoAtAngle) / 1024 / 1024;
}

void CPreviousCH::BuildSequenceTree()
{
	ComputeChildrenOfSource();
	bool fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	while (depthOfResultingTree < model.GetNumOfFaces() && !(m_QueueForPseudoSources.empty() && m_QueueForWindows.empty()))
	{		
		if ((int)m_QueueForWindows.size() > nMaxLenOfWindowQueue)
			nMaxLenOfWindowQueue = (int)m_QueueForWindows.size();
		if (m_QueueForPseudoSources.size() > nMaxLenOfPseudoSources)
			nMaxLenOfPseudoSources = (int)m_QueueForPseudoSources.size();
		if (fFromQueueOfPseudoSources) //pseudosource
		{				
			int indexOfVert = m_QueueForPseudoSources.front().indexOfVert;
			m_QueueForPseudoSources.pop();		
			if (!model.IsConvexVert(indexOfVert))
				ComputeChildrenOfPseudoSource(indexOfVert);	
		}
		else			
		{
			QuoteWindow quoteW = m_QueueForWindows.front();
			m_QueueForWindows.pop();
			ComputeChildrenOfWindow(quoteW);		
			delete quoteW.pWindow;
		}
		fFromQueueOfPseudoSources = UpdateTreeDepthBackWithChoice();
	}
}

void CPreviousCH::FillExperimentalResults()
{
	NPE = 1;
	//memory += double(nMaxLenOfPseudoSources) * sizeof(QuoteInfoAtVertex) / 1024 / 1024;
	//memory += double(nMaxLenOfWindowQueue) * (sizeof(QuoteWindow) + 64) / 1024 / 1024;		
}

void CPreviousCH::ClearContainers()
{
	while (!m_QueueForWindows.empty())
	{
		delete m_QueueForWindows.front().pWindow;
		m_QueueForWindows.pop();
	}

	while (!m_QueueForPseudoSources.empty())
	{
		m_QueueForPseudoSources.pop();
	}
}


void CPreviousCH::AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource)
{
	m_QueueForPseudoSources.push(quoteOfPseudoSource);
}

void CPreviousCH::AddIntoQueueOfWindows(QuoteWindow& quoteW)
{
	m_QueueForWindows.push(quoteW);
	++nCountOfWindows;
}

bool CPreviousCH::UpdateTreeDepthBackWithChoice()
{
	while (!m_QueueForPseudoSources.empty()
		&& m_QueueForPseudoSources.front().birthTime != m_InfoAtVertices[m_QueueForPseudoSources.front().indexOfVert].birthTime)
		m_QueueForPseudoSources.pop();

	while (!m_QueueForWindows.empty())
	{
		const QuoteWindow& quoteW = m_QueueForWindows.front();
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
			const InfoAtVertex& infoOfHeadElemOfPseudoSources = m_InfoAtVertices[m_QueueForPseudoSources.front().indexOfVert];
			depthOfResultingTree = max(depthOfResultingTree, 
				infoOfHeadElemOfPseudoSources.level);
			fFromQueueOfPseudoSources = true;
		}
	}
	else 
	{
		if (m_QueueForPseudoSources.empty())
		{
			const Window& infoOfHeadElemOfWindows = *m_QueueForWindows.front().pWindow;
			depthOfResultingTree = max(depthOfResultingTree,
				infoOfHeadElemOfWindows.level);
			fFromQueueOfPseudoSources = false;
		}
		else
		{
			const InfoAtVertex& infoOfHeadElemOfPseudoSources = m_InfoAtVertices[m_QueueForPseudoSources.front().indexOfVert];
			const Window& infoOfHeadElemOfWindows = *m_QueueForWindows.front().pWindow;
			if (infoOfHeadElemOfPseudoSources.level <= 
				infoOfHeadElemOfWindows.level)
			{
				depthOfResultingTree = max(depthOfResultingTree,
					infoOfHeadElemOfPseudoSources.level);
				fFromQueueOfPseudoSources = true;
			}
			else
			{
				depthOfResultingTree = max(depthOfResultingTree,
					infoOfHeadElemOfWindows.level);
				fFromQueueOfPseudoSources = false;
			}
		}
	}	
	return fFromQueueOfPseudoSources;
}

bool CPreviousCH::CheckValidityOfWindow(Window& w)
{
	return true;
}



void CPreviousCH::ComputeChildrenOfWindow(QuoteWindow& quoteParentWindow)
{
	const Window& w = *quoteParentWindow.pWindow;
	const CRichModel::CEdge& edge = model.Edge(w.indexOfCurEdge);
	double entryProp = model.ProportionOnEdgeByImage(w.indexOfCurEdge, w.xUponUnfolding, w.yUponUnfolding);
	double disToAngle = model.DistanceToIncidentAngle(w.indexOfCurEdge, w.xUponUnfolding, w.yUponUnfolding);
	if (entryProp >= w.proportions[1])
	{
		ComputeTheOnlyLeftChild(w, disToAngle);
		return;
	}
	if (entryProp <= w.proportions[0])
	{
		ComputeTheOnlyRightChild(w, disToAngle);
		return;
	}
	int incidentVertex = edge.indexOfOppositeVert;
	bool fLeftChildToCompute(false), fRightChildToCompute(false);
	bool fWIsWinning(false);
	double totalDis = w.disToRoot + disToAngle;

	if (m_InfoAtAngles[w.indexOfCurEdge].birthTime == -1)
	{
		fLeftChildToCompute = fRightChildToCompute = true;
		fWIsWinning = true;
	}
	else
	{
		if (totalDis < m_InfoAtAngles[w.indexOfCurEdge].disUptodate
			- LENGTH_EPSILON_CONTROL)
		{
			fLeftChildToCompute = fRightChildToCompute = true;
			fWIsWinning = true;
		}
		else
		{
			fLeftChildToCompute = entryProp < m_InfoAtAngles[w.indexOfCurEdge].entryProp;
			fRightChildToCompute = !fLeftChildToCompute;
			fWIsWinning = false;
		}

	}
	if (!fWIsWinning)
	{
		if (fLeftChildToCompute)
		{
			ComputeTheOnlyLeftTrimmedChild(w, disToAngle);
		}
		if (fRightChildToCompute)
		{
			ComputeTheOnlyRightTrimmedChild(w, disToAngle);
		}
		return;
	}

	m_InfoAtAngles[w.indexOfCurEdge].disUptodate = totalDis;
	m_InfoAtAngles[w.indexOfCurEdge].entryProp = entryProp;
	++m_InfoAtAngles[w.indexOfCurEdge].birthTime;

	ComputeLeftTrimmedChildWithParent(w, disToAngle);
	ComputeRightTrimmedChildWithParent(w, disToAngle);
	if (totalDis < m_InfoAtVertices[incidentVertex].disUptodate - LENGTH_EPSILON_CONTROL)
	{
		m_InfoAtVertices[incidentVertex].fParentIsPseudoSource = false;
		++m_InfoAtVertices[incidentVertex].birthTime;
		m_InfoAtVertices[incidentVertex].indexOfParent = w.indexOfCurEdge;
		m_InfoAtVertices[incidentVertex].indexOfRootVertOfParent = w.indexOfRoot;
		m_InfoAtVertices[incidentVertex].level = w.level + 1;
		m_InfoAtVertices[incidentVertex].disUptodate = totalDis;
		m_InfoAtVertices[incidentVertex].entryProp = entryProp;

		//if (!model.IsConvexVert(incidentVertex))
		AddIntoQueueOfPseudoSources(QuoteInfoAtVertex(m_InfoAtVertices[incidentVertex].birthTime,
			incidentVertex, totalDis));
	}
}

void CPreviousCH::ComputeChildrenOfPseudoSource(int indexOfParentVertex)
{
	if (m_InfoAtVertices[indexOfParentVertex].fParentIsPseudoSource)
		ComputeChildrenOfPseudoSourceFromPseudoSource(indexOfParentVertex);
	else
		ComputeChildrenOfPseudoSourceFromWindow(indexOfParentVertex);
}

void CPreviousCH::CreateIntervalChildOfPseudoSource(int source, int subIndexOfIncidentEdge)
{
	int indexOfIncidentEdge = model.Neigh(source)[subIndexOfIncidentEdge].first;
	if (model.IsExtremeEdge(indexOfIncidentEdge))
		return;
	const CRichModel::CEdge& edge = model.Edge(indexOfIncidentEdge);
	int edgeIndex = edge.indexOfRightEdge;
	if (model.IsExtremeEdge(edgeIndex))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	//quoteW.pWindow->fIsOnLeftSubtree;
	quoteW.pWindow->fParentIsPseudoSource = true;
	//quoteW.pWindow->fDirectParentEdgeOnLeft;
	quoteW.pWindow->fDirectParenIsPseudoSource = true;
	quoteW.pWindow->birthTimeOfParent = m_InfoAtVertices[source].birthTime;
	quoteW.pWindow->indexOfParent = source;
	quoteW.pWindow->indexOfRoot = source;
	quoteW.pWindow->indexOfCurEdge = edgeIndex;
	quoteW.pWindow->level = m_InfoAtVertices[source].level + 1;
	quoteW.pWindow->disToRoot = m_InfoAtVertices[source].disUptodate;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = 1;
	//quoteW.pWindow->entryPropOfParent;
	double rightLen = edge.length;
	quoteW.pWindow->rightLen = rightLen;
	indexOfIncidentEdge = edge.indexOfLeftEdge;
	double leftLen = model.Edge(indexOfIncidentEdge).length;
	quoteW.pWindow->leftLen = leftLen;
	double bottom = model.Edge(edgeIndex).length;
	double x = ((leftLen * leftLen - rightLen * rightLen) / bottom + bottom) / 2.0;
	quoteW.pWindow->xUponUnfolding = x;
	quoteW.pWindow->yUponUnfolding = -sqrt(std::max(0.0, leftLen * leftLen - x * x));
	AddIntoQueueOfWindows(quoteW);
}

void CPreviousCH::FillVertChildOfPseudoSource(int source, int subIndexOfVert)
{
	const CRichModel::CEdge& edge = model.Edge(model.Neigh(source)[subIndexOfVert].first);
	int index = edge.indexOfRightVert;
	double dis = m_InfoAtVertices[source].disUptodate + edge.length;
	if (dis >= m_InfoAtVertices[index].disUptodate - LENGTH_EPSILON_CONTROL)
		return;
	m_InfoAtVertices[index].fParentIsPseudoSource = true;
	++m_InfoAtVertices[index].birthTime;
	m_InfoAtVertices[index].indexOfParent = source;
	//m_InfoAtVertices[index].indexOfRootVertOfParent;
	m_InfoAtVertices[index].level = m_InfoAtVertices[source].level + 1;
	m_InfoAtVertices[index].disUptodate = dis;
	//m_InfoAtVertices[index].entryProp;		
	//if (!model.IsConvexVert(index))
	AddIntoQueueOfPseudoSources(QuoteInfoAtVertex(m_InfoAtVertices[index].birthTime,
		index, dis));
}

void CPreviousCH::ComputeTheOnlyLeftChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = std::max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = std::min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = true;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfLeftEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = w.leftLen;
	quoteW.pWindow->rightLen = disToAngle;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	AddIntoQueueOfWindows(quoteW);
}

void CPreviousCH::ComputeTheOnlyRightChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = std::max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = std::min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = false;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfRightEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = disToAngle;
	quoteW.pWindow->rightLen = w.rightLen;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	AddIntoQueueOfWindows(quoteW);
}

void CPreviousCH::ComputeTheOnlyLeftTrimmedChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = std::max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = 1;
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = true;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfLeftEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = w.leftLen;
	quoteW.pWindow->rightLen = disToAngle;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	AddIntoQueueOfWindows(quoteW);
}

void CPreviousCH::ComputeTheOnlyRightTrimmedChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = std::min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = w.fParentIsPseudoSource;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = false;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfRightEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = disToAngle;
	quoteW.pWindow->rightLen = w.rightLen;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = w.birthTimeOfParent;
	quoteW.pWindow->indexOfParent = w.indexOfParent;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = w.fIsOnLeftSubtree;
	quoteW.pWindow->entryPropOfParent = w.entryPropOfParent;
	AddIntoQueueOfWindows(quoteW);
}

void CPreviousCH::ComputeLeftTrimmedChildWithParent(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = std::max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = 1;
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = false;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = true;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfLeftEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = w.leftLen;
	quoteW.pWindow->rightLen = disToAngle;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->birthTimeOfParent = m_InfoAtAngles[w.indexOfCurEdge].birthTime;
	quoteW.pWindow->indexOfParent = w.indexOfCurEdge;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->fIsOnLeftSubtree = true;
	quoteW.pWindow->entryPropOfParent = m_InfoAtAngles[w.indexOfCurEdge].entryProp;
	AddIntoQueueOfWindows(quoteW);
}

void CPreviousCH::ComputeRightTrimmedChildWithParent(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = std::min(1., quoteW.pWindow->proportions[1]);
	if (IsTooNarrowWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fParentIsPseudoSource = false;
	quoteW.pWindow->fDirectParenIsPseudoSource = false;
	quoteW.pWindow->fDirectParentEdgeOnLeft = false;
	quoteW.pWindow->indexOfCurEdge = model.Edge(w.indexOfCurEdge).indexOfRightEdge;
	quoteW.pWindow->disToRoot = w.disToRoot;
	quoteW.pWindow->leftLen = disToAngle;
	quoteW.pWindow->rightLen = w.rightLen;
	model.GetPointByRotatingAround(quoteW.pWindow->indexOfCurEdge,
		quoteW.pWindow->leftLen, quoteW.pWindow->rightLen,
		quoteW.pWindow->xUponUnfolding, quoteW.pWindow->yUponUnfolding);
	if (!CheckValidityOfWindow(*quoteW.pWindow))
	{
		delete quoteW.pWindow;
		return;
	}
	quoteW.pWindow->fIsOnLeftSubtree = false;
	quoteW.pWindow->birthTimeOfParent = m_InfoAtAngles[w.indexOfCurEdge].birthTime;
	quoteW.pWindow->indexOfParent = w.indexOfCurEdge;
	quoteW.pWindow->indexOfRoot = w.indexOfRoot;
	quoteW.pWindow->level = w.level + 1;
	quoteW.pWindow->entryPropOfParent = m_InfoAtAngles[w.indexOfCurEdge].entryProp;
	AddIntoQueueOfWindows(quoteW);
}
