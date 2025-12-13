#pragma once

#include <algorithm>

namespace Payoff {

template <typename T>
struct Call {
    static constexpr T zero{0};

    T operator()(T spot, T strike) const {
        return std::max(spot - strike, zero);
    }
};

} // namespace Payoff
