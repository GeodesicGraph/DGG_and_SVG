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

#ifndef WM4INTRELLIPSE2ELLIPSE2_H
#define WM4INTRELLIPSE2ELLIPSE2_H

#include "Wm4FoundationLIB.h"
#include "Wm4Intersector.h"
#include "Wm4Ellipse2.h"
#include "Wm4Polynomial1.h"

namespace Wm4
{

template <class Real>
class WM4_FOUNDATION_ITEM IntrEllipse2Ellipse2
    : public Intersector<Real,Vector2<Real> >
{
public:
    IntrEllipse2Ellipse2 (const Ellipse2<Real>& rkEllipse0,
        const Ellipse2<Real>& rkEllipse1);

    // object access
    const Ellipse2<Real>& GetEllipse0 () const;
    const Ellipse2<Real>& GetEllipse1 () const;

    // static intersection queries
    virtual bool Test ();
    virtual bool Find ();

    // Intersection set for static find-intersection query.  The quantity Q
    // satisfies 0 <= Q <= 4.  When Q > 0, the interpretation depends on the
    // intersection type.
    //   IT_POINT:  Q distinct points of intersection
    //   IT_OTHER:  The ellipses are the same.  One of the ellipse objects is
    //              returned by GetIntersectionEllipse.  Q is invalid.
    int GetQuantity () const;
    const Vector2<Real>& GetPoint (int i) const;
    const Ellipse2<Real>& GetIntersectionEllipse () const;

private:
    using Intersector<Real,Vector2<Real> >::IT_EMPTY;
    using Intersector<Real,Vector2<Real> >::IT_POINT;
    using Intersector<Real,Vector2<Real> >::IT_OTHER;
    using Intersector<Real,Vector2<Real> >::m_iIntersectionType;

    static void GetAABB (const Ellipse2<Real>& rkEllipse, Real& rfXMin,
        Real& rfXMax, Real& rfYMin, Real& rfYMax);

    static Polynomial1<Real> GetQuartic (const Ellipse2<Real>& rkEllipse0,
        const Ellipse2<Real>& rkEllipse1);

    void ScaleEllipses (Ellipse2<Real>& rkScaled0, Ellipse2<Real>& rkScaled1,
        Vector2<Real>& rkTranslate, Real& rfScale);

    Real EvaluateQuadratic (const Real afQP[6], Real fX, Real fY);

    // the objects to intersect
    const Ellipse2<Real>& m_rkEllipse0;
    const Ellipse2<Real>& m_rkEllipse1;

    // points of intersection
    int m_iQuantity;
    Vector2<Real> m_akPoint[4];
};

typedef IntrEllipse2Ellipse2<float> IntrEllipse2Ellipse2f;
typedef IntrEllipse2Ellipse2<double> IntrEllipse2Ellipse2d;

}

#endif
