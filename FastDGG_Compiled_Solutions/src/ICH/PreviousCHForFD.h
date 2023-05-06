// PreviousCHForFD.h: interface for the  CPreviousCHForFD class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _PREVIOUSCHFORFD_H_
#define _PREVIOUSCHFORFD_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ExactMethodForDGP.h"
class  CPreviousCHForFD : public CExactMethodForDGP
{
protected:
	double epsilon; // epsilon value for DGG
	struct InfoAtAngle
	{
		char birthTime;
		double disUptodate;
		double entryProp;
		//bool remove;
		InfoAtAngle()
		{
			birthTime = -1; //remove = false;
			disUptodate = DBL_MAX;
		}
	};
	struct Window
	{
		bool fIsOnLeftSubtree;
		bool fParentIsPseudoSource;
		bool fDirectParentEdgeOnLeft; //may removed
		bool fDirectParenIsPseudoSource; //may removed
		bool usefulvertex;
		char birthTimeOfParent;
		int indexOfParent;
		int indexOfRoot;
		int indexOfCurEdge;
		int level;//may removed
		double disToRoot;
		double proportions[2];
		double entryPropOfParent;
		double leftLen;
		double rightLen;
		double maxDistance = 1e10; // max distance bound variable of the window
		double distanceLeftCHVertex;
		double distanceRightCHVertex;
		double xUponUnfolding;
		double yUponUnfolding;
	};//at least 64 bytes.
	struct QuoteWindow
	{
		Window* pWindow;
		double disUptodate;
		bool operator<(const QuoteWindow& another) const
		{
			return disUptodate > another.disUptodate;
		}
	};
protected:
	queue<QuoteWindow> m_QueueForWindows;
	queue<QuoteInfoAtVertex> m_QueueForPseudoSources;
	map<int, InfoAtAngle> m_InfoAtAngles;
	//vector<InfoAtAngle> m_InfoAtAngles;
protected:
	inline bool IsTooNarrowWindow(const Window& w) const;

	virtual void InitContainers();
	virtual void ClearContainers();
	virtual void BuildSequenceTree();
	virtual void AddIntoQueueOfPseudoSources(QuoteInfoAtVertex& quoteOfPseudoSource);
	virtual void AddIntoQueueOfWindows(QuoteWindow& quoteW);
	virtual bool UpdateTreeDepthBackWithChoice();
	virtual bool CheckValidityOfWindow(Window& w);
	virtual void FillExperimentalResults();

	inline void ComputeChildrenOfPseudoSource(int indexOfParentVertex);
	inline void ComputeChildrenOfPseudoSourceFromPseudoSource(int indexOfParentVertex);
	inline void ComputeChildrenOfPseudoSourceFromWindow(int indexOfParentVertex);
	inline void CreateIntervalChildOfPseudoSource(int source, int subIndexOfIncidentEdge);
	inline void FillVertChildOfPseudoSource(int source, int subIndexOfVert);
	inline void ComputeChildrenOfSource();
	inline void ComputeChildrenOfSource(int indexOfSourceVert);

	inline void ComputeTheOnlyLeftChild(const Window& w, double disToAngle);
	inline void ComputeTheOnlyLeftTrimmedChild(const Window& w, double disToAngle);
	inline void ComputeLeftTrimmedChildWithParent(const Window& w, double disToAngle);
	inline void ComputeTheOnlyRightChild(const Window& w, double disToAngle);
	inline void ComputeTheOnlyRightTrimmedChild(const Window& w, double disToAngle);
	inline void ComputeRightTrimmedChildWithParent(const Window& w, double disToAngle);
	inline void ComputeChildrenOfWindow(QuoteWindow& quoteParentWindow);

	/* Here, we update the 'maxDistance' variable of window w, in order to bound its termination.
	These 2 functions needs only to be called whenever the window's trimmed
	PS : Make sure to propagate 'maxDistance' variable of the previous window when a new window is projected.
	We can check whether a window is still useful enough to be propagated everytime before we add them to the window queue */
	inline void ComputeAndStoreFirstMaxDistanceBound(Window& w, const Window& prevW);  // Theorem 2.1 max distance bound computation..
	inline void ComputeAndStoreSecondMaxDistanceBound(Window& w, const Window& prevW); // Theorem 2.2 max distance bound computation..
	//inline void ComputeAndStoreThirdMaxDistanceBound(Window &w, const Window& prevW);
public:
	 CPreviousCHForFD(const CRichModel& inputModel, const vector<int> &indexOfSourceVerts);
	virtual ~ CPreviousCHForFD();
};

void  CPreviousCHForFD::ComputeAndStoreSecondMaxDistanceBound(Window& w, const Window& prevW){
	//w.maxDistance = prevW.maxDistance;
	//if (w.distanceLeftCHVertex > 0.8*w.distanceRightCHVertex && w.distanceRightCHVertex > 0.8*w.distanceLeftCHVertex)return;

	/*double edgelength = model.Edge(w.indexOfCurEdge).length;
	double closerCHVertexVecX, closerCHVertexVecY;
	double windowLeftXCoord = model.Edge(w.indexOfCurEdge).length*w.proportions[0];
	double LeftXDifference = windowLeftXCoord - w.xUponUnfolding;
	double windowRightXCoord = model.Edge(w.indexOfCurEdge).length*w.proportions[1];
	double RightXDifference = windowRightXCoord - w.xUponUnfolding;
	double distanceLeft = sqrtf(w.yUponUnfolding*w.yUponUnfolding + LeftXDifference*LeftXDifference);
	double distanceRight = sqrtf(w.yUponUnfolding*w.yUponUnfolding + RightXDifference*RightXDifference);
	double projectedDistance, altitude,ratio,largerdistance;
	if (w.distanceLeftCHVertex < w.distanceRightCHVertex){
	closerCHVertexVecY = (-w.distanceLeftCHVertex / distanceLeft)*w.yUponUnfolding;
	closerCHVertexVecX = (w.distanceLeftCHVertex / distanceLeft)*(LeftXDifference);
	projectedDistance = (closerCHVertexVecX*RightXDifference + closerCHVertexVecY*(-w.yUponUnfolding)) / distanceRight;
	if (projectedDistance <= 0)return;
	altitude = sqrtf(w.distanceLeftCHVertex*w.distanceLeftCHVertex - projectedDistance*projectedDistance);

	}
	else {
	closerCHVertexVecY = (-w.distanceRightCHVertex / distanceRight)*w.yUponUnfolding;
	closerCHVertexVecX = (w.distanceRightCHVertex / distanceRight)*(RightXDifference);
	projectedDistance = (closerCHVertexVecX*LeftXDifference + closerCHVertexVecY*(-w.yUponUnfolding)) / distanceLeft;
	if (projectedDistance <= 0)return;
	altitude = sqrtf(w.distanceRightCHVertex*w.distanceRightCHVertex - projectedDistance*projectedDistance);

	}


	double computedBound = altitude*altitude / (2 * 4*epsilon*projectedDistance) + projectedDistance;
	if (computedBound < w.maxDistance) w.maxDistance = computedBound;*/


}

void  CPreviousCHForFD::ComputeAndStoreFirstMaxDistanceBound(Window& w, const Window& prevW){
	//w.maxDistance = prevW.maxDistance;
	double sourceXCoord = w.xUponUnfolding;
	double sourceYCoord = w.yUponUnfolding;
	double edgelength = model.Edge(w.indexOfCurEdge).length;
	double windowLeftXCoord = w.proportions[0] * edgelength;
	double windowRightXCoord = w.proportions[1] * edgelength;
	double RightXDifference = windowRightXCoord - sourceXCoord; // difference between point b1 to source in term of x coordinate
	double LeftXDifference = windowLeftXCoord - sourceXCoord; // difference between point b0 to source in term of x coordinate
	double LeftDistanceSquared = (LeftXDifference)*(LeftXDifference)+sourceYCoord*sourceYCoord; // (distance of source to b0)^2.
	double RightDistanceSquared = (RightXDifference)*(RightXDifference)+sourceYCoord*sourceYCoord; // (distance of source to b1)^2.
	double isoscelesBottomSquared;

	double RightDistance;
	if (w.distanceRightCHVertex == w.rightLen)RightDistance = w.rightLen;
	else RightDistance = sqrtf(RightDistanceSquared);
	double LeftDistance;
	if (w.distanceLeftCHVertex == w.leftLen) LeftDistance = w.leftLen;
	else LeftDistance = sqrtf(LeftDistanceSquared);

	double triangledistance;
	if (RightDistanceSquared < LeftDistanceSquared){ // extend right hand side of the window since it's the one that's shorter
		float ratio = (LeftDistance / RightDistance);
		double isoscelesRightProjectionX = sourceXCoord + ratio*RightXDifference;
		double isoscelesRightProjectionY = sourceYCoord - ratio*(sourceYCoord);
		isoscelesBottomSquared = (isoscelesRightProjectionX - windowLeftXCoord)*(isoscelesRightProjectionX - windowLeftXCoord)
			+ isoscelesRightProjectionY*isoscelesRightProjectionY; // (isosceles triangle base length)^2
		//isoscelesAltitude = sqrtf(LeftDistanceSquared - 0.25*isoscelesBottomSquared); // calculate triangle altitude
		triangledistance = LeftDistance;
	}
	else{ // extend left hand side of the window since it's the one that's shorter
		float ratio = RightDistance / LeftDistance;
		double isoscelesLeftProjectionX = sourceXCoord + ratio*LeftXDifference;
		double isoscelesLeftProjectionY = sourceYCoord - ratio*(sourceYCoord);
		isoscelesBottomSquared = (isoscelesLeftProjectionX - windowRightXCoord)*(isoscelesLeftProjectionX - windowRightXCoord)
			+ isoscelesLeftProjectionY*isoscelesLeftProjectionY; // (isosceles triangle base length)^2
		//isoscelesAltitude = sqrtf(RightDistanceSquared - 0.25*isoscelesBottomSquared); // calculate triangle altitude
		triangledistance = RightDistance;
	}
	//double bound = isoscelesBottomSquared / (8 * epsilon*isoscelesAltitude) + isoscelesAltitude;
	//if (bound < w.maxDistance) w.maxDistance = bound;
	if (isoscelesBottomSquared >= 2 * triangledistance*triangledistance)return; // check if angle > 90 degrees
	double CHdistance_ratio, largerdistance;
	if (w.distanceLeftCHVertex < w.distanceRightCHVertex){
		CHdistance_ratio = w.distanceLeftCHVertex / w.distanceRightCHVertex;
		largerdistance = w.distanceRightCHVertex;
	}
	else{
		CHdistance_ratio = w.distanceRightCHVertex / w.distanceLeftCHVertex;
		largerdistance = w.distanceLeftCHVertex;
	}
	double scale_factor = 2.25;
	double beta, bottom, oneminratio;
	oneminratio = 1 / (1 - CHdistance_ratio);
	bottom = (largerdistance / triangledistance)*sqrtf(isoscelesBottomSquared);
	if (0.99 < CHdistance_ratio && CHdistance_ratio < 1.01){ beta = 0.5; }
	else{
		beta = oneminratio - sqrtf(oneminratio*(oneminratio - 1) - largerdistance*(largerdistance - bottom) * 2 * scale_factor * epsilon / (bottom*bottom));
	}
	if (beta < 0.5 || beta >= 1 || isnan(beta)) beta = 1;
	double bound = (beta)*(beta)*CHdistance_ratio*bottom*bottom / (2 * scale_factor * epsilon*(largerdistance - bottom)) + CHdistance_ratio*largerdistance;
	if (bound < w.maxDistance && largerdistance > bottom)w.maxDistance = bound;
}

bool  CPreviousCHForFD::IsTooNarrowWindow(const Window& w) const
{
	return w.proportions[1] - w.proportions[0] < LENGTH_EPSILON_CONTROL;
}

void  CPreviousCHForFD::ComputeChildrenOfSource(int indexOfSourceVert)
{
	//m_InfoAtVertices[indexOfSourceVert].fParentIsPseudoSource;
	++m_InfoAtVertices[indexOfSourceVert].birthTime;
	//m_InfoAtVertices[indexOfSourceVert].indexOfParent;
	//m_InfoAtVertices[indexOfSourceVert].indexOfRootVertOfParent;
	m_InfoAtVertices[indexOfSourceVert].level = 0;
	m_InfoAtVertices[indexOfSourceVert].disUptodate = 0;
	//m_InfoAtVertices[indexOfSourceVert].entryProp;	

	int degree = (int)model.Neigh(indexOfSourceVert).size();
	for (int i = 0; i < degree; ++i) // vertex-nodes
	{
		FillVertChildOfPseudoSource(indexOfSourceVert, i);
	}

	for (int i = 0; i < degree; ++i)
	{
		CreateIntervalChildOfPseudoSource(indexOfSourceVert, i);
	}
}

void  CPreviousCHForFD::ComputeChildrenOfSource()
{
	for (int i = 0; i < (int)indexOfSourceVerts.size(); ++i)
	{
		if (indexOfSourceVerts[i] >= model.GetNumOfVerts())
			continue;
		ComputeChildrenOfSource(indexOfSourceVerts[i]);
	}
}

void  CPreviousCHForFD::ComputeChildrenOfPseudoSourceFromPseudoSource(int indexOfParentVertex)
{
	int degree = (int)model.Neigh(indexOfParentVertex).size();
	const vector<pair<int, double> >& neighs = model.Neigh(indexOfParentVertex);
	int indexOfParentOfParent = m_InfoAtVertices[indexOfParentVertex].indexOfParent;
	int subIndex = model.GetSubindexToVert(indexOfParentVertex, indexOfParentOfParent);
	double angleSum(0);
	int indexPlus;
	for (indexPlus = subIndex; indexPlus != (subIndex - 1 + degree) % degree; indexPlus = (indexPlus + 1) % degree)
	{
		angleSum += neighs[indexPlus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	angleSum = 0;
	int indexMinus;
	for (indexMinus = (subIndex - 1 + degree) % degree;
		indexMinus == (subIndex - 1 + degree) % degree || indexMinus != (indexPlus - 1 + degree) % degree;
		indexMinus = (indexMinus - 1 + degree) % degree)
	{
		angleSum += neighs[indexMinus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	if (indexMinus == (indexPlus - 1 + degree) % degree)
		return;
	//vertices;
	for (int i = (indexPlus + 1) % degree; i != (indexMinus + 1) % degree; i = (i + 1) % degree)
	{
		FillVertChildOfPseudoSource(indexOfParentVertex, i);
	}

	//windows
	for (int i = indexPlus; i != (indexMinus + 1) % degree; i = (i + 1) % degree)
	{
		CreateIntervalChildOfPseudoSource(indexOfParentVertex, i);
	}
}

void  CPreviousCHForFD::ComputeChildrenOfPseudoSourceFromWindow(int indexOfParentVertex)
{
	int degree = (int)model.Neigh(indexOfParentVertex).size();
	const vector<pair<int, double> >& neighs = model.Neigh(indexOfParentVertex);
	int indexOfParentOfParent = m_InfoAtVertices[indexOfParentVertex].indexOfParent;
	int leftVert = model.Edge(indexOfParentOfParent).indexOfLeftVert;
	int rightVert = model.Edge(indexOfParentOfParent).indexOfRightVert;
	int subIndexLeft = model.GetSubindexToVert(indexOfParentVertex, leftVert);
	int subIndexRight = (subIndexLeft + 1) % degree;
	double x1 = m_InfoAtVertices[indexOfParentVertex].entryProp * model.Edge(indexOfParentOfParent).length;
	double y1 = 0;
	double x2 = model.Edge(indexOfParentOfParent).length;
	double y2 = 0;
	x1 -= model.Edge(indexOfParentOfParent).xOfPlanarCoordOfOppositeVert;
	y1 -= model.Edge(indexOfParentOfParent).yOfPlanarCoordOfOppositeVert;
	x2 -= model.Edge(indexOfParentOfParent).xOfPlanarCoordOfOppositeVert;
	y2 -= model.Edge(indexOfParentOfParent).yOfPlanarCoordOfOppositeVert;

	double anglePlus = acos((x1 * x2 + y1 * y2) / sqrt((x1 * x1 + y1 * y1) * (x2 * x2 + y2 * y2)));
	double angleSum(anglePlus);
	int indexPlus;
	for (indexPlus = subIndexRight; indexPlus != subIndexLeft; indexPlus = (indexPlus + 1) % degree)
	{
		angleSum += neighs[indexPlus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	angleSum = neighs[subIndexLeft].second - anglePlus;
	int indexMinus;
	for (indexMinus = (subIndexLeft - 1 + degree) % degree; indexMinus != (indexPlus - 1 + degree) % degree; indexMinus = (indexMinus - 1 + degree) % degree)
	{
		angleSum += neighs[indexMinus].second;
		if (angleSum > PI - ToleranceOfConvexAngle)
			break;
	}
	if (indexMinus == (indexPlus - 1 + degree) % degree)
		return;
	for (int i = 0; i < degree; ++i)
	{
		FillVertChildOfPseudoSource(indexOfParentVertex, i);
	}
	//windows
	for (int i = indexPlus; i != (indexMinus + 1) % degree; i = (i + 1) % degree)
	{
		CreateIntervalChildOfPseudoSource(indexOfParentVertex, i);
	}
}

void  CPreviousCHForFD::ComputeChildrenOfWindow(QuoteWindow& quoteParentWindow)
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


		double edgelength = model.Edge(w.indexOfCurEdge).length;
		double leftCHVertexX, leftCHVertexY, rightCHVertexX, rightCHVertexY;
		double windowLeftXCoord = model.Edge(w.indexOfCurEdge).length*w.proportions[0];
		double LeftXDifference = windowLeftXCoord - w.xUponUnfolding;
		double windowRightXCoord = model.Edge(w.indexOfCurEdge).length*w.proportions[1];
		double RightXDifference = windowRightXCoord - w.xUponUnfolding;
		double distanceLeft = sqrtf(w.yUponUnfolding*w.yUponUnfolding + LeftXDifference*LeftXDifference);
		double distanceRight = sqrtf(w.yUponUnfolding*w.yUponUnfolding + RightXDifference*RightXDifference);


		leftCHVertexY = (1 - (w.distanceLeftCHVertex / distanceLeft))*w.yUponUnfolding;
		leftCHVertexX = (w.distanceLeftCHVertex / distanceLeft)*(LeftXDifference)+w.xUponUnfolding;


		rightCHVertexY = (1 - (w.distanceRightCHVertex / distanceRight))*w.yUponUnfolding;
		rightCHVertexX = (w.distanceRightCHVertex / distanceRight)*(RightXDifference)+w.xUponUnfolding;


		double disfromLeftCHVertex = model.DistanceToIncidentAngle(w.indexOfCurEdge, leftCHVertexX, leftCHVertexY);
		double disfromRightCHVertex = model.DistanceToIncidentAngle(w.indexOfCurEdge, rightCHVertexX, rightCHVertexY);
		if ((w.distanceLeftCHVertex + disfromLeftCHVertex) > (1 + epsilon)*totalDis && (w.distanceRightCHVertex + disfromRightCHVertex) > (1 + epsilon)*totalDis){
			AddIntoQueueOfPseudoSources(QuoteInfoAtVertex(m_InfoAtVertices[incidentVertex].birthTime, incidentVertex, totalDis));
		}

	}

}

void  CPreviousCHForFD::ComputeChildrenOfPseudoSource(int indexOfParentVertex)
{
	if (m_InfoAtVertices[indexOfParentVertex].fParentIsPseudoSource)
		ComputeChildrenOfPseudoSourceFromPseudoSource(indexOfParentVertex);
	else
		ComputeChildrenOfPseudoSourceFromWindow(indexOfParentVertex);
}
void  CPreviousCHForFD::CreateIntervalChildOfPseudoSource(int source, int subIndexOfIncidentEdge)
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
	quoteW.pWindow->distanceRightCHVertex = rightLen;

	indexOfIncidentEdge = edge.indexOfLeftEdge;
	double leftLen = model.Edge(indexOfIncidentEdge).length;

	quoteW.pWindow->leftLen = leftLen;
	quoteW.pWindow->distanceLeftCHVertex = leftLen;

	double bottom = model.Edge(edgeIndex).length;
	double x = ((leftLen * leftLen - rightLen * rightLen) / bottom + bottom) / 2.0;
	quoteW.pWindow->xUponUnfolding = x;
	quoteW.pWindow->yUponUnfolding = -sqrt(max(0.0, leftLen * leftLen - x * x));
	AddIntoQueueOfWindows(quoteW);
}

void  CPreviousCHForFD::FillVertChildOfPseudoSource(int source, int subIndexOfVert)
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


void  CPreviousCHForFD::ComputeTheOnlyLeftChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
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

	quoteW.pWindow->maxDistance = w.maxDistance;
	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

	AddIntoQueueOfWindows(quoteW);
}

void  CPreviousCHForFD::ComputeTheOnlyRightChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
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
	quoteW.pWindow->maxDistance = w.maxDistance;

	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

	AddIntoQueueOfWindows(quoteW);
}

void  CPreviousCHForFD::ComputeTheOnlyLeftTrimmedChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
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

	quoteW.pWindow->distanceRightCHVertex = quoteW.pWindow->rightLen = disToAngle;
	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;

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
	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}

void  CPreviousCHForFD::ComputeTheOnlyRightTrimmedChild(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
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
	quoteW.pWindow->distanceLeftCHVertex = quoteW.pWindow->leftLen = disToAngle;

	quoteW.pWindow->rightLen = w.rightLen;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

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
	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}

void  CPreviousCHForFD::ComputeLeftTrimmedChildWithParent(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfLeftEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = model.ProportionOnLeftEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[0]);
	quoteW.pWindow->proportions[0] = max(0., quoteW.pWindow->proportions[0]);
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
	quoteW.pWindow->distanceRightCHVertex = quoteW.pWindow->rightLen = disToAngle;
	quoteW.pWindow->distanceLeftCHVertex = w.distanceLeftCHVertex;

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

	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}


void  CPreviousCHForFD::ComputeRightTrimmedChildWithParent(const Window& w, double disToAngle)
{
	if (model.IsExtremeEdge(model.Edge(w.indexOfCurEdge).indexOfRightEdge))
		return;
	QuoteWindow quoteW;
	quoteW.pWindow = new Window;
	quoteW.pWindow->proportions[0] = 0;
	quoteW.pWindow->proportions[1] = model.ProportionOnRightEdgeByImage(w.indexOfCurEdge,
		w.xUponUnfolding, w.yUponUnfolding, w.proportions[1]);
	quoteW.pWindow->proportions[1] = min(1., quoteW.pWindow->proportions[1]);
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
	quoteW.pWindow->distanceLeftCHVertex = quoteW.pWindow->leftLen = disToAngle;
	quoteW.pWindow->distanceRightCHVertex = w.distanceRightCHVertex;

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

	quoteW.pWindow->maxDistance = w.maxDistance;
	ComputeAndStoreFirstMaxDistanceBound(*(quoteW.pWindow), w);
	ComputeAndStoreSecondMaxDistanceBound(*(quoteW.pWindow), w);
	AddIntoQueueOfWindows(quoteW);
}


#endif