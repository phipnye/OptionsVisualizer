#pragma once

#include "OptionsVisualizer/math/generic_math.hpp"
#include "OptionsVisualizer/utils/typing.hpp"

namespace Payoff {

template <typename T>
struct Payoff {
    virtual ~Payoff() = default;
    virtual T operator()(utils::type::ParamT<T> spot, utils::type::ParamT<T> strike) const = 0;
};

} // namespace Payoff