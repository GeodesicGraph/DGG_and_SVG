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
#include "IMesh.h"

#include <limits>

using namespace rms;

unsigned int IMesh::InvalidID = std::numeric_limits<unsigned int>::max();

//! determine if a vertex is on the mesh boundary (if there is one)
bool IMesh::IsBoundaryVertex( VertexID vID )
{
	// RMS TODO: it should be possible to do this just using
	//  a 3-vertex buffer (ie determine direction by looking
	//  at last and second-last vertices...)
	//  (Maybe even 2-vertex buffer?)

	VtxNbrItr v(vID);
	BeginVtxTriangles(v);
	int nCount = 0;
	while ( GetNextVtxTriangle(v) != InvalidID )
		++nCount;
	if ( nCount == 0 )
		return true;

	std::vector< TriangleID > vTris;
	vTris.resize( nCount-1 );
	BeginVtxTriangles(v);
	TriangleID tID;
	unsigned int i = 0;
	while ( (tID = GetNextVtxTriangle(v)) != InvalidID )
		vTris[i++] = tID;

	// pick first edge
	VertexID vCurID = InvalidID;
	VertexID vStopID = InvalidID;
	VertexID vTri[3];
	GetTriangle( vTris[0], vTri );
	for ( int i = 0; i < 3; ++i ) {
		if ( vTri[i] == vID ) {
			vCurID = vTri[ (i+1) % 3 ];
			vStopID = vTri[ (i+2) % 3];
			break;
		} else if ( vTri[ (i+1) % 3 ] == vID ) {
			vCurID = vTri[i];
			vStopID = vTri[ (i+2) % 3 ];
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
			GetTriangle( vTris[i], vTri );
			if ( vTri[0] == vCurID || vTri[1] == vCurID || vTri[2] == vCurID ) {
				nCurTri = i;
				break;
			}
		}
		if ( nCurTri == InvalidID )
			return true;			// 1-ring is not connected - must be invalid!

		// mark tri as done
		vTris[ nCurTri ] = InvalidID;

		// go to next tri in one-ring
		GetTriangle( vTris[nCurTri], vTri );
		if ( vTri[0] == vID ) 
			vCurID = ( vTri[1] == vCurID ) ? vTri[2] : vTri[1];
		else if ( vTri[1] == vID )
			vCurID = ( vTri[0] == vCurID ) ? vTri[2] : vTri[0];
		else
			vCurID = ( vTri[0] == vCurID ) ? vTri[1] : vTri[0];
	}

	return false;
}


void IMesh::NeighbourIteration( VertexID vID, IMesh::NeighborTriCallback * pCallback )
{
	IMesh::VtxNbrItr itr(vID);
	BeginVtxTriangles(itr);
	TriangleID tID = GetNextVtxTriangle(itr);

	pCallback->BeginTriangles();

	while ( tID != InvalidID ) {
		pCallback->NextTriangle(tID);
		tID = GetNextVtxTriangle(itr);
	}

	pCallback->EndTriangles();
}





