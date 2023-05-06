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
#include "VectorUtil.h"

using namespace rms;
using namespace Wml;

#include <algorithm>
#include <limits>


template <class Real>
void rms::ComputeAlignZAxisMatrix( const Vector3<Real> & vAlignWith,
								  Matrix3<Real> & matrix, bool bInvert )
{
	// compute cosine of angle between vectors
	Real axisDot = vAlignWith.Dot( Vector3<Real>::UNIT_Z );

	// compute rotation axis
	Vector3<Real> axisCross( Vector3<Real>::UNIT_Z.Cross( vAlignWith ) );

	Real fInverter = (bInvert) ? (Real)-1 : (Real)1;

	// apply rotation if necessary
	if (axisCross.SquaredLength() > Wml::Math<Real>::EPSILON) {

		// compute normalized axis and angle, then create rotation around axis
		axisCross.Normalize();
		Real fAngle = Math<Real>::ACos( axisDot / vAlignWith.Length() );
		matrix.FromAxisAngle( axisCross, fAngle * fInverter );

	} else if (axisDot < (Real)0) {
		matrix.FromAxisAngle( Vector3<Real>::UNIT_X, (Real)180 * Math<Real>::DEG_TO_RAD * fInverter );
	} else {
		matrix = Matrix3<Real>::IDENTITY;
	}
}


template <class Real>
void rms::ComputeAlignAxisMatrix( const Vector3<Real> & vInitial,
								  const Vector3<Real> & vAlignWith, Matrix3<Real> & matrix )
{
	// compute cosine of angle between vectors
	Real axisDot = vAlignWith.Dot( vInitial );

	// compute rotation axis
	Vector3<Real> axisCross( vInitial.Cross( vAlignWith ) );

	// apply rotation if necessary
	if (axisCross.SquaredLength() > Wml::Math<Real>::EPSILON) {

		// compute normalized axis and angle, then create rotation around axis
		axisCross.Normalize();
		Real fAngle = Math<Real>::ACos( axisDot / vAlignWith.Length() );
		matrix.FromAxisAngle( axisCross, fAngle );

	} else if (axisDot < (Real)0) {

		// find some perpendicular vectors
		Wml::Vector3<Real> vPerp1, vPerp2;
		ComputePerpVectors( vInitial, vPerp1, vPerp2 );

		matrix.FromAxisAngle( vPerp1, (Real)180 * Math<Real>::DEG_TO_RAD );
	} else {
		matrix = Matrix3<Real>::IDENTITY;
	}
}


template <class Real>
void rms::ComputePerpVectors( const Vector3<Real> & vIn,
							  Vector3<Real> & vOut1, Vector3<Real> & vOut2, bool bInIsNormalized )
{
	Wml::Vector3<Real> vPerp(vIn);
	if ( ! bInIsNormalized )
		vPerp.Normalize();

	if ( Wml::Math<Real>::FAbs(vPerp.X()) >= Wml::Math<Real>::FAbs(vPerp.Y())
		 &&   Wml::Math<Real>::FAbs(vPerp.X()) >= Wml::Math<Real>::FAbs(vPerp.Z()) )
    {
        vOut1.X() = -vPerp.Y();
        vOut1.Y() = vPerp.X();
        vOut1.Z() = (Real)0.0;
    }
    else
    {
        vOut1.X() = (Real)0.0;
        vOut1.Y() = vPerp.Z();
        vOut1.Z() = -vPerp.Y();
    }

    vOut1.Normalize();
    vOut2 = vPerp.Cross(vOut1);	
}


template<class Real>
Real rms::Clamp( const Real & fValue, const Real & fMin, const Real & fMax )
{
	if ( fValue < fMin )
		return fMin;
	else if ( fValue > fMax )
		return fMax;
	else
		return fValue;
}


Wml::Vector3d rms::VectorCastfd( const Wml::Vector3f & vec )
{
	return Wml::Vector3d( (double)vec.X(), (double)vec.Y(), (double)vec.Z() );
}

Wml::Vector3f rms::VectorCastdf( const Wml::Vector3d & vec )
{
	return Wml::Vector3f( (float)vec.X(), (float)vec.Y(), (float)vec.Z() );
}



template <class Real>
void rms::ToGLMatrix( const Matrix3<Real> & matrix, Real glMatrix[16] )
{
	for (int r = 0; r < 3; ++r)
		for (int c = 0; c < 4; ++c)
			glMatrix[c*4 + r] = (c < 3) ? matrix(r,c) : 0;
	glMatrix[3] = glMatrix[7] = glMatrix[11] = 0;
	glMatrix[15] = 1;
}




template <class Real>
Wml::Vector3<Real> rms::Center( const Wml::AxisAlignedBox3<Real> & box )
{
	Wml::Vector3<Real> v;
	v.X() = (Real)0.5 * ( box.GetXMax() + box.GetXMin() );
	v.Y() = (Real)0.5 * ( box.GetYMax() + box.GetYMin() );
	v.Z() = (Real)0.5 * ( box.GetZMax() + box.GetZMin() );
	return v;
}

template <class Real>
Wml::Vector2<Real> rms::Center( const Wml::AxisAlignedBox2<Real> & box )
{
	Wml::Vector2<Real> v;
	v.X() = (Real)0.5 * ( box.GetXMax() + box.GetXMin() );
	v.Y() = (Real)0.5 * ( box.GetYMax() + box.GetYMin() );
	return v;
}


template <class Real>
Real rms::GetDimension( const Wml::AxisAlignedBox3<Real> & box, int nDimension )
{
	switch(nDimension) {
		case 0:
			return box.GetXMax() - box.GetXMin();
		case 1:
			return box.GetYMax() - box.GetYMin();
		case 2:
			return box.GetZMax() - box.GetZMin();
		default:
			return (Real)0;
	};
}


template <class Real>
Real rms::GetDimension( const Wml::AxisAlignedBox2<Real> & box, int nDimension )
{
	switch(nDimension) {
		case 0:
			return box.GetXMax() - box.GetXMin();
		case 1:
			return box.GetYMax() - box.GetYMin();
		default:
			return (Real)0;
	};
}


template <class Real>
Real rms::MaxDimension( const Wml::AxisAlignedBox3<Real> & box )
{
	Real width = box.GetXMax() - box.GetXMin();
	Real height = box.GetYMax() - box.GetYMin();
	Real depth = box.GetZMax() - box.GetZMin();
	return std::max<Real>( width, std::max<Real>(height,depth) );
}


void rms::Union( Wml::AxisAlignedBox3f & dest, const Wml::AxisAlignedBox3f & with )
{
	if ( with.GetXMin() < dest.XMin() )
		dest.XMin() = with.GetXMin();
	if ( with.GetXMax() > dest.XMax() )
		dest.XMax() = with.GetXMax();
	if ( with.GetYMin() < dest.YMin() )
		dest.YMin() = with.GetYMin();
	if ( with.GetYMax() > dest.YMax() )
		dest.YMax() = with.GetYMax();
	if ( with.GetZMin() < dest.ZMin() )
		dest.ZMin() = with.GetZMin();
	if ( with.GetZMax() > dest.ZMax() )
		dest.ZMax() = with.GetZMax();
}

void rms::Union( Wml::AxisAlignedBox3f & dest, const Wml::Vector3f & point )
{
	if ( point.X() < dest.XMin() )
		dest.XMin() = point.X();
	if ( point.X() > dest.XMax() )
		dest.XMax() = point.X();
	if ( point.Y() < dest.YMin() )
		dest.YMin() = point.Y();
	if ( point.Y() > dest.YMax() )
		dest.YMax() = point.Y();
	if ( point.Z() < dest.ZMin() )
		dest.ZMin() = point.Z();
	if ( point.Z() > dest.ZMax() )
		dest.ZMax() = point.Z();
}

template <class Real>
void rms::Union( Wml::AxisAlignedBox2<Real> & dest, const Wml::AxisAlignedBox2<Real> & with )
{
	if ( with.GetXMin() < dest.XMin() )
		dest.XMin() = with.GetXMin();
	if ( with.GetXMax() > dest.XMax() )
		dest.XMax() = with.GetXMax();
	if ( with.GetYMin() < dest.YMin() )
		dest.YMin() = with.GetYMin();
	if ( with.GetYMax() > dest.YMax() )
		dest.YMax() = with.GetYMax();
}

template <class Real>
void rms::Union( Wml::AxisAlignedBox2<Real> & dest, const Wml::Vector2<Real> & with )
{
	if ( with.X() < dest.XMin() )
		dest.XMin() = with.X();
	if ( with.X() > dest.XMax() )
		dest.XMax() = with.X();
	if ( with.Y() < dest.YMin() )
		dest.YMin() = with.Y();
	if ( with.Y() > dest.YMax() )
		dest.YMax() = with.Y();
}


float rms::Volume( const Wml::AxisAlignedBox3f & box )
{
	return ( box.GetXMax() - box.GetXMin() ) * (box.GetYMax() - box.GetYMin()) * (box.GetZMax() - box.GetZMin());
}

template <class Real>
void rms::FitAABox( const Wml::Vector3<Real> & vCenter, Real fRadius, Wml::AxisAlignedBox3<Real> & aaBox )
{
	aaBox.XMin() = vCenter.X() - fRadius;
	aaBox.XMax() = vCenter.X() + fRadius;
	aaBox.YMin() = vCenter.Y() - fRadius;
	aaBox.YMax() = vCenter.Y() + fRadius;
	aaBox.ZMin() = vCenter.Z() - fRadius;
	aaBox.ZMax() = vCenter.Z() + fRadius;
}




bool rms::Contained( const Wml::AxisAlignedBox3f & box, float fX, float fY, float fZ )
{
	return (fX >= box.GetXMin() && fX <= box.GetXMax() 
		&& fY >= box.GetYMin() && fY <= box.GetYMax()
		&& fZ >= box.GetZMin() && fZ <= box.GetZMax() );
}

bool rms::Contained( const Wml::AxisAlignedBox2f & box, float fX, float fY )
{
	return (fX >= box.GetXMin() && fX <= box.GetXMax() 
		&& fY >= box.GetYMin() && fY <= box.GetYMax() );
}


template<class Real>
void rms::Translate( Wml::AxisAlignedBox2<Real> & box, Real fX, Real fY, bool bRelative )
{
	if ( bRelative ) {
		box.XMin() += fX;
		box.XMax() += fX;
		box.YMin() += fY;
		box.YMax() += fY;
	} else {
		Real fWidth = rms::GetDimension(box, 0);
		Real fHeight = rms::GetDimension(box, 1);
		box.XMin() = fX;
		box.XMax() = fX + fWidth;
		box.YMin() = fY;
		box.YMax() = fY + fHeight;
	}
}


template <class Real>
void rms::BarycentricCoords( const Vector3<Real> & vTriVtx1, 
							 const Vector3<Real> & vTriVtx2,
							 const Vector3<Real> & vTriVtx3,
							 const Vector3<Real> & vVertex,
							 Real & fBary1, Real & fBary2, Real & fBary3 )
{

	Wml::Vector3<Real> kV02 = vTriVtx1 - vTriVtx3;
    Wml::Vector3<Real> kV12 = vTriVtx2 - vTriVtx3;
    Wml::Vector3<Real> kPV2 = vVertex - vTriVtx3;

    Real fM00 = kV02.Dot(kV02);
    Real fM01 = kV02.Dot(kV12);
    Real fM11 = kV12.Dot(kV12);
    Real fR0 = kV02.Dot(kPV2);
    Real fR1 = kV12.Dot(kPV2);
    Real fDet = fM00*fM11 - fM01*fM01;
//    assert( Wml::Math<Real>::FAbs(fDet) > (Real)0.0 );
    Real fInvDet = ((Real)1.0)/fDet;

    fBary1 = (fM11*fR0 - fM01*fR1)*fInvDet;
    fBary2 = (fM00*fR1 - fM01*fR0)*fInvDet;
    fBary3 = (Real)1.0 - fBary1 - fBary2;
}

template <class Real>
Real rms::Area( const Vector3<Real> & vTriVtx1, 
				 const Vector3<Real> & vTriVtx2,
				 const Vector3<Real> & vTriVtx3 )
{
	Wml::Vector3<Real> edge1( vTriVtx2 - vTriVtx1 );
	Wml::Vector3<Real> edge2( vTriVtx3 - vTriVtx1 );
	Wml::Vector3<Real> vCross( edge1.Cross(edge2) );

	return (Real)0.5 * vCross.Length();	
}


template <class Real>
void rms::BarycentricCoords( const Vector2<Real> & vTriVtx1, 
							 const Vector2<Real> & vTriVtx2,
							 const Vector2<Real> & vTriVtx3,
							 const Vector2<Real> & vVertex,
							 Real & fBary1, Real & fBary2, Real & fBary3 )
{

	Wml::Vector2<Real> kV02 = vTriVtx1 - vTriVtx3;
    Wml::Vector2<Real> kV12 = vTriVtx2 - vTriVtx3;
    Wml::Vector2<Real> kPV2 = vVertex - vTriVtx3;

    Real fM00 = kV02.Dot(kV02);
    Real fM01 = kV02.Dot(kV12);
    Real fM11 = kV12.Dot(kV12);
    Real fR0 = kV02.Dot(kPV2);
    Real fR1 = kV12.Dot(kPV2);
    Real fDet = fM00*fM11 - fM01*fM01;
//    assert( Wml::Math<Real>::FAbs(fDet) > (Real)0.0 );
    Real fInvDet = ((Real)1.0)/fDet;

    fBary1 = (fM11*fR0 - fM01*fR1)*fInvDet;
    fBary2 = (fM00*fR1 - fM01*fR0)*fInvDet;
    fBary3 = (Real)1.0 - fBary1 - fBary2;
}


template <class Real>
Real rms::Area( const Vector2<Real> & vTriVtx1, 
				 const Vector2<Real> & vTriVtx2,
				 const Vector2<Real> & vTriVtx3 )
{
	Wml::Vector2<Real> edge1( vTriVtx2 - vTriVtx1 );
	Wml::Vector2<Real> edge2( vTriVtx3 - vTriVtx1 );
	Real fCross( edge1.DotPerp(edge2) );

	return (Real)0.5 * (Real)fabs(fCross);	
}





template <class Real>
Wml::Vector3<Real> rms::Normal( const Vector3<Real> & vTriVtx1, 
								const Vector3<Real> & vTriVtx2,
								const Vector3<Real> & vTriVtx3, Real * pArea )
{
	Wml::Vector3<Real> edge1( vTriVtx2 - vTriVtx1 );
	Wml::Vector3<Real> edge2( vTriVtx3 - vTriVtx1 );
	Wml::Vector3<Real> vCross( edge1.Cross(edge2) );
	Real fLength = vCross.Normalize();
	if ( pArea )
		*pArea = (Real)0.5 * fLength;	
	return vCross;
}




template <class Real>
void rms::StretchMetric1( const Vector3<Real> & q1, 
						 const Vector3<Real> & q2,
						 const Vector3<Real> & q3,
						 const Vector2<Real> & p1,
						 const Vector2<Real> & p2,
						 const Vector2<Real> & p3,
						 Real & MaxSV, Real & MinSV, Real & L2Norm, Real & LInfNorm )
{
	Real s1 = p1.X();
	Real t1 = p1.Y();
	Real s2 = p2.X();
	Real t2 = p2.Y();
	Real s3 = p3.X();
	Real t3 = p3.Y();

	Real A = (Real)0.5 * ( (s2 - s1) * (t3 - t1) - (s3 - s1) * (t2 - t1));
	if ( A > 0 ) {

		Wml::Vector3<Real> Ss = 
			(q1 * (t2-t3) + q2 * (t3-t1) + q3 * (t1-t2)) / (2*A);
		Wml::Vector3<Real> St = 
			(q1 * (s3-s2) + q2 * (s1-s3) + q3 * (s2-s1)) / (2*A);

		Real a = Ss.Dot(Ss);
		Real b = Ss.Dot(St);
		Real c = St.Dot(St);

		Real discrim = (Real)sqrt( (a-c)*(a-c) + 4*b*b );

		MaxSV = (Real)sqrt( (Real)0.5 * ( (a+c) + discrim ) );
		MinSV = (Real)sqrt( (Real)0.5 * ( (a+c) - discrim ) );

		L2Norm = (Real)sqrt( (Real)0.5 * (a+c)  );
		LInfNorm = MaxSV;
	} else {
		MaxSV = MinSV = L2Norm = LInfNorm = std::numeric_limits<Real>::max();
	}

}



namespace rms
{
template void ToGLMatrix( const Matrix3<float> & matrix, float glMatrix[16] );
template void ToGLMatrix( const Matrix3<double> & matrix, double glMatrix[16] );

template void ComputeAlignZAxisMatrix( const Vector3<float> & vAlignWith, Matrix3<float> & matrix, bool bInvert );
template void ComputeAlignZAxisMatrix( const Vector3<double> & vAlignWith, Matrix3<double> & matrix, bool bInvert );

template void ComputeAlignAxisMatrix( const Vector3<float> & vInitial, const Vector3<float> & vAlignWith, Matrix3<float> & matrix );
template void ComputeAlignAxisMatrix( const Vector3<double> & vInitial, const Vector3<double> & vAlignWith, Matrix3<double> & matrix );

template void ComputePerpVectors( const Vector3<float> & vIn, Vector3<float> & vOut1, Vector3<float> & vOut2, bool bInIsNormalized );
template void ComputePerpVectors( const Vector3<double> & vIn, Vector3<double> & vOut1, Vector3<double> & vOut2, bool bInIsNormalized );

template Wml::Vector3<float> rms::Center( const Wml::AxisAlignedBox3<float> & box );
template Wml::Vector3<double> rms::Center( const Wml::AxisAlignedBox3<double> & box );

template Wml::Vector2<float> rms::Center( const Wml::AxisAlignedBox2<float> & box );
template Wml::Vector2<double> rms::Center( const Wml::AxisAlignedBox2<double> & box );

template void rms::Union(  Wml::AxisAlignedBox2<float> & dest, const Wml::AxisAlignedBox2<float> & with );
template void rms::Union(  Wml::AxisAlignedBox2<double> & dest, const Wml::AxisAlignedBox2<double> & with );
template void rms::Union(  Wml::AxisAlignedBox2<float> & dest, const Wml::Vector2<float> & with );
template void rms::Union(  Wml::AxisAlignedBox2<double> & dest, const Wml::Vector2<double> & with );


template void rms::FitAABox( const Wml::Vector3<float> & vCenter, float fRadius, Wml::AxisAlignedBox3<float> & aaBox );
template void rms::FitAABox( const Wml::Vector3<double> & vCenter, double fRadius, Wml::AxisAlignedBox3<double> & aaBox );

template float rms::MaxDimension( const Wml::AxisAlignedBox3<float> & box );
template double rms::MaxDimension( const Wml::AxisAlignedBox3<double> & box );

template float rms::GetDimension( const Wml::AxisAlignedBox3<float> & box, int nDimension );
template double rms::GetDimension( const Wml::AxisAlignedBox3<double> & box, int nDimension );

template float rms::GetDimension( const Wml::AxisAlignedBox2<float> & box, int nDimension );
template double rms::GetDimension( const Wml::AxisAlignedBox2<double> & box, int nDimension );

template void rms::Translate( Wml::AxisAlignedBox2<float> & box, float fX, float fY, bool bRelative );
template void rms::Translate( Wml::AxisAlignedBox2<double> & box, double fX, double fY, bool bRelative );

template float rms::Clamp( const float & fValue, const float & fMin, const float & fMax );
template double rms::Clamp( const double & fValue, const double & fMin, const double & fMax );

template void BarycentricCoords( const Vector3<float> & TriVtx1, const Vector3<float> & TriVtx2,
											 const Vector3<float> & TriVtx3, const Vector3<float> & vVertex,
											 float & fWeight1, float & fWeight2, float & fWeight3 );
template  void BarycentricCoords( const Vector3<double> & TriVtx1, const Vector3<double> & TriVtx2,
											 const Vector3<double> & TriVtx3, const Vector3<double> & vVertex,
											 double & fWeight1, double & fWeight2, double & fWeight3 );
template  void BarycentricCoords( const Vector2<float> & TriVtx1, const Vector2<float> & TriVtx2,
											 const Vector2<float> & TriVtx3, const Vector2<float> & vVertex,
											 float & fWeight1, float & fWeight2, float & fWeight3 );
template  void BarycentricCoords( const Vector2<double> & TriVtx1, const Vector2<double> & TriVtx2,
											 const Vector2<double> & TriVtx3, const Vector2<double> & vVertex,
											 double & fWeight1, double & fWeight2, double & fWeight3 );


template  float Area( const Vector3<float> & TriVtx1, const Vector3<float> & TriVtx2,
											 const Vector3<float> & TriVtx3 );
template  double Area( const Vector3<double> & TriVtx1, const Vector3<double> & TriVtx2,
											 const Vector3<double> & TriVtx3 );
template  float  Area( const Vector2<float> & TriVtx1, const Vector2<float> & TriVtx2,
											 const Vector2<float> & TriVtx3 );
template  double Area( const Vector2<double> & TriVtx1, const Vector2<double> & TriVtx2,
											 const Vector2<double> & TriVtx3 );


template  Vector3<float> Normal( const Vector3<float> & TriVtx1, const Vector3<float> & TriVtx2,
											 const Vector3<float> & TriVtx3, float * pArea );
template  Vector3<double> Normal( const Vector3<double> & TriVtx1, const Vector3<double> & TriVtx2,
											 const Vector3<double> & TriVtx3, double * pArea );


template  void StretchMetric1( const Vector3<float> & q1, const Vector3<float> & q2, const Vector3<float> & q3,
										  const Vector2<float> & p1, const Vector2<float> & p2, const Vector2<float> & p3,
										  float & MaxSV, float & MinSV, float & L2Norm, float & LInfNorm );
template  void StretchMetric1( const Vector3<double> & q1, const Vector3<double> & q2, const Vector3<double> & q3,
										  const Vector2<double> & p1, const Vector2<double> & p2, const Vector2<double> & p3,
										  double & MaxSV, double & MinSV, double & L2Norm, double & LInfNorm );


}

