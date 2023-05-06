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

#ifndef WM4BSPLINEFIT_H
#define WM4BSPLINEFIT_H

#include "Wm4FoundationLIB.h"
#include "Wm4BSplineFitBasis.h"
#include "Wm4BandedMatrix.h"

namespace Wm4
{

template <class Real>
class WM4_FOUNDATION_ITEM BSplineFit
{
public:
    // Construction and destruction.  The preconditions for calling the
    // constructor are
    //   1 <= iDegree && iDegree < iControlQuantity <= iSampleQuantity
    // The samples point are contiguous blocks of iDimension real value
    // stored in afSampleData.
    BSplineFit (int iDimension, int iSampleQuantity, const Real* afSampleData,
        int iDegree, int iControlQuantity);
    ~BSplineFit ();

    // Access to input sample information.
    int GetDimension () const;
    int GetSampleQuantity () const;
    const Real* GetSampleData () const;

    // Access to output control point and curve information.
    int GetDegree () const;
    int GetControlQuantity () const;
    const Real* GetControlData () const;
    const BSplineFitBasis<Real>& GetBasis () const;

    // Evaluation of the B-spline curve.  It is defined for 0 <= t <= 1.  If
    // a t-value is outside [0,1], an open spline clamps it to [0,1].  The
    // caller must ensure that afPosition[] has (at least) 'dimension'
    // elements.
    void GetPosition (Real fT, Real* afPosition) const;

private:
    // The matric inversion calculations are performed with double-precision,
    // even when the type Real is 'float'.
    bool CholeskyFactor (BandedMatrixd& rkMatrix) const;
    bool SolveLower (BandedMatrixd& rkMatrix, double* adControlData) const;
    bool SolveUpper (BandedMatrixd& rkMatrix, double* adControlData) const;

    // Input sample information.
    int m_iDimension;
    int m_iSampleQuantity;
    const Real* m_afSampleData;

    // The fitted B-spline curve, open and with uniform knots.
    int m_iDegree;
    int m_iControlQuantity;
    Real* m_afControlData;
    BSplineFitBasis<Real> m_kBasis;
};

typedef BSplineFit<float> BSplineFitf;
typedef BSplineFit<double> BSplineFitd;

}

#endif
