// Author: Ryan Schmidt  (rms@unknownroad.com)  http://www.unknownroad.com
// Copyright (c) 2008. All Rights Reserved.
// The ExpMapDemo package is supplied "AS IS". The Author disclaims all warranties, expressed or implied, including, 
// without limitation, the warranties of merchantability and of fitness for any purpose. The Author assume no liability for 
// direct, indirect, incidental, special, exemplary, or consequential damages, which may result from the use of the 
// ExpMapDemo package, even if advised of the possibility of such damage. Permission is hereby granted to use, copy, 
// modify, and distribute this source code, or portions hereof, for any purpose, without fee, subject to the following restrictions:
// 1) The origin of this source code must not be misrepresented.
// 2) This Copyright notice may not be removed or altered from any source or altered source distribution.
// The Author specifically permits, without fee, the use of this source code as a component in commercial products.
#include <stdafx.h>
#include ".\IMeshBVTree.h"

#include <limits>
#include <Wm4IntrRay3Triangle3.h>
#include <Wm4DistVector3Triangle3.h>
#include <Wm4DistVector3Line3.h>

#include "VectorUtil.h"

using namespace rms;

IMeshBVTree::IMeshBVTree( )
{
	m_pMesh = NULL;
	m_pRoot = NULL;
}

IMeshBVTree::IMeshBVTree( IMesh * pMesh )
{
	m_pMesh = pMesh;
	m_pRoot = NULL;
}

void IMeshBVTree::SetMesh( IMesh * pMesh )
{
	Clear();
	m_pMesh = pMesh;
}

void IMeshBVTree::GetMeshBounds( Wml::AxisAlignedBox3f & bounds )
{
	if ( m_pRoot == NULL )
		Initialize();
	if ( m_pRoot ) {
		bounds = m_pRoot->Box;
	}
}


bool IMeshBVTree::FindRayIntersection( const Wml::Vector3f & vOrigin, const Wml::Vector3f & vDirection,
								 	   Wml::Vector3f & vHit, IMesh::TriangleID & nHitTri )
{
	if ( m_pRoot == NULL )
		Initialize();
	if ( ! m_pRoot )
		return false;

	float fNearest = std::numeric_limits<float>::max();
	Ray ray( vOrigin, vDirection );
	bool bFound = FindRayIntersection( m_pRoot, ray, vHit, fNearest, nHitTri );
	return bFound;
}



bool IMeshBVTree::FindRayIntersection( IMeshBVTree::IMeshBVNode * pNode, Ray & ray, 
									   Wml::Vector3f & vHit, float & fNearest, IMesh::TriangleID & nHitTri )
{
	float fNear, fFar;

	// if node is leaf, test box and then tri
	if ( pNode->IsLeaf() ) {
		if ( TestIntersection(pNode, ray, fNear, fFar)  && fNear < fNearest ) {

			// ok, hit box, now try the triangle
			Wml::Triangle3f tri;
			m_pMesh->GetTriangle(pNode->GetTriangleID(), tri.V);
			Wml::IntrRay3Triangle3f intr(ray.wmlRay, tri);
			if ( intr.Find() == true ) {
				Wml::Vector3f vTmp = ray.origin + intr.GetRayT() * ray.direction;
				float fDist = (vTmp - ray.origin).Length();
				if ( fDist < fNearest ) {
					fNearest = fDist;
					vHit = vTmp;
					nHitTri = pNode->GetTriangleID();
					return true;
				}
			}
		}

	} else { 

		if ( TestIntersection( pNode, ray, fNear, fFar )  && fNear < fNearest ) {

			// ok, hit this box, drop to children. Have to check and maybe expand first...
			if ( pNode->HasChildren() == false ) 
				ExpandNode( pNode );
			assert( pNode->pLeft && pNode->pRight );

			bool bLeft = FindRayIntersection( pNode->pLeft, ray, vHit, fNearest, nHitTri );
			bool bRight = FindRayIntersection( pNode->pRight, ray, vHit, fNearest, nHitTri );
			if ( bLeft || bRight )
				return true;
		}
	}

	return false;
}

bool IMeshBVTree::FindNearest(const Wml::Vector3f & vPoint, IMesh::TriangleID & nNearestTri)
{
	if ( m_pRoot == NULL )
		Initialize();
	if ( ! m_pRoot )
		return false;

	float fNearest = std::numeric_limits<float>::max();
    Wml::Vector3f  vNearest;
	FindNearest( m_pRoot, vPoint, vNearest, fNearest, nNearestTri );
	return true;
}



bool IMeshBVTree::FindNearest( const Wml::Vector3f & vPoint, Wml::Vector3f & vNearest, IMesh::TriangleID & nNearestTri )
{
	if ( m_pRoot == NULL )
		Initialize();
	if ( ! m_pRoot )
		return false;

	float fNearest = std::numeric_limits<float>::max();
	FindNearest( m_pRoot, vPoint, vNearest, fNearest, nNearestTri );
	return true;
}


bool IMeshBVTree::FindNearestVtx( const Wml::Vector3f & vPoint, IMesh::VertexID & nNearestVtx )
{
	IMesh::TriangleID tID;
	Wml::Vector3f vNearest;
	if ( ! FindNearest(vPoint, vNearest, tID) )
		return false;
	IMesh::VertexID nTri[3];
	m_pMesh->GetTriangle(tID, nTri);
	Wml::Vector3f vTri[3];
	m_pMesh->GetTriangle(tID, vTri);
	float fNearest = std::numeric_limits<float>::max();
	int nNearest = 0;
	for ( int j = 0; j < 3; ++j ) {
		float fDist = (vPoint - vTri[j]).SquaredLength();
		if ( fDist < fNearest ) {
			fNearest = fDist;
			nNearest = j;
		}
	}
	nNearestVtx = nTri[nNearest];
	return true;
}


bool IMeshBVTree::FindNearest( IMeshBVTree::IMeshBVNode * pNode, const Wml::Vector3f & vPoint, 
							   Wml::Vector3f & vNearest, float & fNearest, IMesh::TriangleID & nNearestTri )
{
	float fDistance;

	// if node is leaf, test box and then tri
	if ( pNode->IsLeaf() ) {
		if ( (fDistance = MinDistance(pNode,vPoint)) < fNearest ) {

			// ok, hit box, now try the triangle
			Wml::Triangle3f tri;
			m_pMesh->GetTriangle(pNode->GetTriangleID(), tri.V);
			Wml::DistVector3Triangle3f dist(vPoint, tri);
			float fTriDist = dist.Get();
			if ( fTriDist < fNearest ) {
				fNearest = fTriDist;
				vNearest = Wml::Vector3f::ZERO;
				for ( int j = 0 ; j < 3; ++j ) 
					vNearest += tri.V[j] * dist.GetTriangleBary(j);
				nNearestTri = pNode->GetTriangleID();
				return true;
			}
		}

	} else { 

		if ( (fDistance = MinDistance(pNode,vPoint)) < fNearest ) {

			// ok, hit this box, drop to children. Have to check and maybe expand first...
			if ( pNode->HasChildren() == false ) 
				ExpandNode( pNode );
			assert( pNode->pLeft && pNode->pRight );

			bool bLeft = FindNearest( pNode->pLeft, vPoint, vNearest, fNearest, nNearestTri );
			bool bRight = FindNearest( pNode->pRight, vPoint, vNearest, fNearest, nNearestTri );
			if ( bLeft || bRight )
				return true;
		}
	}

	return false;
}



void IMeshBVTree::ExpandAll()
{
	if ( m_pRoot == NULL )
		Initialize();
	if ( m_pRoot )
		ExpandAll(m_pRoot);
}

void IMeshBVTree::ExpandAll( IMeshBVTree::IMeshBVNode * pNode )
{
	if ( ! pNode->IsLeaf() ) {
		ExpandNode(pNode);
		ExpandAll(pNode->pLeft);
		ExpandAll(pNode->pRight);
	}
}


void IMeshBVTree::Clear()
{
	m_vTriangles.resize(0);
	m_vNodePool.ClearAll();
	m_pRoot = NULL;
	m_nNodeIDGen = 1;
	m_nMaxTriangle = 0;
}

IMeshBVTree::IMeshBVNode * IMeshBVTree::GetNewNode()
{
	IMeshBVNode * pNode = m_vNodePool.Allocate();
	pNode->ID = m_nNodeIDGen++;
	pNode->pLeft = NULL;
	pNode->pRight = NULL;
	pNode->SetTriangleID(0);
	return pNode;
}

void IMeshBVTree::Initialize()
{
	Clear();

	// count tris
	unsigned int nTris = 0;
	IMesh::ITriIterator curt(m_pMesh->BeginITriangles()), endt(m_pMesh->EndITriangles());
	while ( curt != endt ) {
		++nTris;  ++curt;
	}
	if ( nTris < 2 )
		return;
	
	m_vTriangles.resize(nTris);
	m_nMaxTriangle = nTris;
	m_pRoot = GetNewNode();

	unsigned int nIndex = 0;
	curt = m_pMesh->BeginITriangles();
	while ( curt != endt ) {
		IMesh::TriangleID tID = *curt;  ++curt;
		m_vTriangles[nIndex].nNodeID = m_pRoot->ID;
		m_vTriangles[nIndex].triID = tID;
		m_vTriangles[nIndex].nJump = 1;
		++nIndex;
	}

	m_pRoot->SetIndex(0);
	ComputeBox(m_pRoot);
}


void IMeshBVTree::ComputeBox( IMeshBVNode * pNode )
{
	Wml::Vector3f vTri[3];
	
	if ( pNode->IsLeaf() ) {
		m_pMesh->GetTriangle( pNode->GetTriangleID(), vTri );
		pNode->Box = Wml::AxisAlignedBox3f(
			vTri[0].X(), vTri[0].X(), vTri[0].Y(), vTri[0].Y(), vTri[0].Z(), vTri[0].Z() );
		for ( int j = 1; j < 3; ++j )
			pNode->Union( vTri[j] );

	} else { 
		unsigned int nIndex = pNode->GetIndex();

		// initialize first box
		m_pMesh->GetTriangle( m_vTriangles[nIndex].triID, vTri );
		pNode->Box = Wml::AxisAlignedBox3f(
			vTri[0].X(), vTri[0].X(), vTri[0].Y(), vTri[0].Y(), vTri[0].Z(), vTri[0].Z() );
		for ( int j = 1; j < 3; ++j )
			pNode->Union( vTri[j] );
		nIndex = GetNextEntryIdx(pNode, nIndex);

		// iterate over rest of boxes
		while ( nIndex < m_nMaxTriangle ) {
			m_pMesh->GetTriangle( m_vTriangles[nIndex].triID, vTri );
			for ( int j = 0; j < 3; ++j )
				pNode->Union( vTri[j] );
			unsigned int nNextIdx = GetNextEntryIdx(pNode, nIndex);
			nIndex = nNextIdx;
		}
	}
}



void IMeshBVTree::ExpandNode( IMeshBVTree::IMeshBVNode * pNode )
{
	// have to split - will have 2 children, no matter what
	IMeshBVNode * pLeft = GetNewNode();    pNode->pLeft = pLeft;
	IMeshBVNode * pRight = GetNewNode();   pNode->pRight = pRight;
		
	// find axis means
	Wml::Vector3f vMeans( Wml::Vector3f::ZERO );
	Wml::Vector3f vTri[3];

	unsigned int nCount = 0;
	unsigned int nIndex = pNode->GetIndex();
	while ( nIndex < m_nMaxTriangle ) {
		m_pMesh->GetTriangle( m_vTriangles[nIndex].triID, vTri );
		vMeans += (1.0f/3.0f) * Wml::Vector3f( vTri[0] + vTri[1] + vTri[2] );
		++nCount;
		nIndex = GetNextEntryIdx(pNode, nIndex);
	}
	vMeans *= 1.0f / (float)nCount;

	// split box on mean of largest axis
	float fWidth = pNode->Box.XMax() - pNode->Box.XMin();
	float fHeight = pNode->Box.YMax() - pNode->Box.YMin();
	float fDepth = pNode->Box.ZMax() - pNode->Box.ZMin();
	float fMax = std::max( fWidth, std::max(fHeight, fDepth) );
	int nSplit = (fMax == fWidth) ? 0 :
				(fMax == fDepth) ? 2 : 1;

	unsigned int nLastLeft = -1;
	unsigned int nLastRight = -1;
	unsigned int nLeftCount = 0;
	unsigned int nRightCount = 0;

	// split tris between left and right boxes
	nIndex = pNode->GetIndex();
	while ( nIndex < m_nMaxTriangle ) {
		unsigned int nNextIndex = GetNextEntryIdx(pNode, nIndex);

		m_pMesh->GetTriangle( m_vTriangles[nIndex].triID, vTri );
		Wml::Vector3f vCentroid( vTri[0] + vTri[1] + vTri[2] );
		vCentroid *= (1.0f / 3.0f);

		if ( vCentroid[nSplit] < vMeans[nSplit] ) {		// assign to left
			m_vTriangles[nIndex].nNodeID = pLeft->ID;
			if ( nLastLeft == -1 ) {
				pLeft->SetIndex( nIndex );
			} else
				SetJump(nLastLeft, nIndex);
			nLastLeft = nIndex;
			++nLeftCount;

		} else {		// assign to right
			m_vTriangles[nIndex].nNodeID = pRight->ID;
			if ( nLastRight == -1 ) {
				pRight->SetIndex( nIndex );
			} else
				SetJump(nLastRight, nIndex);
			nLastRight = nIndex;
			++nRightCount;
		}

		nIndex = nNextIndex;
	}

	// handle bad case where some child ended up with no nodes
	if ( nLeftCount == 0 || nRightCount == 0 ) {
		IMeshBVNode * pUseNode = (nLeftCount == 0) ? pRight : pLeft;
		int nLastLeft = -1;
		int nLastRight = -1;
		nLeftCount = nRightCount = 0;

		bool bLeft = true;
		nIndex = pUseNode->GetIndex();
		while ( nIndex < m_nMaxTriangle ) {	
			unsigned int nNextIndex = GetNextEntryIdx(pUseNode, nIndex);

			if ( bLeft ) {		// assign to left
				m_vTriangles[nIndex].nNodeID = pLeft->ID;
				if ( nLastLeft == -1 ) {
					pLeft->SetIndex( nIndex );
				} else
					SetJump(nLastLeft, nIndex);
				nLastLeft = nIndex;
				++nLeftCount;

			} else {		// assign to right
				m_vTriangles[nIndex].nNodeID = pRight->ID;
				if ( nLastRight == -1 ) {
					pRight->SetIndex( nIndex );
					nLastRight = nIndex;
				} else
					SetJump(nLastRight, nIndex);
				nLastRight = nIndex;
				++nRightCount;
			}

			nIndex = nNextIndex;
			bLeft = ! bLeft;
		}
	}

	assert( nLeftCount != 0 && nRightCount != 0 );
	if ( nLeftCount == 0 || nRightCount == 0 )
		DebugBreak();

	// set leaf flags
	if ( nLeftCount == 1 ) {
		pLeft->SetTriangleID( m_vTriangles[pLeft->GetIndex()].triID );
		assert(pLeft->IsLeaf());
	}
	if ( nRightCount == 1 ) {
		pRight->SetTriangleID( m_vTriangles[pRight->GetIndex()].triID );
		assert(pRight->IsLeaf());
	}

	ComputeBox( pLeft );
	ComputeBox( pRight );
}




IMeshBVTree::Ray::Ray( const Wml::Vector3f & vOrigin, const Wml::Vector3f & vDirection )
{
	origin = vOrigin;
	direction = vDirection;
	inv_direction = Wml::Vector3f(1/direction.X(), 1/direction.Y(), 1/direction.Z());
	sign[0] = (inv_direction.X() < 0);
	sign[1] = (inv_direction.Y() < 0);
	sign[2] = (inv_direction.Z() < 0);	

	wmlRay.Origin = vOrigin;
	wmlRay.Direction = vDirection;
}


bool IMeshBVTree::TestIntersection( IMeshBVTree::IMeshBVNode * pNode, Ray & r, float & fNear, float & fFar )
{
	// code from JGT paper on fast robust AABB intersection tests

	Wml::Vector3f parameters[2] = {
			Wml::Vector3f( pNode->Box.XMin(), pNode->Box.YMin(), pNode->Box.ZMin() ),
			Wml::Vector3f( pNode->Box.XMax(), pNode->Box.YMax(), pNode->Box.ZMax() )  };

	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	tmin = (parameters[r.sign[0]].X() - r.origin.X()) * r.inv_direction.X();
	tmax = (parameters[1-r.sign[0]].X() - r.origin.X()) * r.inv_direction.X();
	tymin = (parameters[r.sign[1]].Y() - r.origin.Y()) * r.inv_direction.Y();
	tymax = (parameters[1-r.sign[1]].Y() - r.origin.Y()) * r.inv_direction.Y();
	if ( (tmin > tymax) || (tymin > tmax) ) 
		return false;
	if (tymin > tmin)
		tmin = tymin;
	if (tymax < tmax)
		tmax = tymax;
	tzmin = (parameters[r.sign[2]].Z() - r.origin.Z()) * r.inv_direction.Z();
	tzmax = (parameters[1-r.sign[2]].Z() - r.origin.Z()) * r.inv_direction.Z();
	if ( (tmin > tzmax) || (tzmin > tmax) ) 
		return false;
	if (tzmin > tmin)
		tmin = tzmin;
	if (tzmax < tmax)
		tmax = tzmax;
//	return ( (tmin < t1) && (tmax > t0) );		// [RMS] we have no max bounds, ray starts at 0
	fNear = tmin;
	fFar = tmax;
	return ( tmin > 0 );
}


float IMeshBVTree::MinDistance( IMeshBVNode * pNode, const Wml::Vector3f & vPoint )
{
	Wml::AxisAlignedBox3f & box = pNode->Box;
	float fDist[3] = {0,0,0};
	bool bIn[3] = {false,false,false};
	if ( vPoint.X() > box.XMax() )			fDist[0] = vPoint.X() - box.XMax();
	else if ( vPoint.X()  < box.XMin() )	fDist[0] = box.XMin() - vPoint.X();
	else									bIn[0] = true;

	if ( vPoint.Y() > box.YMax() )			fDist[1] = vPoint.Y() - box.YMax();
	else if ( vPoint.Y()  < box.YMin() )	fDist[1] = box.YMin() - vPoint.Y();
	else									bIn[1] = true;

	if ( vPoint.Z() > box.ZMax() )			fDist[2] = vPoint.Z() - box.ZMax();
	else if ( vPoint.Z()  < box.ZMin() )	fDist[2] = box.ZMin() - vPoint.Z();
	else									bIn[2] = true;

    if ( bIn[0] && bIn[1] && bIn[2] )
		return 0.0f;
	else
		return (float)sqrt( fDist[0]*fDist[0] + fDist[1]*fDist[1] + fDist[2]*fDist[2] );
}