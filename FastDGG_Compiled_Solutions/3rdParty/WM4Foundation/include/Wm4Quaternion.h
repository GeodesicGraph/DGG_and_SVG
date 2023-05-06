// Wild Magic Source Code
// David Eberly
// http://www.geometrictools.com
// Copyright (c) 1998-2007
//
// This library is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 2.1 of the License, or (at
// your option) any later version.  The license is available for reading at
// either of the locations:
//     http://www.gnu.org/copyleft/lgpl.html
//     http://www.geometrictools.com/License/WildMagicLicense.pdf
// The license applies to versions 0 through 4 of Wild Magic.
//
// Version: 4.0.0 (2006/06/28)

#ifndef WM4QUATERNION_H
#define WM4QUATERNION_H

#include "Wm4FoundationLIB.h"
#include "Wm4Matrix3.h"

namespace Wm4
{

template <class Real>
class Quaternion
{
public:
    // A quaternion is q = w + x*i + y*j + z*k where (w,x,y,z) is not
    // necessarily a unit length vector in 4D.

    // construction
    Quaternion ();  // uninitialized
    Quaternion (Real fW, Real fX, Real fY, Real fZ);
    Quaternion (const Quaternion& rkQ);

    // quaternion for the input rotation matrix
    Quaternion (const Matrix3<Real>& rkRot);

    // quaternion for the rotation of the axis-angle pair
    Quaternion (const Vector3<Real>& rkAxis, Real fAngle);

    // quaternion for the rotation matrix with specified columns
    Quaternion (const Vector3<Real> akRotColumn[3]);

    // member access:  0 = w, 1 = x, 2 = y, 3 = z
    operator const Real* () const;
    operator Real* ();
    Real operator[] (int i) const;
    Real& operator[] (int i);
    Real W () const;
    Real& W ();
    Real X () const;
    Real& X ();
    Real Y () const;
    Real& Y ();
    Real Z () const;
    Real& Z ();

    // assignment
    Quaternion& operator= (const Quaternion& rkQ);

    // comparison
    bool operator== (const Quaternion& rkQ) const;
    bool operator!= (const Quaternion& rkQ) const;
    bool operator<  (const Quaternion& rkQ) const;
    bool operator<= (const Quaternion& rkQ) const;
    bool operator>  (const Quaternion& rkQ) const;
    bool operator>= (const Quaternion& rkQ) const;

    // arithmetic operations
    Quaternion operator+ (const Quaternion& rkQ) const;
    Quaternion operator- (const Quaternion& rkQ) const;
    Quaternion operator* (const Quaternion& rkQ) const;
    Quaternion operator* (Real fScalar) const;
    Quaternion operator/ (Real fScalar) const;
    Quaternion operator- () const;

    // arithmetic updates
    Quaternion& operator+= (const Quaternion& rkQ);
    Quaternion& operator-= (const Quaternion& rkQ);
    Quaternion& operator*= (Real fScalar);
    Quaternion& operator/= (Real fScalar);

    // conversion between quaternions, matrices, and axis-angle
    Quaternion& FromRotationMatrix (const Matrix3<Real>& rkRot);
    void ToRotationMatrix (Matrix3<Real>& rkRot) const;
    Quaternion& FromRotationMatrix (const Vector3<Real> akRotColumn[3]);
    void ToRotationMatrix (Vector3<Real> akRotColumn[3]) const;
    Quaternion& FromAxisAngle (const Vector3<Real>& rkAxis, Real fAngle);
    void ToAxisAngle (Vector3<Real>& rkAxis, Real& rfAngle) const;

    // functions of a quaternion
    Real Length () const;  // length of 4-tuple
    Real SquaredLength () const;  // squared length of 4-tuple
    Real Dot (const Quaternion& rkQ) const;  // dot product of 4-tuples
    Real Normalize ();  // make the 4-tuple unit length
    Quaternion Inverse () const;  // apply to non-zero quaternion
    Quaternion Conjugate () const;
    Quaternion Exp () const;  // apply to quaternion with w = 0
    Quaternion Log () const;  // apply to unit-length quaternion

    // rotation of a vector by a quaternion
    Vector3<Real> Rotate (const Vector3<Real>& rkVector) const;

    // spherical linear interpolation
    Quaternion& Slerp (Real fT, const Quaternion& rkP, const Quaternion& rkQ);

    Quaternion& SlerpExtraSpins (Real fT, const Quaternion& rkP,
        const Quaternion& rkQ, int iExtraSpins);

    // intermediate terms for spherical quadratic interpolation
    Quaternion& Intermediate (const Quaternion& rkQ0,
        const Quaternion& rkQ1, const Quaternion& rkQ2);

    // spherical quadratic interpolation
    Quaternion& Squad (Real fT, const Quaternion& rkQ0,
        const Quaternion& rkA0, const Quaternion& rkA1,
        const Quaternion& rkQ1);

    // Compute a quaternion that rotates unit-length vector V1 to unit-length
    // vector V2.  The rotation is about the axis perpendicular to both V1 and
    // V2, with angle of that between V1 and V2.  If V1 and V2 are parallel,
    // any axis of rotation will do, such as the permutation (z2,x2,y2), where
    // V2 = (x2,y2,z2).
    Quaternion& Align (const Vector3<Real>& rkV1, const Vector3<Real>& rkV2);

    // Decompose a quaternion into q = q_twist * q_swing, where q is 'this'
    // quaternion.  If V1 is the input axis and V2 is the rotation of V1 by
    // q, q_swing represents the rotation about the axis perpendicular to
    // V1 and V2 (see Quaternion::Align), and q_twist is a rotation about V1.
    void DecomposeTwistTimesSwing (const Vector3<Real>& rkV1,
        Quaternion& rkTwist, Quaternion& rkSwing);

    // Decompose a quaternion into q = q_swing * q_twist, where q is 'this'
    // quaternion.  If V1 is the input axis and V2 is the rotation of V1 by
    // q, q_swing represents the rotation about the axis perpendicular to
    // V1 and V2 (see Quaternion::Align), and q_twist is a rotation about V1.
    void DecomposeSwingTimesTwist (const Vector3<Real>& rkV1,
        Quaternion& rkSwing, Quaternion& rkTwist);

    // special values
    WM4_FOUNDATION_ITEM static const Quaternion IDENTITY;
    WM4_FOUNDATION_ITEM static const Quaternion ZERO;

private:
    // support for comparisons
    int CompareArrays (const Quaternion& rkQ) const;

    // support for FromRotationMatrix
    WM4_FOUNDATION_ITEM static int ms_iNext[3];

    Real m_afTuple[4];
};

template <class Real>
Quaternion<Real> operator* (Real fScalar, const Quaternion<Real>& rkQ);

#include "Wm4Quaternion.inl"

typedef Quaternion<float> Quaternionf;
typedef Quaternion<double> Quaterniond;

}

#endif
