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
#include "WmlPolygon2.h"
#include "Wm4Segment2.h"
#include "VectorUtil.h"

#include <limits>
#include <fstream>

#include "rmsdebug.h"

using namespace rms;
using namespace Wml;


template <class Real>
Polygon2<Real>::Polygon2()
{
}

template <class Real>
Polygon2<Real>::Polygon2( const Polygon2<Real> & copy )
{
	m_vVertices = copy.m_vVertices;
}

template <class Real>
Polygon2<Real>::~Polygon2()
{
}

template <class Real>
bool Polygon2<Real>::IsClockwise() const
{
	// check clockwise-ness by determining poly area. Positive area == CCW
	Real fArea = 0;
	size_t nCount = m_vVertices.size();
	for (unsigned int i = 0; i < nCount-1; ++i) {
		const Wml::Vector2<Real> & v1 = m_vVertices[i];
		const Wml::Vector2<Real> & v2 = m_vVertices[i+1];

		fArea += v1.X() * v2.Y() - v1.Y() * v2.X();
	}
	return fArea < 0;
}


template <class Real>
void Polygon2<Real>::FlipOrientation()
{
	size_t nCount = m_vVertices.size();
	for (unsigned int i = 0; i < nCount/2; ++i) {
		Wml::Vector2<Real> tmp = m_vVertices[i];
		m_vVertices[i] = m_vVertices[ (nCount-1)-i ];
		m_vVertices[ (nCount-1)-i ] = tmp;
	}	
}


// inside/outside test

// code adapted from http://softsurfer.com/Archive/algorithm_0103/algorithm_0103.htm


//    Return: >0 for P2 left of the line through P0 and P1
//            =0 for P2 on the line
//            <0 for P2 right of the line
template<class Real>
static inline Real
isLeft( const Wml::Vector2<Real> & P0, const Wml::Vector2<Real> & P1, const Wml::Vector2<Real> & P2 )
{
    return ( (P1.X() - P0.X()) * (P2.Y() - P0.Y())
                  - (P2.X() - P0.X()) * (P1.Y() - P0.Y()) );
}


template<class Real>
bool Polygon2<Real>::IsInside( const Wml::Vector2<Real> & vTest ) const
{
	int nWindingNumber = 0;   // winding number counter

	size_t nVtxCount = m_vVertices.size();
	for (unsigned int i = 0; i < nVtxCount; ++i) {

		unsigned int iNext = (i+1)%nVtxCount;

		if (m_vVertices[i].Y() <= vTest.Y()) {         
			// start y <= P.y
			if (m_vVertices[iNext].Y() > vTest.Y())                          // an upward crossing
				if (isLeft( m_vVertices[i], m_vVertices[iNext], vTest) > 0)  // P left of edge
					++nWindingNumber;                                      // have a valid up intersect
		} else {                       
			// start y > P.y (no test needed)
			if (m_vVertices[iNext].Y() <= vTest.Y())                         // a downward crossing
				if (isLeft( m_vVertices[i], m_vVertices[iNext], vTest) < 0)  // P right of edge
					--nWindingNumber;                                      // have a valid down intersect
		}
	}

	return nWindingNumber != 0;
}

// bounding things
template <class Real>
void Polygon2<Real>::BoundingBox( Wml::AxisAlignedBox2<Real> & box ) const
{
	if( IsEmpty() ) {
		box = Wml::AxisAlignedBox2<Real>();
		return;
	}

	box = Wml::AxisAlignedBox2<Real>( m_vVertices[0].X(), m_vVertices[0].X(), m_vVertices[0].Y(), m_vVertices[0].Y() );
	size_t nVertices = m_vVertices.size();
	for (unsigned int i = 1; i < nVertices; ++i) {
		const Wml::Vector2<Real> & vtx = m_vVertices[i];
		if ( vtx.X() < box.XMin() ) 
			box.XMin() = vtx.X();
		else if (vtx.X() > box.XMax() )
			box.XMax() = vtx.X();
		if ( vtx.Y() < box.YMin() ) 
			box.YMin() = vtx.Y();
		else if (vtx.Y() > box.YMax() )
			box.YMax() = vtx.Y();
	}
}

// translation / rotation functions
template <class Real>
void Polygon2<Real>::Transform( const Wml::Matrix2<Real> & transform )
{
	size_t nCount = m_vVertices.size();
	for (unsigned int i = 0; i < nCount; ++i)
		m_vVertices[i] = transform * m_vVertices[i];
}

template <class Real>
void Polygon2<Real>::Translate( Real fX, Real fY )
{
	size_t nCount = m_vVertices.size();
	for (unsigned int i = 0; i < nCount; ++i) {
		m_vVertices[i].X() += fX;
		m_vVertices[i].Y() += fY;
	}
}

template <class Real>
void Polygon2<Real>::Scale( Real fScaleX, Real fScaleY, const Wml::Vector2<Real> & vOrigin )
{
	size_t nCount = m_vVertices.size();
	for (unsigned int i = 0; i < nCount; ++i) {
		m_vVertices[i].X() = (m_vVertices[i].X() - vOrigin.X()) * fScaleX + vOrigin.X();
		m_vVertices[i].Y() = (m_vVertices[i].Y() - vOrigin.Y()) * fScaleY + vOrigin.Y();
	}
}




// Polygon simplification
// code adapted from: http://softsurfer.com/Archive/algorithm_0205/algorithm_0205.htm

// dist_Point_to_Segment(): get the distance of a point to a segment.
//    Input:  a Point P and a Segment S (in any dimension)
//    Return: the shortest distance from P to S
template<class Real>
static Real
dist_Point_to_Segment( Wml::Vector2<Real> & P, Wml::Segment2<Real> & S)
{
	Wml::Vector2<Real> & v = S.Direction();
	Wml::Vector2<Real> & w = P - S.Origin();

    Real c1 = w.Dot(v);
    if ( c1 <= 0 )
		return w.Length();

    Real c2 = v.Dot(v);
    if ( c2 <= c1 )
		return (P - (S.Origin() + S.Direction())).Length();

    Real b = c1 / c2;
	Wml::Vector2<Real> Pb = S.Origin() + v * b;
	return (P - Pb).Length();
}


template <class Real>
void Polygon2<Real>::SmoothResample( float fFactor )
{
	Real fMaxEdgeLen = (Real)0.0;
	size_t nVerts = m_vVertices.size();
	for ( unsigned int i = 0; i  < nVerts; ++i ) {
		Real fLen = ( m_vVertices[(i+1) % nVerts] - m_vVertices[i] ).Length();
		if ( fLen > fMaxEdgeLen )
			fMaxEdgeLen = fLen;
	}
	Real fSnapDist = fMaxEdgeLen * (Real)0.5;

	std::vector<Wml::Vector2<Real> > vNewPts;
	bool bDone = false;
	while ( ! bDone ) {
		bDone = true;
		vNewPts.resize(0);

		vNewPts.push_back( m_vVertices[0] );

		Real fDistSum = 0.0f;
		for ( unsigned int i = 1; i < nVerts; ++i ) {
			fDistSum += (m_vVertices[i] - m_vVertices[i-1]).Length();
			if ( fDistSum > fSnapDist ) {
				vNewPts.push_back( m_vVertices[i] );
				fDistSum = 0.0f;
			} else
				bDone = false;
		}
		m_vVertices = vNewPts;
		nVerts = m_vVertices.size();
	}

	// ok now subdivide once
	nVerts = m_vVertices.size();
	vNewPts.resize(0);
	for ( unsigned int i = 1; i < nVerts+1; ++i ) {
		Wml::Vector2<Real> & vPrev = m_vVertices[i-1];
		Wml::Vector2<Real> & vCur = m_vVertices[ i % nVerts];
		Wml::Vector2<Real> & vNext = m_vVertices[(i+1) % nVerts];
		vNewPts.push_back(	(Real)0.25*vPrev + (Real)0.75*vCur );
		vNewPts.push_back(	(Real)0.75*vCur + (Real)0.25*vNext );
	}
	m_vVertices = vNewPts;
	
}






//----------------------------------------------------------------------------
// explicit instantiation
//----------------------------------------------------------------------------
namespace rms
{
template class Polygon2<float>;
template class Polygon2<double>;
}
//----------------------------------------------------------------------------
