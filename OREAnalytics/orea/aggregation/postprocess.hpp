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

/*! \file orea/aggregation/postprocess.hpp
    \brief Exposure aggregation and XVA calculation
    \ingroup analytics
*/

#pragma once

#include <orea/aggregation/collatexposurehelper.hpp>
#include <orea/cube/inmemorycube.hpp>
#include <orea/scenario/aggregationscenariodata.hpp>

#include <ored/portfolio/portfolio.hpp>
#include <ored/portfolio/nettingsetmanager.hpp>

#include <ql/time/date.hpp>

#include <boost/shared_ptr.hpp>

using namespace QuantLib;

namespace openriskengine {
using namespace data;
using namespace data;
namespace analytics {

enum class AllocationMethod {
    None,
    Marginal, // Pykhtin & Rosen, 2010
    RelativeFairValueGross,
    RelativeFairValueNet,
    RelativeXVA
};

std::ostream& operator<<(std::ostream& out, AllocationMethod m);

AllocationMethod parseAllocationMethod(const string& s);

//! Exposure Aggregation and XVA Calculation
/*!
  This class aggregates NPV cube data, computes exposure statistics
  and various XVAs, all at trade and netting set level:

  1) Exposures
  - Expected Positive Exposure, EPE: E[max(NPV(t),0) / N(t)]
  - Expected Negative Exposure, ENE: E[max(-NPV(t),0) / N(t)]
  - Basel Expected Exposure, EE_B: EPE(t)/P(t)
  - Basel Expected Positive Exposure, EPE_B
  - Basel Effective Expected Exposure, EEE_B: max( EEE_B(t-1), EE_B(t))
  - Basel Effective Expected Positive Exposure, EEPE_B
  - Potential Future Exposure, PFE: q-Quantile of the distribution of

  2) Dynamic Initial Margin via regression

  3) XVAs:
  - Credit Value Adjustment, CVA
  - Debit Value Adjustment, DVA
  - Funding Value Adjustment, FVA
  - Collateral Value Adjustment, COLVA
  - Margin Value Adjustment, MVA

  4) Allocation from netting set to trade level such that allocated contributions
  add up to the netting set
  - CVA and DVA
  - EPE and ENE

  All analytics are precomputed when the class constructor is called.
  A number of inspectors described below then return the individual analytics results.

  Note:
  - exposures are discounted at the numeraire N(t) used in the
  Monte Carlo simulation which produces the NPV cube.
  - NPVs take collateral into account, depending on CSA settings

  \ingroup analytics

  \todo Introduce enumeration for TradeAction type and owner
  \todo Interpolation for DIM(t-MPOR) when the simulation grid spacing is different from MPOR
  \todo Revise alternatives to the RelativeXVA exposure and XVA allocation method
  \todo Add trade-level MVA
  \todo Take the spread received on posted initial margin into account in MVA calculation
*/
class PostProcess {
public:
    //! Constructor
    PostProcess( //! Trade portfolio to identidy e.g. netting set, maturity, break dates for each trade
        const boost::shared_ptr<Portfolio>& portfolio,
        //! Netting set manager to access CSA details for each netting set
        const boost::shared_ptr<NettingSetManager>& nettingSetManager,
        //! Market data object to access e.g. discounting and funcing curves
        const boost::shared_ptr<Market>& market,
        //! Market configuration to use
        const std::string& configuration,
        //! Input NPV Cube
        const boost::shared_ptr<NPVCube>& cube,
        //! Subset of simulated market data, index fixings and FX spot rates, associated with the NPV cube
        const boost::shared_ptr<AggregationScenarioData>& scenarioData,
        //! Selection of analytics to be produced
        const map<string, bool>& analytics,
        //! Expression currency for all results
        const string& baseCurrency,
        //! Method to be used for Exposure/XVA allocation down to trade level
        const string& allocationMethod,
        //! Cutoff parameter for the marginal allocation method below which we switch to equal disctribution
        Real cvaMarginalAllocationLimit,
        //! Quantile for Potential Future Exposure output
        Real quantile = 0.95,
        //! Collateral calculation type to be used, see class %CollateralExposureHelper
        const string& calculationType = "Symmetric",
        //! Credit curve name to be used for "our" credit risk in DVA calculations
        const string& dvaName = "",
        //! Borrowing curve name to be used in FVA calculations
        const string& fvaBorrowingCurve = "",
        //! Lending curve name to be used in FVA calculations
        const string& fvaLendingCurve = "",
        //! Collateral spread to be used in COLVA calculations
        Real collateralSpread = 0.0,
        //! Quantile used in dynamic initial margin calculation
        Real dimQuantile = 0.99,
        //! Initial margin horizon in calendar days, 2 weeks = 14 days
        Size dimHorizonCalendarDays = 14,
        //! Order of the regression polynomial used in DIM estmation
        Size dimRegressionOrder = 0,
        //! Regressors to be used in the DIM estimation by regression, each must match an additional scenario data key
        vector<string> dimRegressors = vector<string>(),
        //! Number of local regression evaluations, e.g. to validata DIM by regression
        Size dimLocalRegressionEvaluations = 0,
        //! Local regression band width in standard deviations of the regression variable
        Real dimLocalRegressionBandwidth = 0,
        //! Scaling factor applied to all DIM values
        Real dimScaling = 1.0);

    //! Return list of Trade IDs in the portfolio
    const vector<string>& tradeIds() { return tradeIds_; }
    //! Return list of netting set IDs in the portfolio
    const vector<string>& nettingSetIds() { return nettingSetIds_; }

    //! Return trade level Expected Positive Exposure evolution
    const vector<Real>& tradeEPE(const string& tradeId);
    //! Return trade level Expected Negative Exposure evolution
    const vector<Real>& tradeENE(const string& tradeId);
    //! Return trade level Basel Expected Exposure evolution
    const vector<Real>& tradeEE_B(const string& tradeId);
    //! Return trade level Basel Expected Positive Exposure evolution
    const Real& tradeEPE_B(const string& tradeId);
    //! Return trade level Effective Expected Exposure evolution
    const vector<Real>& tradeEEE_B(const string& tradeId);
    //! Return trade level Effective Expected Positive Exposure evolution
    const Real& tradeEEPE_B(const string& tradeId);
    //! Return trade level Potential Future Exposure evolution
    const vector<Real>& tradePFE(const string& tradeId);
    // const vector<Real>& tradeVAR(const string& tradeId);

    //! Return Netting Set Expected Positive Exposure evolution
    const vector<Real>& netEPE(const string& nettingSetId);
    //! Return Netting Set Expected Negative Exposure evolution
    const vector<Real>& netENE(const string& nettingSetId);
    //! Return Netting Set Basel Expected Exposure evolution
    const vector<Real>& netEE_B(const string& nettingSetId);
    //! Return Netting Set Basel Expected Positive Exposure evolution
    const Real& netEPE_B(const string& nettingSetId);
    //! Return Netting Set Effective Expected Exposure evolution
    const vector<Real>& netEEE_B(const string& nettingSetId);
    //! Return Netting Set Effective Expected Positive Exposure evolution
    const Real& netEEPE_B(const string& nettingSetId);
    //! Return Netting Set Potential Future Exposure evolution
    const vector<Real>& netPFE(const string& nettingSetId);
    // const vector<Real>& netVAR(const string& nettingSetId);

    //! Return the netting set's expected collateral evolution
    const vector<Real>& expectedCollateral(const string& nettingSetId);
    //! Return the netting set's expected COLVA increments through time
    const vector<Real>& colvaIncrements(const string& nettingSetId);
    //! Return the netting set's expected Collateral Floor increments through time
    const vector<Real>& collateralFloorIncrements(const string& nettingSetId);

    //! Return the trade EPE, allocated down from the netting set level
    const vector<Real>& allocatedTradeEPE(const string& tradeId);
    //! Return trade ENE, allocated down from the netting set level
    const vector<Real>& allocatedTradeENE(const string& tradeId);

    //! Return trade (stand-alone) CVA
    Real tradeCVA(const string& tradeId);
    //! Return trade (stand-alone) DVA
    Real tradeDVA(const string& tradeId);
    //! Return trade (stand-alone) MVA
    Real tradeMVA(const string& tradeId);
    //! Return trade (stand-alone) FBA (Funding Benefit Adjustment)
    Real tradeFBA(const string& tradeId);
    //! Return trade (stand-alone) FCA (Funding Cost Adjustment)
    Real tradeFCA(const string& tradeId);
    //! Return allocated trade CVA (trade CVAs add up to netting set CVA)
    Real allocatedTradeCVA(const string& tradeId);
    //! Return allocated trade DVA (trade DVAs add up to netting set DVA)
    Real allocatedTradeDVA(const string& tradeId);
    //! Return netting set CVA
    Real nettingSetCVA(const string& nettingSetId);
    //! Return netting set DVA
    Real nettingSetDVA(const string& nettingSetId);
    //! Return netting set MVA
    Real nettingSetMVA(const string& nettingSetId);
    //! Return netting set FBA
    Real nettingSetFBA(const string& nettingSetId);
    //! Return netting set FCA
    Real nettingSetFCA(const string& nettingSetId);
    //! Return netting set COLVA
    Real nettingSetCOLVA(const string& nettingSetId);
    //! Return netting set Collateral Floor value
    Real nettingSetCollateralFloor(const string& nettingSetId);

    //! Inspector for the input NPV cube (by trade, time, scenario)
    const boost::shared_ptr<NPVCube>& cube() { return cube_; }
    //! Return the  for the input NPV cube after netting and collateral (by netting set, time, scenario)
    const boost::shared_ptr<NPVCube>& netCube() { return nettedCube_; }
    //! Return the dynamic initial margin cube (regression approach)
    const boost::shared_ptr<NPVCube>& dimCube() { return dimCube_; }
    //! Write average (over samples) DIM evolution through time for given netting set
    void exportDimEvolution(const std::string& fileName, const std::string& nettingSet);
    //! Write DIM as a function of sample netting set NPV for a given time step
    void exportDimRegression(const std::string& fileName, const std::string& nettingSet, Size timeStep);

private:
    //! Helper function to return the collateral account evolution for a given netting set
    boost::shared_ptr<vector<boost::shared_ptr<CollateralAccount>>>
    collateralPaths(const string& nettingSetId, const boost::shared_ptr<NettingSetManager>& nettingSetManager,
                    const boost::shared_ptr<Market>& market, const std::string& configuration,
                    const boost::shared_ptr<AggregationScenarioData>& scenarioData, Size dates, Size samples,
                    const vector<vector<Real>>& nettingSetValue, Real nettingSetValueToday,
                    const Date& nettingSetMaturity);

    void updateStandAloneXVA();
    void updateAllocatedXVA();

    //! Fill dynamic initial margin cube (per netting set, date and sample)
    void dynamicInitialMargin();
    //! Compile the array of DIM regressors for the specified date and sample index
    Array regressorArray(Size dateIndex, Size sampleIndex);

    boost::shared_ptr<Portfolio> portfolio_;
    boost::shared_ptr<NettingSetManager> nettingSetManager_;
    boost::shared_ptr<Market> market_;
    const std::string configuration_;
    boost::shared_ptr<NPVCube> cube_;
    boost::shared_ptr<AggregationScenarioData> scenarioData_;
    map<string, bool> analytics_;

    map<string, vector<vector<Real>>> nettingSetNPV_, nettingSetFLOW_, nettingSetDIM_, nettingSetLocalDIM_,
        nettingSetDeltaNPV_;
    map<string, vector<vector<Array>>> regressorArray_;
    map<string, vector<Real>> nettingSetExpectedDIM_, nettingSetZeroOrderDIM_;
    map<string, vector<Real>> tradeEPE_, tradeENE_, tradeEE_B_, tradeEEE_B_, tradePFE_, tradeVAR_;
    map<string, Real> tradeEPE_B_, tradeEEPE_B_;
    map<string, vector<Real>> allocatedTradeEPE_, allocatedTradeENE_;
    map<string, vector<Real>> netEPE_, netENE_, netEE_B_, netEEE_B_, netPFE_, netVAR_, expectedCollateral_;
    map<string, Real> netEPE_B_, netEEPE_B_;
    map<string, vector<Real>> colvaInc_, eoniaFloorInc_;
    map<string, Real> tradeCVA_, tradeDVA_, tradeMVA_, tradeFBA_, tradeFCA_;
    map<string, Real> sumTradeCVA_, sumTradeDVA_; // per netting set
    map<string, Real> allocatedTradeCVA_, allocatedTradeDVA_;
    map<string, Real> nettingSetCVA_, nettingSetDVA_, nettingSetMVA_;
    map<string, Real> nettingSetCOLVA_, nettingSetCollateralFloor_;
    map<string, Real> nettingSetFCA_, nettingSetFBA_;
    boost::shared_ptr<NPVCube> nettedCube_;
    boost::shared_ptr<NPVCube> dimCube_;

    vector<string> tradeIds_;
    vector<string> nettingSetIds_;
    map<string, string> counterpartyId_; // for each nettingSetId
    string baseCurrency_;
    Real quantile_;
    CollateralExposureHelper::CalculationType calcType_;
    string dvaName_;
    string fvaBorrowingCurve_;
    string fvaLendingCurve_;
    Real collateralSpread_;
    Real dimQuantile_;
    Size dimHorizonCalendarDays_;
    Size dimRegressionOrder_;
    vector<string> dimRegressors_;
    Size dimLocalRegressionEvaluations_;
    Real dimLocalRegressionBandwidth_;
    Real dimScaling_;
};
}
}
