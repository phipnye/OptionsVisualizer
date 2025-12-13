#pragma once

#include <algorithm>

namespace Payoff {

template <typename T>
struct Put {
    static constexpr T zero{0};

    T operator()(T spot, T strike) const {
        return std::max(strike - spot, zero);
    }
};

} // namespace Payoff
