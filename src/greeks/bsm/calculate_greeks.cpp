#include "OptionsVisualizer/greeks/bsm/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include <cmath>
#include <torch/torch.h>
#include <utility>

namespace greeks::bsm {

GreeksResult callGreeks(const torch::Tensor& spot, const torch::Tensor& strikes, const torch::Tensor& r,
                        const torch::Tensor& q, const torch::Tensor& sigmas, const torch::Tensor& tau) {
    // --- Setup

    // Disable gradient computations
    torch::NoGradGuard noGrad{};

    // Broadcast strikes and sigmas
    torch::Tensor strikesGrid{strikes.unsqueeze(1).expand({strikes.size(0), sigmas.size(0)})};
    torch::Tensor sigmaGrid{sigmas.unsqueeze(0).expand_as(strikesGrid)};

    // BSM intermediate term d1
    torch::Tensor sqrtTau{torch::sqrt(tau)};
    torch::Tensor sigmaSqrtTau{sigmaGrid * sqrtTau};
    torch::Tensor d1{(torch::log(spot / strikesGrid) + (r - q + 0.5 * sigmaGrid.pow(2)) * tau) / sigmaSqrtTau};

    // BSM intermediate term d2
    torch::Tensor d2{d1 - sigmaSqrtTau};

    // --- Standard normal CDF and PDF using erf
    torch::Tensor cdfD1{0.5 * (1.0 + torch::erf(d1 / std::sqrt(2.0)))};
    torch::Tensor cdfD2{0.5 * (1.0 + torch::erf(d2 / std::sqrt(2.0)))};
    torch::Tensor pdfD1{(1.0 / std::sqrt(2.0 * M_PI)) * torch::exp(-0.5 * d1.pow(2))};

    // Constant exponential factors
    torch::Tensor expQTau{torch::exp(-q * tau)};
    torch::Tensor expRTau{torch::exp(-r * tau)};

    // --- Calculate results

    // price = (S * e^(-qT) * N(d1)) - (K * e^(-rT) * N(d2))
    torch::Tensor price{spot * expQTau * cdfD1 - strikesGrid * expRTau * cdfD2};

    // See Hull (ch. 18 - 398)
    // delta = e^(-qT) * N(d1)
    torch::Tensor delta{expQTau * cdfD1};

    // gamma = (N'(d1) * e^(-qT)) / (S * sigma * sqrt(T))
    torch::Tensor gamma{(pdfD1 * expQTau) / (spot * sigmaSqrtTau)};

    // vega = S * sqrt(T) * N'(d1) * e^(-qT)
    torch::Tensor vega{spot * sqrtTau * pdfD1 * expQTau};

    // theta = (-S * N'(d1) * sigma * e^(-qT) / (2 * sqrt(T))) + (q * S * N(d1) * e^(-qT)) - (r * K * e^(-rT) * N(d2))
    torch::Tensor theta{(-spot * pdfD1 * sigmaGrid * expQTau / (2 * sqrtTau)) + (q * spot * cdfD1 * expQTau) -
                        (r * strikesGrid * expRTau * cdfD2)};

    return GreeksResult{std::move(price), std::move(delta), std::move(gamma), std::move(vega), std::move(theta)};
}

GreeksResult putGreeks(const torch::Tensor& spot, const torch::Tensor& strikes, const torch::Tensor& r,
                       const torch::Tensor& q, const torch::Tensor& sigmas, const torch::Tensor& tau) {
    // --- Setup

    // Disable gradient computations
    torch::NoGradGuard noGrad{};

    // Retrieve call counterparts
    GreeksResult callResults{callGreeks(spot, strikes, r, q, sigmas, tau)};

    // Constant exponential factors
    torch::Tensor expQTau{torch::exp(-q * tau)};
    torch::Tensor expRTau{torch::exp(-r * tau)};

    // --- Calculate results

    // Put-Call Parity: P = C - S * e^(-qT) + K * e^(-rT)
    torch::Tensor price{callResults.price - spot * expQTau +
                        strikes.unsqueeze(1).expand_as(callResults.price) * expRTau};

    // See Hull (ch. 18 - 398)
    // delta_put = e^(-qT) * (N(d1) - 1) = (e^(-qT) * N(d1)) - e^(-qT) = delta_call - e^(-qT)
    torch::Tensor delta{callResults.delta - expQTau};

    /*
       theta_call - theta_put = -d/dt[C - P]
                              = -d/dt[S * e^(-qT) - K * e^(-rT)]
        -> theta_put = theta_call - S * q * e^(-qT) + K * r * e^(-rT)
    */
    torch::Tensor theta{callResults.theta - (q * spot * expQTau) +
                        (strikes.unsqueeze(1).expand_as(callResults.price) * r * expRTau)};

    return GreeksResult{std::move(price), std::move(delta), std::move(callResults.gamma), std::move(callResults.vega),
                        std::move(theta)};
}

} // namespace greeks::bsm
