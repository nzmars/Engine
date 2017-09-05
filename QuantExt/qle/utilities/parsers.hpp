/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
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

/*! \file ored/utilities/parsers.hpp
    \ingroup utilities
*/

#pragma once

#include <ql/time/date.hpp>

using std::string;

using namespace QuantLib;
namespace QuantExt {

//! Convert std::string to QuantLib::Date, Expects: IM1, IM2,...,IM9, IMA, IMB, IMC, IMD
/*!
\ingroup utilities
*/
QuantLib::Date parseIMMDate(QuantLib::Date asof, const string& s);

};
