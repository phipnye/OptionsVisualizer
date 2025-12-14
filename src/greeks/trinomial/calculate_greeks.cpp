#include "OptionsVisualizer/greeks/trinomial/calculate_greeks.hpp"
#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include "OptionsVisualizer/greeks/trinomial/details/calculate_greeks.hpp"
#include <torch/torch.h>

namespace greeks::trinomial {

GreeksResult trinomialCallGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                                 torch::Tensor sigmas, torch::Tensor tau) {
    return details::americanGreeks(spot, strikes, r, q, sigmas, tau, true);
}

GreeksResult trinomialPutGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                                torch::Tensor sigmas, torch::Tensor tau) {
    return details::americanGreeks(spot, strikes, r, q, sigmas, tau, false);
}

} // namespace greeks::trinomial
