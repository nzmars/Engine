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

/*! \file scenario/scenario.hpp
    \brief Scenario class
    \ingroup scenario
*/

#pragma once

#include <vector>
#include <map>

#include <ored/utilities/serializationdate.hpp> 
#include <ql/time/date.hpp>
#include <ql/types.hpp>
#include <boost/shared_ptr.hpp>

using QuantLib::Real;
using QuantLib::Size;
using QuantLib::Date;
using std::string;

namespace openriskengine {
namespace analytics {

//! Data types stored in the scenario class
/*! \ingroup scenario
*/
class RiskFactorKey {
public:
    //! Risk Factor types
    enum class KeyType { DiscountCurve, YieldCurve, IndexCurve, SwaptionVolatility, FXSpot, FXVolatility };

    //! Constructor
    RiskFactorKey() {}
    //! Constructor
    RiskFactorKey(const KeyType& iKeytype, const string& iName, const Size& iIndex = 0)
        : keytype(iKeytype), name(iName), index(iIndex) {}

    //! Key type
    KeyType keytype;
    //! Key name.
    /*! For FX this is a pair ("EURUSD") for Discount or swaption it's just a currency ("EUR")
     *  and for an index it's the index name
     */
    std::string name;
    //! Index
    Size index;
private: 
    friend class boost::serialization::access; 
    template <class Archive> void serialize(Archive& ar, const unsigned int) { 
        ar& keytype; 
        ar& name; 
        ar& index; 
    } 
};

inline bool operator<(const RiskFactorKey& lhs, const RiskFactorKey& rhs) {
    if (lhs.keytype == rhs.keytype) {
        if (lhs.name == rhs.name)
            return lhs.index < rhs.index;
        else
            return lhs.name < rhs.name;
    }
    return lhs.keytype < rhs.keytype;
}

inline bool operator==(const RiskFactorKey& lhs, const RiskFactorKey& rhs) {
    return lhs.keytype == rhs.keytype && lhs.name == rhs.name && lhs.index == rhs.index;
}

inline bool operator>(const RiskFactorKey& lhs, const RiskFactorKey& rhs) { return rhs < lhs; }
inline bool operator<=(const RiskFactorKey& lhs, const RiskFactorKey& rhs) { return !(lhs > rhs); }
inline bool operator>=(const RiskFactorKey& lhs, const RiskFactorKey& rhs) { return !(lhs < rhs); }
inline bool operator!=(const RiskFactorKey& lhs, const RiskFactorKey& rhs) { return !(lhs == rhs); }

std::ostream& operator<<(std::ostream& out, const RiskFactorKey::KeyType& type);
std::ostream& operator<<(std::ostream& out, const RiskFactorKey& key);

//-----------------------------------------------------------------------------------------------
//! Scenario Base Class
/*! A scenario contains a single cross asset model sample in terms of
  yield curves by currency, FX rates, etc.

  This base class provides the interface to add and retrieve data to and from a scenario.
  Concrete simple and memory optimized "compact" scenario classes are derived from this.

  \ingroup scenario
*/
class Scenario {
public:
    //! Destructor
    virtual ~Scenario() {}

    //! Return the scenario asof date
    virtual const Date& asof() const = 0;

    //! Get the scenario label
    virtual const string& label() const = 0;
    //! Set the scenario label
    virtual void label(const string&) = 0;

    //! Get Numeraire ratio n = N(t) / N(0) so that Price(0) = N(0) * E [Price(t) / N(t) ]
    virtual Real getNumeraire() const = 0;
    //! Set the Numeraire ratio n = N(t) / N(0) so that Price(0) = N(0) * E [Price(t) / N(t) ]
    virtual void setNumeraire(Real n) = 0;

    //! Check whether this scenario provides the data for the given key
    virtual bool has(const RiskFactorKey& key) const = 0;
    //! Risk factor keys for which this scenario provides data
    virtual const std::vector<RiskFactorKey>& keys() const = 0;
    //! Add an element to the scenario
    virtual void add(const RiskFactorKey& key, Real value) = 0;
    //! Get an element from the scenario
    virtual Real get(const RiskFactorKey& key) const = 0;
private: 
    friend class boost::serialization::access; 
    template <class Archive> void serialize(Archive&, const unsigned int) { 
    } 
};
}
}
