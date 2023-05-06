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
#ifndef __WMLEXT_EXT_PLANE3_H__
#define __WMLEXT_EXT_PLANE3_H__

#include <Wm4Plane3.h>

namespace Wml
{

template<class Real>
class  ExtPlane3 : public Plane3<Real>
{
public:
	ExtPlane3();
	ExtPlane3( const Wml::Vector3<Real> & vNormal, const Wml::Vector3<Real> & vOrigin );
	ExtPlane3( const Wml::Vector3<Real> & vPoint1, const Wml::Vector3<Real> & vPoint2, const Wml::Vector3<Real> & vPoint3 );

	Wml::Vector3<Real> & Origin() { return m_vOrigin; }
	const Wml::Vector3<Real> & Origin() const { return m_vOrigin; }

	Wml::Vector3<Real> ProjectPointToPlane( const Wml::Vector3<Real> & vPoint ) const;

	//! preserves distance from vPoint to plane origin - assumes plane normal is normalized!
	Wml::Vector3<Real> RotatePointIntoPlane( const Wml::Vector3<Real> & vPoint, Wml::Vector3<Real> * pNormal = NULL ) const;	

	Real IntersectRay( const Wml::Vector3<Real> & vOrigin,
					   const Wml::Vector3<Real> & vDirection ) const;

protected:
	Wml::Vector3<Real> m_vOrigin;
};

typedef ExtPlane3<float> ExtPlane3f;
typedef ExtPlane3<double> ExtPlane3d;


}	// namespace Wml



#endif	// __WMLEXT_EXT_PLANE3_H__