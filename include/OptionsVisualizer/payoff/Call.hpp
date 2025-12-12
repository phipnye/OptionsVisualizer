#pragma once

#include <algorithm>

namespace Payoff {

template <typename T>
struct Call {
    static constexpr T zero{0};

    T operator()(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike) const {
        return std::max(spot - strike, zero);
    }
};

} // namespace Payoff
