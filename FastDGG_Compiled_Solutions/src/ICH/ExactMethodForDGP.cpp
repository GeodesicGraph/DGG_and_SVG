// ExactMethodForDGP.cpp: implementation of the CExactMethodForDGP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExactMethodForDGP.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExactMethodForDGP::CExactMethodForDGP(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts) : model(inputModel)
{
	this->indexOfSourceVerts = indexOfSourceVerts;
	nCountOfWindows = 0;
	nMaxLenOfPseudoSources = 0;
	nMaxLenOfWindowQueue = 0;
	depthOfResultingTree = 0;
	totalLen = 0;
	fComputationCompleted = false;
	fLocked = false;
	NPE = 0;
	memory = 0;
	nTotalCurves = 0;
	nameOfAlgorithm = "";
	//m_InfoAtVertices.resize(model.GetNumOfVerts());
	memory += double(model.GetNumOfVerts()) * sizeof(InfoAtVertex) / 1024 / 1024;
}

CExactMethodForDGP::~CExactMethodForDGP()
{
}

void CExactMethodForDGP::PickShortestPaths(int num)
{
	if (num >= model.GetNumOfVerts())
		num = model.GetNumOfVerts();
	nTotalCurves = num;
	m_tableOfResultingPaths.clear();
	if (num == 0)
		return;
	if (model.GetNumOfFaces() * num < 4e6)
	{
		if (num >= model.GetNumOfVerts())
		{
			m_tableOfResultingPaths.reserve(model.GetNumOfVerts());
			for (int i = 0; i < model.GetNumOfVerts(); ++i)
			{
				BackTrace(i);
			}
		}
		else
		{
			float step = model.GetNumOfVerts() / float(num);
			step = max(1.f, step);
			m_tableOfResultingPaths.reserve(int(model.GetNumOfVerts() / step) + 1);
			for (float i = FLT_EPSILON; i < model.GetNumOfVerts(); i += step)
			{
				BackTrace(int(i));
			}
		}
	}	
}

void CExactMethodForDGP::BackTrace(int indexOfVert)
{
	if (m_InfoAtVertices[indexOfVert].birthTime == -1)
	{
		assert(model.GetNumOfComponents() != 1 || model.Neigh(indexOfVert).empty());
		return;
	}
	m_tableOfResultingPaths.push_back(list<CPoint3D>());
	vector<int> vertexNodes;
	int index = indexOfVert;
	vertexNodes.push_back(index);
	while (m_InfoAtVertices[index].disUptodate > FLT_EPSILON)
	{
		int indexOfParent = m_InfoAtVertices[index].indexOfParent;
		if (m_InfoAtVertices[index].fParentIsPseudoSource)
		{
			index = indexOfParent;
		}
		else
		{
			index = m_InfoAtVertices[index].indexOfRootVertOfParent;
		}
		vertexNodes.push_back(index);
	};
	int indexOfSourceVert = index;
	int posOfTable = (int)m_tableOfResultingPaths.size() - 1;
	for (int i = 0; i < (int)vertexNodes.size() - 1; ++i)
	{
		int lastVert = vertexNodes[i];
		CPoint3D pt = model.ComputeShiftPoint(lastVert);
		m_tableOfResultingPaths[posOfTable].push_back(pt);
		
		if (m_InfoAtVertices[lastVert].fParentIsPseudoSource)
		{
			continue;
		}
		int parentEdgeIndex = m_InfoAtVertices[lastVert].indexOfParent;
		int edgeIndex = model.Edge(parentEdgeIndex).indexOfReverseEdge;
		double leftLen = model.Edge(model.Edge(parentEdgeIndex).indexOfRightEdge).length;
		double rightLen = model.Edge(model.Edge(parentEdgeIndex).indexOfLeftEdge).length;
		double xBack = model.Edge(parentEdgeIndex).length - model.Edge(parentEdgeIndex).xOfPlanarCoordOfOppositeVert;
		double yBack = -model.Edge(parentEdgeIndex).yOfPlanarCoordOfOppositeVert;
		double disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);
		
		double proportion = 1 - m_InfoAtVertices[lastVert].entryProp;
		while (1) 
		{
			CPoint3D pt1 = model.ComputeShiftPoint(model.Edge(edgeIndex).indexOfLeftVert);
			CPoint3D pt2 = model.ComputeShiftPoint(model.Edge(edgeIndex).indexOfRightVert);
			CPoint3D ptIntersection = CRichModel::CombineTwoNormalsTo(pt1, 1 - proportion, pt2, proportion);			
			m_tableOfResultingPaths[posOfTable].push_back(ptIntersection);

			if (model.Edge(edgeIndex).indexOfOppositeVert == vertexNodes[i + 1])
				break;
			double oldProprotion = proportion;
			proportion = model.ProportionOnLeftEdgeByImage(edgeIndex,xBack, yBack, oldProprotion);
			if (proportion >= -LENGTH_EPSILON_CONTROL && proportion <= 1)
			{
				proportion = max(proportion, 0.);
				edgeIndex = model.Edge(edgeIndex).indexOfLeftEdge;
				rightLen = disToAngle;				
			}
			else
			{
				proportion = model.ProportionOnRightEdgeByImage(edgeIndex, xBack, yBack, oldProprotion);
				proportion = max(proportion, 0.);
				proportion = min(proportion, 1.);
				edgeIndex = model.Edge(edgeIndex).indexOfRightEdge;
				leftLen = disToAngle;				
			}
			model.GetPointByRotatingAround(edgeIndex, leftLen, rightLen, xBack, yBack);			
			disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);
		};
	}
	m_tableOfResultingPaths[posOfTable].push_back(model.ComputeShiftPoint(indexOfSourceVert));	
}


int CExactMethodForDGP::FindSourceVertex(int indexOfVert, vector<CPoint3D>& resultingPath) const
{
	resultingPath.clear();

	if (m_InfoAtVertices.find(indexOfVert) == m_InfoAtVertices.end())
	{
		assert(model.GetNumOfComponents() != 1 || model.Neigh(indexOfVert).empty());
		return -1;
	}
	vector<int> vertexNodes;
	int index = indexOfVert;
	vertexNodes.push_back(index);
	while (true)
	{
		map<int, InfoAtVertex>::const_iterator it = m_InfoAtVertices.find(index);
		if (it == m_InfoAtVertices.end())
			break;
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
		vertexNodes.push_back(index);
	};
	int indexOfSourceVert = index;

	for (int i = 0; i < (int)vertexNodes.size() - 1; ++i)
	{
		int lastVert = vertexNodes[i];
		//if (lastVert != indexOfVert)
		if (resultingPath.empty() || (resultingPath.back() - model.Vert(lastVert)).Len() > 1e-5)
			resultingPath.push_back(model.Vert(lastVert));
		if (m_InfoAtVertices.find(lastVert)->second.fParentIsPseudoSource)
		{
			continue;
		}
		int parentEdgeIndex = m_InfoAtVertices.find(lastVert)->second.indexOfParent;
		int edgeIndex = model.Edge(parentEdgeIndex).indexOfReverseEdge;
		double leftLen =       model.Edge(model.Edge(parentEdgeIndex).indexOfRightEdge).length;
		double rightLen = model.Edge(model.Edge(parentEdgeIndex).indexOfLeftEdge).length;
		double xBack = model.Edge(parentEdgeIndex).length - model.Edge(parentEdgeIndex).xOfPlanarCoordOfOppositeVert;
		double yBack = -model.Edge(parentEdgeIndex).yOfPlanarCoordOfOppositeVert;
		double disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);

		double proportion = 1 - m_InfoAtVertices.find(lastVert)->second.entryProp;
		while (1) 
		{
			if (resultingPath.empty() || (resultingPath.back() - IntersectionWithPath(edgeIndex, proportion).GetPosition(model)).Len() > 1e-5)
				resultingPath.push_back(IntersectionWithPath(edgeIndex, proportion).GetPosition(model));
			if (model.Edge(edgeIndex).indexOfOppositeVert == vertexNodes[i + 1])
				break;
			double oldProprotion = proportion;
			proportion = model.ProportionOnLeftEdgeByImage(edgeIndex,xBack, yBack, oldProprotion);
			if (model.Edge(edgeIndex).indexOfLeftEdge == -1 || model.Edge(edgeIndex).indexOfRightEdge == -1)
			{
				break;
			}

			if (proportion >= -LENGTH_EPSILON_CONTROL && proportion <= 1)
			{
				proportion = max(proportion, 0.);
				edgeIndex = model.Edge(edgeIndex).indexOfLeftEdge;
				rightLen = disToAngle;				
			}
			else
			{
				proportion = model.ProportionOnRightEdgeByImage(edgeIndex, xBack, yBack, oldProprotion);
				proportion = max(proportion, 0.);
				proportion = min(proportion, 1.);
				edgeIndex = model.Edge(edgeIndex).indexOfRightEdge;
				leftLen = disToAngle;				
			}
			model.GetPointByRotatingAround(edgeIndex, leftLen, rightLen, xBack, yBack);			
			disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);
		};
	}
	if (resultingPath.empty() || (resultingPath.back() - model.Vert(indexOfSourceVert)).Len() > 1e-5)
	resultingPath.push_back(model.Vert(indexOfSourceVert));
	return indexOfSourceVert;
}


int CExactMethodForDGP::FindSourceVertex(int indexOfVert, vector<IntersectionWithPath>& resultingPath) const
{
	resultingPath.clear();

	if (m_InfoAtVertices.find(indexOfVert) == m_InfoAtVertices.end())
	{
		assert(model.GetNumOfComponents() != 1 || model.Neigh(indexOfVert).empty());
		return -1;
	}
	vector<int> vertexNodes;
	int index = indexOfVert;
	vertexNodes.push_back(index);
	while (true)
	{
		map<int, InfoAtVertex>::const_iterator it = m_InfoAtVertices.find(index);
		if (it == m_InfoAtVertices.end())
			break;
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
		vertexNodes.push_back(index);
	};
	int indexOfSourceVert = index;

	for (int i = 0; i < (int)vertexNodes.size() - 1; ++i)
	{
		int lastVert = vertexNodes[i];
		//if (lastVert != indexOfVert)
		resultingPath.push_back(IntersectionWithPath(lastVert));
		if (m_InfoAtVertices.find(lastVert)->second.fParentIsPseudoSource)
		{
			continue;
		}
		int parentEdgeIndex = m_InfoAtVertices.find(lastVert)->second.indexOfParent;
		int edgeIndex = model.Edge(parentEdgeIndex).indexOfReverseEdge;
		double leftLen =       model.Edge(model.Edge(parentEdgeIndex).indexOfRightEdge).length;
		double rightLen = model.Edge(model.Edge(parentEdgeIndex).indexOfLeftEdge).length;
		double xBack = model.Edge(parentEdgeIndex).length - model.Edge(parentEdgeIndex).xOfPlanarCoordOfOppositeVert;
		double yBack = -model.Edge(parentEdgeIndex).yOfPlanarCoordOfOppositeVert;
		double disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);

		double proportion = 1 - m_InfoAtVertices.find(lastVert)->second.entryProp;
		while (1) 
		{
			resultingPath.push_back(IntersectionWithPath(edgeIndex, proportion));
			if (model.Edge(edgeIndex).indexOfOppositeVert == vertexNodes[i + 1])
				break;
			double oldProprotion = proportion;
			proportion = model.ProportionOnLeftEdgeByImage(edgeIndex,xBack, yBack, oldProprotion);
			if (model.Edge(edgeIndex).indexOfLeftEdge == -1 || model.Edge(edgeIndex).indexOfRightEdge == -1)
			{
				break;
			}

			if (proportion >= -LENGTH_EPSILON_CONTROL && proportion <= 1)
			{
				proportion = max(proportion, 0.);
				edgeIndex = model.Edge(edgeIndex).indexOfLeftEdge;
				rightLen = disToAngle;				
			}
			else
			{
				proportion = model.ProportionOnRightEdgeByImage(edgeIndex, xBack, yBack, oldProprotion);
				proportion = max(proportion, 0.);
				proportion = min(proportion, 1.);
				edgeIndex = model.Edge(edgeIndex).indexOfRightEdge;
				leftLen = disToAngle;				
			}
			model.GetPointByRotatingAround(edgeIndex, leftLen, rightLen, xBack, yBack);			
			disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);
		};
	}
	resultingPath.push_back(IntersectionWithPath(indexOfSourceVert));
	return indexOfSourceVert;
}

void CExactMethodForDGP::Execute()
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
		BuildSequenceTree();
		nTotalMilliSeconds = GetTickCount() - nTotalMilliSeconds;
		//FillExperimentalResults();
		ClearContainers();
		
		fComputationCompleted = true;
		fLocked = false;
	}
}


CPoint3D CExactMethodForDGP::BackTraceDirectionOnly(int indexOfVert){
	CPoint3D ret;
	if (m_InfoAtVertices[indexOfVert].birthTime == -1)
	{
		assert(model.GetNumOfComponents() != 1 || model.Neigh(indexOfVert).empty());
		throw "What's wrong????";
	}
	//m_tableOfResultingPaths.push_back(list<CPoint3D>());
	vector<int> vertexNodes;
	int index = indexOfVert;
	vertexNodes.push_back(index);
	while (m_InfoAtVertices[index].disUptodate > FLT_EPSILON)
	{
		int indexOfParent = m_InfoAtVertices[index].indexOfParent;
		if (m_InfoAtVertices[index].fParentIsPseudoSource)
		{
			index = indexOfParent;
		}
		else
		{
			index = m_InfoAtVertices[index].indexOfRootVertOfParent;
		}
		vertexNodes.push_back(index);
	};
	int indexOfSourceVert = index;
	//int posOfTable = (int)m_tableOfResultingPaths.size() - 1;
	for (int i = max(0,(int)vertexNodes.size() - 2); i < (int)vertexNodes.size() - 1; ++i)
	{
		int lastVert = vertexNodes[i];
		//CPoint3D pt = model.ComputeShiftPoint(lastVert);
        CPoint3D pt = model.Vert(lastVert);
		//m_tableOfResultingPaths[posOfTable].push_back(pt);
		ret = pt;
		
		if (m_InfoAtVertices[lastVert].fParentIsPseudoSource)
		{
			continue;
		}
        //printf("in exp map,line(603) indexOfVert = i = %d , vertexNodes.size() - 1 =%d\n" ,
        //      indexOfVert , i , vertexNodes.size() - 1 );
		int parentEdgeIndex = m_InfoAtVertices[lastVert].indexOfParent;
		int edgeIndex = model.Edge(parentEdgeIndex).indexOfReverseEdge;
		double leftLen = model.Edge(model.Edge(parentEdgeIndex).indexOfRightEdge).length;
		double rightLen = model.Edge(model.Edge(parentEdgeIndex).indexOfLeftEdge).length;
		double xBack = model.Edge(parentEdgeIndex).length - model.Edge(parentEdgeIndex).xOfPlanarCoordOfOppositeVert;
		double yBack = -model.Edge(parentEdgeIndex).yOfPlanarCoordOfOppositeVert;
		double disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);
		
		double proportion = 1 - m_InfoAtVertices[lastVert].entryProp;
		int cnt=0;
		while (1) 
		{
			cnt++ ;
			if( cnt > 100 ) break;
			//CPoint3D pt1 = model.ComputeShiftPoint(model.Edge(edgeIndex).indexOfLeftVert);
			//CPoint3D pt2 = model.ComputeShiftPoint(model.Edge(edgeIndex).indexOfRightVert);
			CPoint3D pt1 = model.Vert(model.Edge(edgeIndex).indexOfLeftVert);
			CPoint3D pt2 = model.Vert(model.Edge(edgeIndex).indexOfRightVert);
			CPoint3D ptIntersection = CRichModel::CombineTwoNormalsTo(pt1, 1 - proportion, pt2, proportion);			
			//m_tableOfResultingPaths[posOfTable].push_back(ptIntersection);
			ret = ptIntersection;

			if (model.Edge(edgeIndex).indexOfOppositeVert == vertexNodes[i + 1])
				break;
			
			double oldProprotion = proportion;
			proportion = model.ProportionOnLeftEdgeByImage(edgeIndex,xBack, yBack, oldProprotion);
			if (proportion >= -LENGTH_EPSILON_CONTROL && proportion <= 1)
			{
				proportion = max(proportion, 0.0);
				edgeIndex = model.Edge(edgeIndex).indexOfLeftEdge;
				rightLen = disToAngle;				
			}
			else
			{
				proportion = model.ProportionOnRightEdgeByImage(edgeIndex, xBack, yBack, oldProprotion);
				proportion = max(proportion, 0.0);
				proportion = min(proportion, 1.);
				edgeIndex = model.Edge(edgeIndex).indexOfRightEdge;
				leftLen = disToAngle;				
			}
			model.GetPointByRotatingAround(edgeIndex, leftLen, rightLen, xBack, yBack);			
			disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);
		};
	}
	return ret;
	//m_tableOfResultingPaths[posOfTable].push_back(model.ComputeShiftPoint(indexOfSourceVert));
}


CPoint3D CExactMethodForDGP::BackTraceDirectionOnly(int indexOfVert, bool& isVert, int& id) {
	CPoint3D ret;
	if (m_InfoAtVertices[indexOfVert].birthTime == -1)
	{
		assert(model.GetNumOfComponents() != 1 || model.Neigh(indexOfVert).empty());
		throw "What's wrong????";
	}
	//m_tableOfResultingPaths.push_back(list<CPoint3D>());
	vector<int> vertexNodes;
	int index = indexOfVert;
	vertexNodes.push_back(index);
	while (m_InfoAtVertices[index].disUptodate > FLT_EPSILON)
	{
		int indexOfParent = m_InfoAtVertices[index].indexOfParent;
		if (m_InfoAtVertices[index].fParentIsPseudoSource)
		{
			index = indexOfParent;
		}
		else
		{
			index = m_InfoAtVertices[index].indexOfRootVertOfParent;
		}
		vertexNodes.push_back(index);
	};
	int indexOfSourceVert = index;
	//int posOfTable = (int)m_tableOfResultingPaths.size() - 1;
	for (int i = max(0, (int)vertexNodes.size() - 2); i < (int)vertexNodes.size() - 1; ++i)
	{
		int lastVert = vertexNodes[i];
		//CPoint3D pt = model.ComputeShiftPoint(lastVert);
		CPoint3D pt = model.Vert(lastVert);
		//m_tableOfResultingPaths[posOfTable].push_back(pt);
		ret = pt;
		isVert = true;
		id = lastVert;

		if (m_InfoAtVertices[lastVert].fParentIsPseudoSource)
		{
			continue;
		}
		//printf("in exp map,line(603) indexOfVert = i = %d , vertexNodes.size() - 1 =%d\n" ,
		//      indexOfVert , i , vertexNodes.size() - 1 );
		int parentEdgeIndex = m_InfoAtVertices[lastVert].indexOfParent;
		int edgeIndex = model.Edge(parentEdgeIndex).indexOfReverseEdge;
		double leftLen = model.Edge(model.Edge(parentEdgeIndex).indexOfRightEdge).length;
		double rightLen = model.Edge(model.Edge(parentEdgeIndex).indexOfLeftEdge).length;
		double xBack = model.Edge(parentEdgeIndex).length - model.Edge(parentEdgeIndex).xOfPlanarCoordOfOppositeVert;
		double yBack = -model.Edge(parentEdgeIndex).yOfPlanarCoordOfOppositeVert;
		double disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);

		double proportion = 1 - m_InfoAtVertices[lastVert].entryProp;
		int cnt = 0;
		while (1)
		{
			cnt++;
			if (cnt > 100) break;
			//CPoint3D pt1 = model.ComputeShiftPoint(model.Edge(edgeIndex).indexOfLeftVert);
			//CPoint3D pt2 = model.ComputeShiftPoint(model.Edge(edgeIndex).indexOfRightVert);
			CPoint3D pt1 = model.Vert(model.Edge(edgeIndex).indexOfLeftVert);
			CPoint3D pt2 = model.Vert(model.Edge(edgeIndex).indexOfRightVert);
			CPoint3D ptIntersection = CRichModel::CombineTwoNormalsTo(pt1, 1 - proportion, pt2, proportion);
			//m_tableOfResultingPaths[posOfTable].push_back(ptIntersection);
			ret = ptIntersection;
			isVert = false;
			id = edgeIndex;

			if (model.Edge(edgeIndex).indexOfOppositeVert == vertexNodes[i + 1])
				break;

			double oldProprotion = proportion;
			proportion = model.ProportionOnLeftEdgeByImage(edgeIndex, xBack, yBack, oldProprotion);
			if (proportion >= -LENGTH_EPSILON_CONTROL && proportion <= 1)
			{
				proportion = max(proportion, 0.0);
				edgeIndex = model.Edge(edgeIndex).indexOfLeftEdge;
				rightLen = disToAngle;
			}
			else
			{
				proportion = model.ProportionOnRightEdgeByImage(edgeIndex, xBack, yBack, oldProprotion);
				proportion = max(proportion, 0.0);
				proportion = min(proportion, 1.);
				edgeIndex = model.Edge(edgeIndex).indexOfRightEdge;
				leftLen = disToAngle;
			}
			model.GetPointByRotatingAround(edgeIndex, leftLen, rightLen, xBack, yBack);
			disToAngle = model.DistanceToIncidentAngle(edgeIndex, xBack, yBack);
		};
	}
	return ret;
	//m_tableOfResultingPaths[posOfTable].push_back(model.ComputeShiftPoint(indexOfSourceVert));
}
