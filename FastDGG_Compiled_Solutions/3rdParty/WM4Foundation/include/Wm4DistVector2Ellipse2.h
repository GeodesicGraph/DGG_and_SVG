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

#ifndef WM4DISTVECTOR2ELLIPSE2_H
#define WM4DISTVECTOR2ELLIPSE2_H

#include "Wm4FoundationLIB.h"
#include "Wm4Distance.h"
#include "Wm4Ellipse2.h"

namespace Wm4
{

template <class Real>
class WM4_FOUNDATION_ITEM DistVector2Ellipse2
    : public Distance<Real,Vector2<Real> >
{
public:
    DistVector2Ellipse2 (const Vector2<Real>& rkVector,
        const Ellipse2<Real>& rkEllipse);

    // object access
    const Vector2<Real>& GetVector () const;
    const Ellipse2<Real>& GetEllipse () const;

    // static distance queries
    virtual Real Get ();
    virtual Real GetSquared ();

    // function calculations for dynamic distance queries
    virtual Real Get (Real fT, const Vector2<Real>& rkVelocity0,
        const Vector2<Real>& rkVelocity1);
    virtual Real GetSquared (Real fT, const Vector2<Real>& rkVelocity0,
        const Vector2<Real>& rkVelocity1);

private:
    using Distance<Real,Vector2<Real> >::m_kClosestPoint0;
    using Distance<Real,Vector2<Real> >::m_kClosestPoint1;

    const Vector2<Real>& m_rkVector;
    const Ellipse2<Real>& m_rkEllipse;
};

typedef DistVector2Ellipse2<float> DistVector2Ellipse2f;
typedef DistVector2Ellipse2<double> DistVector2Ellipse2d;

}

#endif
