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

#ifndef WM4INTRLOZENGE3LOZENGE3_H
#define WM4INTRLOZENGE3LOZENGE3_H

#include "Wm4FoundationLIB.h"
#include "Wm4Intersector.h"
#include "Wm4Lozenge3.h"

namespace Wm4
{

template <class Real>
class WM4_FOUNDATION_ITEM IntrLozenge3Lozenge3
    : public Intersector<Real,Vector3<Real> >
{
public:
    IntrLozenge3Lozenge3 (const Lozenge3<Real>& rkLozenge0,
        const Lozenge3<Real>& rkLozenge1);

    // object access
    const Lozenge3<Real>& GetLozenge0 () const;
    const Lozenge3<Real>& GetLozenge1 () const;

    // static intersection query
    virtual bool Test ();

private:
    // the objects to intersect
    const Lozenge3<Real>& m_rkLozenge0;
    const Lozenge3<Real>& m_rkLozenge1;
};

typedef IntrLozenge3Lozenge3<float> IntrLozenge3Lozenge3f;
typedef IntrLozenge3Lozenge3<double> IntrLozenge3Lozenge3d;

}

#endif
