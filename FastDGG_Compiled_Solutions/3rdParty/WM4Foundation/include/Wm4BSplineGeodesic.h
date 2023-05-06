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

#ifndef WM4BSPLINEGEODESIC_H
#define WM4BSPLINEGEODESIC_H

#include "Wm4FoundationLIB.h"
#include "Wm4RiemannianGeodesic.h"
#include "Wm4BSplineRectangle.h"

namespace Wm4
{

template <class Real>
class WM4_FOUNDATION_ITEM BSplineGeodesic : public RiemannianGeodesic<Real>
{
public:
    BSplineGeodesic (const BSplineRectangle<Real>& rkBSpline);
    virtual ~BSplineGeodesic ();

    virtual void ComputeMetric (const GVector<Real>& rkPoint);
    virtual void ComputeChristoffel1 (const GVector<Real>& rkPoint);

private:
    using RiemannianGeodesic<Real>::m_kMetric;
    using RiemannianGeodesic<Real>::m_akChristoffel1;

    const BSplineRectangle<Real>& m_rkBSpline;
};

typedef BSplineGeodesic<float> BSplineGeodesicf;
typedef BSplineGeodesic<double> BSplineGeodesicd;

}

#endif
