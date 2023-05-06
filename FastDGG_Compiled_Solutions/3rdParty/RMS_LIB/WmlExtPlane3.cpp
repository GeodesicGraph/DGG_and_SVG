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
#include <Wm4Matrix3.h>
#include "WmlExtPlane3.h"

#include <limits>

using namespace Wml;

template<class Real>
ExtPlane3<Real>::ExtPlane3() 
	: Plane3<Real>(), m_vOrigin(Wml::Vector3<Real>::ZERO) 
{
}

template<class Real>
ExtPlane3<Real>::ExtPlane3( const Wml::Vector3<Real> & vNormal, const Wml::Vector3<Real> & vOrigin ) 
	: Plane3<Real>( vNormal, vOrigin ), m_vOrigin(vOrigin) 
{
}

template<class Real>
ExtPlane3<Real>::ExtPlane3( const Wml::Vector3<Real> & vPoint1, 
						    const Wml::Vector3<Real> & vPoint2, 
							const Wml::Vector3<Real> & vPoint3 )
							: Plane3<Real>( vPoint1, vPoint2, vPoint3 )
{
	this->Normal.Normalize();
	m_vOrigin = vPoint1;
}


template<class Real>
Wml::Vector3<Real> ExtPlane3<Real>::ProjectPointToPlane( const Wml::Vector3<Real> & vPoint ) const 
{
	return vPoint - DistanceTo(vPoint) * this->Normal;
}

template<class Real>
Wml::Vector3<Real> ExtPlane3<Real>::RotatePointIntoPlane( const Wml::Vector3<Real> & vPoint, Wml::Vector3<Real> * pNormal ) const
{
	// find projected point in plane
	Wml::Vector3<Real> vProjected = ProjectPointToPlane( vPoint );
	Wml::Vector3<Real> vOrig( vPoint - Origin() );
	Wml::Vector3<Real> vProj( vProjected - Origin() );

#if 1
	Real fScale = vOrig.Length() / vProj.Length();
//	Real fScale = 1.0f;
	return Origin() + vProj * fScale;

#else
	// [RMS] following code uses rotation to do the same thing, just less efficiently
	//   (but conceivably a bit more robustly...)

	// find angle between projected point and original point
	Real CosTheta = vOrig.Dot(vProj) / (vOrig.Length() * vProj.Length());
	Real Theta = std::acos( CosTheta );

	if ( pNormal && Normal.Dot(*pNormal) < 0 ) {
		_RMSInfo("reversed!\n");
		Theta = Wml::Math<Real>::PI - Theta;
	}

	// compute rotation axis
	Wml::Vector3<Real> vAxis( vOrig.Cross(vProj) );
	vAxis.Normalize();

	// rotate point
	Wml::Matrix3<Real> matRotate( vAxis, Theta );
	return (matRotate * vOrig) + Origin();
#endif
}


template<class Real>
Real ExtPlane3<Real>::IntersectRay( const Wml::Vector3<Real> & vOrigin,
								    const Wml::Vector3<Real> & vDirection ) const
{
	Wml::Vector3<Real> vN(this->Normal);
	Real fDenom = vDirection.Dot(vN);
	if ( fDenom == 0 )
		return std::numeric_limits<Real>::max();
	return (this->Constant - vOrigin.Dot(vN)) / fDenom; 
}



//----------------------------------------------------------------------------
// explicit instantiation
//----------------------------------------------------------------------------
namespace Wml
{
template class ExtPlane3<float>;
template class ExtPlane3<double>;
}
//----------------------------------------------------------------------------
