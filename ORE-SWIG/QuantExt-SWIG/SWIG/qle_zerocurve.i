/*
 Copyright (C) 2018, 2020 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#ifndef qle_zerocurve_i
#define qle_zerocurve_i

%include zerocurve.i
%include qle_interpolation.i

%shared_ptr(InterpolatedZeroCurve<LinearFlat>);
%shared_ptr(InterpolatedZeroCurve<LogLinearFlat>);
%shared_ptr(InterpolatedZeroCurve<HermiteFlat>);

%template(LinearFlatZeroCurve) InterpolatedZeroCurve<LinearFlat>;
%template(LogLinearFlatZeroCurve) InterpolatedZeroCurve<LogLinearFlat>;
%template(HermiteFlatZeroCurve) InterpolatedZeroCurve<HermiteFlat>;

#endif

