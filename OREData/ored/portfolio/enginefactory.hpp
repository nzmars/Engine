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

/*! \file ored/portfolio/enginefactory.hpp
    \brief Pricing Engine Factory
    \ingroup tradedata
*/

#pragma once

#include <ored/marketdata/market.hpp>
#include <ored/portfolio/enginedata.hpp>

#include <ql/pricingengine.hpp>

#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>

using std::map;
using std::string;
using std::pair;
using openriskengine::data::Market;
using QuantLib::PricingEngine;

namespace openriskengine {
namespace data {

class Trade;

/*! Market configuration contexts. Note that there is only one pricing context.
  If several are needed (for different trade types, different collateral
  currencies etc.), several engine factories should be set up for each such
  portfolio subset. */
enum class MarketContext { irCalibration, fxCalibration, pricing };

//! Base PricingEngine Builder class for a specific model and engine
/*!
 *  The EngineBuilder is responsible for building pricing engines for a specific
 *  Model and Engine.
 *
 *  Each builder should implement a method with a signature
 *  @code
 *  boost::shared_ptr<PricingEngine> engine (...);
 *  @endcode
 *  The exact parameters of each method can vary depending on the type of engine.
 *
 *  An EngineBuilder can cache engines and return the same PricingEngine multiple
 *  times, alternatively the Builder can build a unique PricingEngine each time it
 *  is called.
 *
 *  For example a swap engine builder can have the interface
 *  @code
 *  boost::shared_ptr<PricingEngine> engine (const Currency&);
 *  @endcode
 *  and so returns the same (cached) engine every time it is asked for a particular
 *  currency.
 *
 *  The interface of each type of engine builder can be different, then there can
 *  be further sub-classes for different models and engines.
 *
 *  EngineBuilders are registered in an EngineFactory, multiple engine builders for
 *  the same trade type can be registered with the EngineFactory and it will select
 *  the appropriate one based on configuration.
 *
 *  Each EngineBuilder must return it's Model and Engine.

    \ingroup tradedata
 */
class EngineBuilder {
public:
    /*! Constructor that takes a model and engine name
     *  @param model the model name
     *  @param engine the engine name
     */
    EngineBuilder(const string& model, const string& engine) : model_(model), engine_(engine) {}

    //! Virtual destructor
    virtual ~EngineBuilder() {}

    //! Return the model name
    const string& model() const { return model_; }
    //! Return the engine name
    const string& engine() const { return engine_; }

    //! Return a configuration (or the default one if key not found)
    const string& configuration(const MarketContext& key) {
        if (configurations_.count(key) > 0) {
            return configurations_.at(key);
        } else {
            return Market::defaultConfiguration;
        }
    }

    //! Initialise this Builder with the market and parameters to use
    /*! This method should not be called directly, it is called by the EngineFactory
     *  before it is returned.
     */
    void init(const boost::shared_ptr<Market> market, const map<MarketContext, string>& configurations,
              const map<string, string>& modelParameters, const map<string, string>& engineParameters) {
        market_ = market;
        configurations_ = configurations;
        modelParameters_ = modelParameters;
        engineParameters_ = engineParameters;
    }

protected:
    string model_;
    string engine_;
    boost::shared_ptr<Market> market_;
    map<MarketContext, string> configurations_;
    map<string, string> modelParameters_;
    map<string, string> engineParameters_;
};

//! Pricing Engine Factory class
/*! A Pricing Engine Factory is used when building a portfolio, it provides
 *  QuantLib::PricingEngines to each of the Trade objects.
 *
 *  An Engine Factory is configured on two levels, both from EngineData.
 *  The first level is the type of builder (Model and Engine) to use for each
 *  trade type, so given a trade type one asks the factory for a builder and
 *  then the builder for a PricingEngine. Each builder must be registered with
 *  the factory and then the configuration defines which builder to use.
 *
 *  Secondly, the factory maintains builder specific parameters for each Model
 *  and Engine.

    \ingroup tradedata
 */
class EngineFactory {
public:
    //! Create an engine factory
    EngineFactory( //! Configuration data
        const boost::shared_ptr<EngineData>& data,
        //! The market that is passed to each builder
        const boost::shared_ptr<Market>& market,
        //! The market configurations that are passed to each builder
        const map<MarketContext, string>& configurations = std::map<MarketContext, string>());

    //! Return the market used by this EngineFactory
    const boost::shared_ptr<Market>& market() const { return market_; };
    //! Return the market configurations used by this EngineFactory
    const map<MarketContext, string>& configurations() const { return configurations_; };

    //! Register a builder with the factory
    void registerBuilder(const boost::shared_ptr<EngineBuilder>& builder);

    //! Get a builder by trade type
    /*! This will look up configured model/engine for that trade type
        the returned builder can be cast to the type required for the tradeType.

        The factory will call EngineBuilder::init() before returning it.
     */
    boost::shared_ptr<EngineBuilder> builder(const string& tradeType);

    //! Add a set of default builders
    void addDefaultBuilders();

    //! Clear all builders
    void clear() { builders_.clear(); }

private:
    boost::shared_ptr<Market> market_;
    boost::shared_ptr<EngineData> engineData_;
    map<MarketContext, string> configurations_;
    map<pair<string, string>, boost::shared_ptr<EngineBuilder>> builders_;
};

} // namespace data
} // namespace openriskengine
