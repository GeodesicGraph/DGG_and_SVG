// PreviousCH.cpp: implementation of the CPreviousCH class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PreviousCH.h"

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

