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
#ifndef __RMS_VECTOR_UTIL_H__
#define __RMS_VECTOR_UTIL_H__

#include <Wm4Vector2.h>
#include <Wm4Vector3.h>
#include <Wm4Matrix3.h>
#include <Wm4AxisAlignedBox2.h>
#include <Wm4AxisAlignedBox3.h>

namespace rms
{

	/*
	 * if bToZAxis is false, compute matrix that rotates Z axis into vAlignWith
	 * if bToZAxis is true, compute matrix that rotates vAlignWith into Z axis
	 */
	template <class Real>
		void ComputeAlignZAxisMatrix( const Wml::Vector3<Real> & vAlignWith,
											  Wml::Matrix3<Real> & matrix, bool bToZAxis = false );

	template <class Real>
	void ComputeAlignAxisMatrix( const Wml::Vector3<Real> & vInitial,
											 const Wml::Vector3<Real> & vAlignWith, Wml::Matrix3<Real> & matrix );

	//! compute vectors in a plane perpendicular to vIn
	template <class Real>
	void ComputePerpVectors( const Wml::Vector3<Real> & vIn,
										 Wml::Vector3<Real> & vOut1, Wml::Vector3<Real> & vOut2,
										 bool bInIsNormalized = false);

	template <class Real>
	void ToGLMatrix( const Wml::Matrix3<Real> & matrix, Real glMatrix[16] );

	void Union( Wml::AxisAlignedBox3f & dest, const Wml::AxisAlignedBox3f & with );
	void Union( Wml::AxisAlignedBox3f & dest, const Wml::Vector3f & point );

	template <class Real>
	void Union( Wml::AxisAlignedBox2<Real> & dest, const Wml::AxisAlignedBox2<Real> & with );
	template <class Real>
	void Union( Wml::AxisAlignedBox2<Real> & dest, const Wml::Vector2<Real> & with );

	float Volume( const Wml::AxisAlignedBox3f & box );

	//! create box that contains sphere
	template <class Real>
	void FitAABox( const Wml::Vector3<Real> & vCenter, Real fRadius, Wml::AxisAlignedBox3<Real> & aaBox );

	bool Contained( const Wml::AxisAlignedBox3f & box, float fX, float fY, float fZ );
	bool Contained( const Wml::AxisAlignedBox2f & box, float fX, float fY );

	template <class Real>
	Wml::Vector3<Real> Center( const Wml::AxisAlignedBox3<Real> & box );
	template <class Real>
	Wml::Vector2<Real> Center( const Wml::AxisAlignedBox2<Real> & box );

	template <class Real>
	Real GetDimension( const Wml::AxisAlignedBox3<Real> & box, int nDimension );
	template <class Real>
	Real GetDimension( const Wml::AxisAlignedBox2<Real> & box, int nDimension );

	template <class Real>
	Real MaxDimension( const Wml::AxisAlignedBox3<Real> & box );

	template<class Real>
	void Translate( Wml::AxisAlignedBox2<Real> & box, Real fX, Real fY, bool bRelative );

	// casting operators for vector
	Wml::Vector3f VectorCastdf( const Wml::Vector3d & vec );
	Wml::Vector3d VectorCastfd( const Wml::Vector3f & vec );

	template<class Real>
	Real Clamp( const Real & fValue, const Real & fMin, const Real & fMax );


	template <class Real>
	void BarycentricCoords( const Wml::Vector3<Real> & vTriVtx1, 
										const Wml::Vector3<Real> & vTriVtx2,
										const Wml::Vector3<Real> & vTriVtx3,
										const Wml::Vector3<Real> & vVertex,
										Real & fBary1, Real & fBary2, Real & fBary3 );

	template <class Real>
	Real Area( const Wml::Vector3<Real> & vTriVtx1, 
						   const Wml::Vector3<Real> & vTriVtx2,
						   const Wml::Vector3<Real> & vTriVtx3 );

	template <class Real>
	void BarycentricCoords( const Wml::Vector2<Real> & vTriVtx1, 
										const Wml::Vector2<Real> & vTriVtx2,
										const Wml::Vector2<Real> & vTriVtx3,
										const Wml::Vector2<Real> & vVertex,
										Real & fBary1, Real & fBary2, Real & fBary3 );

	template <class Real>
	Real Area( const Wml::Vector2<Real> & vTriVtx1, 
						   const Wml::Vector2<Real> & vTriVtx2,
						   const Wml::Vector2<Real> & vTriVtx3 );


	template <class Real>
	Wml::Vector3<Real> Normal( const Wml::Vector3<Real> & vTriVtx1, 
								const Wml::Vector3<Real> & vTriVtx2,
								const Wml::Vector3<Real> & vTriVtx3, Real * pArea = NULL );


	//! This metric is from Texture Mapping Progressive Meshes, Sander et al, Siggraph 2001
	template <class Real>
	void StretchMetric1( const Wml::Vector3<Real> & vTriVtx1, 
									 const Wml::Vector3<Real> & vTriVtx2,
									 const Wml::Vector3<Real> & vTriVtx3,
									 const Wml::Vector2<Real> & vVtxParam1,
									 const Wml::Vector2<Real> & vVtxParam2,
									 const Wml::Vector2<Real> & vVtxParam3,
									 Real & MaxSV, Real & MinSV, Real & L2Norm, Real & LInfNorm );



}  // namespace Wml



#endif // __RMS_VECTOR_UTIL_H__