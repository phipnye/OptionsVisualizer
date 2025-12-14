#pragma once

#include "OptionsVisualizer/greeks/GreeksResult.hpp"
#include <torch/torch.h>

namespace greeks::trinomial::details {

GreeksResult americanGreeks(torch::Tensor spot, torch::Tensor strikes, torch::Tensor r, torch::Tensor q,
                            torch::Tensor sigmas, torch::Tensor tau, bool isCall);

} // namespace greeks::trinomial::details
