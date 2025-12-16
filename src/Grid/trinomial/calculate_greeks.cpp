#include "OptionsVisualizer/Grid/Enums.hpp"
#include "OptionsVisualizer/Grid/Grid.hpp"
#include "OptionsVisualizer/models/trinomial/calculate_price.hpp"
#include "OptionsVisualizer/models/trinomial/constants.hpp"
#include <unsupported/Eigen/CXX11/Tensor>
#include <utility>

Grid::GreeksResult Grid::trinomialGreeks(OptionType optType) const {
    // --- Compute the base price

    using models::trinomial::calculatePrice;

    Eigen::Tensor<double, 2> price{
        calculatePrice(spot_, strikesGrid_, r_, q_, sigmasGrid_, tau_, nSigma_, nStrike_, optType)};

    // --- First-order derivatives (delta, vega, theta)

    using models::trinomial::constants::fdmStep;

    static auto firstOrderCDM{[](const Eigen::Tensor<double, 2>& lo, const Eigen::Tensor<double, 2>& hi, double eps) {
        return (hi - lo) / (2.0 * eps);
    }};

    // Compute delta (first derivative w.r.t spot) using CDM
    // delta = (price(spot + dSpot) - price(spot - dSpot)) / (2 * dSpot)
    const Eigen::Tensor<double, 2> priceLoSpot{
        calculatePrice(spot_ - fdmStep, strikesGrid_, r_, q_, sigmasGrid_, tau_, nSigma_, nStrike_, optType)};
    const Eigen::Tensor<double, 2> priceHiSpot{
        calculatePrice(spot_ + fdmStep, strikesGrid_, r_, q_, sigmasGrid_, tau_, nSigma_, nStrike_, optType)};
    Eigen::Tensor<double, 2> delta{firstOrderCDM(priceLoSpot, priceHiSpot, fdmStep)};

    // Compute vega (first derivative w.r.t sigma) using CDM
    // Vega = (price(sigma + dSigma) - price(sigma - dSigma)) / (2 * dSigma)
    const Eigen::Tensor<double, 2> priceLoSigma{
        calculatePrice(spot_, strikesGrid_, r_, q_, sigmasGrid_ - fdmStep, tau_, nSigma_, nStrike_, optType)};
    const Eigen::Tensor<double, 2> priceHiSigma{
        calculatePrice(spot_, strikesGrid_, r_, q_, sigmasGrid_ + fdmStep, tau_, nSigma_, nStrike_, optType)};
    Eigen::Tensor<double, 2> vega{firstOrderCDM(priceLoSigma, priceHiSigma, fdmStep)};

    // Compute theta (negative first derivative w.r.t tau) using CDM
    // Theta = -(price(tau + dTau) - price(tau - dTau)) / (2 * dTau)
    const Eigen::Tensor<double, 2> priceLoTau{
        calculatePrice(spot_, strikesGrid_, r_, q_, sigmasGrid_, tau_ - fdmStep, nSigma_, nStrike_, optType)};
    const Eigen::Tensor<double, 2> priceHiTau{
        calculatePrice(spot_, strikesGrid_, r_, q_, sigmasGrid_, tau_ + fdmStep, nSigma_, nStrike_, optType)};
    Eigen::Tensor<double, 2> theta{-firstOrderCDM(priceLoTau, priceHiTau, fdmStep)};

    // --- Second-order derivatives (gamma)

    // Compute gamma (second derivative w.r.t spot) using CDM
    // Gamma = (price(spot + dSpot) - 2 * price(spot) + price(spot - dSpot)) / (dSpot ^ 2)
    Eigen::Tensor<double, 2> gamma{(priceHiSpot - (2.0 * price) + priceLoSpot) / (fdmStep * fdmStep)};

    return GreeksResult{std::move(price), std::move(delta), std::move(gamma), std::move(vega), std::move(theta)};
}
