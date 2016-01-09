/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd.
*/

/*! \file irlgm1fconstantparametrization.hpp
    \brief piecewise constant model parametrization
*/

#ifndef quantext_piecewiseconstant_irlgm1f_parametrization_hpp
#define quantext_piecewiseconstant_irlgm1f_parametrization_hpp

#include <qle/models/irlgm1fparametrization.hpp>
#include <qle/models/piecewiseconstanthelper.hpp>

namespace QuantExt {

class IrLgm1fPiecewiseConstantParametrization
    : public IrLgm1fParametrization,
      private PiecewiseConstantHelper1,
      private PiecewiseConstantHelper2 {
  public:
    /*! note that if a non unit scaling is provided, then
        the parameterValues method returns the unscaled alpha,
        while all other method return scaled (and shifted) values */
    IrLgm1fPiecewiseConstantParametrization(
        const Currency &currency,
        const Handle<YieldTermStructure> &termStructure,
        const Array &alphaTimes, const Array &alpha, const Array &kappaTimes,
        const Array &kappa, const Real shift = 0.0, const Real scaling = 1.0);
    Real zeta(const Time t) const;
    Real H(const Time t) const;
    Real alpha(const Time t) const;
    Real kappa(Time t) const;
    Real Hprime(const Time t) const;
    Real Hprime2(const Time t) const;
    const Array &parameterTimes(const Size) const;
    const boost::shared_ptr<Parameter> parameter(const Size) const;
    void update() const;

  protected:
    Real direct(const Size i, const Real x) const;
    Real inverse(const Size j, const Real y) const;

  private:
    const Real shift_, scaling_;
};

// inline

inline Real
IrLgm1fPiecewiseConstantParametrization::direct(const Size i,
                                                const Real x) const {
    return i == 0 ? PiecewiseConstantHelper1::direct(x)
                  : PiecewiseConstantHelper2::direct(x);
}

inline Real
IrLgm1fPiecewiseConstantParametrization::inverse(const Size i,
                                                 const Real y) const {
    return i == 0 ? PiecewiseConstantHelper1::inverse(y)
                  : PiecewiseConstantHelper2::inverse(y);
}

inline Real IrLgm1fPiecewiseConstantParametrization::zeta(const Time t) const {
    return PiecewiseConstantHelper1::int_y_sqr(t) / (scaling_ * scaling_);
}

inline Real IrLgm1fPiecewiseConstantParametrization::H(const Time t) const {
    return scaling_ * PiecewiseConstantHelper2::int_exp_m_int_y(t) + shift_;
}

inline Real IrLgm1fPiecewiseConstantParametrization::alpha(const Time t) const {
    return PiecewiseConstantHelper1::y(t) / scaling_;
}

inline Real IrLgm1fPiecewiseConstantParametrization::kappa(const Time t) const {
    return PiecewiseConstantHelper2::y(t);
}

inline Real
IrLgm1fPiecewiseConstantParametrization::Hprime(const Time t) const {
    return scaling_ * PiecewiseConstantHelper2::exp_m_int_y(t);
}

inline Real
IrLgm1fPiecewiseConstantParametrization::Hprime2(const Time t) const {
    return -scaling_ * PiecewiseConstantHelper2::exp_m_int_y(t) * kappa(t);
}

inline void IrLgm1fPiecewiseConstantParametrization::update() const {
    PiecewiseConstantHelper1::update();
    PiecewiseConstantHelper2::update();
}

inline const Array &
IrLgm1fPiecewiseConstantParametrization::parameterTimes(const Size i) const {
    QL_REQUIRE(i < 2, "parameter " << i << " does not exist, only have 0..1");
    if (i == 0)
        return PiecewiseConstantHelper1::t_;
    else
        return PiecewiseConstantHelper2::t_;
}

inline const boost::shared_ptr<Parameter>
IrLgm1fPiecewiseConstantParametrization::parameter(const Size i) const {
    QL_REQUIRE(i < 2, "parameter " << i << " does not exist, only have 0..1");
    if (i == 0)
        return PiecewiseConstantHelper1::y_;
    else
        return PiecewiseConstantHelper2::y_;
}

} // namespace QuantExt

#endif
