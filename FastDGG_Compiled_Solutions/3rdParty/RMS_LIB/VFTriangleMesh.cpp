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
#include "StdAfx.h"
#include "VFTriangleMesh.h"
#include "VectorUtil.h"
#include "MeshUtils.h"

using namespace rms;

#include "rmsdebug.h"

// comment this out to remove edge support (significantly speeds up mesh construction)
#define CREATE_EDGES

VFTriangleMesh::VFTriangleMesh(void)
{
}

VFTriangleMesh::~VFTriangleMesh(void)
{
}

VFTriangleMesh::VFTriangleMesh( VFTriangleMesh & copy, std::vector<VertexID> * vVertMap )
{
	Copy(copy, vVertMap);
}

VFTriangleMesh::VFTriangleMesh( IMesh & copy, std::vector<VertexID> * vVertMap )
{
	Copy(copy, vVertMap);
}


IMesh::VertexID VFTriangleMesh::AppendVertex( const Wml::Vector3f & vVertex, const Wml::Vector3f * pNormal )
{ 
	VertexID vNewID = 
		(VertexID)m_vVertices.insert( Vertex(vVertex, (pNormal) ? *pNormal : Wml::Vector3f::UNIT_Z ) );
	if ( m_vVertices[vNewID].pData == NULL ) {
		m_vVertices[vNewID].pData = m_VertDataMemPool.Allocate();
		m_vVertices[vNewID].pData->vTriangles = m_VertListPool.GetList();
		m_vVertices[vNewID].pData->vEdges = m_VertListPool.GetList();
	}

	// clear lists...
	VertexData & v = * m_vVertices[vNewID].pData;
	m_VertListPool.Clear( v.vTriangles );
	m_VertListPool.Clear( v.vEdges );

	return vNewID;
}


int int_compare(const void * a, const void * b) 
{
	return *(const int *)a < *(const int *)b;
}

IMesh::TriangleID VFTriangleMesh::AppendTriangle( IMesh::VertexID v1, IMesh::VertexID v2, IMesh::VertexID v3 )
{ 
	assert( m_vVertices.isValid(v1) && m_vVertices.isValid(v2) && m_vVertices.isValid(v3) );

	// insert new triangle
	TriangleID tID = (TriangleID)m_vTriangles.insert( Triangle(v1,v2,v3) ); 

	// increment reference counts
	m_vVertices.increment( v1 );
	m_vVertices.increment( v2 );
	m_vVertices.increment( v3 );

	// add to triangle lists
	AddTriEntry( tID, v1 );
	AddTriEntry( tID, v2 );
	AddTriEntry( tID, v3 );

#ifdef CREATE_EDGES
	// add edges
	AddTriangleEdge(tID, v1, v2);
	AddTriangleEdge(tID, v2, v3);
	AddTriangleEdge(tID, v3, v1);
#endif

	return tID;
}



bool VFTriangleMesh::SetTriangle( IMesh::TriangleID tID, IMesh::VertexID v1, IMesh::VertexID v2, IMesh::VertexID v3 )
{
	if ( m_vTriangles.isValid(tID) ) {
		Triangle & t = m_vTriangles[tID];

		// this should never happen and indicates supreme broken-ness..
		if ( FindEdge(t.nVertices[0], t.nVertices[1]) == InvalidID ||
			 FindEdge(t.nVertices[1], t.nVertices[2]) == InvalidID ||
			 FindEdge(t.nVertices[2], t.nVertices[0]) == InvalidID )
			 return false;

		// remove existing edges
		RemoveTriangleEdge(tID, t.nVertices[0], t.nVertices[1]);
		RemoveTriangleEdge(tID, t.nVertices[1], t.nVertices[2]);
		RemoveTriangleEdge(tID, t.nVertices[2], t.nVertices[0]);

		// decrement existing reference counts
		assert( m_vVertices.isValid(t.nVertices[0]) && m_vVertices.isValid(t.nVertices[1]) && m_vVertices.isValid(t.nVertices[2]) );
		RemoveTriEntry( tID, t.nVertices[0] );
		RemoveTriEntry( tID, t.nVertices[1] );
		RemoveTriEntry( tID, t.nVertices[2] );
		m_vVertices.decrement(t.nVertices[0]);
		m_vVertices.decrement(t.nVertices[1]);
		m_vVertices.decrement(t.nVertices[2]);

		// set new IDs
		t.nVertices[0] = v1;
		t.nVertices[1] = v2;
		t.nVertices[2] = v3;

		// increment new reference counts
		assert( m_vVertices.isValid(v1) && m_vVertices.isValid(v2) && m_vVertices.isValid(v3) );
		m_vVertices.increment(v1);
		m_vVertices.increment(v2);
		m_vVertices.increment(v3);
		AddTriEntry( tID, v1 );
		AddTriEntry( tID, v2 );
		AddTriEntry( tID, v3 );

		// TODO: need to check that these are boundary edges (should check before we do the
		//   remove above !!!! )

		// add new edges
		AddTriangleEdge( tID, v1, v2 );
		AddTriangleEdge( tID, v2, v3 );
		AddTriangleEdge( tID, v3, v1 );

		return true;
	}
	return false;
}


void VFTriangleMesh::Clear( bool bFreeMem )
{
	IMesh::Clear(bFreeMem);
	m_vVertices.clear( bFreeMem );
	m_vEdges.clear( bFreeMem );
	m_vTriangles.clear( bFreeMem );
	m_VertDataMemPool.ClearAll();
	m_VertListPool.Clear(bFreeMem);
}


IMesh::EdgeID VFTriangleMesh::AddTriangleEdge( TriangleID tID, VertexID v1, VertexID v2 )
{
	if ( v1 == v2 )
		DebugBreak();
	if ( v1 > v2 ) {
		VertexID tmp = v1; v1 = v2; v2 = tmp;
	}
	EdgeID eID = FindEdge(v1, v2);
	if ( eID == InvalidID ) {
		EdgeID eID = (EdgeID)m_vEdges.insert( Edge(v1, v2, tID, InvalidID) );
		AddEdgeEntry( eID, v1 );
		AddEdgeEntry( eID, v2 );
	} else {
		assert( m_vEdges.isValid(eID) );
		Edge & e = m_vEdges[eID];
		assert(e.nVertices[0] == v1  && e.nVertices[1] == v2);
		assert(e.nTriangles[1] == InvalidID );
		e.nTriangles[1] = tID;
	}
	return eID;
}

// RMS TODO OPTIMIZE add a version that takes Edge & e, to avoid multiple FindEdge....
bool VFTriangleMesh::RemoveTriangleEdge( TriangleID tID, VertexID v1, VertexID v2 )
{
	if ( v1 > v2 ) {
		VertexID tmp = v1; v1 = v2; v2 = tmp;
	}
	EdgeID eID = FindEdge(v1, v2);
	if ( ! IsEdge(eID) )
		return false;
	Edge & e = m_vEdges[eID];
	if ( e.nTriangles[1] == InvalidID ) {	// this edge will no longer be used
		assert(e.nTriangles[0] == tID);
		e.nTriangles[0] = InvalidID;		// clear for later (?)
		RemoveEdgeEntry(eID, v1);
		RemoveEdgeEntry(eID, v2);
		m_vEdges.remove(eID);
	} else {
		e.nTriangles[0] = (e.nTriangles[0] == tID) ? e.nTriangles[1] : e.nTriangles[0];
		e.nTriangles[1] = InvalidID;
	}
	return true;
}



void VFTriangleMesh::RemoveVertex( VertexID vID )
{
	if ( ! m_vVertices.isValid( vID ) )
		return;

	VFTriangleMesh::Vertex & v = m_vVertices[vID];

	// [RMS] HACK! This case shouldn't happen, but it does in ::Weld() because
	//  SetTriangle() doesn't remove un-referenced vertices (which it really
	//   shouldn't, since we might be performing mesh surgery stuff....
	if ( v.pData->vTriangles.pFirst == NULL ) {
		if ( m_vVertices.refCount( vID ) == 1 )
			m_vVertices.remove( vID );
		else
			DebugBreak();
	} else { 
		// remove each attached face
		while ( m_vVertices.isValid( vID ) && v.pData->vTriangles.pFirst != NULL )
			RemoveTriangle(v.pData->vTriangles.pFirst->data );
	}

	// vertex should be gone now because no attached faces remain! 
	assert( ! m_vVertices.isValid( vID ) );
}


void VFTriangleMesh::RemoveTriangle( TriangleID tID )
{
	Triangle & t = m_vTriangles[tID];

	// decrement existing reference counts
	assert( m_vVertices.isValid(t.nVertices[0]) && m_vVertices.isValid(t.nVertices[1]) && m_vVertices.isValid(t.nVertices[2]) );
	for ( int i = 0; i < 3; ++i ) {
		RemoveTriEntry( tID, t.nVertices[i] );
		m_vVertices.decrement(t.nVertices[i]);
	}

	// remove edges
	RemoveTriangleEdge(tID, t.nVertices[0], t.nVertices[1]);
	RemoveTriangleEdge(tID, t.nVertices[1], t.nVertices[2]);
	RemoveTriangleEdge(tID, t.nVertices[2], t.nVertices[0]);

	// remove vertex if refcount == 1  (means that it is only referenced by self, so is safe to delete)
	for ( int i = 0; i < 3; ++i ) {
		if ( m_vVertices.refCount( t.nVertices[i] ) == 1 )
			m_vVertices.remove( t.nVertices[i]  );
	}

	// remove triangle
	m_vTriangles.remove( tID );
}

void VFTriangleMesh::RemoveUnreferencedGeometry()
{
	
}


//! initialize vertex neighbour iteration
void VFTriangleMesh::BeginVtxTriangles( VtxNbrItr & v )
{
	assert( m_vVertices.isValid(v.vID) );
	VFTriangleMesh::Vertex & vtx = m_vVertices[v.vID];
	if ( vtx.pData == NULL || vtx.pData->vTriangles.pFirst == NULL )
		v.nData[0] = IMesh::InvalidID;
	else
		v.nData[0] = (unsigned long long)(vtx.pData->vTriangles.pFirst);
	v.nData[1] = 1234567890;
}

//! (possibly) un-ordered iteration around one-ring of a vertex. Returns InvalidID when done
IMesh::TriangleID VFTriangleMesh::GetNextVtxTriangle( VtxNbrItr & v )
{
	if ( v.nData[0] == IMesh::InvalidID )
		return IMesh::InvalidID;
	
	TriListEntry * pCur = (TriListEntry *)v.nData[0];
	IMesh::TriangleID tID = pCur->data;

	if ( pCur->pNext == NULL )
		v.nData[0] = IMesh::InvalidID;
	else
		v.nData[0] = (unsigned long long)(pCur->pNext);
	
	return tID;
}



//! initialize vertex neighbour iteration
void VFTriangleMesh::BeginVtxEdges( VtxNbrItr & v )
{
	assert( m_vVertices.isValid(v.vID) );
	VFTriangleMesh::Vertex & vtx = m_vVertices[v.vID];
	if ( vtx.pData == NULL || vtx.pData->vEdges.pFirst == NULL  )
		v.nData[0] = IMesh::InvalidID;
	else
		v.nData[0] = (unsigned long long)(vtx.pData->vEdges.pFirst);
	v.nData[1] = 1234567890;
}

//! (possibly) un-ordered iteration around one-ring of a vertex. Returns InvalidID when done
IMesh::EdgeID VFTriangleMesh::GetNextVtxEdges( VtxNbrItr & v )
{
	if ( v.nData[0] == IMesh::InvalidID )
		return IMesh::InvalidID;
	
	EdgeListEntry * pCur = (EdgeListEntry *)v.nData[0];
	IMesh::EdgeID eID = pCur->data;

	if ( pCur->pNext == NULL )
		v.nData[0] = IMesh::InvalidID;
	else
		v.nData[0] = (unsigned long long)(pCur->pNext);
	
	return eID;
}



void VFTriangleMesh::EdgeIteration( VertexID vID, VFTriangleMesh::NeighborEdgeCallback * pCallback )
{
	VtxNbrItr itr(vID);
	BeginVtxEdges(itr);
	EdgeID eID = GetNextVtxEdges(itr);

	pCallback->BeginEdges();

	while ( eID != InvalidID ) {
		pCallback->NextEdge(eID);
		eID = GetNextVtxEdges(itr);
	}

	pCallback->EndEdges();
}





bool VFTriangleMesh::IsBoundaryEdge( VertexID v1, VertexID v2 )
{
	return IsBoundaryEdge( FindEdge(v1,v2) );
}

bool VFTriangleMesh::IsBoundaryEdge( EdgeID eID )
{
	if ( eID == InvalidID || ! m_vEdges.isValid(eID) ) {
		assert( m_vEdges.isValid(eID) );
		return false;
	}
	Edge & e = m_vEdges[eID];
	return ( e.nTriangles[0] == InvalidID || e.nTriangles[1] == InvalidID );
}



bool VFTriangleMesh::IsBoundaryVertex( VertexID vID )
{
	// RMS TODO: it should be possible to do this just using
	//  a 3-vertex buffer (ie determine direction by looking
	//  at last and second-last vertices...)
	//  (Maybe even 2-vertex buffer?)

	Vertex & v = m_vVertices[vID];

	// count triangles and make a list of them
	int nCount = 0;
	TriListEntry * pCur = v.pData->vTriangles.pFirst;
	while ( pCur != NULL ) {
		pCur = pCur->pNext;
		nCount++;
	}
	if ( nCount == 0 )
		return true;

	std::vector< TriangleID > vTris;
	vTris.resize( nCount-1 );
	pCur = v.pData->vTriangles.pFirst->pNext;
	int i = 0;
	while ( pCur != NULL ) {
		vTris[ i++ ] = pCur->data;
		pCur = pCur->pNext;
	}

	// pick first edge
	VertexID vCurID = InvalidID;
	VertexID vStopID = InvalidID;
	pCur = v.pData->vTriangles.pFirst;
	VertexID * pTri = m_vTriangles[ pCur->data ].nVertices;
	for ( int i = 0; i < 3; ++i ) {
		if ( pTri[i] == vID ) {
			vCurID = pTri[ (i+1) % 3 ];
			vStopID = pTri[ (i+2) % 3];
			break;
		} else if ( pTri [ (i+1) % 3 ] == vID ) {
			vCurID = pTri[i];
			vStopID = pTri[ (i+2) % 3 ];
			break;
		}
	}
	nCount--;   // used up first tri

	// loop until we hit vStopID
	while ( vCurID != vStopID ) {
		
		// find unused tri w/ [vID, vCurID, X]
		int nCurTri = InvalidID;
		for ( int i = 0; i < nCount; ++i ) {
			if ( vTris[i] == InvalidID )
				continue;
			VertexID * pTri = m_vTriangles[ vTris[i] ].nVertices;
			if ( pTri[0] == vCurID || pTri[1] == vCurID || pTri[2] == vCurID ) {
				nCurTri = i;
				break;
			}
		}
		if ( nCurTri == InvalidID )
			return true;			// 1-ring is not connected - must be invalid!

		// mark tri as done
		VertexID * pTri = m_vTriangles[ vTris[nCurTri] ].nVertices;
		vTris[ nCurTri ] = InvalidID;

		// go to next tri in one-ring
		if ( pTri[0] == vID ) 
			vCurID = ( pTri[1] == vCurID ) ? pTri[2] : pTri[1];
		else if ( pTri[1] == vID )
			vCurID = ( pTri[0] == vCurID ) ? pTri[2] : pTri[0];
		else
			vCurID = ( pTri[0] == vCurID ) ? pTri[1] : pTri[0];
	}

	return false;
}


void VFTriangleMesh::FindNeighbours( TriangleID tID, TriangleID vNbrs[3] )
{
	Triangle & t = m_vTriangles[tID];
	for ( int i = 0; i < 3; ++i ) {

		VertexID v1 = t.nVertices[i];
		VertexID v2 = t.nVertices[ (i+1) % 3];

		// iterate over triangles of v1, looking for another tri with edge [v1,v2]
		Vertex & v = m_vVertices[v1];
		TriListEntry * pCur = v.pData->vTriangles.pFirst;
		TriListEntry * pLast = NULL;
		bool bFound = false;
		while ( pCur != NULL && ! bFound ) {
			pLast = pCur;
			TriangleID tCurID = pCur->data; pCur = pCur->pNext;
			if ( tCurID == tID )
				continue;
			VertexID * vTri2 = m_vTriangles[tCurID].nVertices;
			if ( vTri2[0] == v1 && ( vTri2[1] == v2 || vTri2[2] == v2 ) )
				bFound = true;
			else if ( vTri2[1] == v1 && ( vTri2[0] == v2 || vTri2[2] == v2 ) )
				bFound = true;
			else if ( vTri2[2] == v1 && ( vTri2[0] == v2 || vTri2[1] == v2 ) )
				bFound = true;
		}
		if ( bFound )
			vNbrs[i] = pLast->data;
		else
			vNbrs[i] = IMesh::InvalidID;
	}
}



IMesh::EdgeID VFTriangleMesh::FindEdge( VertexID v1, VertexID v2 )
{
	if ( v1 == v2 )
		DebugBreak();
	if ( v1 > v2 ) {
		VertexID tmp = v1; v1 = v2; v2 = tmp;
	}

	Vertex & v = m_vVertices[v1];
	EdgeListEntry * pCur = v.pData->vEdges.pFirst;
	while ( pCur != NULL ) {
		Edge & e = m_vEdges[ pCur->data ];
		if ( (e.nVertices[0] == v1 && e.nVertices[1] == v2) ||
			 (e.nVertices[0] == v2 && e.nVertices[1] == v1 ) )
			 return pCur->data;
		pCur = pCur->pNext;
	}
	return IMesh::InvalidID;
}



void VFTriangleMesh::Copy( VFTriangleMesh & mesh )
{
	std::vector<VertexID> vIDMap;
	vIDMap.resize( mesh.GetMaxVertexID() );

	Clear(true);
	Wml::Vector3f vVertex, vNormal;
	vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		VertexID vID = *curv;  ++curv;
		mesh.GetVertex(vID, vVertex, &vNormal);
		VertexID vNew = AppendVertex(vVertex, &vNormal);
		vIDMap[vID] = vNew;
	}

	VertexID nTri[3];
	triangle_iterator curt(mesh.BeginTriangles()), endt(mesh.EndTriangles());
	while ( curt != endt ) { 
		TriangleID tID = *curt; ++curt;
		mesh.GetTriangle(tID, nTri);
		TriangleID tNew = AppendTriangle( vIDMap[nTri[0]], vIDMap[nTri[1]], vIDMap[nTri[2]]);
		//if ( tNew != tID )
		//	DebugBreak();
	}
}

void VFTriangleMesh::CopyVertInfo( VFTriangleMesh & mesh )
{
	if ( mesh.GetVertexCount() != GetVertexCount() )
		DebugBreak();
	Wml::Vector3f vVertex, vNormal;
	vertex_iterator curv(mesh.BeginVertices()), endv(mesh.EndVertices());
	while ( curv != endv ) {
		VertexID vID = *curv;  ++curv;
		mesh.GetVertex(vID, vVertex, &vNormal);
		SetVertex(vID, vVertex, &vNormal);
	}
}


void VFTriangleMesh::Copy( IMesh & copy, std::vector<VertexID> * vVertMap )
{
	Clear(false);

	std::vector< IMesh::VertexID > vInternalVertMap;
	std::vector< IMesh::VertexID > & vMapV = (vVertMap) ? *vVertMap : vInternalVertMap;
	vMapV.resize( copy.GetMaxVertexID() );

	Wml::Vector3f vVertex, vNormal;
	IMesh::IVtxIterator curv(copy.BeginIVertices()), endv(copy.EndIVertices());
	while ( curv != endv ) {
		VertexID vID = *curv;  ++curv;
		copy.GetVertex(vID, vVertex, &vNormal );
		vMapV[vID] = AppendVertex(vVertex, &vNormal);
	}

	TriangleID nTri[3];
	IMesh::ITriIterator curt(copy.BeginITriangles()), endt(copy.EndITriangles());
	while ( curt != endt ) {
		TriangleID tID = *curt;  ++curt;
		copy.GetTriangle(tID, nTri);
		AppendTriangle( vMapV[nTri[0]], vMapV[nTri[1]], vMapV[nTri[2]] );
	}
}

bool VFTriangleMesh::Append( VFTriangleMesh & mAppend, std::vector<IMesh::VertexID> & vVertexMap )
{
	unsigned int nMaxID = mAppend.GetMaxVertexID();
	std::vector<IMesh::VertexID> vNewIDs;
	bool bUseInternal = (vVertexMap.size() < nMaxID);
	if ( bUseInternal )
		DebugBreak();
	std::vector<IMesh::VertexID> & vMapV = 
		bUseInternal ? vNewIDs : vVertexMap;
	vMapV.resize( nMaxID );

	Wml::Vector3f vVertex, vNormal;
	vertex_iterator curv( mAppend.BeginVertices() ), endv( mAppend.EndVertices() );
	while ( curv != endv ) {
		IMesh::VertexID vID =  *curv;  ++curv;
		mAppend.GetVertex( vID, vVertex, &vNormal );
		IMesh::VertexID vNewID = AppendVertex( vVertex, &vNormal );
		vMapV[vID] = vNewID;
	}

	IMesh::VertexID nTri[3];
	triangle_iterator curt( mAppend.BeginTriangles() ), endt( mAppend.EndTriangles() );
	while ( curt != endt ) {
		IMesh::TriangleID tID =  *curt;  ++curt;
		mAppend.GetTriangle( tID, nTri );
		AppendTriangle( 
			vMapV[ nTri[0] ], vMapV[ nTri[1] ], vMapV[ nTri[2] ] );
	}

	return ! bUseInternal;
}



void VFTriangleMesh::Weld( VertexID vKeep, VertexID vDiscard )
{
	assert( IsVertex(vKeep) && IsVertex(vDiscard) );
	
	DebugBreak();		// This function is broken! One problem is that the TriListEntry iteration
						// will break once the triangle is changed...but that still doesn't explain
						// all the broken-ness...
	_RMSInfo("Start weld\n");

	Vertex & v = m_vVertices[vDiscard];
	TriListEntry * pCur = v.pData->vTriangles.pFirst;
	while ( pCur != NULL && IsVertex(vDiscard) ) {
		TriangleID tID = pCur->data;
		pCur = pCur->pNext;
		VertexID nTri[3];
		GetTriangle(tID, nTri);
		bool bModified = false;
		for ( int j = 0; j < 3; ++j ) {
			if ( nTri[j] == vDiscard ) {
				nTri[j] = vKeep;
				bModified = true;
			}
		}
		if ( bModified ) {
			SetTriangle(tID, nTri[0], nTri[1], nTri[2]);
			_RMSInfo(" Removed Tri - new count %d %d\n", GetTriangleCount(vDiscard), GetTriangleCount(vKeep));
			pCur = v.pData->vTriangles.pFirst;
		}
	}

	// should be unreferenced now...
	_RMSInfo("  Vtx has %d %d triangles\n",GetTriangleCount(vDiscard), GetTriangleCount(vKeep));
	if ( IsVertex(vDiscard) && GetTriangleCount(vDiscard) == 0 )
		RemoveVertex(vDiscard);
}


void VFTriangleMesh::SplitEdge( VertexID e1, VertexID e2 )
{
	TriangleID t[2] = { InvalidID, InvalidID };
	int ti = 0;

	Vertex * pV1 = &m_vVertices[e1];
	Vertex * pV2 = &m_vVertices[e2];

	// find two triangles with edge e1e2
	TriListEntry * pCur = pV1->pData->vTriangles.pFirst;
	while ( pCur != NULL ) {
		Triangle * pTri = &m_vTriangles[pCur->data];
		if ( (pTri->nVertices[0] == e1 || pTri->nVertices[1] == e1 || pTri->nVertices[2] == e1)
			&& (pTri->nVertices[0] == e2 || pTri->nVertices[1] == e2 || pTri->nVertices[2] == e2) )
			t[ti++] = pCur->data;
		pCur = pCur->pNext;
	}
	if ( ti != 2 )
		return;

	// append new vertex
	Wml::Vector3f vInterp = 0.5f * (pV1->vVertex + pV2->vVertex);
	Wml::Vector3f nInterp = 0.5f * (pV1->vNormal + pV2->vNormal);
	VertexID vNew = AppendVertex(vInterp, &nInterp);
	
	// update triangles
	for ( int j = 0; j < 2; ++j ) {

		// figure out which index is which
		VertexID nTri[3];
		GetTriangle(t[j],nTri);
		VertexID nE1, nE2, nOther;
		for ( int k = 0; k < 3; ++k ) {
			if ( nTri[k] == e1 ) nE1 = k;
			else if ( nTri[k] == e2 ) nE2 = k;
			else nOther = k;
		}
		
		// set triangles
		nTri[nE1] = vNew;
		AppendTriangle(nTri[0], nTri[1], nTri[2]);
		nTri[nE1] = e1;  nTri[nE2] = vNew;
		SetTriangle(t[j], nTri[0], nTri[1], nTri[2]);
	}
}


bool VFTriangleMesh::CollapseEdge( EdgeID eID )
{
	//if ( MeshUtils::CheckForFins(*this) )
	//	DebugBreak();

	if ( ! IsEdge(eID) )
		return false;
	if ( IsBoundaryEdge(eID) )
		return false;

	Edge & e = m_vEdges[eID];
	VertexID vKeep = e.nVertices[0];
	VertexID vErase = e.nVertices[1];

	TriangleID tErase1 = e.nTriangles[0];
	TriangleID tErase2 = e.nTriangles[1];

	// find other verts of diamond
	VertexID vOther1, vOther2;
	Triangle & t1 = m_vTriangles[tErase1];
	Triangle & t2 = m_vTriangles[tErase2];
	for ( int j = 0; j < 3; ++j ) {
		if ( t1.nVertices[j] != vKeep && t1.nVertices[j] != vErase )
			vOther1 = t1.nVertices[j];
		if ( t2.nVertices[j] != vKeep && t2.nVertices[j] != vErase )
			vOther2 = t2.nVertices[j];
	}

	std::vector<IMesh::VertexID> vOneRingKeep, vOneRingErase;
	MeshUtils::FindOneRing(*this, vKeep, vOneRingKeep);
	MeshUtils::FindOneRing(*this, vErase, vOneRingErase);

	// if the one rings of the verts on the edge share any
	// other vertices, the result will be non-manifold. We
	// can't have that...
	// TODO: this could be more efficient if we just iterate over
	//  edges - no need to grab the one-ring vectors...
	bool bOneRingsShareVerts = false;
	for ( unsigned int i = 0; i < vOneRingKeep.size(); ++i ) {
		VertexID v1 = vOneRingKeep[i];
		if ( v1 == vKeep || v1 == vErase || v1 == vOther1 || v1 == vOther2 )
			continue;
		for ( unsigned int j = 0; j < vOneRingErase.size(); ++j )
			if ( v1 == vOneRingErase[j] )
				bOneRingsShareVerts = true;
	}
	if  ( bOneRingsShareVerts )
		return false;


	// algorithm currently can't handle 'tip' collapse...  (but this doesn't actually stop it!)
	if ( GetTriangleCount(vErase) <= 3 || GetTriangleCount(vKeep) <= 3 || 
		 GetTriangleCount(vOther1) <= 3 || GetTriangleCount(vOther2) <= 3 )	
		return false;

	// get tris connected to vErase that need to be fixed
	std::vector<TriangleID> vUpdate;
	TriListEntry * pCur = m_vVertices[vErase].pData->vTriangles.pFirst;
	while ( pCur != NULL ) {
		if ( pCur->data != tErase1 && pCur->data != tErase2 )
			vUpdate.push_back(pCur->data);
		pCur = pCur->pNext;
	}

	// sanity check - if we do this collapse we will make a tri w/ two of the same vertex!
	TriangleID triVerts[3];
	for ( unsigned int k = 0; k < vUpdate.size(); ++k ) {
		GetTriangle(vUpdate[k], triVerts);
		if ( triVerts[0] == vKeep || triVerts[1] == vKeep || triVerts[2] == vKeep ) {
			return false;
		}
	}

	// find new position for vKeep
	Wml::Vector3f vNewPos = 0.5f * (m_vVertices[vKeep].vVertex + m_vVertices[vErase].vVertex);
	Wml::Vector3f vNewNorm = 0.5f * (m_vVertices[vKeep].vNormal + m_vVertices[vErase].vNormal);
	vNewNorm.Normalize();
	SetVertex(vKeep, vNewPos, &vNewNorm);

	// erase edge faces
	RemoveTriangle( tErase1 );
	RemoveTriangle( tErase2 );

	// connect vErase to vKeep
	for ( unsigned int k = 0; k < vUpdate.size(); ++k ) {
		GetTriangle(vUpdate[k], triVerts);

		for ( int j = 0; j < 3; ++j )
			if ( triVerts[j] == vErase )
				triVerts[j] = vKeep;

		// if this happens we're fucked...
		if ( triVerts[0] == triVerts[1] || triVerts[0] == triVerts[2] || triVerts[1] == triVerts[2] )
			DebugBreak();

		bool bOK = SetTriangle(vUpdate[k], triVerts[0], triVerts[1], triVerts[2]);
		if ( ! bOK )
			DebugBreak();
	}

	// have to manually remove vertices, because we don't want to automatically
	// discard them during mesh surgery... (TODO: rewrite algorithms so that would be ok...)
	RemoveVertex(vErase);
	if ( IsVertex(vErase) )
		DebugBreak();

	return true;
}



static float GetOrientation( VFTriangleMesh & mesh, int v1, int v2, int v3 )
{
	Wml::Vector3f vVerts[3], vNorms[3];
	mesh.GetVertex(v1, vVerts[0], & vNorms[0] );
	mesh.GetVertex(v2, vVerts[1], & vNorms[1] );
	mesh.GetVertex(v3, vVerts[2], & vNorms[2] );
	vVerts[1] -= vVerts[0];   vVerts[1].Normalize();
	vVerts[2] -= vVerts[0];   vVerts[2].Normalize();
	Wml::Vector3f vCross(vVerts[1].Cross(vVerts[2]));
	//vCross.Normalize();
	return vCross.Dot(vNorms[0]); // > 0 ? 1 : -1;
}

bool VFTriangleMesh::FlipEdge( EdgeID eID )
{
	Edge & e = m_vEdges[eID];
	VertexID vEdgeV[2] = {e.nVertices[0], e.nVertices[1]};
	TriangleID vEdgeT[2] = {e.nTriangles[0], e.nTriangles[1]};
	Triangle & t1 = m_vTriangles[ vEdgeT[0] ];
	Triangle & t2 = m_vTriangles[ vEdgeT[1] ];

	// figure out triangle indices
	int iOther1, iOther2, ie1[2], ie2[2];
	for ( int j = 0; j < 3; ++j ) {
		if ( t1.nVertices[j] == vEdgeV[0] )			ie1[0] = j;
		else if ( t1.nVertices[j] == vEdgeV[1] )	ie1[1] = j;
		else										iOther1 = j;

		if ( t2.nVertices[j] == vEdgeV[0] )			ie2[0] = j;
		else if ( t2.nVertices[j] == vEdgeV[1] )	ie2[1] = j;
		else										iOther2 = j;
	}			

	int nOther[2] = { t1.nVertices[iOther1], t2.nVertices[iOther2] };
	EdgeID eFlipped = FindEdge( nOther[0], nOther[1] );

	if ( eFlipped != InvalidID )		// flipped edge already exists!
		return false;

	// avoid orientation flips...
	// [RMS TODO] this can be done using indices, which will be much cheaper...
	float fOrient = GetOrientation( *this, nOther[0], nOther[1], vEdgeV[0] );
	if ( fOrient < 0 ) {
		int nTmp = nOther[0];  nOther[0] = nOther[1];  nOther[1] = nTmp;
	}

	// remove edges
	RemoveTriangleEdge(vEdgeT[0], t1.nVertices[0], t1.nVertices[1]);
	RemoveTriangleEdge(vEdgeT[0], t1.nVertices[1], t1.nVertices[2]);
	RemoveTriangleEdge(vEdgeT[0], t1.nVertices[2], t1.nVertices[0]);

	RemoveTriangleEdge(vEdgeT[1], t2.nVertices[0], t2.nVertices[1]);
	RemoveTriangleEdge(vEdgeT[1], t2.nVertices[1], t2.nVertices[2]);
	RemoveTriangleEdge(vEdgeT[1], t2.nVertices[2], t2.nVertices[0]);

	// remove triangle references from vertices
	for ( int j = 0; j < 3; ++j ) {
		RemoveTriEntry( vEdgeT[0], t1.nVertices[j] );
		RemoveTriEntry( vEdgeT[1], t2.nVertices[j] );
	}

	// update triangles
	t1.nVertices[0] = nOther[0];  t1.nVertices[1] = nOther[1];  t1.nVertices[2] = vEdgeV[0];
	AddTriEntry( vEdgeT[0], nOther[0] );
	AddTriEntry( vEdgeT[0], nOther[1] );
	AddTriEntry( vEdgeT[0], vEdgeV[0] );
	
	t2.nVertices[0] = nOther[1];  t2.nVertices[1] = nOther[0];  t2.nVertices[2] = vEdgeV[1];
	AddTriEntry( vEdgeT[1], nOther[1] );
	AddTriEntry( vEdgeT[1], nOther[0] );
	AddTriEntry( vEdgeT[1], vEdgeV[1] );

	// add new edges
	AddTriangleEdge(vEdgeT[0], t1.nVertices[0], t1.nVertices[1]);
	AddTriangleEdge(vEdgeT[0], t1.nVertices[1], t1.nVertices[2]);
	AddTriangleEdge(vEdgeT[0], t1.nVertices[2], t1.nVertices[0]);

	AddTriangleEdge(vEdgeT[1], t2.nVertices[0], t2.nVertices[1]);
	AddTriangleEdge(vEdgeT[1], t2.nVertices[1], t2.nVertices[2]);
	AddTriangleEdge(vEdgeT[1], t2.nVertices[2], t2.nVertices[0]);

	return true;
}



void VFTriangleMesh::ReverseOrientation()
{
	triangle_iterator curt(BeginTriangles()), endt(EndTriangles());
	while ( curt != endt ) {
		TriangleID tID = *curt;  ++curt;
		Triangle & tri = m_vTriangles[tID];
		VertexID tmp = tri.nVertices[2];
		tri.nVertices[2] = tri.nVertices[1];
		tri.nVertices[1] = tmp;
	}
}


void VFTriangleMesh::Cleanup()
{
	// remove duplicate tris
	int nRemoved = 0;
	bool bDone = false;
	while ( ! bDone ) {
		bDone = true;

		std::vector<TriangleID> vTri1(3), vTri2(3);
		triangle_iterator curt(BeginTriangles()), endt(EndTriangles());
		while ( curt != endt && bDone ) {
			TriangleID tID1 = *curt; ++curt;
			GetTriangle(tID1, &vTri1[0]);
			std::sort(vTri1.begin(), vTri1.end());

			triangle_iterator curt2 = curt;  ++curt2;
			while ( curt2 != endt && bDone ) {
				TriangleID tID2 = *curt2; ++curt2;
				GetTriangle(tID2, &vTri2[0]);
				std::sort( vTri2.begin(), vTri2.end() );
				if ( vTri1[0]==vTri2[0] && vTri1[1]==vTri2[1] && vTri1[2]==vTri2[2] ) {
					RemoveTriangle(tID2);
					bDone = false;
					nRemoved++;
				}
			}

		}
	}

	_RMSInfo("Removed %d tris\n", nRemoved);
}


void VFTriangleMesh::ClipEarTriangles()
{
	bool bDone = false;
	while ( ! bDone ) {
		bDone = true;
		vertex_iterator curv(BeginVertices()), endv(EndVertices());
		while ( curv != endv && bDone) {
			VertexID vID = *curv; ++curv;
			if ( GetTriangleCount(vID) == 1 ) {
				TriangleID tID = m_vVertices[vID].pData->vTriangles.pFirst->data;
				RemoveTriangle(tID);
				bDone = false;
			}
		}
	}
}



void VFTriangleMesh::GetBoundingBox( Wml::AxisAlignedBox3f & bounds )
{
	bounds.XMin() = std::numeric_limits<float>::max();

	vertex_iterator curv( BeginVertices() ), endv( EndVertices() );
	while ( curv != endv ) {
		VertexID vID = *curv;  ++curv;
		Wml::Vector3f vVertex;
		GetVertex(vID, vVertex);
		if ( bounds.XMin() == std::numeric_limits<float>::max() )
			bounds = Wml::AxisAlignedBox3f( vVertex.X(), vVertex.X(), vVertex.Y(), vVertex.Y(), vVertex.Z(), vVertex.Z() );
		else
			Union( bounds, vVertex);
	}
}


void VFTriangleMesh::GetEdgeLengthStats(float & fMin, float & fMax, float & fAverage)
{
	fMax = 0.0f;
	fMin = std::numeric_limits<float>::max();
	double fAverageEdge = 0.0f;

	int nCount = 0;
	triangle_iterator curt( BeginTriangles()), endt(EndTriangles());
	while ( curt != endt ) {
		TriangleID nID = *curt;
		curt++;

		Wml::Vector3f vVertices[3];
		GetTriangle(nID, vVertices);

		for ( int i = 0; i < 3; ++i ) {
			float fLen = (vVertices[i] - vVertices[(i+1)%3]).Length();
			if ( fLen < fMin )
				fMin = fLen;
			if ( fLen > fMax )
				fMax = fLen;
			fAverageEdge += fLen;
			++nCount;
		}
	}

	fAverage = (float)(fAverageEdge / (double)nCount);	
}




void VFTriangleMesh::ClearBit( unsigned int nBit )
{
	RefCountedVector<Vertex>::item_iterator 
		curv(m_vVertices.begin_items()), endv(m_vVertices.end_items());
	while ( curv != endv ) {
		(*curv).vecData.nBits &= ~(1<<nBit);
		++curv;
	}
}



bool VFTriangleMesh::ReadOBJ(CRichModel & wxn_model, std::string & errString)
{
	Clear(false);

	//ifstream in(pFilename);
	//if(!in){
	//	errString = string("Cannot open file ") + string(pFilename);
	//	cerr << errString << endl;
	//	return false;
	//}

	string command;
	char c1,c2;
	unsigned int tv1, tv2, tv3, tn1, tn2, tn3, tt1, tt2, tt3;
	char linebuf[1024];

	Wml::Vector3f fvec = Wml::Vector3f::ZERO;

	bool bHasTextures = false;

	// need to save normals separately and then match to vertices (maya
	//  "optimizes" the mesh...argh!)
	std::vector<Wml::Vector3f> vNormals;
	std::vector<Wml::Vector2f> vUVs;

	unsigned int ivec[3];

  for (int i = 0; i < wxn_model.GetNumOfVerts();++i) {
    const CPoint3D& p = wxn_model.Vert(i);
    fvec.X() = p.x; fvec.Y() = p.y; fvec.Z() = p.z;
    AppendVertex(fvec);
  }
  for (int i = 0; i < wxn_model.GetNumOfVerts();++i) {
    const CPoint3D& p = wxn_model.Normal(i);
    fvec.X() = p.x; fvec.Y() = p.y; fvec.Z() = p.z;
    fvec.Normalize();
    vNormals.push_back(fvec);
  }

  for (int i = 0; i < wxn_model.GetNumOfFaces();++i) {
    auto& f = wxn_model.Face(i);
    ivec[0] = f[0]; ivec[1] = f[1]; ivec[2] = f[2];
    AppendTriangle(ivec[0], ivec[1], ivec[2]);
    SetNormal( ivec[0], vNormals[ ivec[0] ] );
    SetNormal( ivec[1], vNormals[ ivec[1] ] );
    SetNormal( ivec[2], vNormals[ ivec[2] ] );
  }

	return true;

}



bool VFTriangleMesh::ReadOBJ( const char * pFilename, std::string & errString )
{
	Clear(false);

	ifstream in(pFilename);
	if(!in){
		errString = string("Cannot open file ") + string(pFilename);
		cerr << errString << endl;
		return false;
	}

	string command;
	char c1,c2;
	unsigned int tv1, tv2, tv3, tn1, tn2, tn3, tt1, tt2, tt3;
	char linebuf[1024];

	Wml::Vector3f fvec = Wml::Vector3f::ZERO;

	bool bHasTextures = false;

	// need to save normals separately and then match to vertices (maya
	//  "optimizes" the mesh...argh!)
	std::vector<Wml::Vector3f> vNormals;
	std::vector<Wml::Vector2f> vUVs;

	unsigned int ivec[3];
	while(in){
		ostrstream s;
		in >> command;
		if(!in)
			continue;
		switch(command.c_str()[0]){

		case 'v':    
			in >> fvec[0] >> fvec[1];
			if(!in)
				continue;
			switch(command.c_str()[1]){
			case '\0':  // vertex
				in >> fvec[2];
				AppendVertex(fvec);
				break;
			case 'n': // vertex normal
				in >> fvec[2];
				fvec.Normalize();
				vNormals.push_back( fvec );
				break;
			case 't':
				if ( ! bHasTextures ) {
					AppendUVSet();
					InitializeUVSet(0);
					bHasTextures = true;
				}
				vUVs.push_back( Wml::Vector2f(fvec) );
				break;
			default:
				string err("Got unknown OBJ command ");
				err += command;
				cerr << err << endl;
			}
		break;

		case 'f':
			if ( bHasTextures ) {
				in >> tv1 >> c1 >> tt1 >> c2 >> tn1;
				in >> tv2 >> c1 >> tt2 >> c2 >> tn2;
				in >> tv3 >> c1 >> tt3 >> c2 >> tn3;
				
			} else {
				in >> tv1 >> c1 >> c2 >> tn1;
				in >> tv2 >> c1 >> c2 >> tn2;
				in >> tv3 >> c1 >> c2 >> tn3;
			}
			ivec[0] = tv1-1; ivec[1] = tv2-1; ivec[2] = tv3-1;
			AppendTriangle(ivec[0], ivec[1], ivec[2]);

			// set proper normals
			SetNormal( ivec[0], vNormals[ tn1-1 ] );
			SetNormal( ivec[1], vNormals[ tn2-1 ] );
			SetNormal( ivec[2], vNormals[ tn3-1 ] );

			if ( bHasTextures ) {
				SetUV( ivec[0], 0, vUVs[tt1-1] );
				SetUV( ivec[1], 0, vUVs[tt2-1] );
				SetUV( ivec[2], 0, vUVs[tt3-1] );
			}

			break;

		default:
			in.getline(linebuf, 1023, '\n');
		}
	}

	return true;
}


bool VFTriangleMesh::WriteOBJ( const char * pFilename, std::string & errString )
{
	std::ofstream out(pFilename);
	if (!out)
		return false;

	bool bHaveVertexTexCoords = HasUVSet(0);

	std::vector<VertexID> vertMap;
	vertMap.resize( GetMaxVertexID(), IMesh::InvalidID );
	unsigned int nCounter = 0;
	vertex_iterator curv(BeginVertices()), endv(EndVertices());
	while ( curv != endv ) {
		VertexID vID = *curv;  ++curv;
		vertMap[vID] = nCounter++;

		Wml::Vector3f vert, norm;

		GetVertex(vID, vert, &norm);
		out << "v " << vert.X() << " " << vert.Y() << " " << vert.Z() << std::endl;
		out << "vn " << norm.X() << " " << norm.Y() << " " << norm.Z() << std::endl; 
		if ( bHaveVertexTexCoords ) {
			Wml::Vector2f tex;
			if ( GetUV(vID, 0, tex) )
				out << "vt " << tex.X() << " " << tex.Y() << std::endl; 
			else
				out << "vt -5 -5" << std::endl;
		}
	}

	Wml::Vector2f vTriUV[3];

	triangle_iterator curt(BeginTriangles()), endt(EndTriangles());
	while ( curt != endt ) {
		TriangleID tID = *curt; ++curt;
		VertexID vTri[3];
		GetTriangle(tID, vTri);
		unsigned int tri[3];
		for ( int j = 0; j < 3; ++j )
			tri[j] = vertMap[vTri[j]];


		if ( bHaveVertexTexCoords ) {
			out << "f " << (tri[0]+1) << "/" << (tri[0]+1) << "/" << (tri[0]+1) 
				<< " " << (tri[1]+1) << "/" << (tri[1]+1) << "/" << (tri[1]+1)
				<< " " << (tri[2]+1) << "/" << (tri[2]+1) << "/" << (tri[2]+1) << std::endl;
		} else {
			out << "f " << (tri[0]+1) << "//" << (tri[0]+1) 
				<< " " << (tri[1]+1) << "//" << (tri[1]+1)
				<< " " << (tri[2]+1) << "//" << (tri[2]+1) << std::endl;
		}
	}

	out.close();

	errString = string("no error");
	return true;
}