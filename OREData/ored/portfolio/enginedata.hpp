/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of OpenRiskEngine, a free-software/open-source library
 for transparent pricing and risk analysis - http://openriskengine.org

 OpenRiskEngine is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program; if not, please email
 <users@openriskengine.org>. The license is also available online at
 <http://openriskengine.org/license.shtml>.

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

/*! \file ored/portfolio/enginedata.hpp
    \brief A class to hold pricing engine parameters
    \ingroup tradedata
*/

#pragma once

#include <ored/utilities/xmlutils.hpp>
#include <ored/utilities/parsers.hpp>
#include <qle/termstructures/dynamicstype.hpp>

using std::vector;
using std::string;
using openriskengine::data::XMLSerializable;
using openriskengine::data::XMLDocument;
using openriskengine::data::XMLNode;
using openriskengine::data::XMLUtils;

namespace openriskengine {
namespace data {

//! Pricing engine description
/*! \ingroup tradedata
*/
class EngineData : public XMLSerializable {
public:
    //! Default constructor
    EngineData(){};

    //! \name Inspectors
    //@{
    bool hasProduct(const string& productName);
    const string& model(const string& productName) const { return model_.at(productName); }
    const map<string, string>& modelParameters(const string& productName) const { return modelParams_.at(productName); }
    const string& engine(const string& productName) const { return engine_.at(productName); }
    const map<string, string>& engineParameters(const string& productName) const {
        return engineParams_.at(productName);
    }
    //@}

    //! \name Setters
    //@{
    string& model(const string& productName) { return model_[productName]; }
    map<string, string>& modelParameters(const string& productName) { return modelParams_[productName]; }
    string& engine(const string& productName) { return engine_[productName]; }
    map<string, string>& engineParameters(const string& productName) { return engineParams_[productName]; }
    //@}

    //! Clear all data
    void clear();

    //! \name Serialisation
    //@{
    virtual void fromXML(XMLNode* node);
    virtual XMLNode* toXML(XMLDocument& doc);
    //@}

private:
    map<string, string> model_;
    map<string, map<string, string>> modelParams_;
    map<string, string> engine_;
    map<string, map<string, string>> engineParams_;
};
}
}
