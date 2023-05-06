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
#ifndef __RMS_WMLEXT_POLYGON_H__
#define __RMS_WMLEXT_POLYGON_H__

#include <Wm4Vector2.h>
#include <Wm4Matrix2.h>
#include <Wm4AxisAlignedBox2.h>

#include <vector>

namespace rms {

template<class Real>
class Polygon2
{
public:

	Polygon2();
	Polygon2( const Polygon2<Real> & copy );
	~Polygon2();

	// add/remove vertex functions
	inline unsigned int AppendVertex( const Wml::Vector2<Real> & vVertex );
	inline void AppendVertices( const std::vector<Wml::Vector2<Real> > & vVerts );
	inline void ClearVertices();

	// vertex count stuff
	inline unsigned int VertexCount() const;
	inline bool IsEmpty() const;

	// element access
	inline const Wml::Vector2<Real> & Vertex( unsigned int nVertex ) const;
	inline Wml::Vector2<Real> & Vertex( unsigned int nVertex );
	inline Wml::Vector2<Real> & operator[]( int i );
	inline const Wml::Vector2<Real> & operator[] (int i ) const;
	inline const std::vector< Wml::Vector2<Real> > & Vertices() const;

	// orientation bits
	bool IsClockwise() const;
	bool IsCCW() const { return ! IsClockwise(); }
	void FlipOrientation();

	// inside/outside test
	bool IsInside( const Wml::Vector2<Real> & vTest ) const;

	// bounding things
	void BoundingBox( Wml::AxisAlignedBox2<Real> & dest ) const;

	// translation / rotation functions
	void Transform( const Wml::Matrix2<Real> & transform );
	void Translate( Real fX, Real fY );
	void Scale( Real fScaleX, Real fScaleY, const Wml::Vector2<Real> & vOrigin );

	// smooth out polygon and then subdivide 
	void SmoothResample( float fFactor );

protected:
	std::vector< Wml::Vector2<Real> > m_vVertices;
};

typedef Polygon2<float> Polygon2f;
typedef Polygon2<double> Polygon2d;


/*
 * inlines
 */

template<class Real>
inline unsigned int Polygon2<Real>::AppendVertex( const Wml::Vector2<Real> & vVertex )
{
	size_t nVerts = m_vVertices.size();
	m_vVertices.push_back(vVertex);
	return (unsigned int)nVerts;
}

template<class Real>
inline void Polygon2<Real>::AppendVertices( const std::vector<Wml::Vector2<Real> > & vVerts )
{
	m_vVertices = vVerts;
}


template<class Real>
inline void Polygon2<Real>::ClearVertices( )
{
	m_vVertices.resize(0);
}

template<class Real>
inline unsigned int Polygon2<Real>::VertexCount() const
{
	return (unsigned int)m_vVertices.size();
}

template<class Real>
inline bool Polygon2<Real>::IsEmpty() const
{
	return m_vVertices.empty();
}

template<class Real>
inline const Wml::Vector2<Real> & Polygon2<Real>::Vertex( unsigned int nVertex ) const
{
	return m_vVertices[nVertex];
}

template<class Real>
inline Wml::Vector2<Real> & Polygon2<Real>::Vertex( unsigned int nVertex )
{
	return m_vVertices[nVertex];
}

template<class Real>
inline const Wml::Vector2<Real> & Polygon2<Real>::operator[]( int i ) const
{
	return m_vVertices[i];
}

template<class Real>
inline Wml::Vector2<Real> & Polygon2<Real>::operator[]( int i )
{
	return m_vVertices[i];
}

template<class Real>
inline const std::vector< Wml::Vector2<Real> > & Polygon2<Real>::Vertices() const
{
	return m_vVertices;
}




} // namespace rmssketch



#endif // __RMS_POLYGON_H__