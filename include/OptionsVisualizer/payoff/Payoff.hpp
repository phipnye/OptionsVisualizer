#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace Payoff {

template <typename T>
struct Payoff {
    virtual ~Payoff() = default;
    virtual T operator()(utils::type::ParamT<T> S_t, utils::type::ParamT<T> K) const = 0;
};

} // namespace Payoff