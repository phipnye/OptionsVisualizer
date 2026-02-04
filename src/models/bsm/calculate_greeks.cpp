#include <Eigen/Dense>
#include <cmath>
#include <numbers>
#include <unsupported/Eigen/SpecialFunctions>  // error function
#include <utility>

#include "OptionsVisualizer/pricing/GreeksResult.hpp"
#include "OptionsVisualizer/pricing/PricingSurface.hpp"

GreeksResult PricingSurface::bsmCallGreeks() const {
  // BSM intermediate term d1 = (log(S / K) + ((r - q - simga^2 / 2) * T)) /
  // sigma * sqrt(T)
  const double sqrtTau{std::sqrt(tau_)};
  const Eigen::ArrayXXd sigmaSqrtTau{sigmasGrid_ * sqrtTau};
  const Eigen::ArrayXXd d1{((spot_ / strikesGrid_).log() +
                            ((r_ - q_ + 0.5 * sigmasGrid_.square()) * tau_)) /
                           sigmaSqrtTau};

  // BSM intermediate term d2 = d1 - sigma * sqrt(T)
  const auto d2{d1 - sigmaSqrtTau};

  // --- Standard normal CDF and PDF using error function
  const Eigen::ArrayXXd cdfD1{0.5 * (1.0 + (d1 / std::sqrt(2.0)).erf())};
  const Eigen::ArrayXXd cdfD2{0.5 * (1.0 + (d2 / std::sqrt(2.0)).erf())};
  const Eigen::ArrayXXd pdfD1{(1.0 / std::sqrt(2.0 * std::numbers::pi)) *
                              (-0.5 * d1.square()).exp()};

  // Constant exponential factors
  const double expQTau{std::exp(-q_ * tau_)};
  const double expRTau{std::exp(-r_ * tau_)};

  // --- Calculate results

  // price = (S * e^(-qT) * N(d1)) - (K * e^(-rT) * N(d2))
  Eigen::ArrayXXd price{((spot_ * expQTau) * cdfD1) -
                        (strikesGrid_ * expRTau * cdfD2)};

  // See Hull (ch. 18 - 398)
  // delta = e^(-qT) * N(d1)
  Eigen::ArrayXXd delta{expQTau * cdfD1};

  // gamma = (N'(d1) * e^(-qT)) / (S * sigma * sqrt(T))
  Eigen::ArrayXXd gamma{(pdfD1 * expQTau) / (spot_ * sigmaSqrtTau)};

  // vega = S * sqrt(T) * N'(d1) * e^(-qT)
  Eigen::ArrayXXd vega{(spot_ * sqrtTau * expQTau) * pdfD1};

  // theta = (-S * N'(d1) * sigma * e^(-qT) / (2 * sqrt(T))) + (q * S * N(d1) *
  // e^(-qT)) - (r * K * e^(-rT) * N(d2))
  Eigen::ArrayXXd theta{
      ((-spot_ * expQTau / (2.0 * sqrtTau)) * pdfD1 * sigmasGrid_) +
      ((q_ * spot_ * expQTau) * cdfD1) -
      ((r_ * expRTau) * strikesGrid_ * cdfD2)};

  // rho = K * T * e^(-rT) * N(d2)
  Eigen::ArrayXXd rho{strikesGrid_ * (tau_ * expRTau) * cdfD2};

  return GreeksResult{std::move(price), std::move(delta), std::move(gamma),
                      std::move(vega),  std::move(theta), std::move(rho)};
}

GreeksResult PricingSurface::bsmPutGreeks(
    const GreeksResult& callResults) const {
  // Constant exponential factors
  const double expQTau{std::exp(-q_ * tau_)};
  const double expRTau{std::exp(-r_ * tau_)};

  // --- Calculate results

  // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
  Eigen::ArrayXXd price{callResults.price_ - spot_ * expQTau +
                        strikesGrid_ * expRTau};

  // See Hull (ch. 18 - 398)
  // delta_put = e^(-qT) * (N(d1) - 1) = (e^(-qT) * N(d1)) - e^(-qT) =
  // delta_call - e^(-qT)
  Eigen::ArrayXXd delta{callResults.delta_ - expQTau};

  /*
     theta_call - theta_put = -d/dt[C - P]
                            = -d/dt[S * e^(-qT) - K * e^(-rT)]
      -> theta_put = theta_call - S * q * e^(-qT) + K * r * e^(-rT)
  */
  Eigen::ArrayXXd theta{callResults.theta_ - (spot_ * q_ * expQTau) +
                        (strikesGrid_ * r_ * expRTau)};

  /*
      rho_call - rho_put = d/dr[C - P]
                         = d/dr[S - K * e^(-rT)]
                         = K * T * e^(-rT)
      -> rho_put = rho_call - K * T * e^(-rT)
  */
  Eigen::ArrayXXd rho{callResults.rho_ - (strikesGrid_ * tau_ * expRTau)};

  return GreeksResult{std::move(price),
                      std::move(delta),
                      Eigen::ArrayXXd{callResults.gamma_},
                      Eigen::ArrayXXd{callResults.vega_},
                      std::move(theta),
                      std::move(rho)};
}
