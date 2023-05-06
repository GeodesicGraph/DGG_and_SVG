// ExactMethodForDGP.h: interface for the CExactMethodForDGP class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_EXACTMETHODFORDGP_H__2D7CA115_02A8_4808_838A_AE74834C0D93__INCLUDED_)
#define AFX_EXACTMETHODFORDGP_H__2D7CA115_02A8_4808_838A_AE74834C0D93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "RichModel.h"


struct IntersectionWithPath
{
	bool isVertex;
	int index;
	double proportion; //[0 --> left endpoint; 1 --> right endpoint]
	IntersectionWithPath()
	{
	}
	IntersectionWithPath(int index) : index(index), isVertex(true){}
	IntersectionWithPath(int index, double proportion) : index(index), proportion(proportion), isVertex(false) {}

	CPoint3D GetPosition(const CRichModel& model) {
		if (isVertex)
			return model.Vert(index);
		return (1 - proportion) * model.Vert(model.Edge(index).indexOfLeftVert)
			+ proportion * model.Vert(model.Edge(index).indexOfRightVert);
	}
};

class CExactMethodForDGP  
{
public:
	struct InfoAtVertex
	{
		bool fParentIsPseudoSource;
		char birthTime;		
		int indexOfParent;
		int indexOfRootVertOfParent;
		int level;
		double disUptodate;
		double entryProp;

		InfoAtVertex()
		{
			birthTime = -1;
			disUptodate = 2 * FLT_MAX;
		}		
	};
	struct QuoteInfoAtVertex
	{
		char birthTime;
		int indexOfVert;
		double disUptodate;
		bool operator<(const QuoteInfoAtVertex& another) const
		{
			return disUptodate > another.disUptodate;
		}
		QuoteInfoAtVertex(){}
		QuoteInfoAtVertex(char birthTime, int indexOfVert, double disUptodate)
		{
			this->birthTime = birthTime;
			this->indexOfVert = indexOfVert;
			this->disUptodate = disUptodate;
		}
	};
	map<int, InfoAtVertex> m_InfoAtVertices;
	//vector<InfoAtVertex> m_InfoAtVertices;

protected:	
	bool fComputationCompleted;	
	bool fLocked;
	double totalLen;
	int nTotalCurves;

	//map<pair<int, int>, int> specialEdgesForDelaunay;
	vector<int> indexOfSourceVerts;
	int nCountOfWindows;
	double nTotalMilliSeconds;
	int nMaxLenOfWindowQueue;
	double nMaxLenOfPseudoSources;
	int depthOfResultingTree;
	double NPE;
	double memory;
	double farestDis;
	vector<list<CPoint3D> > m_tableOfResultingPaths;
	const CRichModel& model;
	string nameOfAlgorithm;
protected:	
	void BackTrace(int indexOfVert);
	void BackTraceWithoutStoring(int indexOfVert) const;
public:
	CExactMethodForDGP(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts);
	virtual ~CExactMethodForDGP();
	inline int GetRootSourceOfVert(int index) const;
	int FindSourceVertex(int indexOfVert, vector<IntersectionWithPath>& resultingPath) const; 
	int FindSourceVertex(int indexOfVert, vector<CPoint3D>& resultingPath) const; 
    CPoint3D BackTraceDirectionOnly(int indexOfVert);
	CPoint3D BackTraceDirectionOnly(int indexOfVert, bool& isVert, int& id);
	void PickShortestPaths(int num);
	virtual void Execute();
	virtual void InitContainers() = 0;
	virtual void BuildSequenceTree() = 0;
	virtual void ClearContainers() = 0;
	virtual void FillExperimentalResults() = 0;
	inline double GetRunTime() const;
	inline double GetMemoryCost() const;
	inline int GetWindowNum() const;
	inline int GetMaxLenOfQue() const;
	inline double GetNPE() const;
	inline int GetDepthOfSequenceTree() const;
	inline string GetAlgorithmName() const;
	inline bool HasBeenCompleted() const;
};

double CExactMethodForDGP::GetRunTime() const
{
	return nTotalMilliSeconds;
}

double CExactMethodForDGP::GetMemoryCost() const
{
	return memory;
}

int CExactMethodForDGP::GetWindowNum() const
{
	return nCountOfWindows;
}

int CExactMethodForDGP::GetMaxLenOfQue() const
{
	return nMaxLenOfWindowQueue;
}

int CExactMethodForDGP::GetDepthOfSequenceTree() const
{
	return depthOfResultingTree;
}

double CExactMethodForDGP::GetNPE() const
{
	return NPE;
}

string CExactMethodForDGP::GetAlgorithmName() const
{
	return nameOfAlgorithm;
}

bool CExactMethodForDGP::HasBeenCompleted() const
{
	return fComputationCompleted;
}

int CExactMethodForDGP::GetRootSourceOfVert(int index) const
{
	if (m_InfoAtVertices.find(index)->second.disUptodate > FLT_MAX)
		return index;

	while (true)
	{
		map<int, InfoAtVertex>::const_iterator it = m_InfoAtVertices.find(index);
		if (it->second.disUptodate <= FLT_EPSILON)
			break;
		int indexOfParent = it->second.indexOfParent;
		if (it->second.fParentIsPseudoSource)
		{
			index = indexOfParent;
		}
		else
		{
			index = it->second.indexOfRootVertOfParent;
		}
	}
	return index;
}

#endif // !defined(AFX_EXACTMETHODFORDGP_H__2D7CA115_02A8_4808_838A_AE74834C0D93__INCLUDED_)
