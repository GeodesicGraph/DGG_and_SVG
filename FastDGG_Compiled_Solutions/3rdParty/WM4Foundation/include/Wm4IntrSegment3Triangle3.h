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

#ifndef WM4INTRSEGMENT3TRIANGLE3_H
#define WM4INTRSEGMENT3TRIANGLE3_H

#include "Wm4FoundationLIB.h"
#include "Wm4Intersector.h"
#include "Wm4Segment3.h"
#include "Wm4Triangle3.h"

namespace Wm4
{

template <class Real>
class WM4_FOUNDATION_ITEM IntrSegment3Triangle3
    : public Intersector<Real,Vector3<Real> >
{
public:
    IntrSegment3Triangle3 (const Segment3<Real>& rkSegment,
        const Triangle3<Real>& rkTriangle);

    // object access
    const Segment3<Real>& GetSegment () const;
    const Triangle3<Real>& GetTriangle () const;

    // test-intersection query
    virtual bool Test ();

    // Find-intersection query.  The point of intersection is
    //   P = origin + t*direction = b0*V0 + b1*V1 + b2*V2
    virtual bool Find ();
    Real GetSegmentT () const;
    Real GetTriB0 () const;
    Real GetTriB1 () const;
    Real GetTriB2 () const;

private:
    // the objects to intersect
    const Segment3<Real>& m_rkSegment;
    const Triangle3<Real>& m_rkTriangle;

    // information about the intersection set
    Real m_fSegmentT, m_fTriB0, m_fTriB1, m_fTriB2;
};

typedef IntrSegment3Triangle3<float> IntrSegment3Triangle3f;
typedef IntrSegment3Triangle3<double> IntrSegment3Triangle3d;

}

#endif
