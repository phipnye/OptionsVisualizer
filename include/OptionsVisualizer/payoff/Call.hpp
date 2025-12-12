#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace Payoff {

template <typename T>
struct Call {
    static inline T zero{0};

    T operator()(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike) const {
        return generic::max<T, T>(spot - strike, zero);
    }
};

} // namespace Payoff
